//#  GCF_PVUnsigned.cc: 
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


#include <GCF/GCF_PVUnsigned.h>

unsigned int GCFPVUnsigned::unpack(const char* valBuf)
{
  unsigned int result(0);
  unsigned int unpackedBytes = unpackBase(valBuf);
  if (unpackedBytes > 0)
  {
    memcpy((void*) &_value, valBuf + unpackedBytes, sizeof(unsigned int));
    result = sizeof(unsigned int) + unpackedBytes;
  }
  return result;
}

unsigned int GCFPVUnsigned::pack(char* valBuf) const
{
  unsigned int result(0);
  unsigned int packedBytes = packBase(valBuf);
  if (packedBytes > 0)
  {
    memcpy(valBuf + packedBytes, (void*) &_value, sizeof(unsigned int));
    result = sizeof(unsigned int) + packedBytes;
  }
  return result;
}

TGCFResult GCFPVUnsigned::setValue(const string valueData)
{
  TGCFResult result(GCF_VALUESTRING_NOT_VALID);
  
  if (valueData.length() > 0)
  {
    char* validPos(0);
    long int value = strtol(valueData.c_str(), &validPos, 10);
    if (*validPos == '\0')
    {
      if (value >= 0)
      {
        _value = value;
        result = GCF_NO_ERROR;
      }
    }
  }
  
  return result;
}

GCFPValue* GCFPVUnsigned::clone() const
{
  GCFPValue* pNewValue = new GCFPVUnsigned(_value);
  return pNewValue;
}

TGCFResult GCFPVUnsigned::copy(const GCFPValue& newVal)
{
  TGCFResult result(GCF_NO_ERROR);

  if (newVal.getType() == getType())
    _value = ((GCFPVUnsigned *)&newVal)->getValue();
  else
    result = GCF_DIFFERENT_TYPES;
  
  return result;
}
