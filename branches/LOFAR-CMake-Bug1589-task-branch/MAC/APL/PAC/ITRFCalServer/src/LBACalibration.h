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

#include <blitz/array.h>
#include <APL/APLCommon/AntennaField.h>
#include <APL/ICAL_Protocol/AntennaGains.h>
#include <APL/ICAL_Protocol/SubArray.h>
#include "ACCcache.h"
//#include "CasaConverter.h"

namespace LOFAR {
  using APLCommon::AntennaField;
  namespace ICAL {

class LBACalibration
{
public:
	LBACalibration();
	~LBACalibration();
	
	void setACCs(ACCcache&	theACCs) {
		itsACCs = &theACCs;
	}

	// Calibrates the values collected in the ACCcache.
	AntennaGains& run(uint rcumode, const string& antennaField, RCUmask_t rcuMask);

private:
	// Initialize the internal (matlab) frequency array.
	void _initFrequencies(uint	rcumode);

	// Initialize the internal (matlab) antenna postion array.
	void _initAntennaPositions(const string& antennaField);

	// Matlab 'main' routine that is called by calibrate.
	static int	 gCalibration(int argc, const char** argv);
	int	 		doCalibration(int argc, const char** argv);

	ACCcache*				itsACCs;
//	CasaConverter*			itsCasaConverter;
//	ofstream 				logfile;
	mwArray*				itsFrequencies;		// frequencies of the 512 subbands.
	mwArray*				itsXpos;			// x-polarized positions
	mwArray*				itsYpos;			// y-polarized positions

	AntennaField*			itsAntField;
	AntennaGains			itsAntGains;
	uint					itsPrevRCUmode;
	string					itsPrevAntennaField;
};

  }; // namespace ICAL
}; // namespace LOFAR

#endif /* LOFAR_CAL_LBACALIBRATION_H */

