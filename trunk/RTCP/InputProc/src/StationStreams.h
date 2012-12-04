#ifndef __STATIONSTREAMS__
#define __STATIONSTREAMS__

#include "BufferSettings.h"
#include <IONProc/WallClockTime.h>
#include <string>
#include <vector>

namespace LOFAR {
namespace RTCP {


class StationStreams {
public:
  StationStreams( const std::string &logPrefix, const BufferSettings &settings, const std::vector<std::string> &streamDescriptors );

  void process();

  void stop();

protected:
  const std::string logPrefix;
  const BufferSettings settings;
  const std::vector<std::string> streamDescriptors;
  const size_t nrBoards;

  WallClockTime waiter;

  virtual void processBoard( size_t nr ) = 0;
};


}
}

#endif
