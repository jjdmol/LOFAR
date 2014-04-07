//# StationID.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include "StationID.h"

#include <cstring>
#include <cstdio>
#include <Common/LofarLogger.h>

#ifndef HAVE_STRNLEN
static size_t strnlen( const char *s, size_t maxlen )
{
  size_t len = 0;

  for(; *s && len < maxlen; ++s)
    ++len;

  return len;
}
#endif

namespace LOFAR
{
  namespace Cobalt
  {


    StationID::StationID( const std::string &stationName, const std::string &antennaField, unsigned clockMHz, unsigned bitMode)
      :
      clockMHz(clockMHz),
      bitMode(bitMode)
    {
      ASSERTSTR( stationName.size() < sizeof this->stationName, "Station name longer than " << (sizeof this->stationName - 1) << " characters.");
      ASSERTSTR( antennaField.size() < sizeof this->antennaField, "Antenna-set name longer than " << (sizeof this->antennaField - 1) << " characters.");

      snprintf(this->stationName, sizeof this->stationName, "%s", stationName.c_str());
      snprintf(this->antennaField, sizeof this->antennaField, "%s", antennaField.c_str());
    }

    bool StationID::operator==(const struct StationID &other) const
    {
      return !strncmp(stationName, other.stationName, sizeof stationName)
             && !strncmp(antennaField, other.antennaField, sizeof antennaField)
             && clockMHz == other.clockMHz
             && bitMode == other.bitMode;
    }

    bool StationID::operator!=(const struct StationID &other) const
    {
      return !(*this == other);
    }

    uint32 StationID::hash() const
    {
      // convert to 32 bit value (human-readable in hexadecimal):
      //
      //
      // 0x0106020F
      //   \__||\||
      //      || |\_ bit mode:      F  = 16-bit, 8 = 8-bit, 4 = 4-bit
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

      ASSERT( clockMHz == 200 || clockMHz == 160 );
      ASSERT( bitMode == 4 || bitMode == 8 || bitMode == 16 );

      // derive the hash
      unsigned clockMHzNr = clockMHz == 200 ? 0x20 : 0x16;
      unsigned bitModeNr = bitMode == 16 ? 0xF : bitMode;

      return (stationNr << 16) + (antennaFieldNr << 12) + (clockMHzNr << 4) + bitModeNr;
    }

    std::ostream& operator<<( std::ostream &str, const struct StationID &s )
    {
      str << "station " << s.stationName << " antenna field " << s.antennaField << " clockMHz " << s.clockMHz << " bitMode " << s.bitMode;

      return str;
    }

  }
}

