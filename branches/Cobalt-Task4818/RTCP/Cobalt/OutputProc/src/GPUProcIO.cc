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
//# $Id: outputProc.cc 27120 2013-10-29 10:42:21Z mol $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <omp.h>

#include <string>
#include <vector>
#include <boost/format.hpp>

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

namespace LOFAR {
namespace Cobalt {


void process(Stream &controlStream, size_t myRank)
{
  Parset parset(&controlStream);

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

        string logPrefix = str(boost::format("[obs %u correlated stream %3u] ") % parset.observationID() % fileIdx);

        SubbandWriter *writer = new SubbandWriter(parset, fileIdx, logPrefix);
        subbandWriters.push_back(writer);
      }
    }

    map<size_t, SmartPtr<Pool<TABTranspose::Block> > > outputPools;
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

        outputPools[fileIdx] = new Pool<TABTranspose::Block>;

        // Create and fill an outputPool for this fileIdx
        for (size_t i = 0; i < 5; ++i) {
	         outputPools[fileIdx]->free.append(new TABTranspose::Block(
             parset.settings.nrSubbands(file.sapNr),
             stokes.nrSamples(parset.settings.blockSize),
             stokes.nrChannels));
        }

        // Create a collector for this fileIdx
        collectors[fileIdx] = new TABTranspose::BlockCollector(
          *outputPools[fileIdx], fileIdx, parset.nrBeamFormedBlocks(), parset.realTime() ? 4 : 0);

        string logPrefix = str(boost::format("[obs %u beamformed stream %3u] ") % parset.observationID() % fileIdx);

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


#   pragma omp parallel sections num_threads(2)
    {
      // Done signal from controller, by sending the final meta data
#     pragma omp section
      {
        // Add final meta data (broken tile information, etc)
        // that is obtained after the end of an observation.
        LOG_INFO_STR("Waiting for final meta data");

        finalMetaData.read(controlStream);

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
#       pragma omp parallel for num_threads(subbandWriters.size())
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
      try {
        subbandWriters[i]->augment(finalMetaData);
      } catch (Exception &ex) {
        LOG_ERROR_STR("Could not add final meta data: " << ex);
      }

    for (size_t i = 0; i < tabWriters.size(); ++i)
      try {
        tabWriters[i]->augment(finalMetaData);
      } catch (Exception &ex) {
        LOG_ERROR_STR("Could not add final meta data: " << ex);
      }

    /*
     * LTA FEEDBACK
     */

    LOG_INFO_STR("Retrieving LTA feedback");
    Parset feedbackLTA;

    for (size_t i = 0; i < subbandWriters.size(); ++i)
      try {
        feedbackLTA.adoptCollection(subbandWriters[i]->feedbackLTA());
      } catch (Exception &ex) {
        LOG_ERROR_STR("Could not obtain feedback for LTA: " << ex);
      }
    for (size_t i = 0; i < tabWriters.size(); ++i)
      try {
        feedbackLTA.adoptCollection(tabWriters[i]->feedbackLTA());
      } catch (Exception &ex) {
        LOG_ERROR_STR("Could not obtain feedback for LTA: " << ex);
      }

    LOG_INFO_STR("Forwarding LTA feedback");
    feedbackLTA.write(&controlStream);
  }
}

}
}

