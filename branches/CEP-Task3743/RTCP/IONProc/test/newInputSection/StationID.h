#ifndef __STATIONID__
#define __STATIONID__

#include <Common/LofarLogger.h>
#include <ostream>
#include <cstdio>
#include <string>

namespace LOFAR {
namespace RTCP {

struct StationID {
  char stationName[64];
  char antennaSet[64];

  unsigned clock;
  unsigned bitmode;

  StationID( const std::string &stationName = "", const std::string &antennaSet = "", unsigned clock = 200 * 1000 * 1000, unsigned bitmode = 16)
  :
    clock(clock),
    bitmode(bitmode)
  {
    snprintf(this->stationName, sizeof this->stationName, "%s", stationName.c_str());
    snprintf(this->antennaSet, sizeof this->antennaSet, "%s", antennaSet.c_str());
  }

  bool operator==(const struct StationID &other) const {
    return !strncmp(stationName, other.stationName, sizeof stationName)
        && !strncmp(antennaSet, other.antennaSet, sizeof antennaSet)
        && clock == other.clock
        && bitmode == other.bitmode;
  }

  bool operator!=(const struct StationID &other) const {
    return !(*this == other);
  }

  uint32 hash() const {
    // convert to 32 bit value (human-readable in hexadecimal)
    uint32 stationNr = 0;

    const std::string stationNameStr(stationName);
    const std::string antennaSetStr(antennaSet);

    for(std::string::const_iterator c = stationNameStr.begin(); c != stationNameStr.end(); ++c)
      if(*c >= '0' && *c <= '9')
        stationNr = stationNr * 16 + (*c - '0');

    uint32 antennaSetNr = 0;

    if (antennaSetStr == "HBA_ONE" || antennaSetStr == "HBA1" )
      antennaSetNr = 1;
    else
      antennaSetNr = 0;

    ASSERT( stationNr    < (1L << 16) );
    ASSERT( antennaSetNr < (1L << 4)  );

    ASSERT( clock/1000000 == 200 || clock/1000000 == 160 );
    ASSERT( bitmode == 4 || bitmode == 8 || bitmode == 16 );

    unsigned clockNr = clock/1000000 == 200 ? 0x20 : 0x16;
    unsigned bitmodeNr = bitmode == 16 ? 0xF : bitmode;

    return (stationNr << 16) + (antennaSetNr << 12) + (clockNr << 4) + bitmodeNr;
  }

};

std::ostream& operator<<( std::ostream &str, const struct StationID &s ) {
  str << "station " << s.stationName << " antennaset " << s.antennaSet << " clock " << s.clock/1000000 << " bitmode " << s.bitmode;

  return str;
}

}
}


#endif

