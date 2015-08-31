//#  -*- mode: c++ -*-
//#  SpectralWindow.cc: implementation of the SpectralWindow class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#include <MACIO/Marshalling.tcc>
#include <APL/RTCCommon/MarshallBlitz.h>

#include <math.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;

SpectralWindow::SpectralWindow() :
  m_name("undefined"), m_sampling_freq(0), m_nyquist_zone(0), m_numsubbands(0), m_rcucontrol(0)
{
	LOG_TRACE_OBJ("SpectralWindow()");
}

SpectralWindow::SpectralWindow(std::string name, double sampling_freq,
								 int nyquist_zone, int numsubbands, uint32 rcucontrol) :
	m_name(name), 
	m_sampling_freq(sampling_freq),
	m_nyquist_zone(nyquist_zone), 
	m_numsubbands(numsubbands), 
	m_rcucontrol(rcucontrol) 
{
	LOG_TRACE_OBJ(formatString("SpectralWindow(%s,%f,%d,%d,%08X)", 
						name.c_str(), sampling_freq, nyquist_zone, numsubbands, rcucontrol));
}
 
SpectralWindow::~SpectralWindow()
{
	LOG_TRACE_OBJ("~SpectralWindow()");
}

double SpectralWindow::getSubbandFreq(int subband) const
{
//	LOG_TRACE_OBJ_STR("SpectralWindow::getSubbandFreq(" << subband << " of " << m_numsubbands << ")");

	ASSERT(m_numsubbands);
	ASSERT(subband >= 0 && subband <= m_numsubbands);

	bool is160 	   = ::fabs(160e6 - m_sampling_freq) < 1e-4;
	float freqOffset = (is160 ? 80.0e6 : 100.0e6) * (m_nyquist_zone - 1);

	return (freqOffset + ((subband % m_numsubbands) * getSubbandWidth()));
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

//
// rcumode(): rcumode
//
int SpectralWindow::rcumode() const
{
	LOG_DEBUG (formatString("rcumode(%06X)", m_rcucontrol));

	switch (m_rcucontrol) {
	case 0x017900:	return (1);
	case 0x057900:	return (2);
	case 0x037A00:	return (3);
	case 0x077A00:	return (4);
	case 0x07A400:	return (5);
	case 0x079400:	return (6);
	case 0x078400:	return (7);
	default: 		return (0);
	}
}

uint32 SpectralWindow::raw_rcumode() const
{
    LOG_DEBUG (formatString("rcumode(%06X)", m_rcucontrol));
    return (m_rcucontrol);
}

//
// print
//
ostream& SpectralWindow::print(ostream& os) const
{
	os << formatString("SpectralWindow(name=%s,f=%f,nyq=%d,#sub=%d,rcuctrl=%08X)", 
						m_name.c_str(), m_sampling_freq, m_nyquist_zone, m_numsubbands, m_rcucontrol);
	return (os);
}


size_t SpectralWindow::getSize() const
{
  return MSH_size(m_name) +
    sizeof(m_sampling_freq) +
    sizeof(m_nyquist_zone) +
    sizeof(m_numsubbands) +
    sizeof(m_rcucontrol);
}

size_t SpectralWindow::pack(char* buffer) const
{
  size_t offset = 0;

  offset = MSH_pack(buffer, offset, m_name);
  memcpy(buffer + offset, &m_sampling_freq, sizeof(m_sampling_freq));
  offset += sizeof(m_sampling_freq);
  memcpy(buffer + offset, &m_nyquist_zone, sizeof(m_nyquist_zone));
  offset += sizeof(m_nyquist_zone);
  memcpy(buffer + offset, &m_numsubbands, sizeof(m_numsubbands));
  offset += sizeof(m_numsubbands);
  memcpy(buffer + offset, &m_rcucontrol, sizeof(m_rcucontrol));
  offset += sizeof(m_rcucontrol);

  return offset;
}

size_t SpectralWindow::unpack(const char* buffer)
{
  size_t offset = 0;

  offset = MSH_unpack(buffer, offset, m_name);
  memcpy(&m_sampling_freq, buffer + offset, sizeof(m_sampling_freq));
  offset += sizeof(m_sampling_freq);
  memcpy(&m_nyquist_zone, buffer + offset, sizeof(m_nyquist_zone));
  offset += sizeof(m_nyquist_zone);
  memcpy(&m_numsubbands, buffer + offset, sizeof(m_numsubbands));
  offset += sizeof(m_numsubbands);
  memcpy(&m_rcucontrol, buffer + offset, sizeof(m_rcucontrol));
  offset += sizeof(m_rcucontrol);

  return offset;
}
