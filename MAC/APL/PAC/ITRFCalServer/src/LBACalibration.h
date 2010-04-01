//#  -*- mode: c++ -*-
//#  LBACalibration.h: class definition for the Beam Server task.
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
//#  $Id: LBACalibration.h 11655 2008-08-27 12:07:14Z donker $

#ifndef LOFAR_CAL_LBACALIBRATION_H
#define LOFAR_CAL_LBACALIBRATION_H

#include <APL/RTCCommon/ResourceCache.h>
#include <APL/ICAL_Protocol/AntennaGains.h>
#include <APL/ICAL_Protocol/SubArray.h>
#include <APL/ICAL_Protocol/CalibrationInterface.h>
#include <AMCBase/ConverterClient.h>
#include <AMCBase/Direction.h>
#include <blitz/array.h>
#include "DipoleModel.h"
#include "Source.h"

// for debugging
#include <fstream>

namespace LOFAR {
  namespace CAL {

class LBACalibration: public CalibrationInterface
{
public:
	LBACalibration();
	~LBACalibration();
	
	// Tell the class where the mwArrays for the ACCs are located.
	void setACCs(RTC::ResourceCache*	theACCs);

	// Recalc the postion of the calibration sources, called once after a new ACC is available.
	void repositionSources(time_t	theTime);

	// preform the calibration
	void calibrateSubArray(const SubArray&	subArray, AntennaGains& gains);

private:
	// Read the calibration sources from the file.
	int _initSources();

	// Initialize the internal (matlab) frequency array.
	void _initFrequencies(const SpectralWindow&	spw,  mwArray**	theFrequencies);

	// Initialize the internal (matlab) antenna postion array.
	void _initLBAantennas(SubArray&  	subArray, mwArray**	theAntennas);

	// Matlab 'main' routine that is called by calibrate.
	static int	 gCalibration(int argc, const char** argv);
	int	 		doCalibration(int argc, const char** argv);

	RTC::ResourceCache*		itsACCs;
	AMC::ConverterClient*	itsAMCclient;	// interface for coordinate conversion (Astronomical Measures Conversion)
	ofstream 				logfile;
	vector<AMC::Direction>	itsSources;		// J2000 definition of the calibraton sources.
	mwArray*				itsSourcePos;	// actual position of the calibration sources above the horizon.
	mwArray*				itsFrequencies;	// frequencies of the 512 subbands.
};

  }; // namespace CAL
}; // namespace LOFAR

#endif /* LOFAR_CAL_LBACALIBRATION_H */

