//#  GCF_PVBlob.cc: 
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


#include <GCF/GCF_PVBlob.h>

unsigned int GCFPVBlob::unpack(const char* valBuf)
{
  unsigned int result(0);
  unsigned int unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    memcpy((void *) &_size, valBuf + unpackedBytes, sizeof(_size));
    unpackedBytes += sizeof(_size);
    
    _value = (unsigned char*) valBuf + unpackedBytes;
    result = unpackedBytes + _size;
  }
  return result;
}

unsigned int GCFPVBlob::pack(char* valBuf) const
{
  unsigned int result(0);
  unsigned int packedBytes = packBase(valBuf);
  if (packedBytes > 0)
  {        
    memcpy(valBuf + packedBytes, (void *) &_size, sizeof(_size));
    memcpy(valBuf + packedBytes + sizeof(_size), (void *) _value, _size);
    result = sizeof(_size) + packedBytes + _size;
  }
  return result;
}

TGCFResult GCFPVBlob::setValue(unsigned char* value, unsigned int size)
{ 
  TGCFResult result(GCF_NO_ERROR);
  _value = value; 
  _size = size; 
  return result;
}
 
TGCFResult GCFPVBlob::setValue(const string value)
{
  TGCFResult result(GCF_NO_ERROR);

  _value = (unsigned char*) value.c_str();
  _size = value.length();
  
  return result;
}

GCFPValue* GCFPVBlob::clone() const
{
  GCFPValue* pNewValue = new GCFPVBlob(_value, _size);
  return pNewValue;
}

TGCFResult GCFPVBlob::copy(const GCFPValue& newVal)
{
  TGCFResult result(GCF_NO_ERROR);

  if (newVal.getType() == getType())
  {
    _value = ((GCFPVBlob *)&newVal)->getValue();
    _size = ((GCFPVBlob *)&newVal)->getSize();
  }
  else
    result = GCF_DIFFERENT_TYPES;
  
  return result;
}
