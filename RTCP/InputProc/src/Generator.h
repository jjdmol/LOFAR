#ifndef __GENERATOR__
#define __GENERATOR__

#include <IONProc/RSP.h>
#include "BufferSettings.h"
#include <string>
#include <vector>

namespace LOFAR {
namespace RTCP {

class Generator: public StationStreams {
public:
  Generator( const BufferSettings &settings, const std::vector<std::string> &streamDescriptors );

protected:
  void processBoard( size_t nr );

  virtual void makePacket( size_t boardNr, struct RSP &packet, const TimeStamp &timestamp );
};

}
}

#endif
