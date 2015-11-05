//#  -*- mode: c++ -*-
//#  ACC.cc: implementation of the Auto Correlation Cube class
//#
//#  Copyright (C) 2002-2010
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
//#  $Id: ACC.cc 6967 2005-10-31 16:28:09Z wierenga $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include "ACC.h"
#include <fstream>
#include <stdio.h>

static const double	epoch1970 = 719529.0;	// daynumber of 1/1/1070 in Matlab

namespace LOFAR {
  using namespace blitz;
  using namespace RTC;
  namespace ICAL {

ACC::ACC(int nsubbands, int nantennas) :
	itsXACC		 	(0),
	itsYACC		 	(0),
	itsTimestamps	(0),
	itsNeedStart	(false),
	itsIsReady	 	(true),
	itsIsAborted	(false),
	itsAntCount		(nantennas),
	itsSubbandCount	(nsubbands)
{
	// Allocate space for the MatLab arrays.
	const mwSize dim[3] = { nantennas, nantennas, nsubbands };
	itsXACC = new mwArray(3, dim, mxDOUBLE_CLASS, mxCOMPLEX);
	itsYACC = new mwArray(3, dim, mxDOUBLE_CLASS, mxCOMPLEX);
	ASSERTSTR(itsXACC && itsYACC, "Allocation of ACC array's failed.");

	itsTimestamps = new mwArray(1, nsubbands, mxDOUBLE_CLASS, mxREAL);
	ASSERTSTR(itsTimestamps, "Allocation of timestamp array failed.");
}

ACC::~ACC()
{
	if (itsXACC)  		delete itsXACC;
	if (itsYACC)  		delete itsYACC;
	if (itsTimestamps)  delete itsTimestamps;
}

//
// setSelection
//
void ACC::setSelection(const blitz::Array<bool, 2>& antenna_selection)
{
	// antenna_selection: antennas x pol
	// itsACC: subband x pol x pol x antennas x antennas
	ASSERT(antenna_selection.extent(firstDim)  == itsAntCount);
	ASSERT(antenna_selection.extent(secondDim) == 2);
	itsAntennaSelection = antenna_selection;
}

//
// getACM(sb, pol, pol, &timestamp): complex<double>[2]
//
Array<complex<double>, 2> ACC::getACM(int subband, int pol1, int pol2, Timestamp& timestamp)
{
#if 0
	Range all = Range::all();
	timestamp = Timestamp(0,0);

	// check range of subband argument
	ASSERT(subband >= 0 && subband < itsACC.extent(firstDim));
	ASSERT(pol1 == 0 || pol1 == 1);
	ASSERT(pol2 == 0 || pol2 == 1);

	timestamp = itsTimestamps(subband);

	if (itsAntennaCount == itsACC.extent(fourthDim)) {
		// return slice of the full ACC
		return itsACC(subband, pol1, pol2, all, all);
	} else {
		// make selection
		itsCurrentACM.resize(itsAntennaCount, itsAntennaCount);
		int k = 0;
		for (int i = 0; i < itsACC.extent(fourthDim); ++i) {
			if (sum(itsAntennaSelection(i, Range::all())) > 0) {
				int l = 0;
				for (int j = 0; j < itsACC.extent(fifthDim); ++j) {
					if (sum(itsAntennaSelection(j, Range::all())) > 0) {
						itsCurrentACM(k, l) = itsACC(subband, pol1, pol2, i, j);
					}
					l++;
				} // for j
				k++;
			} // if sum
		} // for i
		return (itsCurrentACM);
	}
#else
	return (itsCurrentACM);
#endif
}

//
// updateACM(sb, timestamp, [pol x pol x ant x ant])
//
void ACC::updateACM(int subband, const Timestamp& timestamp, const Array<complex<double>, 4>& newacm)
{
	ASSERT(newacm.extent(firstDim)  == 2);
	ASSERT(newacm.extent(secondDim) == 2);
	ASSERT(newacm.extent(thirdDim)  <= itsAntCount);
	ASSERT(newacm.extent(fourthDim) <= itsAntCount);

	LOG_DEBUG_STR("updateACM(" << subband << "," << timestamp << ")");

	int	nrAnts2Update = newacm.extent(thirdDim);

	// TODO: CHECK SHAPE OF ARRAY
	for (int idx2 = 0; idx2 < 12*nrAnts2Update; idx2++) {
		for (int idx1 = 0; idx1 < 12*nrAnts2Update; idx1++) {
			(*itsXACC)(idx1+1, idx2+1, subband).Real() = newacm(0, 0, idx1%nrAnts2Update, idx2%nrAnts2Update).real();
			(*itsXACC)(idx1+1, idx2+1, subband).Imag() = newacm(0, 0, idx1%nrAnts2Update, idx2%nrAnts2Update).imag();
			(*itsYACC)(idx1+1, idx2+1, subband).Real() = newacm(1, 1, idx1%nrAnts2Update, idx2%nrAnts2Update).real();
			(*itsYACC)(idx1+1, idx2+1, subband).Imag() = newacm(1, 1, idx1%nrAnts2Update, idx2%nrAnts2Update).imag();
		}
	}
	(*itsTimestamps)(subband).Real() = epoch1970 + (double(timestamp) / 86400.0);	// convert to matlab datenum format
}

// print function for operator<<
ostream& ACC::print(ostream& os) const
{
	os << "NeedStart=" << (itsNeedStart?"Yes":"No")  << ", Ready=" << (itsIsReady?"Yes":"No");
	os << ", Aborted=" << (itsIsAborted?"Yes":"No");
	return (os);
}

//
// getFromFile(filename)
//
bool ACC::getFromFile(string filename)
{
#if 0
	LOG_INFO_STR("Attempting to read ACC array with shape='" << itsACC.shape() << "' from '" << filename);

	// try to read in the file
	Array<complex<double>, 5> anACC;
	ifstream accstream(filename.c_str());
	if (accstream.is_open()) {
		accstream >> anACC;
	} else {
		LOG_WARN_STR("Failed to open file: " << filename);
		return (false);
	}

	// check dimensions of the new ACC
	for (int i = 0; i < 5; i++) {
		ASSERT(anACC.extent(i) == itsACC.extent(i));
	}

	// update internal admin
	itsACC.reference(anACC);	// share data
	itsTimestamps.resize(anACC.extent(firstDim));
	itsTimestamps = Timestamp(0,0);

	// select all antennas
	itsAntennaCount = itsACC.extent(fourthDim);
	itsAntennaSelection.resize(itsACC.extent(fourthDim), itsACC.extent(secondDim));
	itsAntennaSelection = true;

	LOG_INFO_STR("Done reading ACC array");
	return (true);
#else
	return (false);
#endif
}

//
// getFromBinairyFile(filename)
//
bool ACC::getFromBinaryFile(string filename)
{
#if 0
	LOG_INFO_STR("Attempting to read binary ACC array with shape='" << itsACC.shape() << "' from '" << filename);

	// try to open the file
	FILE* f = fopen(filename.c_str(), "r");
	if (!f) {
		LOG_WARN_STR("Failed to open file: " << filename);
		return (false);
	}

	// read in the data
	size_t nread = fread(itsACC.data(), sizeof(complex<double>), itsACC.size(), f);
	if (nread != (size_t)itsACC.size()) {
		LOG_WARN_STR("Warning: read " << nread << " items but expected " << itsACC.size());
		fclose(f);
		return (false);
	}

	fclose(f);
	LOG_INFO_STR("Done reading ACC array");
	return (true);
#else
	return (false);
#endif
}

  } // namespace ICAL
} // namespace LOFAR
