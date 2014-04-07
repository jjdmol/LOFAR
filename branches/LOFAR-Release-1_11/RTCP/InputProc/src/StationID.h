#ifndef __STATIONID__
#define __STATIONID__

#include <Common/LofarTypes.h>
#include <ostream>
#include <string>

namespace LOFAR {
namespace RTCP {

struct StationID {
  char stationName[64];
  char antennaField[64];

  unsigned clock;
  unsigned bitmode;

  StationID( const std::string &stationName = "", const std::string &antennaField = "", unsigned clock = 200 * 1000 * 1000, unsigned bitmode = 16);

  bool operator==(const struct StationID &other) const;
  bool operator!=(const struct StationID &other) const;

  uint32 hash() const;
};

std::ostream& operator<<( std::ostream &str, const struct StationID &s );

}
}


#endif

