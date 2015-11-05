//
//  FPDoubleValue.cc:
//
//  Copyright (C) 2003
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
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

#include "FPDoubleValue.h"
#include <cstring>

/** No descriptions */
uint FPDoubleValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    memcpy((void*) &value_, valBuf + unpackedBytes, sizeof(double));
    result = sizeof(double) + unpackedBytes;
  }
  return result;
}

/** No descriptions */
uint FPDoubleValue::pack(char* valBuf) const
{
  uint packedBytes = packBase(valBuf);
  memcpy(valBuf + packedBytes, (void *) &value_, sizeof(double));

  return sizeof(double) + packedBytes;
}

/** No descriptions */
FPValue* FPDoubleValue::clone() const
{
  FPValue* pNewValue = new FPDoubleValue(value_);
  return pNewValue;
}

/** No descriptions */
void FPDoubleValue::copy(const FPValue *pNewVal)
{
  if (pNewVal == 0) return;
  if (pNewVal->getType() == getType())
    value_ = ((FPDoubleValue *)pNewVal)->getValue();
}

/** No descriptions */
uint FPComplexDoubleValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    memcpy((void*) &value_[0], valBuf + unpackedBytes, sizeof(double));
    memcpy((void*) &value_[1], valBuf + unpackedBytes + sizeof(double), sizeof(double));
    result = 2 * sizeof(double) + unpackedBytes;
  }
  return result;
}

/** No descriptions */
uint FPComplexDoubleValue::pack(char* valBuf) const
{
  uint packedBytes = packBase(valBuf);
  memcpy(valBuf + packedBytes, (void *) value_, 2 * sizeof(double));

  return 2 * sizeof(double) + packedBytes;
}

/** No descriptions */
FPValue* FPComplexDoubleValue::clone() const
{
  FPValue* pNewValue = new FPComplexDoubleValue(value_);
  return pNewValue;
}

/** No descriptions */
void FPComplexDoubleValue::copy(const FPValue *pNewVal)
{
  const double *newVal;
  
  if (pNewVal == 0) return;
  if (pNewVal->getType() == getType())
  {
    newVal = ((FPComplexDoubleValue *)pNewVal)->getValue();
    value_[0] = newVal[0];
    value_[1] = newVal[1];
  }
}

