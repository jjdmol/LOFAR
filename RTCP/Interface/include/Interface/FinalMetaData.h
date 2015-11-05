#ifndef LOFAR_INTERFACE_FINAL_METADATA_H
#define LOFAR_INTERFACE_FINAL_METADATA_H

#include <Stream/Stream.h>
#include <string>
#include <vector>
#include <cstddef>

namespace LOFAR {
namespace RTCP {

class FinalMetaData
{
  public:
    struct BrokenRCU {
      std::string station; // CS001, etc
      std::string type;    // RCU, LBA, HBA
      size_t seqnr;        // RCU/antenna number
      std::string time;    // date time of break
    };

    std::vector<BrokenRCU>  brokenRCUsAtBegin, brokenRCUsDuring;

    void write(Stream &s);
    void read(Stream &s);
};

} // namespace RTCP
} // namespace LOFAR

#endif
