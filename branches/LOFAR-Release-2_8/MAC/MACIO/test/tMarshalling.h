//#  tMarshalling.h: test pack and unpack routines.
//#
//#  Copyright (C) 2007
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
//#  $Id: tMarshalling.h 11023 2008-03-18 08:07:14Z overeem $

#ifndef RTC_TMARSHALLING_H
#define RTC_TMARSHALLING_H

// \file tMarshalling.h
// test pack and unpack macros

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
//#include <otherPackage/file.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
//class ...;


// class_description
// ...

class	SubArray 
{
public:
	int		someInt;
	double	someDouble;
	string	someString;

	SubArray(int i, double d, string s);
	SubArray() {};
	size_t getSize() const;
	size_t pack  (char* buffer) const;
	size_t unpack(const char *buffer);
};

class	SubArrayNC
{
public:
	int		someInt;
	double	someDouble;
	string	someString;

	SubArrayNC(int i, double d, string s);
	SubArrayNC() {};
	size_t getSize() const;
	size_t pack  (char* buffer) const;
	size_t unpack(const char *buffer);
private:
	// prevent copy
	SubArrayNC(const SubArrayNC&);
};

} // namespace LOFAR

#endif
