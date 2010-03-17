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
#include <Common/LofarConstants.h>
#include <MD_Protocol/BlAllocArr.h>
#include <MD_Protocol/BeamletAllocation.h>


namespace LOFAR {
  namespace MD_Protocol {

//
// constructor and destructor
//
BlAllocArr::BlAllocArr(int	nrElements) :
	itsSize(nrElements)
{
	itsPool.resize(itsSize);
}

BlAllocArr::BlAllocArr() :
	itsSize(0)
{
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
	ASSERTSTR(index < itsSize, "There are only " << itsSize << " beamletAllocations, not " << index);
	return (itsPool[index]);
}

const BeamletAllocation&	BlAllocArr::operator[](uint	index) const
{
	ASSERTSTR(index < itsSize, "There are only " << itsSize << " beamletAllocations, not " << index);
	return (itsPool[index]);
}


//
// Access functions
//
void BlAllocArr::set		(uint index, uint subband, int beam, uint observation)
{
	ASSERTSTR(index < itsSize, "There are only " << itsSize << " beamletAllocations, not " << index);

	if (itsPool[index].observation) {
		LOG_WARN(formatString("Overwriting beamlet allocation %d: Was: %d %d.%d, will be: %d %d.%d",
			index, itsPool[index].subband, itsPool[index].observation, itsPool[index].beam, subband, observation, beam));
	}
	itsPool[index].subband 	 = subband;
	itsPool[index].beam    	 = beam;
	itsPool[index].observation = observation;
}

void BlAllocArr::clear		(uint index)
{
	ASSERTSTR(index < itsSize, "There are only " << itsSize << " beamletAllocations, not " << index);
	itsPool[index] = BeamletAllocation();
}

void BlAllocArr::clear		(uint index, uint	count)
{
	ASSERTSTR(index < itsSize, "There are only " << itsSize << " beamletAllocations, not " << index);
	ASSERTSTR(index+count < itsSize, "There are only " << itsSize << " beamletAllocations, not " << index+count);
	for (uint i = index; i < index+count; i++) {
		itsPool[i] = BeamletAllocation();
	}
}

void BlAllocArr::clearRange (uint lowlimit, uint upperlimit)
{
	ASSERTSTR(upperlimit < itsSize, "There are only " << itsSize << " beamletAllocations, not " << upperlimit);
	for (uint i = lowlimit; i < upperlimit; i++) {
		itsPool[i] = BeamletAllocation();
	}

}

void BlAllocArr::clearRSP   (uint rspNr)
{
	clear(rspNr * MAX_BEAMLETS_PER_RSP, MAX_BEAMLETS_PER_RSP);
}

//
// print function for operator<<
//
ostream& BlAllocArr::print (ostream& os) const
{
	os << "BAA capacity: " << itsSize << endl;
	os << "index: sub   obs.beam" << endl;
	for (uint i = 0; i < itsSize; i++) {
		if (itsPool[i].observation) {
			os << formatString("%3d: %3d %5d.%d\n", i, itsPool[i].subband, itsPool[i].observation, itsPool[i].beam);
		}
	}
	return (os);
}



//
// getSize()
//
unsigned int BlAllocArr::getSize()
{
	return (sizeof(itsSize) + (itsSize * itsPool[0].getSize()));
}

//
// pack(char*)
//
unsigned int BlAllocArr::pack   (void* buffer)
{
	memcpy(((char*)buffer), &itsSize, sizeof(itsSize));

	uint	offset(sizeof(itsSize));
	for (uint i = 0; i < itsSize; i++) {
		offset += itsPool[i].pack(((char*)buffer) + offset);
	}

	return (offset);
}

//
// unpack(char*)
//
unsigned int BlAllocArr::unpack (void* buffer)
{
	memcpy(&itsSize, ((char*)buffer), sizeof(itsSize));
	itsPool.clear();
	itsPool.resize(itsSize);

	uint	offset(sizeof(uint32));
	for (uint i = 0; i < itsSize; i++) {
		offset += itsPool[i].unpack(((char*)buffer) + offset);
	}

	return (offset);
}

  } // namespace MD_Protocol
} // namespace LOFAR

