#ifndef __GENERATOR__
#define __GENERATOR__

#include "RSP.h"
#include "RSPBoards.h"
#include "BufferSettings.h"
#include <string>
#include <vector>

namespace LOFAR {
namespace RTCP {

/* Generate station input data */

class Generator: public RSPBoards {
public:
  Generator( const BufferSettings &settings, const std::vector<std::string> &streamDescriptors );

protected:
  const BufferSettings settings;
  const std::vector<std::string> streamDescriptors;

  std::vector<size_t> nrSent;

  virtual void processBoard( size_t nr );
  virtual void logStatistics();

  virtual void makePacket( size_t boardNr, struct RSP &packet, const TimeStamp &timestamp );
};

}
}

#endif
