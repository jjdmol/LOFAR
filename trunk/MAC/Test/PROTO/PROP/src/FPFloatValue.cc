//
//  FPFloatValue.cc:
//
//  Copyright (C) 2003
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//

#include "FPFloatValue.h"
#include <cstring>

/** No descriptions */
uint FPFloatValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    memcpy((void *) &value_, valBuf + unpackedBytes, sizeof(float));
    result = sizeof(float) + unpackedBytes;
  }
  return result;
}

/** No descriptions */
uint FPFloatValue::pack(char* valBuf) const
{
  uint packedBytes = packBase(valBuf);
  memcpy(valBuf + packedBytes, (void *) &value_, sizeof(float));

  return sizeof(float) + packedBytes;
}

/** No descriptions */
FPValue* FPFloatValue::clone() const
{
  FPValue* pNewValue = new FPFloatValue(value_);
  return pNewValue;
}

/** No descriptions */
void FPFloatValue::copy(const FPValue *pNewVal)
{
  if (pNewVal == 0) return;
  if (pNewVal->getType() == getType())
    value_ = ((FPFloatValue *)pNewVal)->getValue();
}


/** No descriptions */
uint FPComplexFloatValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    memcpy((void *) value_, valBuf + unpackedBytes, 2 * sizeof(float));
    result = 2 * sizeof(float) + unpackedBytes;
  }
  return result;
}

/** No descriptions */
uint FPComplexFloatValue::pack(char* valBuf) const
{
  uint packedBytes = packBase(valBuf);
  memcpy(valBuf + packedBytes, (void *) value_, 2 * sizeof(float));

  return 2 * sizeof(float) + packedBytes;
}

/** No descriptions */
FPValue* FPComplexFloatValue::clone() const
{
  FPValue* pNewValue = new FPComplexFloatValue(value_);
  return pNewValue;
}

/** No descriptions */
void FPComplexFloatValue::copy(const FPValue *pNewVal)
{
  const float *newVal;
  if (pNewVal == 0) return;
  if (pNewVal->getType() == getType())
  {
    newVal = ((FPComplexFloatValue *)pNewVal)->getValue();
    value_[0] = newVal[0];
    value_[1] = newVal[1];    
  }
}

