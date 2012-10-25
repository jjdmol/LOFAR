#ifndef LOFAR_INTERFACE_FINAL_METADATA_H
#define LOFAR_INTERFACE_FINAL_METADATA_H

namespace LOFAR {
namespace RTCP {

#include <Stream/Stream.h>

class FinalMetaData
{
  public:
    void write(Stream &s) {}
    void read(Stream &s) {}
};

} // namespace RTCP
} // namespace LOFAR

#endif
