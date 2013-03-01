#ifndef __RSPBOARDS__
#define __RSPBOARDS__

#include "BufferSettings.h"
#include "WallClockTime.h"
#include <string>
#include <vector>

namespace LOFAR {
namespace RTCP {

/* A class that generates or processes a set of data streams of a station. */

class RSPBoards {
public:
  RSPBoards( const std::string &logPrefix, const BufferSettings &settings, const std::vector<std::string> &streamDescriptors );

  void process();

  void stop();

protected:
  const std::string logPrefix;
  const BufferSettings settings;
  const std::vector<std::string> streamDescriptors;
  const size_t nrBoards;

  WallClockTime waiter;

  virtual void processBoard( size_t nr ) = 0;
  virtual void logStatistics() = 0;
};


}
}

#endif
