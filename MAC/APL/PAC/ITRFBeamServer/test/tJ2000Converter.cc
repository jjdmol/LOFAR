//#  Class.cc: one_line_description
//#
//#  Copyright (C) 2010
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
#include <APL/RTCCommon/Timestamp.h>

#include <blitz/array.h>
#include <ITRFBeamServer/J2000Converter.h>

using namespace casa;
using namespace blitz;
using namespace LOFAR;
using namespace RTC;
using namespace BS;

int main(int	argc, char*	argv[]) 
{
	INIT_LOGGER("tJ2000Converter");

	// prepare fake input data
	blitz::Array<double,2>		fieldPos(2,3);
	fieldPos(0,0) = 3827945.959728817; 	fieldPos(0,1) = 459792.591297293; 	fieldPos(0,2) = 5063989.988; 	
	fieldPos(1,0) = 3827931.695686471; 	fieldPos(1,1) = 459769.506299131; 	fieldPos(1,2) = 5064002.779; 	
	blitz::Array<double,2>		antPos(3,3);
	antPos(0,0) = 0.0; 		antPos(0,1) = 0.0; 		antPos(0,2) = 0.0; 	
	antPos(1,0) = 100.0; 	antPos(1,1) = 0.0; 		antPos(1,2) = 0.0; 	
	antPos(2,0) = 0.0;	 	antPos(2,1) = 100.0; 	antPos(2,2) = 0.0; 	

	struct tm			jan2007;
	jan2007.tm_sec  = 0;
	jan2007.tm_min  = 0;
	jan2007.tm_hour = 6;
	jan2007.tm_mday = 1;
	jan2007.tm_mon  = 0;
	jan2007.tm_year = 107;
	cout << asctime(&jan2007) << endl;
	Timestamp			theTime(timegm(&jan2007), 0);

	// the actual code
	J2000Converter		theConverter;

	blitz::Array<double,2>	result;
	if (!theConverter.doConversion("ITRF", antPos, fieldPos(0, Range::all()), theTime, result)) {
		LOG_FATAL("The conversion failed");
		exit(1);
	}
	cout << theTime << endl;
	cout << result;

	bool	good = theConverter.isValidType("MOON");
	cout << "'MOON' is " << (good ? "" : "NOT ") << "a supported conversion" << endl;
	bool	bad  = theConverter.isValidType("CYGA");
	cout << "'CYGA' is " << (bad ? "" : "NOT ") << "a supported conversion" << endl;

	vector<string>	allTypes = theConverter.validTypes();
	for (int i = 0; i < allTypes.size(); i++) {
		cout << allTypes[i] << " ";
	}
	cout << endl;
}

