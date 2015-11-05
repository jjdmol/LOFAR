//#  CRstopVector.cc: Serialisable structure holding stop requests for TBBs
//#
//#  Copyright (C) 2011
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
//#  $Id: $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <APL/CR_Protocol/CRstopVector.h>

namespace LOFAR {
  namespace CR_Protocol {

// --- Output function --- 
ostream& CRstopVector::print (ostream& os) const
{
	os << "StopVector:[";
	for (size_t i = 0; i < requests.size(); i++) {
		os << requests[i] << endl;
	}
	os << "]";
	return (os);
}


// --- marshalling methods --- 
unsigned int CRstopVector::getSize()
{
	uint	offset(sizeof(uint32));
	uint32	nrElems(requests.size());
	for (size_t i = 0; i < nrElems; i++) {
		offset +=  requests[i].getSize();
	}
	return (offset);
}

unsigned int CRstopVector::pack  (void* buffer)
{
	unsigned int	offset(sizeof(uint32));
	uint32			nrElems(requests.size());
	memcpy(buffer, &nrElems, sizeof(uint32));
	for (size_t i = 0; i < nrElems; i++) {
		offset +=  requests[i].pack((char*)(buffer)+offset);
	}
	return (offset);
}

unsigned int CRstopVector::unpack(void *buffer)
{
	unsigned int	offset(sizeof(uint32));
	uint32			nrElems;
	memcpy(&nrElems, buffer, sizeof(uint32));
	requests.resize(nrElems);
	for (size_t i = 0; i < nrElems; i++) {
		offset +=  requests[i].unpack((char*)(buffer)+offset);
	}
	return (offset);
}

  } // namespace CR_Protocol
} // namespace LOFAR
