//#  BeamletAllocation.cc: Tripled that describes an allocation of a beamlet
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
#include <MD_Protocol/BeamletAllocation.h>

namespace LOFAR {
  namespace MD_Protocol {

//
// getSize()
//
unsigned int BeamletAllocation::getSize()
{
	return (3 * sizeof(uint32));
}

//
// pack(char*)
//
unsigned int BeamletAllocation::pack   (void* buffer)
{
	uint	size32(sizeof(uint32));

	memcpy(((char*)buffer) + 0, 		 &subband,	   size32);
	memcpy(((char*)buffer) + size32, 	 &beam, 	   size32);
	memcpy(((char*)buffer) + (2*size32), &observation, size32);

	return (3 * size32);
}

//
// unpack(char*)
//
unsigned int BeamletAllocation::unpack (void* buffer)
{
	uint	size32(sizeof(uint32));

	memcpy(&subband, 	 ((char*)buffer) + 0, 		   size32);
	memcpy(&beam, 		 ((char*)buffer) + size32, 	   size32);
	memcpy(&observation, ((char*)buffer) + (2*size32), size32);

	return (3 * size32);
}

  } // namespace MD_Protocol
} // namespace LOFAR
