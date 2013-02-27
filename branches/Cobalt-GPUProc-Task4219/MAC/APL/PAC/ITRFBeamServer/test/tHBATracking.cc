//#
//#  tBeamServer.cc: implementation of tBeamServer class
//#
//#  Copyright (C) 2002-2009
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
//#  $Id: tBeamServer.cc 16173 2010-08-16 15:01:45Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/lofar_complex.h>
#include <Common/StringUtil.h>
#include <ApplCommon/AntennaSets.h>
#include <APL/APLCommon/AntennaField.h>
#include <APL/CAL_Protocol/SpectralWindow.h>

#include <blitz/array.h>
#include <CASATools/CasaConverter.h>

using namespace casa;
using namespace blitz;
using namespace LOFAR;
using namespace RTC;
using namespace CASATools;
using namespace CAL;

int	gBeamformerGain = 8000;
blitz::Array<double, 2> itsTileRelPos;	// [N_HBA_ELEMENTS,x|y|z dipole] = [16,3]
blitz::Array<double, 1>	itsDelaySteps; 	// [32]
blitz::Array<uint16, 2>	itsHBAdelays;   // [rcus, N_HBA_ELEM_PER_TILE]
double					itsMeanElementDelay;

BZ_DECLARE_FUNCTION_RET(convert2complex_int16_t, complex<int16_t>)

//
// Convert the weights to 16-bits signed integer.
//
inline complex<int16_t> convert2complex_int16_t(complex<double> cd)
{
	return complex<int16_t>((int16_t)(round(cd.real() * gBeamformerGain)),
							(int16_t)(round(cd.imag() * gBeamformerGain)));
}

#define	NR_TEST_RCUS		8
#define NR_TEST_SUBBANDS	5

// --------------- READ CONFIGURATIONFILES ---------------

//
// getAllHBADeltas 
//
void getAllHBADeltas(const string& filename)
{
	ifstream	itsFile;
	string 		itsName;

	LOG_DEBUG_STR("Trying to read the relative HBA element positions from file " << filename);
	// open new file
	if (itsFile.is_open()) {
		itsFile.close();
	}
	itsFile.open(filename.c_str());

	ASSERTSTR(itsFile.good(), "Can not open file with relative HBA positions: " << filename);

	// The file may have comment lines at the top, starting with '#'
	// These must be skipped
	getline(itsFile, itsName); // read name or comment
	while (itsName[0] == '#') {
		LOG_DEBUG_STR("HBADeltas comment = " << itsName);
		getline(itsFile, itsName); // read name or comment
	}
	
	if ("" == itsName) {
		itsFile.close();
		return;
	}
	
	LOG_DEBUG_STR("HBADeltas name = " << itsName);
	
	// Now comment lines are skipped, so we can read the full array.
	itsFile >> itsTileRelPos; // read HBA deltas array
	LOG_DEBUG_STR("HBADeltas = " << itsTileRelPos);

	LOG_INFO_STR("Relative HBA element positions read from file " << filename);
}

//
// getAllHBAElementDelays 
//
void getAllHBAElementDelays(const string& filename)
{
	ifstream	itsFile;
	string		itsName;

	LOG_DEBUG_STR("Trying to read the HBA element delay steps from file " << filename);

	// open new file
	if (itsFile.is_open()) { 
		itsFile.close();
	}
	itsFile.open(filename.c_str());

	ASSERTSTR(itsFile.good(), "Can not open file with HBA element delay steps: " << filename);

	// The file may have comment lines at the top, starting with '#'
	// These must be skipped
	getline(itsFile, itsName); // read name or comment
	while (itsName[0] == '#') {
		LOG_DEBUG_STR("HBA ElementDelays comment = " << itsName);
		getline(itsFile, itsName); // read name or comment
	}
	
	if ("" == itsName) {
		itsFile.close();
		return;
	}

	LOG_DEBUG_STR("HBA ElementDelays Name = " << itsName);
	
	// Now comment lines are skipped, so we can read the full array.
	itsFile >> itsDelaySteps; // read HBA element delays array
	//itsDelaySteps *= 1E-9; // convert from nSec to Secs
	LOG_DEBUG_STR("HBA ElementDelays = " << itsDelaySteps);

	LOG_INFO_STR("HBA element delay steps read from file " << filename);
	itsMeanElementDelay = blitz::mean(itsDelaySteps);
	LOG_INFO_STR("mean ElementDelay = " << itsMeanElementDelay);
}

//
// calcDelayStep
//
int calcDelayStep(double	delay)
{
#define MIN2(a,b) ((a)<(b)) ? (a) : (b)
#define MAX2(a,b) ((a)>(b)) ? (a) : (b)
	// calculate approximate DelayStepNr
	int delayStepNr = static_cast<int>(delay / 0.5E-9);
	int	maxStepNr = itsDelaySteps.size()-1;

	// range check for delayStepNr, max. 32 steps (0..31) 
	delayStepNr = MIN2(delayStepNr, maxStepNr);	// limit against upper boundary
	delayStepNr = MAX2(0, delayStepNr);			// limit against lower boundary
	
	// look for nearest matching delay step in range "delayStepNr - 2 .. delayStepNr + 2"
	double minDelayDiff = fabs(delay - itsDelaySteps(delayStepNr));
	double difference;
	int minStepNr = delayStepNr;
	int	stepMinusTwo = MAX2(0, delayStepNr-2);			// limit check to element 0
	int	stepPlusTwo  = MIN2(maxStepNr, delayStepNr+2);	// limit check to element 31
	for (int i = stepMinusTwo; i <= stepPlusTwo; i++){
		if (i == delayStepNr) 
			continue; // skip approximate nr
		difference = fabs(delay - itsDelaySteps(i));
		if (difference < minDelayDiff)	{
			minStepNr = i;
			minDelayDiff = difference;
		}
	}
	delayStepNr = minStepNr;
	return (delayStepNr);
}

int main(int    argc, char* argv[])
{
	if (argc != 2) {
		cerr << "Syntax: tHBATracking timestamp(=seconds since epoch 1970)" << endl << endl;;
		exit(0);
	}

    INIT_LOGGER("tHBATracking");

	// load the HBA Delays file
	getAllHBAElementDelays("HBADelays.conf");
	LOG_DEBUG("Loaded HBADelays file");

	LOG_DEBUG("Checking assignment of delaystepNumbers");
	for (int t = 0; t < 80; t++) {
		int	stepNr = calcDelayStep(0.21e-9 * t);
		LOG_DEBUG(formatString("t=%5.3e : step = %d", 0.21e-9*t, stepNr));
	}

	getAllHBADeltas("iHBADeltas.conf");
	LOG_DEBUG("Loaded HBADeltas file");

	// Calc length of vectors of rel. positions of the tile elements
	blitz::Array<double, 1>		tileRelLength(itsTileRelPos.extent(firstDim));
	for (int i = 0; i < itsTileRelPos.extent(firstDim); i++) {
		tileRelLength(i) = sqrt((itsTileRelPos(i,0) * itsTileRelPos(i,0)) +
								 (itsTileRelPos(i,1) * itsTileRelPos(i,1)) +
								 (itsTileRelPos(i,2) * itsTileRelPos(i,2)));
	}
	LOG_DEBUG_STR("tileRelLength:" << tileRelLength);

	Timestamp	weightTime(atol(argv[1]), 0);
	LOG_INFO_STR("Calculating weights for time " << weightTime);

	// reset all weights
	blitz::Array<complex<double>,2>	itsWeights;
	itsWeights.resize(NR_TEST_RCUS, NR_TEST_SUBBANDS);
	LOG_DEBUG_STR("Weights array has size: " << itsWeights.extent(firstDim) << "x" << itsWeights.extent(secondDim));
	itsWeights(Range::all(), Range::all()) = 0.0;

	string	fieldName("HBA");
	LOG_DEBUG_STR("Checking " << fieldName << " antennas");

	// get ptr to antennafield information
	AntennaField *gAntField = globalAntennaField();

	// Get geographical location of subarray in ITRF
	blitz::Array<double, 1> fieldCentreITRF = gAntField->Centre(fieldName);
	LOG_DEBUG_STR("ITRF position antennaField: " << fieldCentreITRF);

	CasaConverter			itsJ2000Converter("J2000");

	// Get the right pointing
	blitz::Array<double,2>	sourceJ2000xyz;		// [1, xyz]	   target
	blitz::Array<double,2>	curPoint(1,2);		// [1, angles] source
	curPoint(0,0) = 0.7;
	curPoint(0,1) = 0.5;
	if (!itsJ2000Converter.doConversion("J2000", curPoint, fieldCentreITRF, weightTime, sourceJ2000xyz)) {
		LOG_FATAL_STR("Conversion of source to J2000 failed");
		return(1);
	}
	LOG_DEBUG_STR("sourceJ2000xyz:" << sourceJ2000xyz);

	// Convert tile deltas to J2000
	blitz::Array<double, 2> tileRelPosJ2000;
	if (!itsJ2000Converter.doConversion("ITRF", itsTileRelPos, fieldCentreITRF, weightTime, tileRelPosJ2000)) {
		LOG_FATAL("Conversion of deltas to J2000 failed");
		return(1);
	}
	// denormalize length of vectors
	tileRelPosJ2000 = tileRelPosJ2000(tensor::i, tensor::j) * tileRelLength(tensor::i);
	LOG_DEBUG_STR("tileRelPosJ2000:" << tileRelPosJ2000);

	// calculate scaling
	const double speedOfLight = 299792458.0;
	itsHBAdelays.resize(NR_TEST_RCUS, N_HBA_ELEM_PER_TILE);
	for (int rcu = 0; rcu < NR_TEST_RCUS; rcu++) {
		for (int element = 0; element < N_HBA_ELEM_PER_TILE; ++ element) {
			// calculate tile delay.
			double	delay = ( (sourceJ2000xyz(0,0) * tileRelPosJ2000(element,0)) +
							  (sourceJ2000xyz(0,1) * tileRelPosJ2000(element,1)) +
							  (sourceJ2000xyz(0,2) * tileRelPosJ2000(element,2)) ) / speedOfLight;
			
			// signal front stays in the middle of the tile
			delay += itsMeanElementDelay;

			LOG_DEBUG_STR("antenna="<<rcu/2 <<", pol="<<rcu%2 <<", element="<<element  <<", delay("<<rcu<<","<<element<<")="<<delay);
			// calculate approximate DelayStepNr
			int delayStepNr = calcDelayStep(delay);
			
			// bit1=0.25nS(not used), bit2=0.5nS, bit3=1nS, bit4=2nS, bit5=4nS, bit6=8nS 	
			itsHBAdelays(rcu,element) = (delayStepNr * 4) + (1 << 7); // assign
		} // for element
	} // rcus
	LOG_DEBUG_STR("itsHBAdelays : " << itsHBAdelays);

	return (0);
}

