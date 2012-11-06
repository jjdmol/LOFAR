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
//#  $Id: SpectralWindow.cc 14774 2010-01-11 07:41:24Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

#include <APL/ICAL_Protocol/SpectralWindow.h>

#include <MACIO/Marshalling.tcc>
#include <APL/RTCCommon/MarshallBlitz.h>

namespace LOFAR {
  namespace ICAL {

SpectralWindow::SpectralWindow() :
  itsName("undefined"), itsSamplingFreq(0), itsNyquistZone(0), itsLBAfilterOn(0)
{
	LOG_TRACE_OBJ("SpectralWindow()");
}

SpectralWindow::SpectralWindow(uint rcumode)
{
	switch (rcumode) {
	case 1: *this = SpectralWindow("rcumode1", 200.0e6, 1, false);	// 1
	case 2: *this = SpectralWindow("rcumode2", 200.0e6, 1, true);	// 2
	case 3: *this = SpectralWindow("rcumode3", 200.0e6, 1, false);	// 1
	case 4: *this = SpectralWindow("rcumode4", 200.0e6, 1, true);	// 2
	case 5: *this = SpectralWindow("rcumode5", 200.0e6, 2, false);	// 3
	case 6: *this = SpectralWindow("rcumode6", 160.0e6, 3, false);	// 4
	case 7: *this = SpectralWindow("rcumode7", 200.0e6, 3, false);	// 5
	default:
		ASSERTSTR(rcumode >= 1 && rcumode <= (uint)NR_RCU_MODES, "rcumode must have value: 1.." << NR_RCU_MODES);
	}
}

SpectralWindow::SpectralWindow(const string& name, double sampling_freq,
								 int nyquist_zone, bool LBAfilterOn) :
	itsName			(name), 
	itsSamplingFreq(sampling_freq),
	itsNyquistZone (nyquist_zone), 
	itsLBAfilterOn	(LBAfilterOn) 
{
	LOG_TRACE_OBJ(formatString("SpectralWindow(%s,%f,%d,%s)", 
						name.c_str(), sampling_freq, nyquist_zone, (LBAfilterOn ? "ON" : " OFF")));
}
 
SpectralWindow::~SpectralWindow()
{
	LOG_TRACE_OBJ("~SpectralWindow()");
}

//
// rcumodeHBA()
//
uint SpectralWindow::rcumodeHBA() const
{
	switch (itsNyquistZone) {
	case 1:	return (0);	// LBA
	case 2: return (5);
	case 3: return (itsSamplingFreq == 200.0e6 ? 7 : 6);
	default: ASSERTSTR(false, "rcuMode cannot be determined, illegal nyquistzone.");
	}
}

double SpectralWindow::subbandFreq(int subband) const
{
	ASSERT(subband >= 0 && subband < MAX_SUBBANDS);

	bool	is160 	   = ::fabs(160e6 - itsSamplingFreq) < 1e-4;
	float	freqOffset = (is160 ? 80.0e6 : 100.0e6) * (itsNyquistZone - 1);

	return (freqOffset + (subband * subbandWidth()));
}

// print function for operator<<
ostream& SpectralWindow::print(ostream& os) const
{
	os << "SpectralWindow " << itsName << ":sampleFreq=" << itsSamplingFreq << ", nyquistzone=" << itsNyquistZone;
	os << ", LBAfilter=" << (itsLBAfilterOn ? "ON" : "OFF");
	return (os);
}

//
// ---------- pack and unpack functions ----------
//
size_t SpectralWindow::getSize() const
{
  return MSH_size(itsName) +
    sizeof(itsSamplingFreq) +
    sizeof(itsNyquistZone) +
    sizeof(itsLBAfilterOn);
}

size_t SpectralWindow::pack(char* buffer) const
{
  size_t offset = 0;

  MSH_pack(buffer, offset, itsName);
  memcpy(buffer + offset, &itsSamplingFreq, sizeof(itsSamplingFreq));
  offset += sizeof(itsSamplingFreq);
  memcpy(buffer + offset, &itsNyquistZone, sizeof(itsNyquistZone));
  offset += sizeof(itsNyquistZone);
  memcpy(buffer + offset, &itsLBAfilterOn, sizeof(itsLBAfilterOn));
  offset += sizeof(itsLBAfilterOn);

  return offset;
}

size_t SpectralWindow::unpack(const char* buffer)
{
  size_t offset = 0;

  MSH_unpack(buffer, offset, itsName);
  memcpy(&itsSamplingFreq, buffer + offset, sizeof(itsSamplingFreq));
  offset += sizeof(itsSamplingFreq);
  memcpy(&itsNyquistZone, buffer + offset, sizeof(itsNyquistZone));
  offset += sizeof(itsNyquistZone);
  memcpy(&itsLBAfilterOn, buffer + offset, sizeof(itsLBAfilterOn));
  offset += sizeof(itsLBAfilterOn);

  return offset;
}

  }  // namespace ICAL
}  // namespace LOFAR
