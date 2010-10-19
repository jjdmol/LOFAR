//#  GPM_Property.cc: 
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

#include "GPM_Property.h"
#include <SAL/GCF_PValue.h>

GPMProperty::GPMProperty(GCFPValue::TMACValueType type, string name) :
  _isLinked(false), _name(name)
{
  _pCurValue = GCFPValue::createMACTypeObject(type);  
  assert(_pCurValue);
  _pOldValue = _pCurValue->clone();
}

GPMProperty::GPMProperty(const TProperty& propertyFields) :
  _isLinked(false)
{
  _name = propertyFields.propName;
  _pCurValue = GCFPValue::createMACTypeObject((GCFPValue::TMACValueType) propertyFields.type);
  assert(_pCurValue);
  if (propertyFields.defaultValue)
  {
    _pCurValue->setValue(propertyFields.defaultValue);
  }
  _pOldValue = _pCurValue->clone();
  _accessMode = propertyFields.accessMode;
}

GPMProperty::~GPMProperty()
{
  if (_pOldValue)
    delete _pOldValue;
    
  if (_pCurValue)
    delete _pCurValue;
    
  _pOldValue = 0;
  _pCurValue = 0;
}

TPMResult GPMProperty::setValue(const GCFPValue& value)
{
  TPMResult result(PM_NO_ERROR);
  if (!_pOldValue || !_pCurValue) 
    result = PM_PROP_NOT_VALID;
  else if (_pOldValue->copy(*_pCurValue) != GCF_NO_ERROR)
    result = PM_PROP_WRONG_TYPE;
  else if (_pCurValue->copy(value) != GCF_NO_ERROR)
    result = PM_PROP_WRONG_TYPE;
  
  return result;
}

TPMResult GPMProperty::getValue(GCFPValue& value, bool curValue) const
{
  TPMResult result(PM_NO_ERROR);
  if (curValue && _pCurValue) 
  {
    if (value.copy(*_pCurValue) != GCF_NO_ERROR)
      result = PM_PROP_WRONG_TYPE;
  }
  else if (!curValue && _pOldValue)
  {
    if (value.copy(*_pOldValue) != GCF_NO_ERROR)
      result = PM_PROP_WRONG_TYPE;
  }
  else
  {    
    result = PM_PROP_NOT_VALID;
  }
  
  return result;
}

GCFPValue* GPMProperty::getValue(bool curValue) const
{
  if (curValue && _pCurValue) 
    return _pCurValue->clone();
  else if (!curValue && _pOldValue) 
    return _pOldValue->clone();
  else 
    return 0;
}

TPMResult GPMProperty::setLink(bool linkSwitch)
{
  TPMResult result(PM_NO_ERROR);
 
  if (linkSwitch == _isLinked)
    result = PM_PROP_LINK_NOT_IN_SYNC;
  else
    _isLinked = linkSwitch;
  
  return result;
}

void GPMProperty::setAccessMode(TAccessMode mode, bool on)
{
  if (on)
    _accessMode |= mode;
  else
    _accessMode &= ~mode;        
}

bool GPMProperty::testAccessMode(TAccessMode mode) const
{
  return (_accessMode & mode); 
}
