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
#include <Common/StringUtil.h>
#include <MACIO/GCF_Event.h>

using namespace std;

namespace LOFAR {
  namespace MACIO {

//
// ~GCFEvent
//
GCFEvent::~GCFEvent() 
{ 
	LOG_TRACE_CALC(formatString("~GCFEvent: length=%d, _buffer=%08X", length, _buffer));

	if (_buffer)  {
		delete [] _buffer; 
	}
}

//
// pack()
//
void GCFEvent::pack() 
{   
	if (!_buffer) {
		resizeBuf(sizePackedGCFEvent);
	}

	// packs the GCF event fields in an existing buffer
	memcpy(_buffer, &signal, sizeof(signal));
	memcpy(_buffer + sizeof(signal), &length, sizeof(length));
}

//
// resizeBuf(requiredSize)
//
// makes sure that the pack buffer is large enough
//
void GCFEvent::resizeBuf(uint32 requiredSize)
{
	// release old buffer is any.
	if (_buffer) {
		delete [] _buffer;
		_buffer = 0;
	}

	_buffer = new char[requiredSize];
	ASSERTSTR(_buffer, "Can not allocated " << requiredSize << 
						" bytes for packing an event of type " << signal);
	length = requiredSize - sizePackedGCFEvent;
}

//
// clone()
//
GCFEvent* GCFEvent::clone() 
{
	if (length == 0) {
		LOG_TRACE_CALC(formatString("The %04X clone is %d bytes", signal, sizePackedGCFEvent, length));
		// Cloning is easy, just call the copy constructor.
		return (new GCFEvent(*this));
	}

	// Sometimes packed event are cloned (GCF_Scheduler of GCF/TM).
	LOG_DEBUG(formatString("GCFEvent::clone() cloning a packed event of type 0x%04x (size=%d)", signal, bufferSize()));
	GCFEvent*	clonedEvent = new GCFEvent(signal);
	clonedEvent->resizeBuf(bufferSize());
	memcpy (clonedEvent->_buffer, _buffer, bufferSize());
	return (clonedEvent);
}

//
// print(os)
//
ostream& GCFEvent::print (ostream& os) const
{
	os << formatString("signal=0x%04X, length=0x%08X(%d), _buffer=0x%08X\n", signal, length, length, _buffer);
	if (_buffer) {
		string	hd;
		hexdump(hd, _buffer, sizePackedGCFEvent + length);
		os << hd;
	}
	return (os);
}

// -------------------- helper functions --------------------

//
// unpackMember(data, offset, elementnumber, sizeof element)
//
void* GCFEvent::unpackMember(char* data, uint32& offset, uint32& memberNOE, uint32 sizeofMemberType)
{
  void* seqPtr(0);
  memcpy(&memberNOE, data + offset, sizeof(memberNOE));
  seqPtr = data + offset + sizeof(memberNOE);
  offset += sizeof(memberNOE) + (memberNOE * sizeofMemberType);
  return seqPtr;
}

//
// packMember(data, offset, elementnumber, sizeof element)
//
uint32 GCFEvent::packMember(uint32 offset, const void* member, uint32 memberNOE, uint32 sizeofMemberType)
{
	ASSERTSTR(_buffer, "Attempt to pack an eventMember in an unallocated _buffer");

  memcpy(_buffer + offset, &memberNOE, sizeof(memberNOE));
  offset += sizeof(memberNOE);
  if (memberNOE > 0) {
    memcpy(_buffer + offset, member, memberNOE * sizeofMemberType);
  }
  return (memberNOE * sizeofMemberType) + sizeof(memberNOE);
}

//
// unpackString(string, buffer)
//
uint32 GCFEvent::unpackString(string& value, char* buffer)
{
  uint16 stringLength(0);
  memcpy((void *) &stringLength, buffer, sizeof(stringLength));
  value.clear();
  value.append(buffer + sizeof(stringLength), stringLength);
  return stringLength + sizeof(stringLength);
}

//
// packString(string, buffer)
//
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
