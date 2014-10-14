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
//#  $Id: tJ2000Converter.cc 14866 2010-01-23 00:07:02Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <APL/RTCCommon/Timestamp.h>
#include <blitz/array.h>
#include <CASATools/CasaConverter.h>

using namespace blitz;
using namespace LOFAR;
using namespace CASATools;
using namespace RTC;

int main(int, char*	argv[]) 
{
	INIT_LOGGER(argv[0]);

	// prepare fake input data
	blitz::Array<double,2>		fieldPos(2,3);
	fieldPos(0,0) = 3827945.959728817; 	fieldPos(0,1) = 459792.591297293; 	fieldPos(0,2) = 5063989.988; 	
	fieldPos(1,0) = 3827931.695686471; 	fieldPos(1,1) = 459769.506299131; 	fieldPos(1,2) = 5064002.779; 	
	blitz::Array<double,2>		antPos(3,3);
	antPos(0,0) = 0.0; 		antPos(0,1) = 0.0; 		antPos(0,2) = 0.0; 	
	antPos(1,0) = 100.0; 	antPos(1,1) = 0.0; 		antPos(1,2) = 0.0; 	
	antPos(2,0) = 0.0;	 	antPos(2,1) = 200.0; 	antPos(2,2) = 0.0; 	
	blitz::Array<double,1>		antLength(3);
	antLength(0)=sqrt(antPos(0,0)*antPos(0,0)+antPos(0,1)*antPos(0,1)+antPos(0,2)*antPos(0,2));
	antLength(1)=sqrt(antPos(1,0)*antPos(1,0)+antPos(1,1)*antPos(1,1)+antPos(1,2)*antPos(1,2));
	antLength(2)=sqrt(antPos(2,0)*antPos(2,0)+antPos(2,1)*antPos(2,1)+antPos(2,2)*antPos(2,2));

	struct tm			jan2007;
	jan2007.tm_sec  = 0;
	jan2007.tm_min  = 0;
	jan2007.tm_hour = 6;
	jan2007.tm_mday = 1;
	jan2007.tm_mon  = 0;
	jan2007.tm_year = 107;
	cout << asctime(&jan2007) << endl;
	Timestamp			theTime(timegm(&jan2007), 0);

	cout << "antLength:" << antLength << endl;

	// the actual code
	CasaConverter		theConverter("J2000");

	blitz::Array<double,2>	result;
	if (!theConverter.doConversion("ITRF", antPos, fieldPos(0, Range::all()), theTime, result)) {
		LOG_FATAL("The conversion failed");
		exit(1);
	}
	// denormalize length of vector
	result = result(tensor::i, tensor::j) * antLength(tensor::i);
	cout << "J2000 to ITRF @ " << theTime << endl;
	cout << antPos << endl;
	cout << result << endl;

	CasaConverter		backConverter("ITRF");
	if (!backConverter.doConversion("J2000", result, fieldPos(0, Range::all()), theTime, antPos)) {
		LOG_FATAL("The conversion failed");
		exit(1);
	}
	antPos = antPos(tensor::i, tensor::j) * antLength(tensor::i);
	cout << "ITRF to J2000 @ " << theTime << endl;
	cout << result << endl;
	cout << antPos << endl;

	bool	moon = theConverter.isValidType("MOON");
	cout << "'MOON' is " << (moon ? "" : "NOT ") << "a supported conversion" << endl;
	bool	cyga  = theConverter.isValidType("CYGA");
	cout << "'CYGA' is " << (cyga ? "" : "NOT ") << "a supported conversion" << endl;
	bool	j2000 = theConverter.isValidType("J2000");
	cout << "'J2000' is " << (j2000 ? "" : "NOT ") << "a supported conversion" << endl;

	vector<string>	allTypes = theConverter.validTypes();
	for (uint i = 0; i < allTypes.size(); i++) {
		cout << allTypes[i] << " ";
	}
	cout << endl;
}

