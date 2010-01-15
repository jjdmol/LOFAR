//#  J2000Converter.cc: one_line_description
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

#include <measures/Measures.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>

#include "J2000Converter.h"

namespace LOFAR {
  namespace BS {

using namespace casa;
using namespace blitz;
using namespace RTC;

char*	supportedTypes[] = { "ITRF", "B1950", "AZEL", 
							 "MERCURY", "VENUS", "MARS", "JUPITER", "SATURN", "URANUS", "NEPTUNE", "PLUTO", "SUN", "MOON"  
							 "" };


//
// J2000Converter()
//
J2000Converter::J2000Converter()
{
	// First setup a map with the supported conversion types.
	MDirection::Types		aType;
	int	t = 0;
	while (supportedTypes[t]) {
		if (!MDirection::getType(aType, supportedTypes[t])) {
			LOG_ERROR_STR("Type " << supportedTypes[t] << " not supported by this version of casacore!");
		}
		LOG_DEBUG_STR("Adding support for " << supportedTypes[t]);
		itsDirectionTypes[supportedTypes[t]] = aType;
		t++;
	}
	ASSERTSTR(!itsDirectionTypes.empty(), "Can not resolve any coordinatesystem in casacore.");
}

//
// ~J2000Converter()
//
J2000Converter::~J2000Converter()
{
	// cleanup all our variables.
	itsConverters.clear();
	itsDirectionTypes.clear();
}

//
// _getConverter(types)
//
J2000Converter::converter_t* J2000Converter::_getConverter(MDirection::Types		theType)
{
	// simplify logging
	string	typeName(MDirection::showType(theType));

	// try to find the converter. If it is already there then we are done
	map<MDirection::Types, converter_t>::iterator     iter(itsConverters.find(theType));
	if (iter != itsConverters.end()) {
		LOG_INFO_STR("Using existing " << typeName << " to J2000 converter");
		return (&(iter->second));
	}

	// convertor not yet defined
	converter_t		newConverter;
	try {
		newConverter.frame.set(MEpoch(Quantity(55000.0, "d"), MEpoch::UTC)); // somewhere in 2009
		newConverter.frame.set(MPosition(MVPosition(0.0, 0.0, 0.0), MPosition::ITRF));
		newConverter.conv = MDirection::Convert(MDirection::Ref(theType), 
												MDirection::Ref(MDirection::J2000, newConverter.frame));
	}
	catch (AipsError&	e) {
		LOG_FATAL_STR("CASA ERROR while creating an convertor from "<< typeName << " to J2000: " << e.what());
		return (NULL);
	}

	if (newConverter.conv.isNOP()) {
		LOG_FATAL_STR("Unable to create a converter from " << typeName << " to J2000");
		return (NULL);
	}

	LOG_INFO_STR("Created a converter from " << typeName << " to J2000");
	itsConverters[theType] = newConverter;
	return (&itsConverters[theType]);
}

//
// _cartesian(angle2Pi, angle1Pi)
//
blitz::Array<double,1>	J2000Converter::_cartesian(double	angle2Pi, double	angle1Pi) {
	blitz::Array<double,1> tmpArr(3);
	double	cosPi = ::cos(angle1Pi);
	tmpArr(0) = cosPi * ::cos(angle2Pi); // x
	tmpArr(1) = cosPi * ::sin(angle2Pi); // y
	tmpArr(2) = ::sin(angle1Pi); // z
	return (tmpArr);
}

//
// doConversion(vector<MDirectoin>, MPosition, timestamp)
//
// Convert directions or position from whatever coordinate system to J2000 positions.
//
bool  J2000Converter::doConversion(const string&					sourceType,
								   const blitz::Array<double,2>& 	antPos,
								   const blitz::Array<double,1>&	ITRFfieldPos,
								   Timestamp						theTime,
								   blitz::Array<double,2>&			result)
{
	// Are the sources directions or positions?
	bool	srcIsDirection = (antPos.extent(secondDim) == 2);

	// sanity check
	ASSERTSTR(antPos.extent(secondDim) == 3 || antPos.extent(secondDim) == 2, 
				"antPos must have 2 or 3 elements per row not " << antPos.extent(secondDim));
	ASSERTSTR(ITRFfieldPos.extent(firstDim) == 3, 
				"ITRFfieldPos must have 3 elements not " << ITRFfieldPos.extent(firstDim));

	// clear result array
	result.free();

	// sleeping user?
	if (sourceType == "J2000") {
		if (srcIsDirection) {
			result.resize(antPos.extent(firstDim), 3);
			for (int i = antPos.extent(firstDim)-1; i >= 0; i--) {
				result(i, Range::all()) = _cartesian(antPos(i,0), antPos(i,1));
			}
			return (true);
		}
		// source are positions, just copy to the result array.
		result.resize(antPos.shape());
		result = antPos;
		return (true);
	}

	// find converter
	map<string, MDirection::Types>::const_iterator	iter(itsDirectionTypes.find(sourceType));
	if (iter == itsDirectionTypes.end()) {
		LOG_FATAL_STR("No support for conversion from " << sourceType << " to J2000");
		return (false);
	}

	converter_t*		theConv = _getConverter(iter->second);
	if (theConv->conv.isNOP()) {
		LOG_FATAL_STR("No converter available, returning empty result");
		return (false);
	}

	// reinit the frame of the converter
	theConv->frame.resetPosition(MPosition(MVPosition(ITRFfieldPos(0),ITRFfieldPos(1),ITRFfieldPos(2)), MPosition::ITRF));
	double	mjd;
	double	mjdFraction;
	theTime.convertToMJD(mjd, mjdFraction);
	theConv->frame.resetEpoch(MVEpoch(mjd, mjdFraction));

	// do the conversions
	int	nrDirections(antPos.extent(firstDim));
	result.resize(nrDirections, 3);
	MDirection	J2000Dir;
	for (int d = 0; d < nrDirections; d++) {
		if (srcIsDirection) {
			J2000Dir = theConv->conv(MVDirection(antPos(d,0), antPos(d,1)));
		}
		else {
			J2000Dir = theConv->conv(MVDirection(antPos(d,0), antPos(d,1), antPos(d,2)));
		}
		casa::Vector<Double>	angles = J2000Dir.getValue().get();
		result(d, Range::all()) = _cartesian(angles(0), angles(1));
	}

	return (true);
}


  } // namespace BS
} // namespace LOFAR
