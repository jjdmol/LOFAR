#ifndef LOFAR_INTERFACE_FINAL_METADATA_H
#define LOFAR_INTERFACE_FINAL_METADATA_H

#include <Stream/Stream.h>
#include <string>
#include <vector>

namespace LOFAR {
namespace RTCP {

class FinalMetaData
{
  public:
    struct BrokenTile {
      std::string station;
      std::string tile;
      std::string time;
    };

    struct BrokenRCU {
      std::string station;
      std::string rcu;
      std::string time;
    };

    std::vector<BrokenTile> brokenTilesAtBegin, brokenTilesDuring;
    std::vector<BrokenRCU>  brokenRCUsAtBegin, brokenRCUsDuring;

    void write(Stream &s);
    void read(Stream &s);
};

} // namespace RTCP
} // namespace LOFAR

#endif
