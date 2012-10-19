//#  Converter.h: Base class for database based id-name conversions.
//#
//#  Copyright (C) 2002-2004
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

#ifndef LOFAR_OTDB_CONVERTER_H
#define LOFAR_OTDB_CONVERTER_H

// \file
// Base Class for database based id-name conversions.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_map.h>
#include <Common/lofar_string.h>
#include <Common/LofarTypes.h>
#include <pqxx/pqxx>

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
class OTDBconnection;

// Base class for database based id-name conversions.
class Converter {
public:
	typedef map<string, int16>::iterator			iterator;
	typedef map<string, int16>::const_iterator		const_iterator;

	// Show whole conversiontable
	ostream& print (ostream& os) const;

	void	top();
	bool	next();

protected:
	// Only derived classes may construct me
	Converter(OTDBconnection* aConn, const string&	aTypeName);
	~Converter() {};

	// Conversion routines to be called by derived classes
	int16	convert(const string&	aType)   const;
	string	convert(int16			aTypeID) const;

	// routine for getting both values the iterator points at.
	// False is returned if the iterator points at end();
	bool	get(int16&	theValue, string&	theName) const;

private:
	// Copying is not allowed
	Converter(const Converter&	that);
	Converter& operator=(const Converter& that);

	map<string, int16>		itsMap;
	string					itsTypeName;
	const_iterator			itsIter;
};

//#
//# operator<<
//#
inline ostream& operator<< (ostream&			os,
							const Converter&	aConverter)
{
	return (aConverter.print(os));
}

inline void Converter::top() 
{
	itsIter = itsMap.begin();
}

inline bool Converter::next()
{
	return (++itsIter != itsMap.end());
}


// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
