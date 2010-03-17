//#  J2000Converter.h: one_line_description
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

#ifndef BS_J2000_CONVERTER_H
#define BS_J2000_CONVERTER_H

// \file J2000Converter.h
// one_line_description

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <blitz/array.h>
#include <measures/Measures.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <APL/RTCCommon/Timestamp.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace BS {

// \addtogroup package
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.


// class_description
// ...
class J2000Converter
{
public:
	J2000Converter();
	~J2000Converter();

	// Converts the given directions to J2000 directions for Position on Earth at time Timestamp
	bool  doConversion(const string&					sourceType,
					   const blitz::Array<double,2>&	antPos,
					   const blitz::Array<double,1>&	ITRFfieldPos,
					   RTC::Timestamp					theTime,
					   blitz::Array<double,2>&			result);

	// some functions to exploid the supported conversion types.
	bool	isValidType(const string&	refType) 
			{ return (itsDirectionTypes.find(refType) != itsDirectionTypes.end()); }
	vector<string>	validTypes();

private:
	// internal admin structures
	typedef struct converter {
		casa::MDirection::Convert	conv;
		casa::MeasFrame				frame;
	} converter_t;

	// private helper routines
	converter_t*			_getConverter(casa::MDirection::Types);
	blitz::Array<double,1>	_cartesian(double	angle2Pi, double	angle1Pi);

	// Copying is not allowed
	J2000Converter(const J2000Converter&	that);
	J2000Converter& operator=(const J2000Converter& that);

	//# --- Datamembers ---
	// name, type map
	map<string, casa::MDirection::Types>		itsDirectionTypes;

	// type, converter_t map
	map<casa::MDirection::Types, converter_t>	itsConverters;
};

// @}
  } // namespace PACKAGE
} // namespace LOFAR

#endif
