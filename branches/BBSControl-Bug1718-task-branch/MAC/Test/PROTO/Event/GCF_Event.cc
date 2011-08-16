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

#include <GCF/GCF_Event.h>

using namespace std;

GCFEvent::~GCFEvent() 
{ 
  if (_buffer) delete [] _buffer; 
}

void* GCFEvent::pack(unsigned int& packsize) 
{        
  memcpy(_buffer, &signal, sizeof(signal));
  memcpy(_buffer + sizeof(signal), &length, sizeof(length));
  packsize = sizeof(signal) + sizeof(length);
  return _buffer;
}

void GCFEvent::resizeBuf(unsigned int requiredSize)
{
  if (requiredSize > _upperbound && _buffer)
  {
    delete [] _buffer;
    _buffer = 0;
  }
  if (!_buffer)
  {
    _buffer = new char[requiredSize];
    _upperbound = requiredSize;
  }
  length = requiredSize - sizeof(length) - sizeof(signal);
}

void* GCFEvent::unpackMember(char* data, unsigned int& offset, unsigned int& memberDim, unsigned int sizeofMemberType)
{
  void* seqPtr(0);
  memcpy(&memberDim, data + offset, sizeof(unsigned int));
  seqPtr = data + offset + sizeof(unsigned int);
  offset += sizeof(unsigned int) + memberDim * sizeofMemberType;
  return seqPtr;
}

unsigned int GCFEvent::packMember(unsigned int offset, const void* member, unsigned int memberDim, unsigned int sizeofMemberType)
{
  assert(_buffer);
  memcpy(_buffer + offset, &memberDim, sizeof(memberDim));
  offset += sizeof(memberDim);
  memcpy(_buffer + offset, member, memberDim * sizeofMemberType);
  return (memberDim * sizeofMemberType) + sizeof(memberDim);
}

unsigned int GCFEvent::unpackString(string& value, char* buffer)
{
  unsigned int stringLength(0);
  memcpy((void *) &stringLength, buffer,  + sizeof(unsigned int));
  value.clear();
  value.append(buffer + sizeof(unsigned int), stringLength);
  return stringLength + sizeof(unsigned int);
}

unsigned int GCFEvent::packString(char* buffer, const string& value)
{
  unsigned int neededBufLength = value.size() + sizeof(unsigned int);
  unsigned int stringLength(value.size());
  memcpy(buffer, (void *) &stringLength,  + sizeof(unsigned int));
  memcpy(buffer + sizeof(unsigned int), (void *) value.c_str(), value.size());
  return neededBufLength;
}  
