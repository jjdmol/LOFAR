//#  GCF_PVDynArr.cc: 
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


#include <GCF/GCF_PVDynArr.h>

GCFPVDynArr::GCFPVDynArr(TMACValueType itemType, const GCFPValueArray& val) :
  GCFPValue((TMACValueType) (LPT_DYNARR | itemType))
{
  assert(itemType != LPT_DYNARR);
  setValue(val);
}

GCFPVDynArr::GCFPVDynArr(TMACValueType itemType) :
  GCFPValue((TMACValueType) (LPT_DYNARR | itemType))
{
  assert(itemType != LPT_DYNARR);
}

GCFPVDynArr::~GCFPVDynArr()
{
  cleanup();
}

/** No descriptions */
TGCFResult GCFPVDynArr::setValue(const string value)
{
  TGCFResult result(GCF_NO_ERROR);
  
  return result;
}

void GCFPVDynArr::setValue(const GCFPValueArray& newVal)
{
  cleanup();
  for (GCFPValueArray::const_iterator iter = newVal.begin();
       iter != newVal.end(); ++iter)
  {
    if ((*iter)->getType() == (getType() & ~LPT_DYNARR))
      _values.push_back((*iter)->clone());
  }
}

/** No descriptions */
GCFPValue* GCFPVDynArr::clone() const
{
  GCFPValue* pNewValue = new GCFPVDynArr(getType(), _values);
  return pNewValue;
}

/** No descriptions */
TGCFResult GCFPVDynArr::copy(const GCFPValue& newVal)
{
  TGCFResult result(GCF_NO_ERROR);

  if (newVal.getType() == getType())
    setValue(((GCFPVDynArr*)&newVal)->getValue());
  else
    result = GCF_DIFFERENT_TYPES;
  
  return result;
}

void GCFPVDynArr::cleanup()
{
  for (GCFPValueArray::iterator iter = _values.begin();
       iter != _values.end(); ++iter)
  {
    delete *iter;  
  }
  _values.clear();
}
