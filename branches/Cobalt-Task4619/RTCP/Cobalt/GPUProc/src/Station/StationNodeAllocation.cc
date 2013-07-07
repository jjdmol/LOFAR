#include <lofar_config.h>
#include "StationNodeAllocation.h"

#ifdef HAVE_MPI
#include <mpi.h>
#endif
#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>
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
  parset(parset)
{
}

bool StationNodeAllocation::receivedHere() const
{
  /*
   * The parset key
   *
   *   PIC.Core.Station.CS001LBA.RSP.host
   *
   * contains the name of the host at which
   * field CS001LBA will be received.
   *
   * In development however, we want to start everything
   * on 'localhost'. To accomodate that, if the above key
   * does not exist or equals to '*', the station will
   * be received by the MPI rank corresponding to the station's
   * index (modulo the number of MPI ranks), creating a
   * round-robin distribution.
   */
  const string stationName = str(format("%s%s") % stationID.stationName % stationID.antennaField);
  const string stationNode = parset.getString(str(format("PIC.Core.Station.%s.RSP.host") % stationName), "*");

  if (stationNode == "*") {
    // Let MPI rank (if any) determine whether we receive this station
    int rank = 0;
    int size = 1;

#ifdef HAVE_MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
#endif

    // Determine station index
    for (size_t i = 0; i < parset.settings.stations.size(); ++i) {
      if (parset.settings.stations[i].name == stationName) {
        // We distribute the stations over the MPI nodes
        // in a round-robin fashion.
        return static_cast<int>(i) % size == rank;
      }
    }

    // Station not in observation?
    return false;
  } else {
    // If the value is not '*', it must match our (short) host name
    const string hostName = myHostname(false);

    return stationNode == hostName;
  }
}

std::vector< SmartPtr<Stream> > StationNodeAllocation::inputStreams() const
{
  vector<string> inputStreamDescs = parset.getStringVector(str(format("PIC.Core.Station.%s%s.RSP.ports") % stationID.stationName % stationID.antennaField), true);
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
