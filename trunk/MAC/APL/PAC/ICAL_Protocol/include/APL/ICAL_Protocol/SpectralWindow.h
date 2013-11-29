//#  -*- mode: c++ -*-
//#  SpectralWindow.h: class definition for the SpectralWindow class
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
//#  $Id: SpectralWindow.h 13172 2009-04-24 11:21:29Z overeem $

#ifndef SPECTRALWINDOW_H_
#define SPECTRALWINDOW_H_

#include <Common/LofarTypes.h>
#include <Common/LofarConstants.h>

namespace LOFAR {
  namespace ICAL {

// Class which represents a window on the electromagnetic spectrum.
//
// A window is defined by two parameters:
// - sampling frequency
// - nyquist_zone
//
// The band starting at frequency ((nyquist_zone - 1) * (sampling_frequency / 2.0)) and
// ending at (nyquist_zone * (sampling_frequency / 2.0)) is split into MAX_SUBBANDS frequency bins
class SpectralWindow
{
public:
	// Constructors
	SpectralWindow();
	explicit SpectralWindow(uint rcumode);
	SpectralWindow(const string& name, double sampling_freq, int nyquist_zone, bool LBAfilterOn);
	~SpectralWindow();

	// Return the name of the spectral window.
	string name() const { return itsName; }

	// Return the sampling frequency for this window
	double samplingFrequency() const { return itsSamplingFreq; }

	// Return the nyquist zone for this window.
	int nyquistZone() const { return itsNyquistZone; }

	// Return the LBA filter setting
	int LBAfilterOn() const { return itsLBAfilterOn; }

	// Return the rcumode of SPW (only defined for HBA SPW's).
	uint rcumodeHBA() const;

	// Return the width of the subbands.
	double subbandWidth() const { return itsSamplingFreq / (2.0 * MAX_SUBBANDS); }

	// Return centre frequency of a specific subband.
	double subbandFreq(int subband) const;

	/*@{*/
	// marshalling methods
	size_t getSize() const;
	size_t pack  (char* buffer) const;
	size_t unpack(const char *buffer);
	/*@}*/

	// call for operator<<
	ostream& print (ostream& os) const;

private:
	string	itsName;			// name of the spectral window
	double	itsSamplingFreq;	// sampling frequency
	uint16	itsNyquistZone;		// defines the window
	bool	itsLBAfilterOn;		// 10-30Mhz filter switch on/off
};

// operator<<
inline ostream& operator<<(ostream& os, const SpectralWindow& spw)
{
	return (spw.print(os));
}


  }; // namespace ICAL
}; // namespace LOFAR

#endif /* SPECTRALWINDOW_H_ */

