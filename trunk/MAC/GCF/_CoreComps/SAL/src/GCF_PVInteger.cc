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


#include "GCF_PVInteger.h"

/** No descriptions */
TSAResult GCFPVInteger::setValue(const string valueData)
{
  TSAResult result(SA_VALUESTRING_NOT_VALID);
  
  if (valueData.length() > 0)
  {
    char* validPos(0);
    long int value = strtol(valueData.c_str(), &validPos, 10);
    if (*validPos == '\0')
    {
      _value = value;
      result = SA_NO_ERROR;
    }
  }
  
  return result;
}

/** No descriptions */
GCFPValue* GCFPVInteger::clone() const
{
  GCFPValue* pNewValue = new GCFPVInteger(_value);
  return pNewValue;
}

/** No descriptions */
TSAResult GCFPVInteger::copy(const GCFPValue& newVal)
{
  TSAResult result(SA_NO_ERROR);

  if (newVal.getType() == getType())
    _value = ((GCFPVInteger *)&newVal)->getValue();
  else
    result = SA_DIFFERENT_TYPES;
  
  return result;
}
