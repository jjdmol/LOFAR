//#  DoubleVector.h: Wrapper class for protocol files.
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
//#  $Id: DoubleVector.h 14588 2009-12-02 15:31:07Z overeem $

#ifndef MACIO_DOUBLE_VECTOR_H
#define MACIO_DOUBLE_VECTOR_H

// \file DoubleVector.h
// Wrapper class for protocol files.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_vector.h>
#include <Common/StreamUtil.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace MACIO {

// \addtogroup MACIO
// @{


// class_description
class DoubleVector
{
public:
	DoubleVector() {};
	~DoubleVector() {};

	size_t getSize() const;
	size_t pack  (char	*buffer) const;
	size_t unpack(char	*buffer);
	ostream& print (ostream& os) const;

	vector<double>&	operator()()
		{ return (theVector); }

	//# --- Datamember ---
private:
	vector<double>	theVector;
};

// @}

// getSize()
inline size_t DoubleVector::getSize() const
{
	return (MSH_size(theVector));
}

// pack()
inline size_t DoubleVector::pack(char	*buffer) const
{
	size_t offset = 0;
	MSH_pack(buffer, offset, theVector);
	return (offset);
}

// unpack()
inline size_t DoubleVector::unpack(char	*buffer)
{
	size_t offset = 0;
	MSH_unpack(buffer, offset, theVector);
	return (offset);
}

// print(ostream&)
inline ostream& DoubleVector::print (ostream&os) const
{
	writeVector(os, theVector);
	return (os);
}

// operator<< 
inline ostream& operator<< (ostream& os, const DoubleVector&	sv)
{
	return (sv.print(os));
}


  } // namespace MACIO
} // namespace LOFAR

#endif
