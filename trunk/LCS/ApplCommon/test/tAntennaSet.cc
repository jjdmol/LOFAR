//#  tAntennaSet.cc
//#
//#  Copyright (C) 2008
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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <ApplCommon/AntennaSets.h>

using namespace LOFAR;

int main (int	argc, char* argv[]) 
{
        (void)argc;

	INIT_VAR_LOGGER(argv[0], argv[0]);

	AntennaSets	theAS("tAntennaSet.in_1");	// read the AntennaSets.conf file into memory

	// Show the names of the sets.
	LOG_DEBUG_STR("The AntennaSets.conf file containes the following sets:");
	vector<string>	theNames = theAS.antennaSetList();
	for (uint idx = 0; idx < theNames.size(); idx++) {
		LOG_DEBUG_STR(idx << " : " << theNames[idx] << " : " << (theAS.usesLBAfield(theNames[idx], 0) ? "LBA" : "HBA")
						<< " on field " << theAS.antennaField(theNames[idx],0));
	}

	// test namelookup
	LOG_DEBUG_STR("HBA_UNKNOWN is " << (theAS.isAntennaSet("HBA_UNKNOWN") ? "" : "NOT ") << "a set");
	LOG_DEBUG_STR("LBA_SPARSE  is " << (theAS.isAntennaSet("LBA_SPARSE") ? "" : "NOT ") << "a set");

	// show all configurations
	for (uint idx = 0; idx < theNames.size(); idx++) {
		LOG_DEBUG_STR("********** " << theNames[idx] << "**********");
		LOG_DEBUG_STR("RCUs EUROPE:" << theAS.RCUinputs(theNames[idx], 2));
		LOG_DEBUG_STR("RCUs REMOTE:" << theAS.RCUinputs(theNames[idx], 1));
		LOG_DEBUG_STR("RCUs CORE  :" << theAS.RCUinputs(theNames[idx], 0));
		LOG_DEBUG_STR("AntPos EUROPE:" << theAS.positionIndex(theNames[idx], 2));
		LOG_DEBUG_STR("AntPos REMOTE:" << theAS.positionIndex(theNames[idx], 1));
		LOG_DEBUG_STR("AntPos CORE  :" << theAS.positionIndex(theNames[idx], 0));

		// unfortunately strings are printed starting at element 0 and bitsets viceversa
		// to be able to show it logical to the user we must reverse the bitset
		bitset<MAX_RCUS>	theRealBS  = theAS.LBAallocation(theNames[idx], 2);
		bitset<MAX_RCUS>	thePrintableBS;
		for (int i = 0; i < MAX_RCUS; i++) { thePrintableBS[MAX_RCUS-1-i] = theRealBS[i]; }
		LOG_DEBUG_STR("LBAs EUROPE:" << thePrintableBS);
		theRealBS  = theAS.LBAallocation(theNames[idx], 1);
		for (int i = 0; i < MAX_RCUS; i++) { thePrintableBS[MAX_RCUS-1-i] = theRealBS[i]; }
		LOG_DEBUG_STR("LBAs REMOTE:" << thePrintableBS);
		theRealBS  = theAS.LBAallocation(theNames[idx], 0);
		for (int i = 0; i < MAX_RCUS; i++) { thePrintableBS[MAX_RCUS-1-i] = theRealBS[i]; }
		LOG_DEBUG_STR("LBAs CORE  :" << thePrintableBS);

		theRealBS  = theAS.HBAallocation(theNames[idx], 2);
		for (int i = 0; i < MAX_RCUS; i++) { thePrintableBS[MAX_RCUS-1-i] = theRealBS[i]; }
		LOG_DEBUG_STR("HBAs EUROPE:" << thePrintableBS);
		theRealBS  = theAS.HBAallocation(theNames[idx], 1);
		for (int i = 0; i < MAX_RCUS; i++) { thePrintableBS[MAX_RCUS-1-i] = theRealBS[i]; }
		LOG_DEBUG_STR("HBAs REMOTE:" << thePrintableBS);
		theRealBS  = theAS.HBAallocation(theNames[idx], 0);
		for (int i = 0; i < MAX_RCUS; i++) { thePrintableBS[MAX_RCUS-1-i] = theRealBS[i]; }
		LOG_DEBUG_STR("HBAs CORE  :" << thePrintableBS);
	}

	LOG_DEBUG(" ");
	LOG_DEBUG("----- Finally testing some corrupt AntennaSet files, expecting 5 exceptions... -----");
	uint nrExceptions(0);
	try {
		AntennaSets	theWrongAS("tAntennaSet.in_2");	// wrong line format
		LOG_ERROR("==>  Expected exception: wrong line format");
	}
	catch (Exception&	e) { nrExceptions++; }

	try {
		AntennaSets	theWrongAS("tAntennaSet.in_3");	// 1 station type forgotten
		LOG_ERROR("==>  Expected exception: 1 station type forgotten");
	}
	catch (Exception&	e) { nrExceptions++; }

	try {
		AntennaSets	theWrongAS("tAntennaSet.in_4");	// double definition of stationType
		LOG_ERROR("==>  Expected exception: double definition of stationType");
	}
	catch (Exception&	e) { nrExceptions++; }

	try {
		AntennaSets	theWrongAS("tAntennaSet.in_5");	// double definition of AntennaSet
		LOG_ERROR("==>  Expected exception: double definition of AntennaSet");
	}
	catch (Exception&	e) { nrExceptions++; }

	try {
		AntennaSets	theWrongAS("tAntennaSet.in_6");	// incomplete definition at end of file.
		LOG_ERROR("==>  Expected exception: incomplete definition at end of file");
	}
	catch (Exception&	e) { nrExceptions++; }
        
	return (nrExceptions == 5) ? 0 : 1;
}

