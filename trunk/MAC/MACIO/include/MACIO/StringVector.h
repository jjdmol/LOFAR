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
#include <MACIO/Marshalling.h>

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

	unsigned int getSize();
	unsigned int pack  (void	*buffer);
	unsigned int unpack(void	*buffer);

	//# --- PUBLIC Datamember ---
	vector<string>	theVector;
};

// @}

// getSize()
inline unsigned int StringVector::getSize()
{
	unsigned int	offset = 0;
	MSH_SIZE_VECTOR_STRING(offset, theVector);
	return (offset);
}

// pack()
inline unsigned int StringVector::pack(void	*buffer)
{
	unsigned int offset = 0;
	MSH_PACK_VECTOR_STRING(buffer, offset, theVector);
	return (offset);
}

// unpack()
inline unsigned int StringVector::unpack(void	*buffer)
{
	unsigned int offset = 0;
	MSH_UNPACK_VECTOR_STRING(buffer, offset, theVector);
	return (offset);
}


  } // namespace MACIO
} // namespace LOFAR

#endif
