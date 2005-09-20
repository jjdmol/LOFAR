//
//  FPLongLongValue.cc:
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

#include "FPLongLongValue.h"
#include <cstring>

/** No descriptions */
uint FPULongLongValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    memcpy((void *) &value_, valBuf + unpackedBytes, sizeof(ulonglong));
    result = sizeof(ulonglong) + unpackedBytes;
  }
  return result;
}

/** No descriptions */
uint FPULongLongValue::pack(char* valBuf) const
{
  uint packedBytes = packBase(valBuf);
  memcpy(valBuf + packedBytes, (void *) &value_, sizeof(ulonglong));
  return sizeof(ulonglong) + packedBytes;
}

/** No descriptions */
FPValue* FPULongLongValue::clone() const
{
  FPValue* pNewValue = new FPULongLongValue(value_);
  return pNewValue;
}

/** No descriptions */
void FPULongLongValue::copy(const FPValue *pNewVal)
{
  if (pNewVal->getType() == getType())
    value_ = ((FPULongLongValue *)pNewVal)->getValue();
}

/** No descriptions */
uint FPLongLongValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    memcpy((void *) &value_, valBuf + unpackedBytes, sizeof(longlong));
    result = sizeof(longlong) + unpackedBytes;
  }
  return result;
}

/** No descriptions */                                 
uint FPLongLongValue::pack(char* valBuf) const
{
  uint packedBytes = packBase(valBuf);
  memcpy(valBuf + packedBytes, (void *) &value_, sizeof(longlong));
  return sizeof(longlong) + packedBytes;
}

/** No descriptions */
FPValue* FPLongLongValue::clone() const
{
  FPValue* pNewValue = new FPLongLongValue(value_);
  return pNewValue;
}

/** No descriptions */
void FPLongLongValue::copy(const FPValue *pNewVal)
{
  if (pNewVal->getType() == getType())
    value_ = ((FPLongLongValue *)pNewVal)->getValue();
}
