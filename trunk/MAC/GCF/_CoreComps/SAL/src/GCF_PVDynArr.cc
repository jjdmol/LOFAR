//#  GCF_PVDynArr.cc: 
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


#include <GCF/GCF_PVDynArr.h>

GCFPVDynArr::GCFPVDynArr(TMACValueType itemType, const GCFPValueArray& val) :
  GCFPValue((TMACValueType) (LPT_DYNARR | itemType))
{
  assert(itemType != LPT_DYNARR);
  setValue(val);
}

GCFPVDynArr::GCFPVDynArr(TMACValueType itemType) :
  GCFPValue((TMACValueType) (LPT_DYNARR | itemType))
{
  assert(itemType != LPT_DYNARR);
}

GCFPVDynArr::~GCFPVDynArr()
{
  cleanup();
}

unsigned int GCFPVDynArr::unpack(const char* valBuf)
{
  unsigned int result(0);
  unsigned int unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    cleanup();
    unsigned int arraySize(0);
    unsigned int curUnpackedBytes(0);
    GCFPValue* pNewValue(0);
    memcpy((void *) &arraySize, valBuf + unpackedBytes, sizeof(unsigned int));
    unpackedBytes += sizeof(unsigned int);
    for (unsigned int i = 0; i < arraySize; i++)
    {
      pNewValue = GCFPValue::createMACTypeObject((TMACValueType) (getType() | LPT_DYNARR));
      
      curUnpackedBytes = pNewValue->unpack(valBuf + unpackedBytes);
      if (curUnpackedBytes > 0)
      {
        unpackedBytes += curUnpackedBytes;
        _values.push_back(pNewValue);
      }
      else
      {
        unpackedBytes = 0; 
        break;
      }
    }
    result = unpackedBytes;
  }
  return result;
}

unsigned int GCFPVDynArr::pack(char* valBuf) const
{
  unsigned int result(0);  
  unsigned int packedBytes = packBase(valBuf);
  if (packedBytes > 0)
  {
    unsigned int arraySize(_values.size());
    memcpy(valBuf + packedBytes, (void *) &arraySize, sizeof(unsigned int));
    packedBytes += sizeof(unsigned int);
    unsigned int curPackedBytes = 0;
    for (GCFPValueArray::const_iterator iter = _values.begin();
         iter != _values.end(); ++iter)
    {
      curPackedBytes = (*iter)->pack(valBuf + packedBytes);
      packedBytes += curPackedBytes;
      if (curPackedBytes == 0)
      {
        packedBytes = 0;
        break;
      }        
    }  
    result = packedBytes;
  }
  return result;
}

unsigned int GCFPVDynArr::getSize() const
{
  unsigned int totalSize(sizeof(unsigned int));

  for (GCFPValueArray::const_iterator iter = _values.begin();
       iter != _values.end(); ++iter)
  {
    totalSize += (*iter)->getSize();
  }  
  totalSize += getBaseSize();
  return totalSize;
}

TGCFResult GCFPVDynArr::setValue(const string value)
{
  TGCFResult result(GCF_NO_ERROR);
  
  return result;
}

void GCFPVDynArr::setValue(const GCFPValueArray& newVal)
{
  cleanup();
  for (GCFPValueArray::const_iterator iter = newVal.begin();
       iter != newVal.end(); ++iter)
  {
    if ((*iter)->getType() == (getType() & ~LPT_DYNARR))
      _values.push_back((*iter)->clone());
  }
}

GCFPValue* GCFPVDynArr::clone() const
{
  GCFPValue* pNewValue = new GCFPVDynArr(getType(), _values);
  return pNewValue;
}

TGCFResult GCFPVDynArr::copy(const GCFPValue& newVal)
{
  TGCFResult result(GCF_NO_ERROR);

  if (newVal.getType() == getType())
    setValue(((GCFPVDynArr*)&newVal)->getValue());
  else
    result = GCF_DIFFERENT_TYPES;
  
  return result;
}

void GCFPVDynArr::cleanup()
{
  for (GCFPValueArray::iterator iter = _values.begin();
       iter != _values.end(); ++iter)
  {
    delete *iter;  
  }
  _values.clear();
}
