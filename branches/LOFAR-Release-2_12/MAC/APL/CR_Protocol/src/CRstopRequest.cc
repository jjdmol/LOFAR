//#  CRstopRequest.cc: Serialisable structure holding stop requests for TBBs
//#
//#  Copyright (C) 2011
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
#include <APL/CR_Protocol/CRstopRequest.h>
#include <MACIO/Marshalling.tcc>

namespace LOFAR {
  namespace CR_Protocol {

// --- Output function --- 
ostream& CRstopRequest::print (ostream& os) const
{
	os << "StopRequest(" << stationList << "," << rcuList << "," << stopTime << ")";
	return (os);
}


// --- marshalling methods --- 
size_t CRstopRequest::getSize()
{
	return(MSH_size(stationList) + MSH_size(rcuList) + stopTime.getSize());
}

size_t CRstopRequest::pack  (char* buffer) const
{
	size_t offset = 0;
	offset = MSH_pack(buffer, offset, stationList);	
	offset = MSH_pack(buffer, offset, rcuList);	
	offset += stopTime.pack(buffer + offset);
	return (offset);
}
 
size_t CRstopRequest::unpack(const char *buffer)
{
	size_t offset = 0;
	offset = MSH_unpack(buffer, offset, stationList);	
	offset = MSH_unpack(buffer, offset, rcuList);	
	offset += stopTime.unpack(buffer + offset);
	return (offset);
}

  } // namespace CR_Protocol
} // namespace LOFAR
