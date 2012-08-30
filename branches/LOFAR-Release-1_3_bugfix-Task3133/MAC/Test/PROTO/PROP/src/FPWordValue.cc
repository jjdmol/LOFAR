//
//  FPWordValue.cc:
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

#include "FPWordValue.h"
#include <cstring>

/** No descriptions */
uint FPUShortValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    memcpy((void*) &value_, valBuf + unpackedBytes, sizeof(ushort));
    result = sizeof(ushort) + unpackedBytes;
  }
  return result;
}

/** No descriptions */
uint FPUShortValue::pack(char* valBuf) const
{
  uint packedBytes = packBase(valBuf);
  memcpy(valBuf + packedBytes, (void*) &value_, sizeof(ushort));
  return sizeof(ushort) + packedBytes;
}

/** No descriptions */
FPValue* FPUShortValue::clone() const
{
  FPValue* pNewValue = new FPUShortValue(value_);
  return pNewValue;
}

/** No descriptions */
void FPUShortValue::copy(const FPValue *pNewVal)
{
  if (pNewVal->getType() == getType())
    value_ = ((FPUShortValue *)pNewVal)->getValue();
}


/** No descriptions */
uint FPShortValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    memcpy((void*) &value_, valBuf + unpackedBytes, sizeof(short));
    result = sizeof(short) + unpackedBytes;
  }
  return result;
}

/** No descriptions */
uint FPShortValue::pack(char* valBuf) const
{
  uint packedBytes = packBase(valBuf);
  memcpy(valBuf + packedBytes, (void*) &value_, sizeof(short));
  return sizeof(short) + packedBytes;
}

/** No descriptions */
FPValue* FPShortValue::clone() const
{
  FPValue* pNewValue = new FPShortValue(value_);
  return pNewValue;
}

/** No descriptions */
void FPShortValue::copy(const FPValue *pNewVal)
{
  if (pNewVal->getType() == getType())
    value_ = ((FPShortValue *)pNewVal)->getValue();
}
