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

#include <Common/LofarLogger.h>

using namespace std;

namespace LOFAR
{
  namespace Cobalt
  {

StationNodeAllocation::StationNodeAllocation( const StationID &stationID, const Parset &parset, int mpi_rank, int mpi_size )
:
  stationID(stationID),
  parset(parset),
  stationIdx(parset.settings.antennaFieldIndex(stationID.name())),
  mpi_rank(mpi_rank),
  mpi_size(mpi_size)
{
  ASSERTSTR(stationIdx >= 0, "Station not found in observation: " << stationID.name());
}

bool StationNodeAllocation::receivedHere() const
{
  int stationRank = receiverRank();

  if (stationRank == -1) {
    // allocate stations not mentioned in Cobalt.Hardware round-robin
    stationRank = parset.settings.antennaFieldIndex(stationID.name()) % mpi_size;

    if (stationRank == mpi_rank)
      LOG_WARN_STR("Receiving station " << stationID << " due to round-robin allocation across nodes.");
  }

  return stationRank == mpi_rank;
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


  } // namespace Cobalt
} // namespace LOFAR
