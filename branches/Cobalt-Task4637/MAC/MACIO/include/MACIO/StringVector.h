//#  StringVector.h: Wrapper class for protocol files.
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

#ifndef MACIO_STRING_VECTOR_H
#define MACIO_STRING_VECTOR_H

// \file StringVector.h
// Wrapper class for protocol files.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <MACIO/Marshalling.tcc>
#include <Common/StreamUtil.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace MACIO {

// \addtogroup MACIO
// @{


// class_description
class StringVector
{
public:
	StringVector() {};
	~StringVector() {};

	size_t getSize() const;
	size_t pack  (char	*buffer) const;
	size_t unpack(char	*buffer);
	ostream& print (ostream& os) const;

	vector<string>&	operator()()
		{ return (theVector); }


	//# --- Datamember ---
private:
	vector<string>	theVector;
};

// @}

// getSize()
inline size_t StringVector::getSize() const
{
	return (MSH_size(theVector));
}

// pack()
inline size_t StringVector::pack(char	*buffer) const
{
	size_t offset = 0;
	MSH_pack(buffer, offset, theVector);
	return (offset);
}

// unpack()
inline size_t StringVector::unpack(char	*buffer)
{
	size_t offset = 0;
	MSH_unpack(buffer, offset, theVector);
	return (offset);
}

// print(ostream&)
inline ostream& StringVector::print (ostream&os) const
{
	writeVector(os, theVector);
	return (os);
}

// operator<< 
inline ostream& operator<< (ostream& os, const StringVector&	sv)
{
	return (sv.print(os));
}


  } // namespace MACIO
} // namespace LOFAR

#endif
