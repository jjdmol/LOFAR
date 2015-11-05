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
#include <Common/lofar_vector.h>

//# Includes
#include <Common/LofarLogger.h>
#include <APL/RTCCommon/Timestamp.h>

#include <casa/Exceptions/Error.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Arrays/Vector.h>
#include <measures/Measures.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>

#include <AMCBase/Converter.h>
#include <AMCBase/ConverterClient.h>
#include <AMCBase/RequestData.h>
#include <AMCBase/ResultData.h>
#include <AMCBase/Position.h>
#include <AMCBase/Direction.h>
#include <AMCBase/Epoch.h>

#include <blitz/array.h>

using namespace casa;
using namespace blitz;
using namespace LOFAR;
using namespace AMC;
using namespace RTC;

int main(int	argc, char*	argv[]) 
{
	// construct a frame with an epoch (with a trivial time).
	MeasFrame	frame;
	frame.set(MEpoch(Quantity(55000.0, "d"), MEpoch::UTC));	// somewhere in 2009
	frame.set(MPosition(MVPosition(0.0, 0.0, 0.0), MPosition::ITRF));
	// construct a converter (this takes some time).
	MDirection::Convert		I2Jconverter(MDirection::Ref(MDirection::ITRF), MDirection::Ref(MDirection::J2000, frame));

	// prepare fake input data
	blitz::Array<double,2>		fieldPos(2,3);
	fieldPos(0,0) = 3827945.959728817; 	fieldPos(0,1) = 459792.591297293; 	fieldPos(0,2) = 5063989.988; 	
	fieldPos(1,0) = 3827931.695686471; 	fieldPos(1,1) = 459769.506299131; 	fieldPos(1,2) = 5064002.779; 	
	blitz::Array<double,2>		antPos(3,3);
	antPos(0,0) = 0.0; 		antPos(0,1) = 0.0; 		antPos(0,2) = 0.0; 	
	antPos(1,0) = 100.0; 	antPos(1,1) = 0.0; 		antPos(1,2) = 0.0; 	
	antPos(2,0) = 0.0;	 	antPos(2,1) = 100.0; 	antPos(2,2) = 0.0; 	

	cout.precision(9);

	// do the calculations for field 1
	frame.resetPosition(MPosition(MVPosition(fieldPos(0,0), fieldPos(0,1), fieldPos(0,2)), MPosition::ITRF));
	frame.resetEpoch(MVTime(2010,2,2,0.25));		// set some other time
	cout << "02022010 " << MVTime(2010,2,2,0.25) << endl;
	for (int i = 0; i < antPos.extent(firstDim); ++i) {
		MVDirection	ITRFPos(antPos(i,0), antPos(i,1), antPos(i,2));
		cout << i << ": Direction: " << ITRFPos << endl;
		cout << i << ": Position : " << MPosition(MVPosition(fieldPos(0,0), fieldPos(0,1), fieldPos(0,2)), MPosition::ITRF) << endl;
		cout << i << ": Epoch    : " << MVTime(2010,2,2,0.25) << endl;
		MDirection	J2000Dir = I2Jconverter(ITRFPos);
		cout << i << ": Result   : " << J2000Dir << endl;
		casa::Vector<Double> angles = J2000Dir.getValue().get();
		cout << i << " converted : " << Direction(angles(0),  angles(1), Direction::ITRF) << endl;
	}

	Int	nAll, nExtra;
	const uInt* typ;
	const casa::String*	theTypes = MDirection::allMyTypes(nAll, nExtra, typ);
	cout << "nAll=" << nAll << ", nExtra=" << nExtra << ", typ=" << typ << endl;
	for (int i=0; i < nAll; i++) {
		cout << theTypes[i] << "=" << typ[i] << endl;
	}

#if 0
	frame.resetEpoch(MVTime(2007,1,1,0.75));		// set some other time
	cout << "01012007 " << MVTime(2007,1,1,0.75) << endl;
	for (int i = 0; i < antPos.extent(firstDim); ++i) {
		MVDirection	ITRFPos(antPos(i,0), antPos(i,1), MDirection::ITRF);
		MDirection	J2000Dir = I2Jconverter(ITRFPos);
		cout << J2000Dir.getAngle() << endl;
	}

	frame.resetEpoch(MVTime(2007,1,2,0.2472685));		// set some other time
	cout << "02012007 " << MVTime(2007,1,1,0.2472685) << endl;
	for (int i = 0; i < antPos.extent(firstDim); ++i) {
		MVDirection	ITRFPos(antPos(i,0), antPos(i,1), MDirection::ITRF);
		MDirection	J2000Dir = I2Jconverter(ITRFPos);
		cout << J2000Dir.getAngle() << endl;
	}
#endif

	// Now do the same with the use of AMC
	ConverterClient		AMCclient("localhost");
	vector<double>	fieldPosITRFVect(3);
	fieldPosITRFVect[0] = fieldPos(0,0);
	fieldPosITRFVect[1] = fieldPos(0,1);
	fieldPosITRFVect[2] = fieldPos(0,2);
	Position    		fieldPositionITRF(Coord3D(fieldPosITRFVect), Position::ITRF);

	struct tm			feb2010;
	feb2010.tm_sec  = 0;
	feb2010.tm_min  = 0;
	feb2010.tm_hour = 6;
	feb2010.tm_mday = 2;
	feb2010.tm_mon  = 1;
	feb2010.tm_year = 110;
	cout << asctime(&feb2010) << endl;
	Timestamp			theTime(timegm(&feb2010), 0);
	double				mjd, fraction;
	theTime.convertToMJD(mjd, fraction);

	for (int i = 0; i < antPos.extent(firstDim); ++i) {
		cout << Epoch(mjd, fraction) << endl;
		vector<double>	antPosVect(3);
		antPosVect[0] = antPos(i,0);
		antPosVect[1] = antPos(i,1);
		antPosVect[2] = antPos(i,2);
		RequestData 		request(Direction(Coord3D(antPosVect), Direction::ITRF), 
									fieldPositionITRF, 
									Epoch(mjd, fraction));
		ResultData			result;
		cout << i << ": Direction: " << Direction(Coord3D(antPosVect), Direction::ITRF) << endl;
		cout << i << ": Position : " << fieldPositionITRF << endl;
		cout << i << ": Epoch    : " << Epoch(mjd, fraction) << endl;
		AMCclient.itrfToJ2000(result, request);
		vector<double>	direction = result.direction[0].coord().get();
		cout << i << ": Result   : " << direction[0] << ", " << direction[1] << ", " << direction[2] << endl;

#if 0
		double  originalLength = sqrt((antPos(i,0)*antPos(i,0)) +
									  (antPos(i,1)*antPos(i,1)) +
									  (antPos(i,2)*antPos(i,2)));
		direction[0] *= originalLength;
		direction[1] *= originalLength;
		direction[2] *= originalLength;

		cout << "scaled: " << direction[0] << ", " << direction[1] << ", " << direction[2] << endl;
#endif
	}

	complex<double> scaling(0,-0.10643);
	blitz::Array<double,1>	RCUpos(3);
	RCUpos(0) = 1.89484;
	RCUpos(1) = 1.62297;
	RCUpos(2) = 0.812045;
	blitz::Array<double,1>	xyz(3);
	xyz(0) = 0.671212;
	xyz(1) = 0.565354;
	xyz(2) = 0.479426;

	cout << "scaling     = " << scaling << endl;
	cout << "J2000RCUpos = " << RCUpos(Range::all()) << endl;
	cout << "J2000xyz    = " << xyz(Range::all()) << endl;

	cout << "weight = " << exp(scaling *
				(RCUpos(0) * xyz(0) +
				 RCUpos(1) * xyz(1) +
				 RCUpos(2) * xyz(2))) << endl;

}



#if 0
  J2000, 
  B1950, 
  AZEL, 
  ITRF, 
  MERCURY, 
  VENUS, 
  MARS, 
  JUPITER, 
  SATURN, 
  URANUS, 
  NEPTUNE, 
  PLUTO, 
  SUN, 
  MOON, 
#endif

