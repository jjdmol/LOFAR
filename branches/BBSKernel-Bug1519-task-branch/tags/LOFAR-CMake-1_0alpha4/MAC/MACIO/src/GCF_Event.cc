//#  GCF_Event.cc: finite state machine events
//#
//#  Copyright (C) 2002-2003
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

#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Common/hexdump.h>
#include <MACIO/GCF_Event.h>

using namespace std;

namespace LOFAR {
  namespace MACIO {

GCFEvent::~GCFEvent() 
{ 
//	LOG_DEBUG(formatString("~GCFEvent: _buffer=%08X, _upperbound=%d", _buffer, _upperbound));

	if (_buffer)  {
//		LOG_DEBUG("CALLING DELETE");
		delete [] _buffer; 
	}
}

void* GCFEvent::pack(uint32& packsize) 
{   
  // packs the base event fields     
  memcpy(_buffer, &signal, sizeof(signal));
  memcpy(_buffer + sizeof(signal), &length, sizeof(length));
  packsize = sizeof(signal) + sizeof(length);
  return _buffer;
}

void GCFEvent::resizeBuf(uint32 requiredSize)
{
  // resizes the buffer if needed
  if (requiredSize > _upperbound && _buffer) {
    delete [] _buffer;
    _buffer = 0;
  }
  if (!_buffer) {
    _buffer = new char[requiredSize];
    _upperbound = requiredSize;
  }
  length = requiredSize - sizeof(length) - sizeof(signal);
}

GCFEvent* GCFEvent::clone() const
{
	int		mySize = sizeof(GCFEvent) + length;
	char* 	theClone = new char[mySize];
	memcpy(theClone, (const char*)this, mySize);

	if (_buffer) {
		((GCFEvent*)theClone)->_buffer 	  = 0;
		((GCFEvent*)theClone)->_upperbound = 0;
		((GCFEvent*)theClone)->resizeBuf(_upperbound);
		memcpy(((GCFEvent*)theClone)->_buffer, _buffer, _upperbound);
	}
	LOG_TRACE_CALC_STR("The clone is " << mySize << "+" << _upperbound << " bytes");

	return ((GCFEvent*)theClone);
}

void* GCFEvent::unpackMember(char* data, uint32& offset, uint32& memberNOE, uint32 sizeofMemberType)
{
  void* seqPtr(0);
  memcpy(&memberNOE, data + offset, sizeof(memberNOE));
  seqPtr = data + offset + sizeof(memberNOE);
  offset += sizeof(memberNOE) + (memberNOE * sizeofMemberType);
  return seqPtr;
}

uint32 GCFEvent::packMember(uint32 offset, const void* member, uint32 memberNOE, uint32 sizeofMemberType)
{
  ASSERT(_buffer);
  memcpy(_buffer + offset, &memberNOE, sizeof(memberNOE));
  offset += sizeof(memberNOE);
  if (memberNOE > 0) {
    memcpy(_buffer + offset, member, memberNOE * sizeofMemberType);
  }
  return (memberNOE * sizeofMemberType) + sizeof(memberNOE);
}

uint32 GCFEvent::unpackString(string& value, char* buffer)
{
  uint16 stringLength(0);
  memcpy((void *) &stringLength, buffer, sizeof(stringLength));
  value.clear();
  value.append(buffer + sizeof(stringLength), stringLength);
  return stringLength + sizeof(stringLength);
}

uint32 GCFEvent::packString(char* buffer, const string& value)
{
  uint16 stringLength(value.size());
  uint32 neededBufLength = value.size() + sizeof(stringLength);
  memcpy(buffer, (void *) &stringLength, sizeof(stringLength));
  memcpy(buffer + sizeof(stringLength), (void *) value.c_str(), value.size());
  return neededBufLength;
}  
  } // namespace MACIO
} // namespace LOFAR
