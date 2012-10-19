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


#include <lofar_config.h>

#include <GCF/PVSS/GCF_PVChar.h>
#include <Common/StringUtil.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace PVSS 
  {

unsigned int GCFPVChar::unpackConcrete(const char* valBuf)
{
  _value = valBuf[0];
  return 1;
}

/** No descriptions */
unsigned int GCFPVChar::packConcrete(char* valBuf) const
{
  valBuf[0] = _value;
  return 1;
}

TGCFResult GCFPVChar::setValue(const string& valueData)
{
  TGCFResult result(GCF_VALUESTRING_NOT_VALID);
  
  if (valueData.length() == 1)
  {
    _value = valueData[0];
    result = GCF_NO_ERROR;
  }
  else if (valueData.length() > 0)
  {
    char* validPos(0);
    long int value = strtol(valueData.c_str(), &validPos, 10);
    if (*validPos == '\0')
    {
      if (value <= 0xFF || value >= -127)
      {
        _value = value;
        result = GCF_NO_ERROR;
      }
    }
  }
  
  return result;
}

string GCFPVChar::getValueAsString(const string& format) const
{
  string retVal;
  if (format.length() == 0)
  {
    retVal += _value;
  }
  else
  {
    retVal = formatString(format.c_str(), _value);
  }
  return retVal;
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
  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
