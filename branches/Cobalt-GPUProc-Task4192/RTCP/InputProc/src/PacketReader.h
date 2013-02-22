#ifndef __PACKETREADER__
#define __PACKETREADER__

#include <Stream/Stream.h>
#include <Interface/SmartPtr.h>
#include <IONProc/RSP.h>
#include <string>

namespace LOFAR {
namespace RTCP {

/* Receives input of one RSP board. */

class PacketReader {
public:
  PacketReader( const std::string &logPrefix, const std::string &streamDescriptor );

  // Reads a packet from the input stream. Returns true if a packet was
  // succesfully read.
  bool readPacket( struct RSP &packet );

  void logStatistics();

private:
  const std::string logPrefix;

  SmartPtr<Stream> inputStream;
  bool supportPartialReads;

  size_t nrReceived, nrBadSize, nrBadTime, nrBadData, nrBadMode;
  bool hadSizeError, hadModeError;
};


}
}

#endif
