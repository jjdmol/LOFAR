//#  FPValue.cc: 
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

#include "FPValue.h"
#include "FPCharValue.h"
#include "FPBoolValue.h"
#include "FPIntegerValue.h"
#include "FPUnsignedValue.h"
#include "FPStringValue.h"
#include "FPDoubleValue.h"
#include "FPDynArrValue.h"

#include <string.h>

uint FPValue::unpackBase(const char* valBuf)
{
  assert(type_ == (ValueType) valBuf[0]);
  memcpy((void*)&setTime_, valBuf + 1, sizeof(timeval));
  return sizeof(timeval) + 1;
}

uint FPValue::packBase(char* valBuf) const
{
  valBuf[0] = type_;
  memcpy(valBuf + 1, (void*)&setTime_, sizeof(timeval));
  return sizeof(timeval) + 1;
}

FPValue* FPValue::createValueObject(ValueType type)
{
  FPValue* pValue(0);
  switch (type)
  {
    case BOOL_VAL:
      pValue = new FPBoolValue();
      break;
    case CHAR_VAL:
      pValue = new FPCharValue();
      break;
    case INTEGER_VAL:
      pValue = new FPIntegerValue();
      break;
    case UNSIGNED_VAL:
      pValue = new FPUnsignedValue();
      break;
    case STRING_VAL:
      pValue = new FPStringValue();
      break;
    case DOUBLE_VAL:
      pValue = new FPDoubleValue();
      break;
    default:
      if (type > DYNARR_VAL && type <= DYNDATETIME_VAL)
        pValue = new FPDynArrValue(type);
      break;
  }
  return pValue;
}
