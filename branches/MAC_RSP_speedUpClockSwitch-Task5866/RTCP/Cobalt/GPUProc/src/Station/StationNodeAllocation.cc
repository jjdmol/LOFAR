//# StationNodeAllocation.cc: Manages which stations are received on which node
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <lofar_config.h>
#include "StationNodeAllocation.h"

#ifdef HAVE_MPI
#include <InputProc/Transpose/MPIUtil.h>
#endif
#include <boost/format.hpp>
#include <sstream>

#include <Common/LofarLogger.h>
#include <Stream/FileStream.h>
#include <CoInterface/Stream.h>

#include <InputProc/RSPTimeStamp.h>
#include <InputProc/Buffer/BoardMode.h>
#include <InputProc/Station/PacketFactory.h>
#include <InputProc/Station/PacketStream.h>

using namespace std;
using boost::format;

namespace LOFAR
{
  namespace Cobalt
  {

StationNodeAllocation::StationNodeAllocation( const StationID &stationID, const Parset &parset )
:
  stationID(stationID),
  parset(parset),
  stationIdx(parset.settings.antennaFieldIndex(stationID.name()))
{
  ASSERTSTR(stationIdx >= 0, "Station not found in observation: " << stationID.name());
}

bool StationNodeAllocation::receivedHere() const
{
  int rank = MPI_Rank();
  int nrHosts = MPI_Size();

  int stationRank = receiverRank();

  if (stationRank == -1) {
    // allocate stations not mentioned in Cobalt.Hardware round-robin
    stationRank = parset.settings.antennaFieldIndex(stationID.name()) % nrHosts;

    if (stationRank == rank)
      LOG_WARN_STR("Receiving station " << stationID << " due to round-robin allocation across nodes.");
  }

  return stationRank == rank;
}

int StationNodeAllocation::receiverRank() const
{
  /*
   * The parset keys
   *
   *   PIC.Code.<antennaFieldName>.RSP.{receiver,ports}
   *
   * contain the antenna field names (keys) that will be received
   * by the MPI ranks and interfaces (values). See StationStreams.parset
   */

  const string receiver = parset.settings.antennaFields[stationIdx].receiver;

  for (size_t rank = 0; rank < parset.settings.nodes.size(); ++rank) {
    if (parset.settings.nodes[rank].name == receiver)
      return rank;
  }

  return -1;
}

std::vector< SmartPtr<Stream> > StationNodeAllocation::inputStreams() const
{
  const string logPrefix = str(format("[station %s] ") % stationID.name());

  vector<string> inputStreamDescs = parset.settings.antennaFields[stationIdx].inputStreams;

  vector< SmartPtr<Stream> > inputStreams(inputStreamDescs.size());

  // Log all input descriptions
  stringstream inputDescription;

  for (size_t board = 0; board < inputStreamDescs.size(); ++board) {
    const string &desc = inputStreamDescs[board];

    if (board > 0)
      inputDescription << ", ";
    inputDescription << desc;
  }

  LOG_INFO_STR(logPrefix << "Input streams: " << inputDescription.str());

  // Connect to specified input stream
  for (size_t board = 0; board < inputStreamDescs.size(); ++board) {
    const string &desc = inputStreamDescs[board];

    LOG_DEBUG_STR(logPrefix << "Connecting input stream for board " << board << ": " << desc);

    // Sanity checks
    if (parset.settings.realTime) {
      ASSERTSTR(desc.find("udp:") == 0, logPrefix << "Real-time observations should read input from UDP, not " << desc);
    } else {
      ASSERTSTR(desc.find("udp:") != 0, logPrefix << "Non-real-time observations should NOT read input from UDP, got " << desc);
    }

    if (desc == "factory:") {
      const TimeStamp from(parset.startTime() * parset.subbandBandwidth(), parset.clockSpeed());
      const TimeStamp to(parset.stopTime() * parset.subbandBandwidth(), parset.clockSpeed());

      const struct BoardMode mode(parset.settings.nrBitsPerSample, parset.settings.clockMHz);
      PacketFactory factory(mode);

      inputStreams[board] = new PacketStream(factory, from, to, board);
    } else {
      try {
        inputStreams[board] = createStream(desc, true);
      } catch(Exception &ex) {
        if (parset.settings.realTime) {
          LOG_ERROR_STR(logPrefix << "Caught exception: " << ex.what());
          inputStreams[board] = new FileStream("/dev/null"); /* block on read to avoid spamming illegal packets */
        } else {
          throw;
        }
      }
    }
  }

  ASSERTSTR(inputStreams.size() > 0, logPrefix << "No input streams");

  return inputStreams;
}

  } // namespace Cobalt
} // namespace LOFAR
