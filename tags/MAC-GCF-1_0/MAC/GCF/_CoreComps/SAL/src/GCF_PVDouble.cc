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


#include "GCF_PVDouble.h"

/** No descriptions */
TGCFResult GCFPVDouble::setValue(const string valueData)
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

/** No descriptions */
GCFPValue* GCFPVDouble::clone() const
{
  GCFPValue* pNewValue = new GCFPVDouble(_value);
  return pNewValue;
}

/** No descriptions */
TGCFResult GCFPVDouble::copy(const GCFPValue& newVal)
{
  TGCFResult result(GCF_NO_ERROR);

  if (newVal.getType() == getType())
    _value = ((GCFPVDouble *)&newVal)->getValue();
  else
    result = GCF_DIFFERENT_TYPES;
  
  return result;
}
