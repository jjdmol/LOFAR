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
#include <APS/ParameterSet.h>
#include <APL/LBA_Calibration/lba_calibration.h>
#include <APL/RTCCommon/Timestamp.h>
#include <AMCBase/Direction.h>
#include <AMCBase/Position.h>
#include <AMCBase/RequestData.h>
#include <AMCBase/ResultData.h>
//#include "Source.h"
//#include "DipoleModel.h"
#include "LBACalibration.h"

#include <blitz/array.h>
#include <time.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;
using namespace RTC;
using namespace AMC;
using namespace LOFAR::ACC::APS;


// fileformat for writing matrix or vector's to a fileformat
// 0 = do nothing
// 1 = ascii
// 2 = binair
int fileFormat = 1;

LBACalibration*		gLBAInstancePtr = 0;

// -------------------- Construction and destruction --------------------
//
// LBACalibration
//
LBACalibration::LBACalibration() :
	itsACCs(0),
    logfile("CalLog.txt")
{
	mclmcrInitialize();

	ASSERTSTR(mclInitializeApplication(NULL, 0), "Failed to initialize the Matlab Core Engine");

  	ASSERTSTR(liblba_calibrationInitialize(), " Failed to initialize the LBA Calibration module");

	ASSERTSTR(_initSources() > 2, "Failed to read in more than two calibration sources");

	gLBAInstancePtr = this;

	itsAMCclient  = new AMC::ConverterClient("localhost");
	ASSERTSTR(itsAMCclient, "Failed to allocate an AMC client");

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

//
// setACCs(ResourceCache)
//
void LBACalibration::setACCs(ResourceCache*	theACCs)
{
	itsACCs = theACCs;
}

//
// _initSources()
//
int LBACalibration::_initSources()
{
	// read in the file with the calibration sources and fill itsSources.
	// WE ASSUME THE COORDINATES ARE IN J2000
	// file format: name=[longitude,latitude]
	ConfigLocator	cl;
	string	fileName = cl.locate(globalParameterSet()->getString("CalServer.CalibrationSources"));
	LOG_INFO_STR("Reading calibration sources from file " << fileName);
	ParameterSet	sourcesPS(fileName);

	// loop over all sources and store the info in itsSources (vector<Direction>).
	ParameterSet::iterator	iter = sourcesPS.begin();
	ParameterSet::iterator	end  = sourcesPS.end();
	while (iter != end) {
		vector<double>	raDec = sourcesPS.getDoubleVector(iter->first);
		ASSERTSTR(raDec.size() == 2, "Illegal coordinates specified for source " << iter->first);
		itsSources.push_back(Direction(raDec[0], raDec[1], Direction::J2000));
		LOG_INFO_STR("Loaded calibration source " << iter->first << ": " << raDec[0] << ", " << raDec[1]);
		iter++;
	}

	return (itsSources.size());
}

// -------------------- Preparation of the calibration --------------------

//
// repositionSources(time_t		theTime)
//
// Recalc the position of our calibration sources and stored them in the (empty) 
// newSourcePos array that is passed.
void LBACalibration::repositionSources(time_t	atTime)
{
	// each second one subband was calculated, so the acc we pass to the calibration
	// algorithm has different times for each subband. To calculate the position of the
	// calibration-sources we use the average time of the acc.
	Timestamp	centreTime = Timestamp(atTime, 0);
	double mjd(0.0);
	double mjdFraction(0.0);
	centreTime.convertToMJD(mjd, mjdFraction);

	// prepare request to AMC server
	// itsSources   : vector<Direction> J2000
	vector<Position>	curPositions;			// for the answers
	curPositions.resize(itsSources.size());
	RequestData		request(itsSources, curPositions, Epoch(mjd, mjdFraction));
	ResultData		resultData;

	// calc the actual direction of the calibration sources
	itsAMCclient->j2000ToItrf(resultData, request);

	// Make matlab array with the current position of the calibration sources.
	// skip sources that are below the horizon
	// Since matlab arrays can not be extended we must first count the sources above
	// the horizon and store the info in a tmp buffer, then create a matlab array and 
	// copy the tempbuffer to the matlab array.  :-(
	vector<Direction>::iterator		iter = itsSources.begin();
	vector<Direction>::iterator		end  = itsSources.end();
	double*	tmpBuffer = new double [itsSources.size() * 3];
	int		nrUsableSources = 0;
	while (iter != end) {
		if (iter->latitude() > 0.0) {
			// TODO: CONVERT TO PQR!!!
			tmpBuffer[nrUsableSources * 3 + 0] = iter->longitude();
			tmpBuffer[nrUsableSources * 3 + 1] = iter->latitude();
			tmpBuffer[nrUsableSources * 3 + 2] = 0;

			LOG_DEBUG(formatString("CalSource %d at %f, %f, %f", nrUsableSources, 
									tmpBuffer[nrUsableSources * 3 + 0], 
									tmpBuffer[nrUsableSources * 3 + 1], 
									tmpBuffer[nrUsableSources * 3 + 2]));
			nrUsableSources++;
		}
		iter++;
	}
	ASSERTSTR(nrUsableSources > 1, "Need at least 2 sources for the calibration, have " << nrUsableSources);

	// buffers is prepared, count is known, time to move it to matlab
	// itsSourcePos: *mwArray
	if (itsSourcePos) {			// old stuff?
		free(itsSourcePos);		// destroy array
	}
	// Create new array with the right size and contents
	itsSourcePos = new mwArray(nrUsableSources, 3, mxDOUBLE_CLASS);
	itsSourcePos->SetData(tmpBuffer, nrUsableSources * 3);
	free(tmpBuffer);
}

// -------------------- beam related preparation of the calibration --------------------
//
// initFrequencies(freqInMhz, nyquist)
//
// NOTE: THIS ROUTINE MUST BE CHANGED TO WORK ON BEAMS.
//
void LBACalibration::_initFrequencies(const SpectralWindow&	spw, mwArray**	theFrequencies)
{
	if (*theFrequencies) {
		free(*theFrequencies);
		*theFrequencies = 0;
	}

	*itsFrequencies = new mwArray (MAX_SUBBANDS, mxDOUBLE_CLASS);
	ASSERTSTR(*itsFrequencies, "Unable to allocate space for frequency array");

	// note: mwArray* itsFrequencies [ 512 freqs ]
	// TODO: Only calibrate the subbands that the beam uses !!!!!!!
	for (int band = 0; band < MAX_SUBBANDS; band++) {
		(*itsFrequencies)(band) = spw.getSubbandFreq(band);
	}
}

//
// initLBAantennas(AntennaArray&	LBAants)
//
void LBACalibration::_initLBAantennas(SubArray&	subArray, mwArray**	theAntennas)
{
	if (*theAntennas) {
		free (*theAntennas);
		*theAntennas = 0;
	}

	// note: itsAntennaPos [ Antennas * (Lat, Lon, Hght) ]
	int	nrAnts = subArray.getNumAntennas();
	*theAntennas = new mwArray(nrAnts, 3, mxDOUBLE_CLASS);
	ASSERTSTR(theAntennas, "Unable to allocate space for antenna array");

	blitz::Array<double, 3>theAntPos = subArray.getAntennaPos(); // [antennas, pol, (x,y,z)]
	for (int ant = 0; ant < nrAnts; ant++) {
		// TODO: CONVERT THE ITRF COORD TO PQR COORD
		(**theAntennas)(ant, 0) = theAntPos(ant,0,0);
		(**theAntennas)(ant, 1) = theAntPos(ant,0,1);
		(**theAntennas)(ant, 2) = theAntPos(ant,0,2);
	}
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
	mwArray*	theFrequencies  = 0L;
	mwArray*	theAntennaPos   = 0L;

	SubArray*	subArray  = (SubArray*) argv[0];
	mwArray*	matlabACC = (mwArray*) itsACCs->getFront();
	
	_initFrequencies(subArray->getSPW(), &theFrequencies);	// setup frequencies
	_initLBAantennas(*subArray, &theAntennaPos);			// setup the antennes

	// TODO:
	// I guess not the whole matlabACC should be delivered to the MATLAB function when subarraying
	// is used. We should then only pass that part of the ACC that contains the used RCUs.
	lba_calibration(2, calResult, mFlags, *matlabACC, *theAntennaPos, *itsSourcePos, *theFrequencies);

	free(theAntennaPos);
	free(theFrequencies);

	// TODO: ... do something useful with the results ...

	return (1);
}

//
// calibrate(...)
//
void LBACalibration::calibrateSubArray(const SubArray& subarray, AntennaGains& gains)
{
	const char*	theArgs[3];
	theArgs[0] = (char*) &subarray;
	theArgs[1] = (char*) &gains;
	theArgs[2] = 0L;

	mclRunMain((mclMainFcnType)gCalibration, 2, &theArgs[0]);	// note: doCalibration may not be a class-method!!

}
