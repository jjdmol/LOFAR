//#  CRrecordRequest.cc: Serialisable structure holding record requests for TBBs
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
#include <APL/CR_Protocol/CRrecordRequest.h>
#include <MACIO/Marshalling.tcc>

namespace LOFAR {
  namespace CR_Protocol {

// --- Output function --- 
ostream& CRrecordRequest::print (ostream& os) const
{
	os << "ReadRequest(" << stationList << "," << rcuList << ")";
	return (os);
}


// --- marshalling methods --- 
size_t CRrecordRequest::getSize()
{
	return(MSH_size(stationList) + MSH_size(rcuList));
}

size_t CRrecordRequest::pack  (char* buffer) const
{
	size_t offset = 0;
	MSH_pack(buffer, offset, stationList);	
	MSH_pack(buffer, offset, rcuList);	
	return (offset);
}

size_t CRrecordRequest::unpack(const char *buffer)
{
	size_t offset = 0;
	MSH_unpack(buffer, offset, stationList);	
	MSH_unpack(buffer, offset, rcuList);	
	return (offset);
}

  } // namespace CR_Protocol
} // namespace LOFAR
