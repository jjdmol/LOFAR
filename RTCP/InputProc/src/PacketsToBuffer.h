#ifndef __PACKETSTOBUFFER__
#define __PACKETSTOBUFFER__

#include <Interface/SmartPtr.h>
#include <Interface/Stream.h>
#include <Stream/Stream.h>
#include "RSP.h"
#include "RSPBoards.h"
#include "SampleBuffer.h"
#include "BufferSettings.h"
#include "PacketReader.h"
#include <string>
#include <ios>

namespace LOFAR {
namespace RTCP {

/* Receives station input and stores it in shared memory.
 *
 * PacketsToBuffer creates a SampleBuffer based on the provided settings
 * as well as the current clock and bit mode. The latter are auto-sensed
 * from the received packets, which are read from the inputStream. The
 * SampleBuffer is recreated if the clock or bit mode changes.
 *
 * The boardNr indicates the RSP board number for which the packets are
 * received.
 */
class PacketsToBuffer {
public:
  PacketsToBuffer( Stream &inputStream, const BufferSettings &settings, unsigned boardNr );

  // Process data for this board until interrupted or end of data. Auto-senses
  // mode (bit mode & clock).
  void process();

protected:
  const std::string logPrefix;

  // The input stream
  Stream &inputStream;
  uint32 lastlog_timestamp;

  // What to receive
  BufferSettings settings;
  const unsigned boardNr;

private:
  // Log if the received packet is LOG_INTERVAL seconds older than the previous one.
  static const uint32 LOG_INTERVAL = 10;

  // Process data for this board until interrupted or end of data.
  // `packet' is the receive buffer for packets. If a new mode is detected,
  // `packet' is filled with the last read packet, and a BadModeException
  // is thrown.
  //
  // If `writeGivenPacket' is true, the provided `packet' is written as well.
  template<typename T> void process( struct RSP &packet, bool writeGivenPacket ) throw(PacketReader::BadModeException);

  // Triggers statistics logging every LOG_INTERVAL seconds
  void logStatistics( PacketReader &reader, const struct RSP &packet );
};

/*
 * MultiPacketsToBuffer processes data from multiple RSP boards in parallel,
 * instantiating a PacketsToBuffer object for each.
 */
class MultiPacketsToBuffer: public RSPBoards {
public:
  MultiPacketsToBuffer( const BufferSettings &settings, const std::vector<std::string> &streamDescriptors )
  :
    RSPBoards("", streamDescriptors.size()),
    settings(settings),
    streamDescriptors(streamDescriptors)
  {
  }

protected:
  virtual void processBoard( size_t boardNr ) {
    SmartPtr<Stream> inputStream = createStream(streamDescriptors[boardNr], true);
    PacketsToBuffer board(*inputStream, settings, boardNr);

    board.process();
  }


  virtual void logStatistics() {
    // TODO
  }

private:
  const BufferSettings settings;
  const std::vector<std::string> streamDescriptors;
};


}
}

#endif
