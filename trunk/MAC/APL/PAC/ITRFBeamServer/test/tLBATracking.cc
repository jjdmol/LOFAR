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

int main(int    argc, char* argv[])
{
	if (argc != 2) {
		cerr << "Syntax: tLBATracking timestamp(=seconds since epoch 1970)" << endl << endl;;
		exit(0);
	}

    INIT_LOGGER("tLBATracking");

	CasaConverter			itsJ2000Converter("J2000");

	Timestamp	weightTime(atol(argv[1]), 0);
	LOG_INFO_STR("Calculating weights for time " << weightTime);

	// reset all weights
	blitz::Array<complex<double>,2>	itsWeights;
	itsWeights.resize(NR_TEST_RCUS, NR_TEST_SUBBANDS);
	LOG_DEBUG_STR("Weights array has size: " << itsWeights.extent(firstDim) << "x" << itsWeights.extent(secondDim));
	itsWeights(Range::all(), Range::all()) = 0.0;

	string	fieldName("LBA");
	LOG_DEBUG_STR("Checking " << fieldName << " antennas");

	// get ptr to antennafield information
	AntennaField *gAntField = globalAntennaField();

	// Get ITRF position of the RCU's [rcu, xyz]
	blitz::Array<double, 2> rcuPosITRF = gAntField->RCUPos(fieldName);
	if (rcuPosITRF.size() == 0) {
		LOG_DEBUG_STR("No antennas defined in this field");
		return(1);
	}
	LOG_DEBUG_STR("ITRFRCUPos = " << rcuPosITRF);

	// Get geographical location of subarray in ITRF
	blitz::Array<double, 1> fieldCentreITRF = gAntField->Centre(fieldName);
	LOG_DEBUG_STR("ITRF position antennaField: " << fieldCentreITRF);

	// convert ITRF position of all antennas to J2000 for timestamp t
	blitz::Array<double,2>	rcuJ2000Pos; // [rcu, xyz]
	if (!itsJ2000Converter.doConversion("ITRF", rcuPosITRF, fieldCentreITRF, weightTime, rcuJ2000Pos)) {
		LOG_FATAL_STR("Conversion of antennas to J2000 failed");
		return(false);
	}

	// Lengths of the vector of the antennaPosition i.r.t. the fieldCentre,
	blitz::Array<double,1>	rcuPosLengths = gAntField->RCULengths(fieldName);
	LOG_DEBUG_STR("rcuPosLengths = " << rcuPosLengths);

	// denormalize length of vector
	rcuJ2000Pos = rcuJ2000Pos(tensor::i, tensor::j) * rcuPosLengths(tensor::i);
	LOG_DEBUG_STR("J2000RCUPos@fullLength=" << rcuJ2000Pos);

	// Get the right pointing
	blitz::Array<double,2>	sourceJ2000xyz;		// [1, xyz]	   target
	blitz::Array<double,2>	curPoint(1,2);		// [1, angles] source
	curPoint(0,0) = 0.7;
	curPoint(0,1) = 0.5;
	if (!itsJ2000Converter.doConversion("J2000", curPoint, fieldCentreITRF, weightTime, sourceJ2000xyz)) {
		LOG_FATAL_STR("Conversion of source to J2000 failed");
		return(false);
	}
	LOG_DEBUG_STR("sourceJ2000xyz:" << sourceJ2000xyz);

	// calculate scaling
	const double speedOfLight = 299792458.0;
	SpectralWindow	spw("someSPW", 200.0e6, 1, 512, 0x00000000);
	blitz::Array<complex<double>,1>	itsScaling(NR_TEST_SUBBANDS);
	for (int s = 0; s < NR_TEST_SUBBANDS; s++) {
		int		subbandNr = (s+1)*100;
        double  freq  = spw.getSubbandFreq(subbandNr);
        itsScaling(s) = -2.0 * M_PI * freq * complex<double>(0.0,1.0) / speedOfLight;
        LOG_DEBUG_STR("scaling subband[" << subbandNr <<
                          "] = " << itsScaling(s) << ", freq = " << freq);
	}

	// N10ote: RCUallocation is stationbased, rest info is fieldbased, 
	//		 use firstRCU as offsetcorrection
	int	firstRCU(gAntField->ringNr(fieldName) * gAntField->nrAnts(fieldName) * 2);
	LOG_DEBUG_STR("first RCU of field " << fieldName << "=" << firstRCU);
	for (int rcu = 0; rcu < NR_TEST_RCUS; rcu++) {
		//
		// For all beamlets that belong to this beam calculate the weight
		// Note: weight is in-product for RCUpos and source Pos and depends on 
		// the frequency of the subband.
		//
		for (int	beamlet = 0; beamlet < NR_TEST_SUBBANDS; beamlet++) {
			itsWeights(rcu, beamlet) = exp(itsScaling(beamlet) * 
					(rcuJ2000Pos(rcu-firstRCU, 0) * sourceJ2000xyz(0,0) +
					 rcuJ2000Pos(rcu-firstRCU, 1) * sourceJ2000xyz(0,1) +
					 rcuJ2000Pos(rcu-firstRCU, 2) * sourceJ2000xyz(0,2)));

			// some debugging
			if (itsWeights(rcu, beamlet) != complex<double>(1,0)) {
				stringstream	str;
				str.precision(20);
				str << "itsWeights(" << rcu << "," << beamlet << ")=" << itsWeights(rcu, beamlet);
				LOG_DEBUG_STR(str.str());
			}
		} // beamlets
	} // rcus

	// convert the weights from double to int16
	blitz::Array<complex<int16_t>,2>	itsWeights16;
	itsWeights16.resize(itsWeights.shape());
	itsWeights16 = convert2complex_int16_t(itsWeights);

	LOG_DEBUG(formatString("sizeof(itsWeights16) = %d", itsWeights16.size()*sizeof(int16_t)));
	LOG_INFO_STR(itsWeights16);

	return (0);
}

#if 0
//
// send_sbselection()
//
void tBeamServer::send_sbselection()
{
	bool	itsSplitterOn = false;
	for (int ringNr = 0; ringNr <= (itsSplitterOn ? 1 : 0); ringNr++) {
		RSPSetsubbandsEvent ss;
		ss.timestamp.setNow(0);
		ss.rcumask.reset();
		// splitter=OFF: all RCUs ; splitter=ON: 2 runs with half the RCUs
		int	maxRCUs = (itsSplitterOn ? NR_TEST_RCUS / 2 : NR_TEST_RCUS);
		for (int i = 0; i < maxRCUs; i++) {
			ss.rcumask.set((ringNr * maxRCUs) + i);
		}
		LOG_DEBUG_STR("Collecting subbandselection for ring " << ringNr << " (" << maxRCUs << " RCUs)");
		//
		// Always allocate the array as if all beamlets were
		// used. Because of allocation and deallocation of beams
		// there can be holes in the subband selection.
		//
		// E.g. Beamlets 0-63 are used by beam 0, beamlets 64-127 by
		// beam 1, then beam 0 is deallocated, thus there is a hole
		// of 64 beamlets before the beamlets of beam 1.
		//
		ss.subbands.setType(SubbandSelection::BEAMLET);
		ss.subbands().resize(1, NR_TEST_SUBBANDS);
		ss.subbands() = 0;

		// reconstruct the selection
		Beamlet2SubbandMap selection;
		selection[0]=4;
		selection[1]=3;
		selection[2]=2;
		selection[3]=1;
		selection[4]=0;

		LOG_DEBUG(formatString("nrsubbands=%d", selection().size()));
		map<uint16,uint16>::iterator	iter = selection().begin();
		map<uint16,uint16>::iterator	end  = selection().end();
		for ( ; iter != end; ++iter) {
			LOG_DEBUG(formatString("(%d,%d)", iter->first, iter->second));
			// same selection for x and y polarization
			ss.subbands()(0, (int)iter->first) = iter->second;
		}

		if (selection().size()) {
			LOG_DEBUG_STR("Subbandselection for ring segment " << ringNr);
			LOG_DEBUG_STR(ss.subbands());
		} 
		else {
			LOG_DEBUG_STR("No subbandselection for ring segment " << ringNr);
		}
	}
}
#endif

