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
#include <CoInterface/FinalMetaData.h>
#include "Writer.h"

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

namespace LOFAR {
namespace Cobalt {

void readFinalMetaData( Stream &controlStream, vector< SmartPtr<Writer> > &subbandWriters )
{
  // Add final meta data (broken tile information, etc)
  // that is obtained after the end of an observation.
  LOG_INFO_STR("Waiting for final meta data");
  FinalMetaData finalMetaData;
  finalMetaData.read(controlStream);

  LOG_INFO_STR("Processing final meta data");
  for (size_t i = 0; i < subbandWriters.size(); ++i)
    try {
      subbandWriters[i]->augment(finalMetaData);
    } catch (Exception &ex) {
      LOG_WARN_STR("Could not add final meta data: " << ex);
    }
}

void writeFeedbackLTA( Stream &controlStream, vector< SmartPtr<Writer> > &subbandWriters )
{
  LOG_INFO_STR("Retrieving LTA feedback");
  Parset feedbackLTA;
  for (size_t i = 0; i < subbandWriters.size(); ++i)
    try {
      feedbackLTA.adoptCollection(subbandWriters[i]->feedbackLTA());
    } catch (Exception &ex) {
      LOG_WARN_STR("Could not obtain feedback for LTA: " << ex);
    }

  LOG_INFO_STR("Forwarding LTA feedback");
  feedbackLTA.write(&controlStream);
}


void process(Stream &controlStream, size_t myRank)
{
  Parset parset(&controlStream);

  const vector<string> &hostnames = parset.settings.outputProcHosts;
  ASSERT(myRank < hostnames.size());
  string myHostName = hostnames[myRank];

  {
    // make sure "parset" stays in scope for the lifetime of the SubbandWriters

    vector<SmartPtr<Writer> > subbandWriters;

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

        Writer *writer = new SubbandWriter(parset, fileIdx, logPrefix);
        subbandWriters.push_back(writer);
      }
    }

    // Process beam-formed data
    if (parset.settings.beamFormer.enabled) {
      for (size_t fileIdx = 0; fileIdx < parset.settings.beamFormer.files.size(); ++fileIdx)
      {
        if (parset.settings.beamFormer.files[fileIdx].location.host != myHostName) 
          continue;

        string logPrefix = str(boost::format("[obs %u beamformed stream %3u] ") % parset.observationID() % fileIdx);

        Writer *writer = new TABWriter(parset, fileIdx, logPrefix);
        subbandWriters.push_back(writer);
      }
    }

    /*
     * PROCESS
     */

#   pragma omp parallel for num_threads(subbandWriters.size())
    for (int i = 0; i < (int)subbandWriters.size(); ++i)
    {
      subbandWriters[i]->process();
    }

    /*
     * FINAL META DATA
     */
    readFinalMetaData(controlStream, subbandWriters);

    /*
     * LTA FEEDBACK
     */
    writeFeedbackLTA(controlStream, subbandWriters);
  }
}

}
}

