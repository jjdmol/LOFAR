//#  -*- mode: c++ -*-
//#  SpectralWindow.cc: implementation of the SpectralWindow class
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
#include <Common/StringUtil.h>

#include <APL/CAL_Protocol/SpectralWindow.h>

#include <blitz/array.h>
#include <sstream>

#include <MACIO/Marshalling.h>
#include <APL/RTCCommon/MarshallBlitz.h>

#include <math.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;

SpectralWindow::SpectralWindow() :
  m_name("undefined"), m_sampling_freq(0), m_nyquist_zone(0), m_numsubbands(0), m_rcucontrol(0)
{
}

SpectralWindow::~SpectralWindow()
{
}

double SpectralWindow::getSubbandFreq(int subband) const
{
  ASSERT(m_numsubbands);
  ASSERT(subband >= 0 && subband <= m_numsubbands);

  bool is160 	   = ::fabs(160e6 - m_sampling_freq) < 1e-4;
  float freqOffset = (is160 ? 80.0e6 : 100.0e6) * (m_nyquist_zone - 1);

  return (freqOffset + ((subband % m_numsubbands) * getSubbandWidth()));
}

bool SpectralWindow::isSuitable(int subband) const
{
  bool is160 = ::fabs(160e6 - m_sampling_freq) < 1e-4;
  bool is200 = ::fabs(200e6 - m_sampling_freq) < 1e-4;

  switch (m_rcucontrol) {
  case 0xB9: // LB_10_90
    {
      if (m_nyquist_zone == 1) {
	if (is160) {
	  // filter stopband < 10 MHz
	  if (getSubbandFreq(subband) < 10e6) return false;
	  
	  // aliasing > 70 Mhz (filter stopband > 90 MHz)
	  if (getSubbandFreq(subband) > 70e6) return false;

	} else if (is200) {
	  // filter stopband < 10 Mhz
	  if (getSubbandFreq(subband) < 10e6) return false;

	  // filter stoppband > 90 MHz
	  if (getSubbandFreq(subband) > 90e6) return false;
	}
      } else {
	// not suitable at all
	return false;
      }
    }
    break;

  case 0xC6: // HB_110_190
    {
      if (m_nyquist_zone == 2) {
	if (is160) {
	  // filter stopband < 110 MHz
	  if (getSubbandFreq(subband) < 110e6) return false;

	  // aliasing > 130 MHz
	  if (getSubbandFreq(subband) > 130e6) return false;
	} else if (is200) {
	  // filter stopband < 110 MHz
	  if (getSubbandFreq(subband) < 110e6) return false;

	  // filter stopband > 190 MHz
	  if (getSubbandFreq(subband) > 190e6) return false;
	}
      } else {
	// not suitable at all
	return false;
      }
    }
    break;
      
  case 0xCE: // HB_170_230
    {
      if (m_nyquist_zone == 3) {
	if (is160) {
	  // filter stopband < 170 MHz
	  if (getSubbandFreq(subband) < 170e6) return false;

	  // filter stopband > 230 MHz
	  if (getSubbandFreq(subband) > 230e6) return false;
	} else if (is200) {
	  // not suitable at all
	  return false;
	} 
      } else {
	// not suitable at all
	return false;
      }
    }
    break;

  case 0xD6: // HB_210_250
    {
      if (m_nyquist_zone == 3) {
	if (is160) {
	  // filter stopband < 210 MHz
	  if (getSubbandFreq(subband) < 210e6) return false;

	  // filter stopband > 230 MHz
	  if (getSubbandFreq(subband) > 230e6) return false;
	} else if (is200) {
	  // filter stopband < 210 MHz
	  if (getSubbandFreq(subband) < 210e6) return false;

	  // filter stopband > 250 MHz
	  if (getSubbandFreq(subband) > 250e6) return false;
	}
      } else {
	// not suitable at all
	return false;
      }
    }
    break;

  default:
    LOG_WARN(formatString("SpectralWindow::isSuitable: unsupported RCU control setting (0x%x).",
			  m_rcucontrol));
    break;
  }

  return false; // assume that the subband is not suitable
}

//
// isForHBA(): bool
//
bool SpectralWindow::isForHBA() const
{
	LOG_DEBUG (formatString("isForHBA(%06X)", m_rcucontrol));

	switch (m_rcucontrol) {
	case 0x017900:	// LB_10_90
	case 0x057900:	// LB_30_80
	case 0x037A00:	// LBH_10_90
	case 0x077A00:	// LBH_30_80
		return (false);
		break;

	case 0x07A400: // HB_110_190
	case 0x079400: // HB_170_230
	case 0x078400: // HB_210_250
		return (true);
		break;

	default:
		LOG_WARN(formatString("Unknown RCUcontrol setting (0x%X), assuming LBA array",
							m_rcucontrol));
		break;
	}

	return (false);		// assume LBA
}


unsigned int SpectralWindow::getSize() const
{
  return MSH_STRING_SIZE(m_name) +
    sizeof(m_sampling_freq) +
    sizeof(m_nyquist_zone) +
    sizeof(m_numsubbands) +
    sizeof(m_rcucontrol);
}

unsigned int SpectralWindow::pack(void* buffer) const
{
  unsigned int offset = 0;

  MSH_PACK_STRING(buffer, offset, m_name);
  memcpy(((char*)buffer) + offset, &m_sampling_freq, sizeof(m_sampling_freq));
  offset += sizeof(m_sampling_freq);
  memcpy(((char*)buffer) + offset, &m_nyquist_zone, sizeof(m_nyquist_zone));
  offset += sizeof(m_nyquist_zone);
  memcpy(((char*)buffer) + offset, &m_numsubbands, sizeof(m_numsubbands));
  offset += sizeof(m_numsubbands);
  memcpy(((char*)buffer) + offset, &m_rcucontrol, sizeof(m_rcucontrol));
  offset += sizeof(m_rcucontrol);

  return offset;
}

unsigned int SpectralWindow::unpack(void* buffer)
{
  unsigned int offset = 0;

  MSH_UNPACK_STRING(buffer, offset, m_name);
  memcpy(&m_sampling_freq, ((char*)buffer) + offset, sizeof(m_sampling_freq));
  offset += sizeof(m_sampling_freq);
  memcpy(&m_nyquist_zone, ((char*)buffer) + offset, sizeof(m_nyquist_zone));
  offset += sizeof(m_nyquist_zone);
  memcpy(&m_numsubbands, ((char*)buffer) + offset, sizeof(m_numsubbands));
  offset += sizeof(m_numsubbands);
  memcpy(&m_rcucontrol, ((char*)buffer) + offset, sizeof(m_rcucontrol));
  offset += sizeof(m_rcucontrol);

  return offset;
}
