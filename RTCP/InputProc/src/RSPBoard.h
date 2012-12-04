#ifndef __RSPBOARD__
#define __RSPBOARD__

#include <Common/LofarLogger.h>
#include <Stream/Stream.h>
#include <Interface/RSPTimeStamp.h>
#include <IONProc/RSP.h>
#include <IONProc/WallClockTime.h>
#include "SampleBuffer.h"
#include "Ranges.h"
#include "BufferSettings.h"
#include "time.h"
#include <string>

namespace LOFAR {
namespace RTCP {

template<typename T> class RSPBoard {
public:
  RSPBoard( Stream &inputStream, SampleBuffer<T> &buffer, unsigned boardNr, const struct BufferSettings &settings );

  // RSP board number
  const unsigned nr;

  bool readPacket();
  void writePacket();
  
private:
  const std::string logPrefix;

  Stream &inputStream;
  const bool supportPartialReads;
  struct RSP packet;
  TimeStamp last_timestamp;
  time_t last_logtime;

  SampleBuffer<T> &buffer;
  Ranges &flags;
  const struct BufferSettings settings;
  const size_t firstBeamlet;

  size_t nrReceived, nrBadSize, nrBadTime, nrBadData, nrBadMode, nrOutOfOrder;

  void logStatistics();
};

template<typename T> RSPBoard<T>::RSPBoard( Stream &inputStream, SampleBuffer<T> &buffer, unsigned boardNr, const struct BufferSettings &settings )
:
  nr(boardNr),
  logPrefix(str(boost::format("[station %s %s board %u] [RSPBoard] ") % settings.station.stationName % settings.station.antennaSet % nr)),
  inputStream(inputStream),
  supportPartialReads(dynamic_cast<SocketStream *>(&inputStream) == 0 || dynamic_cast<SocketStream &>(inputStream).protocol != SocketStream::UDP),
  last_logtime(0),

  buffer(buffer),
  flags(buffer.flags[boardNr]),
  settings(settings),
  firstBeamlet(settings.nrBeamlets / settings.nrBoards * boardNr),

  nrReceived(0),
  nrBadSize(0),
  nrBadTime(0),
  nrBadData(0),
  nrBadMode(0),
  nrOutOfOrder(0)
{
}

template<typename T> void RSPBoard<T>::writePacket()
{
  const uint8 &nrBeamlets  = packet.header.nrBeamlets;
  const uint8 &nrTimeslots = packet.header.nrBlocks;

  // the timestamp is of the last read packet by definition
  const TimeStamp &timestamp = last_timestamp;

  const size_t from_offset = (int64)timestamp % settings.nrSamples;
  size_t to_offset = ((int64)timestamp + nrTimeslots) % settings.nrSamples;

  if (to_offset == 0)
    to_offset = settings.nrSamples;

  const size_t wrap = from_offset < to_offset ? 0 : settings.nrSamples - from_offset;

  const T *beamlets = reinterpret_cast<const T*>(&packet.data);

  ASSERT( nrBeamlets <= settings.nrBeamlets / settings.nrBoards );

  // mark data we overwrite as invalid
  flags.excludeBefore(timestamp + nrTimeslots - settings.nrSamples);

  // transpose
  for (uint8 b = 0; b < nrBeamlets; ++b) {
    T *dst1 = &buffer.beamlets[firstBeamlet + b][from_offset];

    if (wrap > 0) {
      T *dst2 = &buffer.beamlets[firstBeamlet + b][0];

      memcpy(dst1, beamlets, wrap        * sizeof(T));
      memcpy(dst2, beamlets, to_offset   * sizeof(T));
    } else {
      memcpy(dst1, beamlets, nrTimeslots * sizeof(T));
    }

    beamlets += nrTimeslots;
  }

  // mark as valid
  flags.include(timestamp, timestamp + nrTimeslots);
}

template<typename T> bool RSPBoard<T>::readPacket()
{
  bool valid = true;

  if (supportPartialReads) {
    // read header first
    inputStream.read(&packet, sizeof(struct RSP::Header));

    // read rest of packet
    inputStream.read(&packet.data, packet.packetSize() - sizeof(struct RSP::Header));

    ++nrReceived;
  } else {
    // read full packet at once -- numbytes will tell us how much we've actually read
    size_t numbytes = inputStream.tryRead(&packet, sizeof packet);

    ++nrReceived;

    if( numbytes < sizeof(struct RSP::Header)
     || numbytes != packet.packetSize() ) {
      LOG_WARN_STR( logPrefix << "Packet is " << numbytes << " bytes, but should be " << packet.packetSize() << " bytes" );

      ++nrBadSize;
      valid = false;
    }
  }

  // illegal timestamp means illegal packet
  if (packet.header.timestamp == ~0U) {
    ++nrBadTime;
    valid = false;
  }

  if (valid) {
    // check sanity of packet

    const TimeStamp timestamp(packet.header.timestamp, packet.header.blockSequenceNumber, settings.station.clock);

    // don't accept out-of-order data
    if (last_timestamp > timestamp) {
      ++nrOutOfOrder;
      valid = false;
    }

    // packet has legal timestamp
    last_timestamp = timestamp;

    // discard packets with errors
    if (packet.payloadError()) {
      ++nrBadData;
      valid = false;
    }

    // check whether the station configuration matches ours
    if (packet.clockMHz() * 1000000 != settings.station.clock
     || packet.bitMode() != settings.station.bitmode) {
      ++nrBadMode;
      valid = false;
    }
  }

  // log updated statistics
  // TODO: use separate thread!
  time_t now = time(0);

  if (now >= last_logtime + 1) {
    logStatistics();

    last_logtime = now;
  }

  return valid;
}


template<typename T> void RSPBoard<T>::logStatistics()
{
  LOG_INFO_STR( logPrefix << "Received " << nrReceived << " packets: " << nrOutOfOrder << " out of order, " << nrBadTime << " bad timestamps, " << nrBadSize << " bad sizes, " << nrBadData << " payload errors, " << nrBadMode << " mode errors" );

  nrReceived = 0;
  nrBadTime = 0;
  nrBadSize = 0;
  nrBadData = 0;
  nrBadMode = 0;
  nrOutOfOrder = 0;
}


}
}

#endif
