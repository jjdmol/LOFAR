#include <lofar_config.h>
#include "StationNodeAllocation.h"

#ifdef HAVE_MPI
#include <mpi.h>
#endif
#include <boost/format.hpp>

#include <Common/LofarLogger.h>
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
  stationIdx(parset.settings.stationIndex(stationID.name()))
{
  ASSERTSTR(stationIdx >= 0, "Station not found in observation: " << stationID.name());
}

bool StationNodeAllocation::receivedHere() const
{
  int rank = 0;
  int nrHosts = 1;

#ifdef HAVE_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nrHosts);
#endif

  int stationRank = receiverRank();

  if (stationRank == -1) {
    // allocate stations not mentioned in Cobalt.Hardware round-robin
    stationRank = parset.settings.stationIndex(stationID.name()) % nrHosts;

    if (stationRank == rank)
      LOG_WARN_STR("Receiving station " << stationID << " due to round-robin allocation across nodes.");
  }

  return stationRank == rank;
}

int StationNodeAllocation::receiverRank() const
{
  /*
   * The parset key
   *
   *   Cobalt.Hardware.Node[rank].stations
   *
   * contains the station names that will be received
   * by the specified MPI rank.
   */

  const string receiver = parset.settings.stations[stationIdx].receiver;

  for (size_t rank = 0; rank < parset.settings.nodes.size(); ++rank) {
    if (parset.settings.nodes[rank].name == receiver)
      return rank;
  }

  return -1;
}

std::vector< SmartPtr<Stream> > StationNodeAllocation::inputStreams() const
{
  vector<string> inputStreamDescs = parset.settings.stations[stationIdx].inputStreams;

  vector< SmartPtr<Stream> > inputStreams(inputStreamDescs.size());

  for (size_t board = 0; board < inputStreamDescs.size(); ++board) {
    const string desc = inputStreamDescs[board];

    LOG_DEBUG_STR("Input stream for board " << board << ": " << desc);

    if (desc == "factory:") {
      const TimeStamp from(parset.startTime() * parset.subbandBandwidth(), parset.clockSpeed());
      const TimeStamp to(parset.stopTime() * parset.subbandBandwidth(), parset.clockSpeed());

      const struct BoardMode mode(parset.settings.nrBitsPerSample, parset.settings.clockMHz);
      PacketFactory factory(mode);

      inputStreams[board] = new PacketStream(factory, from, to, board);
    } else {
      inputStreams[board] = createStream(desc, true);
    }
  }

  ASSERTSTR(inputStreams.size() > 0, "No input streams for station " << stationID);

  return inputStreams;
}

  } // namespace Cobalt
} // namespace LOFAR
