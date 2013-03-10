#ifndef __STATION__
#define __STATION__

#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include <Stream/Stream.h>
#include <Stream/SocketStream.h>
#include <Interface/RSPTimeStamp.h>
#include <Interface/SmartPtr.h>
#include <Interface/Stream.h>
#include "RSP.h"
#include "WallClockTime.h"
#include "SampleBuffer.h"
#include "BufferSettings.h"
#include "PacketReader.h"
#include "PacketWriter.h"
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
  // The buffer we'll write to
  SampleBuffer<T> buffer;

  // Keep track of all readers and writers (for logging purposes)
  std::vector< SmartPtr<PacketReader> > readers;
  std::vector< SmartPtr<PacketWriter<T> > > writers;

  // Process data for this board until interrupted or end of data
  virtual void processBoard( size_t nr );

  // Log the statistics gathered since the previous call
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
    // Create the reader
    LOG_INFO_STR( logPrefix << "Connecting to " << streamDescriptors[nr] );
    SmartPtr<Stream> inputStream = createStream(streamDescriptors[nr], true);
    readers[nr] = new PacketReader(logPrefix, *inputStream);
    PacketReader &reader = *readers[nr];

    // Create the writer
    LOG_INFO_STR( logPrefix << "Connecting to shared memory buffer 0x" << std::hex << settings.dataKey );
    size_t firstBeamlet = settings.nrBeamlets / settings.nrBoards * nr;
    writers[nr] = new PacketWriter<T>(logPrefix, buffer, buffer.flags[nr], firstBeamlet, settings);
    PacketWriter<T> &writer = *writers[nr];

    LOG_INFO_STR( logPrefix << "Start" );

    // Transport packets from reader to writer
    struct RSP packet;

    for(;;)
      if (reader.readPacket(packet, settings))
        writer.writePacket(packet);

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

  // Log statistics of all boards. Note that there could
  // still be NULL pointers, assuming processBoards is running
  // in parallel.
  for (size_t nr = 0; nr < nrBoards; nr++) {
    if (readers[nr].get())
      readers[nr]->logStatistics();

    if (writers[nr].get())
      writers[nr]->logStatistics();
  }
}


}
}

#endif
