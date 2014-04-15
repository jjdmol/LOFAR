//# GPUProcIO.cc: Routines for communicating with GPUProc
//# Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <omp.h>

#include <string>
#include <vector>
#include <boost/format.hpp>

#include <ApplCommon/PVSSDatapointDefs.h>
#include <ApplCommon/StationInfo.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/Exceptions.h>
#include <Stream/PortBroker.h>
#include <CoInterface/Exceptions.h>
#include <CoInterface/Parset.h>
#include <CoInterface/Stream.h>
#include "SubbandWriter.h"
#include "OutputThread.h"

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;
using boost::format;

namespace LOFAR {
namespace Cobalt {

bool process(Stream &controlStream, size_t myRank)
{
  bool success(true);
  Parset parset(&controlStream);

  // Send identification string to the MAC Log Processor
  string fmtStr(createPropertySetName(PSN_COBALT_OUTPUT_PROC, "",
                                      parset.getString("_DPname")));
  format prFmt;
  prFmt.exceptions(boost::io::no_error_bits); // avoid throw
  prFmt.parse(fmtStr);
  LOG_INFO_STR("MACProcessScope: " << str(prFmt % myRank));

  const vector<string> &hostnames = parset.settings.outputProcHosts;
  ASSERT(myRank < hostnames.size());
  string myHostName = hostnames[myRank];

  {
    // make sure "parset" stays in scope for the lifetime of the SubbandWriters

    vector<SmartPtr<SubbandWriter> > subbandWriters;
    vector<SmartPtr<TABOutputThread> > tabWriters;

    /*
     * Construct writers
     */

    // Process correlated data
    if (parset.settings.correlator.enabled) {
      for (size_t fileIdx = 0; fileIdx < parset.settings.correlator.files.size(); ++fileIdx)
      {
        if (parset.settings.correlator.files[fileIdx].location.host != myHostName) 
          continue;

        string logPrefix = str(format("[obs %u correlated stream %3u] ") % parset.observationID() % fileIdx);

        SubbandWriter *writer = new SubbandWriter(parset, fileIdx, logPrefix);
        subbandWriters.push_back(writer);
      }
    }

    map<size_t, SmartPtr<Pool<TABTranspose::BeamformedData> > > outputPools;
    TABTranspose::Receiver::CollectorMap collectors;

    // Process beam-formed data
    if (parset.settings.beamFormer.enabled) {
      for (size_t fileIdx = 0; fileIdx < parset.settings.beamFormer.files.size(); ++fileIdx)
      {
        struct ObservationSettings::BeamFormer::File &file = parset.settings.beamFormer.files[fileIdx];

        if (file.location.host != myHostName) 
          continue;

        struct ObservationSettings::BeamFormer::StokesSettings &stokes =
          file.coherent ? parset.settings.beamFormer.coherentSettings
                        : parset.settings.beamFormer.incoherentSettings;

        const size_t nrSubbands = file.lastSubbandIdx - file.firstSubbandIdx;
        const size_t nrChannels = stokes.nrChannels;
        const size_t nrSamples = stokes.nrSamples;

        outputPools[fileIdx] = new Pool<TABTranspose::BeamformedData>;
        ,

        // Create and fill an outputPool for this fileIdx
        for (size_t i = 0; i < 10; ++i) {
	         outputPools[fileIdx]->free.append(new TABTranspose::BeamformedData(
             boost::extents[nrSamples][nrSubbands][nrChannels],
             boost::extents[nrSubbands][nrChannels]
           ));
        }

        // Create a collector for this fileIdx
        collectors[fileIdx] = new TABTranspose::BlockCollector(
          *outputPools[fileIdx], fileIdx, nrSubbands, nrChannels, nrSamples, parset.nrBeamFormedBlocks(), parset.realTime() ? 5 : 0);

        string logPrefix = str(format("[obs %u beamformed stream %3u] ") % parset.observationID() % fileIdx);

        TABOutputThread *writer = new TABOutputThread(parset, fileIdx, *outputPools[fileIdx], logPrefix);
        tabWriters.push_back(writer);
      }
    }

    /*
     * PROCESS
     */

    FinalMetaData finalMetaData;

    // Set up receiver engine for 2nd transpose
    TABTranspose::MultiReceiver mr("2nd-transpose-", collectors);


#   pragma omp parallel sections num_threads(3)
    {
      // Done signal from controller, by sending the final meta data
#     pragma omp section
      {
        // Add final meta data (broken tile information, etc)
        // that is obtained after the end of an observation.
        LOG_INFO_STR("Waiting for final meta data");

        try {
          finalMetaData.read(controlStream);
        } catch (LOFAR::Exception &err) {
          success = false;
          LOG_ERROR_STR("Failed to read broken tile information: " << err);
        }

        if (parset.realTime()) {
          // Real-time observations: stop now. MultiReceiver::kill
          // will stop the TABWriters.
          mr.kill(0);
        } else {
          // Non-real-time observations: wait until all data has been
          // processed. The TABWriters will stop once they received
          // the last block.
        }

        // SubbandWriters finish on their own once their InputThread
        // gets disconnected.
      }

      // SubbandWriters
#     pragma omp section
      {
#       pragma omp parallel for num_threads(subbandWriters.size())
        for (int i = 0; i < (int)subbandWriters.size(); ++i)
          subbandWriters[i]->process();
      }

      // TABWriters
#     pragma omp section
      {
#       pragma omp parallel for num_threads(tabWriters.size())       
        for (int i = 0; i < (int)tabWriters.size(); ++i)
          tabWriters[i]->process();
      }
    }

    /*
     * FINAL META DATA
     */

    // Add final meta data (broken tile information, etc)
    // that is obtained after the end of an observation.
    LOG_INFO_STR("Processing final meta data");

    for (size_t i = 0; i < subbandWriters.size(); ++i)
      subbandWriters[i]->augment(finalMetaData);
    for (size_t i = 0; i < tabWriters.size(); ++i)
      tabWriters[i]->augment(finalMetaData);

    /*
     * LTA FEEDBACK
     */

    LOG_INFO_STR("Retrieving LTA feedback");
    Parset feedbackLTA;

    for (size_t i = 0; i < subbandWriters.size(); ++i)
      feedbackLTA.adoptCollection(subbandWriters[i]->feedbackLTA());
    for (size_t i = 0; i < tabWriters.size(); ++i)
      feedbackLTA.adoptCollection(tabWriters[i]->feedbackLTA());

    LOG_INFO_STR("Forwarding LTA feedback");
    try {
      feedbackLTA.write(&controlStream);
    } catch (LOFAR::Exception &err) {
      success = false;
      LOG_ERROR_STR("Failed to forward LTA feedback information: " << err);
    }

    return success;
  }
}

}
}

