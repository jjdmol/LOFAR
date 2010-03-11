//
//  FPDWordValue.cc:
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

#include "FPDWordValue.h"
#include <cstring>

/** No descriptions */
uint FPULongValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    memcpy((void*) &value_, valBuf + unpackedBytes, sizeof(ulong));
    result = sizeof(ulong) + unpackedBytes;
  }
  return result;
}

/** No descriptions */
uint FPULongValue::pack(char* valBuf) const
{
  uint packedBytes = packBase(valBuf);
  memcpy(valBuf + packedBytes, (void*) &value_, sizeof(ulong));
  return sizeof(ulong) + packedBytes;
}

/** No descriptions */
FPValue* FPULongValue::clone() const
{
  FPValue* pNewValue = new FPULongValue(value_);
  return pNewValue;
}

/** No descriptions */
void FPULongValue::copy(const FPValue *pNewVal)
{
  if (pNewVal->getType() == getType())
    value_ = ((FPULongValue *)pNewVal)->getValue();
}


/** No descriptions */
uint FPLongValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    memcpy((void*) &value_, valBuf + unpackedBytes, sizeof(long));
    result = sizeof(long) + unpackedBytes;
  }
  return result;
}

/** No descriptions */
uint FPLongValue::pack(char* valBuf) const
{
  uint packedBytes = packBase(valBuf);
  memcpy(valBuf + packedBytes, (void*) &value_, sizeof(long));
  return sizeof(long) + packedBytes;
}

/** No descriptions */
FPValue* FPLongValue::clone() const
{
  FPValue* pNewValue = new FPLongValue(value_);
  return pNewValue;
}

/** No descriptions */
void FPLongValue::copy(const FPValue *pNewVal)
{
  if (pNewVal->getType() == getType())
    value_ = ((FPLongValue *)pNewVal)->getValue();
}
