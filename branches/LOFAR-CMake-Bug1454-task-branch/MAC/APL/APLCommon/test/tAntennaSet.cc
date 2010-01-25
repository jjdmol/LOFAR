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
#include <APL/APLCommon/AntennaSets.h>

using namespace LOFAR;
using namespace LOFAR::APLCommon;

int main (int	argc, char* argv[]) 
{
	INIT_VAR_LOGGER(argv[0], argv[0]);

	AntennaSets	theAS("AntennaSets1.conf");	// read the AntennaSets.conf file into memory

	// Show the names of the sets.
	LOG_DEBUG_STR("The AntennaSets.conf file containes the following sets:");
	vector<string>	theNames = theAS.antennaSetList();
	for (uint idx = 0; idx < theNames.size(); idx++) {
		LOG_DEBUG_STR(idx << " : " << theNames[idx]);
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
	try {
		AntennaSets	theWrongAS("AntennaSets2.conf");	// wrong line format
	}
	catch (Exception&	e) {}

	try {
		AntennaSets	theWrongAS("AntennaSets3.conf");	// 1 station type forgotten
	}
	catch (Exception&	e) {}

	try {
		AntennaSets	theWrongAS("AntennaSets4.conf");	// double definition of stationType
	}
	catch (Exception&	e) {}

	try {
		AntennaSets	theWrongAS("AntennaSets5.conf");	// double definition of AntennaSet
	}
	catch (Exception&	e) {}

	try {
		AntennaSets	theWrongAS("AntennaSets6.conf");	// incomplete definition at end of file.
	}
	catch (Exception&	e) {}


	return (0);
}

