//
//  FPBoolValue.cc:
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

#include "FPBoolValue.h"

/** No descriptions */
uint FPBoolValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    value_ = (valBuf[unpackedBytes] == 1 ? true : false);
    result = unpackedBytes + 1;
  }
  return result;
}

/** No descriptions */
uint FPBoolValue::pack(char* valBuf) const
{
  uint result(0);

  uint packedBytes = packBase(valBuf);
  valBuf[packedBytes] = (value_ ? 1 : 0);
  result = packedBytes + 1;

  return result;
}

/** No descriptions */
FPValue* FPBoolValue::clone() const
{
  FPValue* pNewValue = new FPBoolValue(value_);
  return pNewValue;
}

/** No descriptions */
void FPBoolValue::copy(const FPValue *pNewVal)
{
  if (pNewVal->getType() == getType())
    value_ = ((FPBoolValue *)pNewVal)->getValue();
}