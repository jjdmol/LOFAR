#ifndef __STATION__
#define __STATION__

#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include <Stream/Stream.h>
#include <Stream/SocketStream.h>
#include <Interface/RSPTimeStamp.h>
#include <Interface/SmartPtr.h>
#include <Interface/Stream.h>
#include <IONProc/RSP.h>
#include <IONProc/WallClockTime.h>
#include "SampleBuffer.h"
#include "BufferSettings.h"
#include "PacketReader.h"
#include "Ranges.h"
#include "time.h"
#include <boost/format.hpp>
#include <string>
#include <vector>
#include <ios>

namespace LOFAR {
namespace RTCP {

/* Receives station input and stores it in shared memory */

template<typename T> class PacketsToBuffer: public RSPBoards {
public:
  PacketsToBuffer( const BufferSettings &settings, const std::vector<std::string> &streamDescriptors );

protected:
  SampleBuffer<T> buffer;

  class BufferWriter {
  public:
    BufferWriter( const std::string &logPrefix, SampleBuffer<T> &buffer, Ranges &flags, size_t firstBeamlet, const struct BufferSettings &settings );

    // Write a packet to the SampleBuffer
    void writePacket( const struct RSP &packet );

    void logStatistics();

  private:
    const std::string logPrefix;

    SampleBuffer<T> &buffer;
    Ranges &flags;
    const struct BufferSettings settings;
    const size_t firstBeamlet;

    size_t nrWritten;
  };

  std::vector< SmartPtr<PacketReader> > readers;
  std::vector< SmartPtr<BufferWriter> > writers;

  // process data for this board until interrupted or end of data
  virtual void processBoard( size_t nr );

  virtual void logStatistics();
};


template<typename T> PacketsToBuffer<T>::PacketsToBuffer( const BufferSettings &settings, const std::vector<std::string> &streamDescriptors )
:
  RSPBoards(str(boost::format("[station %s %s] ") % settings.station.stationName % settings.station.antennaField), settings, streamDescriptors),

  buffer(settings, true),
  readers(nrBoards, 0),
  writers(nrBoards, 0)
{
  LOG_INFO_STR( logPrefix << "Initialised" );
}

template<typename T> void PacketsToBuffer<T>::processBoard( size_t nr )
{
  const std::string logPrefix(str(boost::format("%s [board %u] ") % this->logPrefix % nr));

  try {
    LOG_INFO_STR( logPrefix << "Connecting to " << streamDescriptors[nr] );
    readers[nr] = new PacketReader(logPrefix, streamDescriptors[nr], settings);

    LOG_INFO_STR( logPrefix << "Connecting to shared memory buffer 0x" << std::hex << settings.dataKey );
    size_t firstBeamlet = settings.nrBeamlets / settings.nrBoards * nr;
    writers[nr] = new BufferWriter(logPrefix, buffer, buffer.flags[nr], firstBeamlet, settings);

    LOG_INFO_STR( logPrefix << "Start" );

    struct RSP packet;

    for(;;)
      if (readers[nr]->readPacket(packet))
        writers[nr]->writePacket(packet);

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


template<typename T> void PacketsToBuffer<T>::logStatistics()
{
  ASSERT(readers.size() == nrBoards);
  ASSERT(writers.size() == nrBoards);

  for (size_t nr = 0; nr < nrBoards; nr++) {
    if (readers[nr].get())
      readers[nr]->logStatistics();

    if (writers[nr].get())
      writers[nr]->logStatistics();
  }
}


template<typename T> PacketsToBuffer<T>::BufferWriter::BufferWriter( const std::string &logPrefix, SampleBuffer<T> &buffer, Ranges &flags, size_t firstBeamlet, const struct BufferSettings &settings )
:
  logPrefix(str(boost::format("%s [BufferWriter] ") % logPrefix)),

  buffer(buffer),
  flags(flags),
  settings(settings),
  firstBeamlet(firstBeamlet),

  nrWritten(0)
{
  // bitmode must coincide with our template
  ASSERT( sizeof(T) == N_POL * 2 * settings.station.bitmode / 8 );
}


template<typename T> void PacketsToBuffer<T>::BufferWriter::writePacket( const struct RSP &packet )
{
  const uint8 &nrBeamlets  = packet.header.nrBeamlets;
  const uint8 &nrTimeslots = packet.header.nrBlocks;

  // should not exceed the number of beamlets in the buffer
  ASSERT( firstBeamlet + nrBeamlets < settings.nrBeamlets );

  const TimeStamp timestamp(packet.header.timestamp, packet.header.blockSequenceNumber, settings.station.clock);

  // determine the time span when cast on the buffer
  const size_t from_offset = (int64)timestamp % settings.nrSamples;
  size_t to_offset = ((int64)timestamp + nrTimeslots) % settings.nrSamples;

  if (to_offset == 0)
    to_offset = settings.nrSamples;

  const size_t wrap = from_offset < to_offset ? 0 : settings.nrSamples - from_offset;

  /*
   * Make sure the buffer and flags are always consistent.
   */

  // mark data we overwrite as invalid
  flags.excludeBefore(timestamp + nrTimeslots - settings.nrSamples);

  // transpose
  const T *beamlets = reinterpret_cast<const T*>(&packet.payload.data);

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

  ++nrWritten;
}


template<typename T> void PacketsToBuffer<T>::BufferWriter::logStatistics()
{
  LOG_INFO_STR( logPrefix << "Written " << nrWritten << " packets");

  nrWritten = 0;
}


}
}

#endif
