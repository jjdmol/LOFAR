#ifndef __PACKETWRITER__
#define __PACKETWRITER__

#include "Station/RSP.h"
#include "SampleBuffer.h"
#include "BufferSettings.h"
#include "Ranges.h"
#include <string>

namespace LOFAR {
namespace RTCP {

/*
 * Writes RSP packets to a SampleBuffer
 */
template<typename T> class PacketWriter {
public:
  PacketWriter( const std::string &logPrefix, SampleBuffer<T> &buffer, unsigned boardNr );

  // Write a packet to the SampleBuffer
  void writePacket( const struct RSP &packet );

  void logStatistics();

private:
  const std::string logPrefix;

  SampleBuffer<T> &buffer;
  Ranges &flags;
  const struct BufferSettings &settings;
  const size_t firstBeamlet;

  size_t nrWritten;
};


}
}

#include "PacketWriter.tcc"

#endif
