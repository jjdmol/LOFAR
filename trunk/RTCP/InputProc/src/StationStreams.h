#ifndef __STATIONSTREAMS__
#define __STATIONSTREAMS__

#include "StationSettings.h"
#include <IONProc/WallClockTime.h>
#include <string>
#include <vector>

namespace LOFAR {
namespace RTCP {


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


}
}

#endif
