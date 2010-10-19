//#  GCF_PVChar.cc: 
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


#include <GCF/GCF_PVChar.h>

unsigned int GCFPVChar::unpack(const char* valBuf)
{
  unsigned int result(0);
  unsigned int unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    _value = valBuf[unpackedBytes];
    result = unpackedBytes + 1;
  }
  return result;
}

/** No descriptions */
unsigned int GCFPVChar::pack(char* valBuf) const
{
  unsigned int result(0);
  unsigned int packedBytes = packBase(valBuf);
  if (packedBytes > 0)
  {
    valBuf[packedBytes] = _value;
    result = packedBytes + 1;
  }
  return result;
}

TGCFResult GCFPVChar::setValue(const string valueData)
{
  TGCFResult result(GCF_NO_ERROR);
  
  if (valueData.length() == 1)
  {
    _value = valueData[0];
  }
  else if (valueData.length() > 0)
  {
    char* validPos(0);
    long int value = strtol(valueData.c_str(), &validPos, 10);
    if (*validPos == '\0')
    {
      if (value <= 0xFF || value >= -127)
        _value = value;
      else
        result = GCF_VALUESTRING_NOT_VALID;      
    }
    else
      result = GCF_VALUESTRING_NOT_VALID;
  }
  else
    result = GCF_VALUESTRING_NOT_VALID;
  
  return result;
}

GCFPValue* GCFPVChar::clone() const
{
  GCFPValue* pNewValue = new GCFPVChar(_value);
  return pNewValue;
}

TGCFResult GCFPVChar::copy(const GCFPValue& newVal)
{
  TGCFResult result(GCF_NO_ERROR);

  if (newVal.getType() == getType())
    _value = ((GCFPVChar *)&newVal)->getValue();
  else
    result = GCF_DIFFERENT_TYPES;
  
  return result;
}
