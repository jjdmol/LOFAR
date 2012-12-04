#ifndef __RSPBOARD__
#define __RSPBOARD__

#include <Common/LofarLogger.h>
#include <Stream/Stream.h>
#include <Interface/RSPTimeStamp.h>
#include <IONProc/RSP.h>
#include <IONProc/WallClockTime.h>
#include "SampleBuffer.h"
#include "Ranges.h"
#include "StationSettings.h"
#include <string>

namespace LOFAR {
namespace RTCP {

template<typename T> class RSPBoard {
public:
  RSPBoard( Stream &inputStream, SampleBuffer<T> &buffer, unsigned boardNr, const struct StationSettings &settings );

  // RSP board  number
  const unsigned nr;

  bool readPacket();
  void writePacket();
  
private:
  const std::string logPrefix;

  Stream &inputStream;
  const bool supportPartialReads;
  struct RSP packet;
  TimeStamp last_timestamp;
  TimeStamp last_logtimestamp;

  SampleBuffer<T> &buffer;
  Ranges &flags;
  const struct StationSettings settings;
  const size_t firstBeamlet;

  size_t nrReceived, nrBadSize, nrBadTime, nrBadData, nrBadConfig, nrOutOfOrder;

  void logStatistics();
};

template<typename T> RSPBoard<T>::RSPBoard( Stream &inputStream, SampleBuffer<T> &buffer, unsigned boardNr, const struct StationSettings &settings )
:
  nr(boardNr),
  logPrefix(str(boost::format("[station %s %s board %u] [RSPBoard] ") % settings.station.stationName % settings.station.antennaSet % nr)),
  inputStream(inputStream),
  supportPartialReads(dynamic_cast<SocketStream *>(&inputStream) == 0 || dynamic_cast<SocketStream &>(inputStream).protocol != SocketStream::UDP),

  buffer(buffer),
  flags(buffer.flags[boardNr]),
  settings(settings),
  firstBeamlet(settings.nrBeamlets / settings.nrBoards * boardNr),

  nrReceived(0),
  nrBadSize(0),
  nrBadTime(0),
  nrBadData(0),
  nrBadConfig(0),
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
      return false;
    }
  }

  // check sanity of packet

  // detect bad timestamp
  if (packet.header.timestamp == ~0U) {
    ++nrBadTime;
    return false;
  }

  const TimeStamp timestamp(packet.header.timestamp, packet.header.blockSequenceNumber, settings.station.clock);

  // detect out-of-order data
  if (timestamp < last_timestamp) {
    ++nrOutOfOrder;
    return false;
  }

  // don't accept big jumps (>10s) in timestamp
  const int64 oneSecond = settings.station.clock / 1024;

  if (last_timestamp && packet.header.timestamp > last_timestamp + 10 * oneSecond) {
    ++nrBadTime;
    return false;
  }

  // discard packets with errors
  if (packet.payloadError()) {
    ++nrBadData;
    return false;
  }

  // check whether the station configuration matches ours
  if (packet.clockMHz() * 1000000 != settings.station.clock) {
    ++nrBadConfig;
    return false;
  }

  if (packet.bitMode() != settings.station.bitmode) {
    ++nrBadConfig;
    return false;
  }

  // packet was read and is sane

  last_timestamp = timestamp;

  if (timestamp > last_logtimestamp + oneSecond) {
    logStatistics();

    last_logtimestamp = timestamp;
  }

  return true;
}


template<typename T> void RSPBoard<T>::logStatistics()
{
  LOG_INFO_STR( logPrefix << "Received " << nrReceived << " packets: " << nrOutOfOrder << " out of order, " << nrBadTime << " bad timestamps, " << nrBadSize << " bad sizes, " << nrBadData << " payload errors, " << nrBadConfig << " configuration errors" );

  nrReceived = 0;
  nrBadTime = 0;
  nrBadSize = 0;
  nrBadData = 0;
  nrBadConfig = 0;
  nrOutOfOrder = 0;
}


}
}

#endif
