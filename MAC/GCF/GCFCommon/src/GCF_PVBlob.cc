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

GCFPVBlob::GCFPVBlob(unsigned char* val, unsigned int size, bool clone) 
  : GCFPValue(LPT_BLOB), _value(val), _size(size), _isDataHolder(clone)
{
  if (clone)
  {
    _value = new unsigned char[size];
    memcpy(_value, val, size);
  }
}

unsigned int GCFPVBlob::unpack(const char* valBuf)
{
  unsigned int result(0);
  unsigned int unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    memcpy((void *) &_size, valBuf + unpackedBytes, sizeof(_size));
    unpackedBytes += sizeof(_size);
    
    if (_isDataHolder)
    {
      delete [] _value;
    }
    _value = new unsigned char[_size];
    memcpy(_value, (unsigned char*) valBuf + unpackedBytes, _size);   
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

TGCFResult GCFPVBlob::setValue(unsigned char* value, unsigned int size, bool clone)
{ 
  TGCFResult result(GCF_NO_ERROR);
  _size = size; 
  if (_isDataHolder)
  {
    delete [] _value;
  }
  if (clone)
  {
    _value = new unsigned char[_size];
    memcpy(_value, value, _size);   
  }
  else
  {
    _value = value; 
  }
  _isDataHolder = clone;
  return result;
}
 
TGCFResult GCFPVBlob::setValue(const string value)
{
  TGCFResult result(GCF_NO_ERROR);

  _size = value.length();
  if (_isDataHolder)
  {
    delete [] _value;
  }
  _value = new unsigned char[_size];
  memcpy(_value, value.c_str(), _size);   

  _value = (unsigned char*) value.c_str(); 
  _isDataHolder = true;
  
  return result;
}

GCFPValue* GCFPVBlob::clone() const
{
  GCFPValue* pNewValue = new GCFPVBlob(_value, _size, true);
  return pNewValue;
}

TGCFResult GCFPVBlob::copy(const GCFPValue& newVal)
{
  TGCFResult result(GCF_NO_ERROR);

  if (newVal.getType() == getType())
  {
    if (_isDataHolder)
    {
      delete [] _value;
    }
    _size = ((GCFPVBlob *)&newVal)->getSize();
    _value = new unsigned char[_size];
    memcpy(_value, ((GCFPVBlob *)&newVal)->getValue(), _size);   
  }
  else
    result = GCF_DIFFERENT_TYPES;
  
  return result;
}
