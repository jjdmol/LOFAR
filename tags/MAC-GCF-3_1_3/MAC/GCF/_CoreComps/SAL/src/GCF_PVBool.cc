//#  GCF_PVBool.cc: 
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


#include <GCF/GCF_PVBool.h>

unsigned int GCFPVBool::unpack(const char* valBuf)
{
  unsigned int result(0);
  unsigned int unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    _value = (valBuf[unpackedBytes] == 1 ? true : false);
    result = unpackedBytes + 1;
  }
  return result;
}

unsigned int GCFPVBool::pack(char* valBuf) const
{
  unsigned int result(0);
  unsigned int packedBytes = packBase(valBuf);
  if (packedBytes > 0)
  {
    valBuf[packedBytes] = (_value ? 1 : 0);
    result = packedBytes + 1;
  }
  return result;
}
TGCFResult GCFPVBool::setValue(const string valueData)
{
  TGCFResult result(GCF_NO_ERROR);

  if (valueData.length() > 0)
  {
    char* validPos(0);
    long int value = strtol(valueData.c_str(), &validPos, 10);
    if (*validPos == '\0')
    {
      _value = (value != 0);
    }
    else if (validPos == valueData.c_str())
    {
      if ((strncasecmp(valueData.c_str(), "false", 5) == 0) || 
          (strncasecmp(valueData.c_str(), "no", 2) == 0) ||
          (strncasecmp(valueData.c_str(), "off", 3) == 0))
      {
        _value = false; 
      }
      else 
      if ((strncasecmp(valueData.c_str(), "true", 5) == 0) || 
          (strncasecmp(valueData.c_str(), "yes", 2) == 0) ||
          (strncasecmp(valueData.c_str(), "on", 3) == 0))
      {
        _value = true;
      }
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

GCFPValue* GCFPVBool::clone() const
{
  GCFPValue* pNewValue = new GCFPVBool(_value);
  return pNewValue;
}

TGCFResult GCFPVBool::copy(const GCFPValue& newVal)
{
  TGCFResult result(GCF_NO_ERROR);

  if (newVal.getType() == getType())
    _value = ((GCFPVBool *)&newVal)->getValue();
  else
    result = GCF_DIFFERENT_TYPES;
  
  return result;
}
