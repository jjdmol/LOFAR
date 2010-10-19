//#  FPStringValue.cc: 
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

#include "FPStringValue.h"
#include <string.h>
/** No descriptions */
uint FPStringValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    uint stringLength(0);
    memcpy((void *) &stringLength, valBuf + unpackedBytes, sizeof(uint));
    value_.clear();
    value_.append(valBuf + unpackedBytes + sizeof(uint), stringLength); 
    result = sizeof(uint) + unpackedBytes + stringLength;
  }
  return result;
}

/** No descriptions */
uint FPStringValue::pack(char* valBuf) const
{
  uint packedBytes = packBase(valBuf);
  uint stringLength(value_.length());
  memcpy(valBuf + packedBytes, (void *) &stringLength, sizeof(uint));
  memcpy(valBuf + packedBytes + sizeof(uint), (void *) value_.c_str(), stringLength);

  return sizeof(uint) + packedBytes + stringLength;
}

/** No descriptions */
FPValue* FPStringValue::clone() const
{
  FPValue* pNewValue = new FPStringValue(value_);
  return pNewValue;
}

/** No descriptions */
void FPStringValue::copy(const FPValue& newVal)
{
  if (newVal.getType() == getType())
    value_ = ((FPStringValue *)&newVal)->getValue();
}
