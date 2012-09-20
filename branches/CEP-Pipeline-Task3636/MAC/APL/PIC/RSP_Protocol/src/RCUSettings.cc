//#  RCUSettings.h: implementation of the RCUSettings class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RCUSettings.h>
#include <APL/RTCCommon/MarshallBlitz.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace RSP_Protocol;

const uint32 RCUSettings::Control::m_mode[] = {
  0x00003000, // MODE_OFF
  0x00017900, // MODE_LBL_HPF10MHZ
  0x00057900, // MODE_LBL_HPF30MHZ
  0x00037A00, // MODE_LBH_HPF10MHZ
  0x00077A00, // MODE_LBH_HPF30MHZ 
  0x0007A400, // MODE_HB_110_190MHZ
  0x00079400, // MODE_HB_170_230MHZ
  0x00078400, // MODE_HB_210_290MHZ
};

int RCUSettings::Control::getNyquistZone() const
{
  int retval = 0; // default, means undeterminted

  // find the mode
  int modeindex = 0;
  for (modeindex = MODE_LBL_HPF10MHZ; modeindex <= MODE_HB_210_290MHZ; modeindex++) {
    if ((m_value & MODE_MASK) == m_mode[modeindex]) break;
  }
  if (modeindex <= MODE_HB_210_290MHZ) {
    switch (modeindex) {
    case MODE_OFF:
    case MODE_LBL_HPF10MHZ:
    case MODE_LBL_HPF30MHZ:
    case MODE_LBH_HPF10MHZ:
    case MODE_LBH_HPF30MHZ:
      retval = 1;
      break;
    case MODE_HB_110_190MHZ:
      retval = 2;
      break;
    case MODE_HB_170_230MHZ:
    case MODE_HB_210_290MHZ:
      retval = 3;
      break;
    default: retval = 0;
      break;
    }
  }

  return retval;
}

unsigned int RCUSettings::getSize()
{
  return MSH_ARRAY_SIZE(m_registers, RCUSettings::Control);
}

unsigned int RCUSettings::pack  (void* buffer)
{
  unsigned int offset = 0;
  
  MSH_PACK_ARRAY(buffer, offset, m_registers, RCUSettings::Control);

  return offset;
}

unsigned int RCUSettings::unpack(void *buffer)
{
  unsigned int offset = 0;

  MSH_UNPACK_ARRAY(buffer, offset, m_registers, RCUSettings::Control, 1);

  return offset;
}
