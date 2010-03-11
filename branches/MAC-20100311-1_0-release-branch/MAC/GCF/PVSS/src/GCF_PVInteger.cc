//#  GCF_PVInteger.cc: 
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

#include <GCF/PVSS/GCF_PVInteger.h>
#include <Common/DataConvert.h>
#include <Common/StringUtil.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace PVSS 
  {

unsigned int GCFPVInteger::unpackConcrete(const char* valBuf)
{
  memcpy((void*) &_value, valBuf, sizeof(int32));
  if (mustConvert()) LOFAR::dataConvert(LOFAR::dataFormat(), &_value, 1);
  return sizeof(int32);
}

unsigned int GCFPVInteger::packConcrete(char* valBuf) const
{
  memcpy(valBuf, (void*) &_value, sizeof(int32));
  return sizeof(int32);
}

TGCFResult GCFPVInteger::setValue(const string& valueData)
{
  TGCFResult result(GCF_VALUESTRING_NOT_VALID);
  
  if (valueData.length() > 0)
  {
    char* validPos(0);
    long int value = strtol(valueData.c_str(), &validPos, 10);
    if (*validPos == '\0')
    {
      _value = value;
      result = GCF_NO_ERROR;
    }
  }
  
  return result;
}

string GCFPVInteger::getValueAsString(const string& format) const
{
  string retVal;
  if (format.length() == 0)
  {
    retVal = formatString("%d", _value);
  }
  else
  {
    retVal = formatString(format.c_str(), _value);
  }
  return retVal;
}

GCFPValue* GCFPVInteger::clone() const
{
  GCFPValue* pNewValue = new GCFPVInteger(_value);
  return pNewValue;
}

TGCFResult GCFPVInteger::copy(const GCFPValue& newVal)
{
  TGCFResult result(GCF_NO_ERROR);

  if (newVal.getType() == getType())
    _value = ((GCFPVInteger *)&newVal)->getValue();
  else
    result = GCF_DIFFERENT_TYPES;
  
  return result;
}
  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
