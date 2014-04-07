#ifndef __STATIONDATA__
#define __STATIONDATA__

#include <Common/LofarLogger.h>
#include <Stream/Stream.h>
#include <Interface/RSPTimeStamp.h>
#include <Interface/SmartPtr.h>
#include <RSP.h>
#include <WallClockTime.h>
#include "SampleBuffer.h"
#include "Ranges.h"
#include "StationSettings.h"
#include <string>
#include <cstring>
#include <omp.h>

namespace LOFAR {
namespace RTCP {

template<typename T> class RSPBoard {
public:
  RSPBoard( Stream &inputStream, SampleBuffer<T> &buffer, unsigned boardNr, const struct StationSettings &settings );

  const unsigned nr;

  bool readPacket();
  void writePacket();

  static size_t packetSize( struct RSP &packet ) {
    return sizeof(struct RSP::Header) + packet.header.nrBeamlets * packet.header.nrBlocks * sizeof(T);
  }
  
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

  size_t nrReceived, nrBadSize, nrBadTime, nrOutOfOrder;

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
    inputStream.read(&packet.data, packetSize(packet) - sizeof(struct RSP::Header));

    ++nrReceived;
  } else {
    // read full packet at once -- numbytes will tell us how much we've actually read
    size_t numbytes = inputStream.tryRead(&packet, sizeof packet);

    ++nrReceived;

    if( numbytes < sizeof(struct RSP::Header)
     || numbytes != packetSize(packet) ) {
      LOG_WARN_STR( logPrefix << "Packet is " << numbytes << " bytes, but should be " << packetSize(packet) << " bytes" );

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
  LOG_INFO_STR( logPrefix << "Received " << nrReceived << " packets: " << nrOutOfOrder << " out of order, " << nrBadTime << " bad timestamps, " << nrBadSize << " bad sizes" );

  nrReceived = 0;
  nrOutOfOrder = 0;
  nrBadTime = 0;
  nrBadSize = 0;
}


class StationStreams {
public:
  StationStreams( const std::string &logPrefix, const StationSettings &settings, const std::vector<std::string> &streamDescriptors );

  void process();

  void stop();

protected:
  const std::string logPrefix;
  const StationSettings settings;
  const std::vector<std::string> streamDescriptors;
  const size_t nrBoards;

  WallClockTime waiter;

  virtual void processBoard( size_t nr ) = 0;
};

StationStreams::StationStreams( const std::string &logPrefix, const StationSettings &settings, const std::vector<std::string> &streamDescriptors )
:
  logPrefix(logPrefix),
  settings(settings),
  streamDescriptors(streamDescriptors),
  nrBoards(streamDescriptors.size())
{
}

void StationStreams::process()
{
  std::vector<OMPThread> threads(nrBoards);

  ASSERT(nrBoards > 0);

  LOG_INFO_STR( logPrefix << "Start" );

  #pragma omp parallel sections num_threads(2)
  {
    #pragma omp section
    {
      // start all boards
      LOG_INFO_STR( logPrefix << "Starting all boards" );
      #pragma omp parallel for num_threads(nrBoards)
      for (size_t i = 0; i < nrBoards; ++i) {
        OMPThread::ScopedRun sr(threads[i]);
  
        processBoard(i);
      }
    }

    #pragma omp section
    {
      // wait until we have to stop
      LOG_INFO_STR( logPrefix << "Waiting for stop signal" );
      waiter.waitForever();     

      // kill all boards
      LOG_INFO_STR( logPrefix << "Stopping all boards" );
      #pragma omp parallel for num_threads(nrBoards)
      for (size_t i = 0; i < nrBoards; ++i)
        threads[i].kill();
    }
  }

  LOG_INFO_STR( logPrefix << "End" );
}

void StationStreams::stop()
{
  waiter.cancelWait();
}


template<typename T> class Station: public StationStreams {
public:
  Station( const StationSettings &settings, const std::vector<std::string> &streamDescriptors );

protected:
  SampleBuffer<T> buffer;

  virtual void processBoard( size_t nr );
};

template<typename T> Station<T>::Station( const StationSettings &settings, const std::vector<std::string> &streamDescriptors )
:
  StationStreams(str(boost::format("[station %s %s] [Station] ") % settings.station.stationName % settings.station.antennaSet), settings, streamDescriptors),

  buffer(settings, true)
{
  LOG_INFO_STR( logPrefix << "Initialised" );
}

template<typename T> void Station<T>::processBoard( size_t nr )
{
  const std::string logPrefix(str(boost::format("[station %s %s board %u] [Station] ") % settings.station.stationName % settings.station.antennaSet % nr));

  try {
    LOG_INFO_STR( logPrefix << "Connecting to " << streamDescriptors[nr] );
    SmartPtr<Stream> s = createStream(streamDescriptors[nr], true);

    LOG_INFO_STR( logPrefix << "Connecting to shared memory buffer 0x" << std::hex << settings.dataKey );
    RSPBoard<T> board(*s, buffer, nr, settings);

    LOG_INFO_STR( logPrefix << "Start" );

    for(;;)
      if (board.readPacket())
        board.writePacket();

  } catch (Stream::EndOfStreamException &ex) {
    LOG_INFO_STR( logPrefix << "End of stream");
  } catch (SystemCallException &ex) {
    if (ex.error == EINTR)
      LOG_INFO_STR( logPrefix << "Aborted: " << ex.what());
    else
      LOG_ERROR_STR( logPrefix << "Caught Exception: " << ex);
  } catch (Exception &ex) {
    LOG_ERROR_STR( logPrefix << "Caught Exception: " << ex);
  }

  LOG_INFO_STR( logPrefix << "End");
}


template<typename T> class Generator: public StationStreams {
public:
  Generator( const StationSettings &settings, const std::vector<std::string> &streamDescriptors );

protected:
  void processBoard( size_t nr );

  virtual void makePacket( struct RSP &header, const TimeStamp &timestamp );
};

template<typename T> Generator<T>::Generator( const StationSettings &settings, const std::vector<std::string> &streamDescriptors )
:
  StationStreams(str(boost::format("[station %s %s] [Generator] ") % settings.station.stationName % settings.station.antennaSet), settings, streamDescriptors)
{
  LOG_INFO_STR( logPrefix << "Initialised" );
}

template<typename T> void Generator<T>::makePacket( struct RSP &packet, const TimeStamp &timestamp )
{
  packet.header.nrBeamlets = settings.nrBeamlets / settings.nrBoards;
  packet.header.nrBlocks = 16;

  packet.header.timestamp = timestamp.getSeqId();
  packet.header.blockSequenceNumber = timestamp.getBlockId();

  int64 data = timestamp;

  memset(packet.data, data & 0xFF, sizeof packet.data);
}

template<typename T> void Generator<T>::processBoard( size_t nr )
{
  const std::string logPrefix(str(boost::format("[station %s %s board %u] [Generator] ") % settings.station.stationName % settings.station.antennaSet % nr));

  try {
    LOG_INFO_STR( logPrefix << "Connecting to " << streamDescriptors[nr] );
    SmartPtr<Stream> s = createStream(streamDescriptors[nr], false);

    LOG_INFO_STR( logPrefix << "Start" );

    TimeStamp current(time(0L) + 1, 0, settings.station.clock);
    for(;;) {
      struct RSP packet;

      makePacket( packet, current );

      ASSERT(RSPBoard<T>::packetSize(packet) <= sizeof packet);
     
      if (!waiter.waitUntil(current))
        break;

      try {
        s->write(&packet, RSPBoard<T>::packetSize(packet));
      } catch (SystemCallException &ex) {
        // UDP can return ECONNREFUSED or EINVAL if server does not have its port open
        if (ex.error != ECONNREFUSED && ex.error != EINVAL)
          throw;
      }

      current += packet.header.nrBlocks;
    }
  } catch (Stream::EndOfStreamException &ex) {
    LOG_INFO_STR( logPrefix << "End of stream");
  } catch (SystemCallException &ex) {
    if (ex.error == EINTR)
      LOG_INFO_STR( logPrefix << "Aborted: " << ex.what());
    else
      LOG_ERROR_STR( logPrefix << "Caught Exception: " << ex);
  } catch (Exception &ex) {
    LOG_ERROR_STR( logPrefix << "Caught Exception: " << ex);
  }

  LOG_INFO_STR( logPrefix << "End");
}

}
}

#endif
