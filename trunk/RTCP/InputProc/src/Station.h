#ifndef __STATION__
#define __STATION__

#include <Common/LofarLogger.h>
#include <Stream/Stream.h>
#include <Interface/SmartPtr.h>
#include "SampleBuffer.h"
#include "StationSettings.h"
#include <string>
#include <vector>

namespace LOFAR {
namespace RTCP {

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

}
}

#endif
