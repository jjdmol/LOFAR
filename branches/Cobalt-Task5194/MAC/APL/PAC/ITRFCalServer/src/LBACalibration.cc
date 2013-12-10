//#  -*- mode: c++ -*-
//#  LBACalibration.cc: class implementation of LBACalibration
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
//#  $Id: LBACalibration.cc 11655 2008-08-27 12:07:14Z donker $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/LofarConstants.h>
#include <Common/ParameterSet.h>
#include <APL/LBA_Calibration/lba_calibration.h>
#include <APL/RTCCommon/Timestamp.h>
#include "LBACalibration.h"

#include <blitz/array.h>
#include <time.h>

using namespace std;
using namespace blitz;
namespace LOFAR {
  using namespace RTC;
  using namespace APLCommon;
  namespace ICAL {


// fileformat for writing matrix or vector's to a fileformat
// 0 = do nothing
// 1 = ascii
// 2 = binair
int fileFormat = 1;

LBACalibration*		gLBAInstancePtr = 0;

const double	RELATIVE_BASELINE_RESTRICTION =	4.0;		// in wavelengths
const double	ABSOLUTE_BASELINE_RESTRICTION =	20.0;		// in meters

// -------------------- Construction and destruction --------------------
//
// LBACalibration
//
LBACalibration::LBACalibration() :
	itsACCs			(0),
//	itsCasaConverter(0),
	itsFrequencies	(0),
	itsXpos			(0),
	itsYpos			(0),
	itsAntField		(globalAntennaField()),
	itsPrevRCUmode	(0)
{
	mclmcrInitialize();

	ASSERTSTR(mclInitializeApplication(NULL, 0), "Failed to initialize the Matlab Core Engine");

  	ASSERTSTR(liblba_calibrationInitialize(), " Failed to initialize the LBA Calibration module");

	gLBAInstancePtr = this;

	itsFrequencies = new mwArray (MAX_SUBBANDS, 1, mxDOUBLE_CLASS);
	ASSERTSTR(itsFrequencies->NumberOfElements() == (uint)MAX_SUBBANDS, 
				"Unable to allocate space for frequency array, numberOfElements=" << itsFrequencies->NumberOfElements());

	LOG_INFO("Matlab code initialized successful");
}

//
// ~LBACalibration
//
LBACalibration::~LBACalibration()
{
	liblba_calibrationTerminate();
	mclTerminateApplication();
}

// -------------------- beam related preparation of the calibration --------------------
//
// initFrequencies(freqInMhz, nyquist)
//
// NOTE: THIS ROUTINE MUST BE CHANGED TO WORK ON BEAMS.
//
void LBACalibration::_initFrequencies(uint	rcumode)
{
	if (rcumode == itsPrevRCUmode) {
		return;
	}

	// note: mwArray* itsFrequencies [ 512 freqs ]
	SpectralWindow	spw(rcumode);
	for (int band = 0; band < MAX_SUBBANDS; band++) {
		(*itsFrequencies)(band+1) = spw.subbandFreq(band);
	}
	itsPrevRCUmode = rcumode;
}

//
// _initAntennaPositions(antennaField)
//
void LBACalibration::_initAntennaPositions(const string& antennaField)
{
	// setup antenna positions
	blitz::Array<double,2> rcuPosITRF      = itsAntField->RCUPos(antennaField);	// [rcu, xyz]
	blitz::Array<double,1> fieldCentreITRF = itsAntField->Centre(antennaField);	// [rcu, xyz]
	int	nrAnts = rcuPosITRF.extent(firstDim) / 2;
	int nrAxis = rcuPosITRF.extent(secondDim);
	if (antennaField != itsPrevAntennaField) {
		if (itsXpos) { free(itsXpos); itsXpos = 0; }
		if (itsYpos) { free(itsYpos); itsYpos = 0; }
		itsXpos = new mwArray(nrAnts, nrAxis, mxDOUBLE_CLASS);
		ASSERTSTR(itsXpos->NumberOfElements() == (uint)(nrAnts*nrAxis), "Expected Xarray with size " << nrAnts << "x" << nrAxis);
		itsYpos = new mwArray(nrAnts, nrAxis, mxDOUBLE_CLASS);
		ASSERTSTR(itsYpos->NumberOfElements() == (uint)(nrAnts*nrAxis), "Expected Yarray with size " << nrAnts << "x" << nrAxis);
	}

//	... casaConvert(...) ...

	for (int ant = 0; ant < nrAnts; ant++) {
		for (int axis = 0; axis < nrAxis; axis++) {
			(*itsXpos)(ant+1, axis+1) = rcuPosITRF(ant>>1,  axis);		// TODO NOT ITRF?
			(*itsYpos)(ant+1, axis+1) = rcuPosITRF(ant>>1 +1, axis);	// TODO NOT ITRF?
		}
	}
	itsPrevAntennaField = antennaField;
}

// -------------------- The Calibration ifself --------------------

int LBACalibration::gCalibration(int argc, const char** argv)
{
	return (gLBAInstancePtr->doCalibration(argc, argv));
}


//
// doCalibration(argc, argv)
//
int LBACalibration::doCalibration(int /*argc*/, const char**  argv)
{
	mwArray		calResult;
	mwArray		mFlags;

	// setup frequencies array
	uint		rcumode	= *((uint*)(argv[0]));
	_initFrequencies(rcumode);

	// setup antennapositions
	const char*		antennaField = argv[1];
	_initAntennaPositions(antennaField);

// TODO
//	??? lon = fieldCentre(0); ??? or cartesian(centre) ???
//	??? lat = fieldCentre(0); ???
	
	mwArray*	accData  = itsACCs->getFront().xdata();
	mwArray*	accTimes = itsACCs->getFront().timestamps();
	
	// TODO:
	// I guess not the whole matlabACC should be delivered to the MATLAB function when subarraying
	// is used. We should then only pass that part of the ACC that contains the used RCUs.
//	lba_calibration(2, calResult, mFlags, *matlabACC, *theAntennaPos, *itsSourcePos, *theFrequencies);

	// TODO: ... do something useful with the results ... ==> fill itsAntGains.

	return (1);
}

//
// calibrate(...)
//
AntennaGains& LBACalibration::run(uint	rcumode, const string&	antennaField, RCUmask_t rcuMask)
{
	ASSERTSTR(itsACCs, "The ACCs are not installed yet, please call 'setACCs' first");

	const char*	theArgs[3];
	theArgs[0] = (char*) &rcumode;
	theArgs[1] = antennaField.c_str();
	theArgs[2] = 0L;

	mclRunMain((mclMainFcnType)gCalibration, 2, &theArgs[0]);	// note: doCalibration may not be a class-method!!

	return (itsAntGains);

}

  } // namespace ICAL
} // namespace LOFAR
