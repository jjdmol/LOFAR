//# tHasDataslots.cc
//#
//# Copyright (C) 2012
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: tObservation.cc 20220 2012-02-23 15:12:05Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include <ApplCommon/Observation.h>

using namespace LOFAR;

//
// _isStationName(name)
//
bool _isStationName(const string&	hostname) 
{
cout << hostname << endl;
	// allow AA999, AA999C and AA999T
	if (hostname.length() != 5 && hostname.length() != 6) 
		return (false);

	// We make a rough guess about the vality of the hostname.
	// If we want to check more secure we have to implement all allowed stationnames
	return (isalpha(hostname[0]) && isalpha(hostname[1]) &&
			isdigit(hostname[2]) && isdigit(hostname[3]) && isdigit(hostname[4]));
}

//
// _hasDataSlots
//
bool _hasDataSlots(const ParameterSet*	aPS) 
{
	ParameterSet::const_iterator	iter = aPS->begin();
	ParameterSet::const_iterator	end  = aPS->end();
	while (iter != end) {
		string::size_type	pos(iter->first.find("Dataslots."));
		// if begin found, what is after it?
		if (pos != string::npos && iter->first.find("Dataslots.DataslotInfo.") == string::npos) {	
cout << iter->first << endl;
			return _isStationName((iter->first.substr(pos+10,5)));
		}
		iter++;	// try next line
	}
	
	return (false);
}

int main (int argc, char* argv[]) 
{
	INIT_LOGGER(argv[0]);
	if (argc != 2) {
		cerr << "Syntax: " << argv[0] << " parameterset" << endl;
		return (1);
	}

	try {
		ParameterSet	userPS(argv[1]);
		cout << (_hasDataSlots(&userPS) ? "YES" : "NO") << endl;
	}
	catch (Exception& e) {
		cout << "Exception: " << e.what() << endl;
		return 1;
	}
	return 0;
}
