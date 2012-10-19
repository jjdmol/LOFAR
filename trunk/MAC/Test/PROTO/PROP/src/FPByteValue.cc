//
//  FPByteValue.cc:
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

#include "FPByteValue.h"

/** No descriptions */
uint FPUCharValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    value_ = (uchar) valBuf[unpackedBytes];
    result = unpackedBytes + 1;
  }
  return result;
}

/** No descriptions */
uint FPUCharValue::pack(char* valBuf) const 
{
  uint packedBytes = packBase(valBuf);
  valBuf[packedBytes] = (char) value_;

  return packedBytes + 1;
}

/** No descriptions */
FPValue* FPUCharValue::clone() const
{
  FPValue* pNewValue = new FPUCharValue(value_);
  return pNewValue;
}                              

/** No descriptions */
void FPUCharValue::copy(const FPValue *pNewVal)
{
  if (pNewVal->getType() == getType())
    value_ = ((FPUCharValue *)pNewVal)->getValue();
}

/** No descriptions */
uint FPCharValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    value_ = valBuf[unpackedBytes];
    result = unpackedBytes + 1;
  }
  return result;
}

/** No descriptions */
uint FPCharValue::pack(char* valBuf) const
{
  uint packedBytes = packBase(valBuf);
  valBuf[packedBytes] = value_;

  return packedBytes + 1;
}

/** No descriptions */
FPValue* FPCharValue::clone() const
{
  FPValue* pNewValue = new FPCharValue(value_);
  return pNewValue;
}

/** No descriptions */
void FPCharValue::copy(const FPValue *pNewVal)
{
  if (pNewVal->getType() == getType())
    value_ = ((FPCharValue *)pNewVal)->getValue();
}

