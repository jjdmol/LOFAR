//#  BlAllocArr.cc: Tripled that describes an allocation of a beamlet
//#
//#  Copyright (C) 2009
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
#include <MD_Protocol/BlAllocArr.h>

namespace LOFAR {
  namespace MAC {

//
// Constructor and destructor
//
BlAllocArr::BlAllocArr(int	nrElements) :
	itsSize(nrElements)
{
	itsPool.resize(itsSize);
}

BlAllocArr::~BlAllocArr()
{
	itsPool.clear();
}

//
// operator[]
//
BeamletAllocation&	BlAllocArr::operator[](uint	index)
{
	ASSERTSTR(index <= itsSize, "BeamletAllocation has only " << itsSize << " elements, not " << index);
	return (itsPool[index]);
}

const BeamletAllocation&	BlAllocArr::operator[](uint	index) const
{
	ASSERTSTR(index <= itsSize, "BeamletAllocation has only " << itsSize << " elements, not " << index);
	return (itsPool[index]);
}


//
// getSize()
//
unsigned int BlAllocArr::getSize()
{
	return (sizeof(uint32) + (itsSize * itsPool[0].getSize()));
}

//
// pack(char*)
//
unsigned int BlAllocArr::pack   (void* buffer)
{
	uint offset(sizeof(itsSize));

	memcpy(((char*)buffer), &itsSize, sizeof(itsSize));
	for (int i = 0; i < itsSize; i++) {
		offset += itsPool[i].pack((char*)buffer + offset);
	}
	return (offset);
}

//
// unpack(char*)
//
unsigned int BlAllocArr::unpack (void* buffer)
{
	uint offset(sizeof(itsSize));

	memcpy(&itsSize, ((char*)buffer), sizeof(itsSize));
	itsPool.clear();
	itsPool.resize(itsSize);
	for (int i = 0; i < itsSize; i++) {
		offset += itsPool[i].unpack(((char*)buffer) + offset);
	}
	return (offset);
}

  } // namespace MAC
} // namespace LOFAR

