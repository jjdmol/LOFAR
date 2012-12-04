#ifndef __STATION__
#define __STATION__

#include <Common/LofarLogger.h>
#include <Stream/Stream.h>
#include <Interface/RSPTimeStamp.h>
#include <Interface/SmartPtr.h>
#include <IONProc/RSP.h>
#include <IONProc/WallClockTime.h>
#include "SampleBuffer.h"
#include "BufferSettings.h"
#include "Ranges.h"
#include "time.h"
#include <boost/format.hpp>
#include <string>
#include <vector>
#include <ios>

namespace LOFAR {
namespace RTCP {

/* Receives station input and stores it in shared memory */

template<typename T> class Station: public StationStreams {
public:
  Station( const BufferSettings &settings, const std::vector<std::string> &streamDescriptors );

protected:
  SampleBuffer<T> buffer;

  /* Receives input of one RSP board and stores it in shared memory. */

  class RSPBoard {
  public:
    RSPBoard( const std::string &streamDescriptor, SampleBuffer<T> &buffer, unsigned boardNr, const struct BufferSettings &settings );

    // RSP board number
    const unsigned nr;

    void processBoard();

    void logStatistics();

  private:
    const std::string logPrefix;

    SmartPtr<Stream> inputStream;
    bool supportPartialReads;

    SampleBuffer<T> &buffer;
    Ranges &flags;
    const struct BufferSettings settings;
    const size_t firstBeamlet;

    size_t nrReceived, nrBadSize, nrBadTime, nrBadData, nrBadMode;

    // Reads a packet from the input stream. Returns true if a packet was
    // succesfully read.
    bool readPacket( struct RSP &packet );

    // Write the packet to the SampleBuffer
    void writePacket( const struct RSP &packet );
  };

  std::vector< SmartPtr<Stream> > streams;
  std::vector< SmartPtr< RSPBoard<T> > > boards;

  // process data for this board until interrupted or end of data
  virtual void processBoard( size_t nr );

  virtual void logStatistics();
};


template<typename T> Station<T>::Station( const BufferSettings &settings, const std::vector<std::string> &streamDescriptors )
:
  StationStreams(str(boost::format("[station %s %s] [Station] ") % settings.station.stationName % settings.station.antennaSet), settings, streamDescriptors),

  buffer(settings, true),
  streams(nrBoards, 0),
  boards(nrBoards, 0)
{
  LOG_INFO_STR( logPrefix << "Initialised" );
}

template<typename T> void Station<T>::processBoard( size_t nr )
{
  const std::string logPrefix(str(boost::format("[station %s %s board %u] [Station] ") % settings.station.stationName % settings.station.antennaSet % nr));

  try {
    LOG_INFO_STR( logPrefix << "Connecting to shared memory buffer 0x" << std::hex << settings.dataKey );
    boards[nr] = new RSPBoard<T>(streamDescriptors[nr], buffer, nr, settings);

    LOG_INFO_STR( logPrefix << "Start" );
    boards[nr]->processBoard();

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


template<typename T> void Station<T>::logStatistics()
{
  for (size_t nr = 0; nr < boards.size(); nr++)
    if (boards[nr].get())
      boards[nr]->logStatistics();
}


template<typename T> Station<T>::RSPBoard::RSPBoard( const std::string &streamDescriptor, SampleBuffer<T> &buffer, unsigned boardNr, const struct BufferSettings &settings )
:
  nr(boardNr),
  logPrefix(str(boost::format("[station %s %s board %u] [RSPBoard] ") % settings.station.stationName % settings.station.antennaSet % nr)),
  last_logtime(0),

  buffer(buffer),
  flags(buffer.flags[boardNr]),
  settings(settings),
  firstBeamlet(settings.nrBeamlets / settings.nrBoards * boardNr),

  nrReceived(0),
  nrBadSize(0),
  nrBadTime(0),
  nrBadData(0),
  nrBadMode(0)
{
  ASSERTSTR(boardNr < settings.nrBoards, "Invalid board number: " << boardNr << ", nrBoards: " << settings.nrBoards);

  LOG_INFO_STR( logPrefix << "Connecting to " << streamDescriptor );
  inputStream = createStream(streamDescriptor, true);

  SocketStream *asSocket = dynamic_cast<SocketStream *>(inputStream.get());
  bool isUDP = asSocket && asSocket->protocol == SocketStream::UDP;

  supportPartialReads = !isUDP;
}

template<typename T> void Station<T>::RSPBoard::writePacket( const struct RSP &packet )
{
  const uint8 &nrBeamlets  = packet.header.nrBeamlets;
  const uint8 &nrTimeslots = packet.header.nrBlocks;

  ASSERT( nrBeamlets <= settings.nrBeamlets / settings.nrBoards );

  const TimeStamp timestamp(packet.header.timestamp, packet.header.blockSequenceNumber, settings.station.clock);

  const size_t from_offset = (int64)timestamp % settings.nrSamples;
  size_t to_offset = ((int64)timestamp + nrTimeslots) % settings.nrSamples;

  if (to_offset == 0)
    to_offset = settings.nrSamples;

  // mark data we overwrite as invalid
  flags.excludeBefore(timestamp + nrTimeslots - settings.nrSamples);

  // transpose
  const T *beamlets = reinterpret_cast<const T*>(&packet.data);
  const size_t wrap = from_offset < to_offset ? 0 : settings.nrSamples - from_offset;

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

template<typename T> bool Station<T>::RSPBoard::readPacket( struct RSP &packet )
{
  bool valid = true;

  if (supportPartialReads) {
    // read header first
    inputStream->read(&packet, sizeof(struct RSP::Header));

    // read rest of packet
    inputStream->read(&packet.data, packet.packetSize() - sizeof(struct RSP::Header));

    ++nrReceived;
  } else {
    // read full packet at once -- numbytes will tell us how much we've actually read
    size_t numbytes = inputStream->tryRead(&packet, sizeof packet);

    ++nrReceived;

    if( numbytes < sizeof(struct RSP::Header)
     || numbytes != packet.packetSize() ) {
      LOG_WARN_STR( logPrefix << "Packet is " << numbytes << " bytes, but should be " << packet.packetSize() << " bytes" );

      ++nrBadSize;
      valid = false;
    }
  }

  // illegal timestamp means illegal packet
  if (valid && packet.header.timestamp == ~0U) {
    ++nrBadTime;
    valid = false;
  }

  if (valid) {
    // check sanity of packet

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

  return valid;
}


template<typename T> void Station<T>::RSPBoard::processBoard()
{
  struct RSP packet;

  for(;;)
    if (readPacket(packet))
      writePacket(packet);
}


template<typename T> void Station<T>::RSPBoard::logStatistics()
{
  LOG_INFO_STR( logPrefix << "Received " << nrReceived << " packets: " << nrBadTime << " bad timestamps, " << nrBadSize << " bad sizes, " << nrBadData << " payload errors, " << nrBadMode << " mode errors" );

  nrReceived = 0;
  nrBadTime = 0;
  nrBadSize = 0;
  nrBadData = 0;
  nrBadMode = 0;
}



}
}

#endif
