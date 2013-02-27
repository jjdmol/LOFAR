#include <lofar_config.h>
#include "StationID.h"
#include <Common/LofarLogger.h>
#include <cstdio>

#include <cstring>

#ifndef HAVE_STRNLEN
static size_t strnlen( const char *s, size_t maxlen )
{
  size_t len = 0;

  for(; *s && len < maxlen; ++s)
    ++len;

  return len;
}
#endif

namespace LOFAR {
namespace RTCP {


StationID::StationID( const std::string &stationName, const std::string &antennaField, unsigned clock, unsigned bitmode)
:
  clock(clock),
  bitmode(bitmode)
{
  ASSERTSTR( stationName.size() < sizeof this->stationName, "Station name longer than " << (sizeof this->stationName - 1) << " characters.");
  ASSERTSTR( antennaField.size() < sizeof this->antennaField, "Antenna-set name longer than " << (sizeof this->antennaField - 1) << " characters.");

  snprintf(this->stationName, sizeof this->stationName, "%s", stationName.c_str());
  snprintf(this->antennaField, sizeof this->antennaField, "%s", antennaField.c_str());
}

bool StationID::operator==(const struct StationID &other) const {
  return !strncmp(stationName, other.stationName, sizeof stationName)
      && !strncmp(antennaField, other.antennaField, sizeof antennaField)
      && clock == other.clock
      && bitmode == other.bitmode;
}

bool StationID::operator!=(const struct StationID &other) const {
  return !(*this == other);
}

uint32 StationID::hash() const {
  // convert to 32 bit value (human-readable in hexadecimal):
  //
  //
  // 0x0106020F
  //   \__||\||
  //      || |\_ bitmode:       F  = 16-bit, 8 = 8-bit, 4 = 4-bit
  //      || \__ clock:         20 = 200 MHz, 16 = 160 MHz
  //      |\____ antenna field: 0 = HBA/HBA0/LBA, 1 = HBA1
  //      \_____ station ID:    0x0106 = RS106
  
  uint32 stationNr = 0;

  const std::string stationNameStr(stationName, strnlen(stationName, sizeof stationName));
  const std::string antennaFieldStr(antennaField, strnlen(antennaField, sizeof antennaField));

  for(std::string::const_iterator c = stationNameStr.begin(); c != stationNameStr.end(); ++c)
    if(*c >= '0' && *c <= '9')
      stationNr = stationNr * 16 + (*c - '0');

  uint32 antennaFieldNr = 0;

  if (antennaFieldStr == "HBA1")
    antennaFieldNr = 1;
  else
    antennaFieldNr = 0;

  // make sure everything fits
  ASSERT( stationNr    < (1L << 16) );
  ASSERT( antennaFieldNr < (1L << 4)  );

  ASSERT( clock/1000000 == 200 || clock/1000000 == 160 );
  ASSERT( bitmode == 4 || bitmode == 8 || bitmode == 16 );

  // derive the hash
  unsigned clockNr = clock/1000000 == 200 ? 0x20 : 0x16;
  unsigned bitmodeNr = bitmode == 16 ? 0xF : bitmode;

  return (stationNr << 16) + (antennaFieldNr << 12) + (clockNr << 4) + bitmodeNr;
}

std::ostream& operator<<( std::ostream &str, const struct StationID &s ) {
  str << "station " << s.stationName << " antenna field " << s.antennaField << " clock " << s.clock/1000000 << " bitmode " << s.bitmode;

  return str;
}

}
}
