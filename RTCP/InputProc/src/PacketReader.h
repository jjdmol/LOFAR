#ifndef __PACKETREADER__
#define __PACKETREADER__

#include <Stream/Stream.h>
#include <Interface/SmartPtr.h>
#include "RSP.h"
#include "BufferSettings.h"
#include <string>

namespace LOFAR {
namespace RTCP {

/*
 * Reads RSP packets from a Stream, and collects statistics.
 *
 * Thread-safefy: none.
 */
class PacketReader {
public:
  PacketReader( const std::string &logPrefix, Stream &inputStream );

  // Reads a packet from the input stream. Returns true if a packet was
  // succesfully read.
  bool readPacket( struct RSP &packet );

  // Reads a packet from the input stream, and validates it against the given
  // settings. Returns true if the packet was read and valid.
  bool readPacket( struct RSP &packet, const struct BufferSettings &settings );

  // Logs (and resets) statistics about the packets read.
  void logStatistics();

private:
  const std::string logPrefix;

  // The stream from which packets are read.
  Stream &inputStream;

  // Whether inputStream can do a small read() without data loss.
  bool supportPartialReads;

  // Statistics covering the packets read so far
  size_t nrReceived;  // nr. of packets received
  size_t nrBadSize;   // nr. of packets with wrong size (only if supportPartialReads == true)
  size_t nrBadTime;   // nr. of packets with an illegal time stamp
  size_t nrBadData;   // nr. of packets with payload errors
  size_t nrBadMode;   // nr. of packets with an incorrect clock/bitmode

  bool hadSizeError;  // already reported about wrongly sized packets since last logStatistics()
  bool hadModeError;  // already reported about wrong clocks/bitmodes since last logStatistics()
};


}
}

#endif
