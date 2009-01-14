//#  GCF_PVDouble.cc: 
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

#include <GCF/PVSS/GCF_PVDouble.h>
#include <Common/DataConvert.h>
#include <Common/StringUtil.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace PVSS 
  {

unsigned int GCFPVDouble::unpackConcrete(const char* valBuf)
{
  memcpy((void *) &_value, valBuf, sizeof(double));
  
  if (mustConvert()) LOFAR::dataConvert(LOFAR::dataFormat(), &_value, 1);
  
  return sizeof(double);
}

unsigned int GCFPVDouble::packConcrete(char* valBuf) const
{
  memcpy(valBuf, (void *) &_value, sizeof(double));
  return sizeof(double);
}

TGCFResult GCFPVDouble::setValue(const string& valueData)
{
  TGCFResult result(GCF_VALUESTRING_NOT_VALID);

  if (valueData.length() > 0)
  {
    char* validPos(0);
    double value = strtod(valueData.c_str(), &validPos);
    if (*validPos == '\0')
    {
      _value = value;
      result = GCF_NO_ERROR;
    }
  }
  
  return result;
}

string GCFPVDouble::getValueAsString(const string& format) const
{
  string retVal;
  if (format.length() == 0)
  {
    retVal = formatString("%f", _value);
  }
  else
  {
    retVal = formatString(format.c_str(), _value);
  }
  return retVal;
}

GCFPValue* GCFPVDouble::clone() const
{
  GCFPValue* pNewValue = new GCFPVDouble(_value);
  return pNewValue;
}

TGCFResult GCFPVDouble::copy(const GCFPValue& newVal)
{
  TGCFResult result(GCF_NO_ERROR);

  if (newVal.getType() == getType())
    _value = ((GCFPVDouble *)&newVal)->getValue();
  else
    result = GCF_DIFFERENT_TYPES;
  
  return result;
}
  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
