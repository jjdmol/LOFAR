//#  FPIntegerValue.cc: 
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

#include "FPIntegerValue.h"
#include <string.h>

/** No descriptions */
uint FPIntegerValue::unpack(const char* valBuf)
{
  uint result(0);
  uint unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    memcpy((void*) &value_, valBuf + unpackedBytes, sizeof(int));
    result = sizeof(int) + unpackedBytes;
  }
  return result;
}

/** No descriptions */
uint FPIntegerValue::pack(char* valBuf) const
{
  uint packedBytes = packBase(valBuf);
  memcpy(valBuf + packedBytes, (void*) &value_, sizeof(int));
  return sizeof(int) + packedBytes;
}

/** No descriptions */
FPValue* FPIntegerValue::clone() const
{
  FPValue* pNewValue = new FPIntegerValue(value_);
  return pNewValue;
}

/** No descriptions */
void FPIntegerValue::copy(const FPValue& newVal)
{
  if (newVal.getType() == getType())
    value_ = ((FPIntegerValue *)&newVal)->getValue();
}
