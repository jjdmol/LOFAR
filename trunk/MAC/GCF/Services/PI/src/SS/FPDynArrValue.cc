//#  FPDynArrValue.cc: 
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

#include "FPDynArrValue.h"

FPDynArrValue::FPDynArrValue(ValueType itemType, const FPValueArray& val) :
  FPValue((ValueType) (DYNARR_VAL | itemType))
{
  assert(itemType != DYNARR_VAL);
  setValue(val);
}

FPDynArrValue::FPDynArrValue(ValueType itemType) :
  FPValue((ValueType) (DYNARR_VAL | itemType))
{
  assert(itemType != DYNARR_VAL);
}

FPDynArrValue::~FPDynArrValue()
{
  cleanup();
}

void FPDynArrValue::setValue(const FPValueArray& newVal)
{
  cleanup();
  FPValue* pValue(0);
  for (FPValueArray::const_iterator iter = newVal.begin();
       iter != newVal.end(); ++iter)
  {
    pValue = (*iter);
    if (pValue->getType() == (getType() & ~DYNARR_VAL))
      values_.push_back(pValue->clone());
  }
}

/** No descriptions */
uint FPDynArrValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    cleanup();
    uint arraySize(0);
    FPValue* pNewValue(0);
    memcpy((void *) &arraySize, valBuf + unpackedBytes, sizeof(uint));
    unpackedBytes += sizeof(uint);
    for (uint i = 0; i < arraySize; i++)
    {
      pNewValue = FPValue::createValueObject((ValueType) (getType() | DYNARR_VAL));
      unpackedBytes += pNewValue->unpack(valBuf + unpackedBytes);
    }
    result = unpackedBytes;
  }
  return result;
}

/** No descriptions */
uint FPDynArrValue::pack(char* valBuf) const
{
  uint packedBytes = packBase(valBuf);
  uint arraySize(values_.size());
  memcpy(valBuf + packedBytes, (void *) &arraySize, sizeof(uint));
  packedBytes += sizeof(uint);
  for (FPValueArray::const_iterator iter = values_.begin();
       iter != values_.end(); ++iter)
  {
    packedBytes += (*iter)->pack(valBuf + packedBytes);
  }

  return packedBytes;
}

/** No descriptions */
FPValue* FPDynArrValue::clone() const
{
  FPValue* pNewValue = new FPDynArrValue(getType(), values_);
  return pNewValue;
}

/** No descriptions */
void FPDynArrValue::copy(const FPValue& newVal)
{
  if (newVal.getType() == getType())
  {
    setValue(((FPDynArrValue*)&newVal)->getValue());
  }
}

void FPDynArrValue::cleanup()
{
  for (FPValueArray::iterator iter = values_.begin();
       iter != values_.end(); ++iter)
  {
    delete *iter;  
  }
  values_.clear();
}
