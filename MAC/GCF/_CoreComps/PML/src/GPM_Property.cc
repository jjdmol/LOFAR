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
#include <SAL/GCF_PVBool.h>

GPMProperty::GPMProperty(GCFPValue::TMACValueType type, string name) :
  _name(name), _isLinked(false)
{
  _pCurValue = createValue(type);
  _pOldValue = createValue(type);
}

GPMProperty::GPMProperty(TProperty& propertyFields)
{
  _name = propertyFields.name;
  _pCurValue = createValue(propertyFields.type);
  _pOldValue = createValue(propertyFields.type);
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

TPMResult GPMProperty::setValue(GCFPValue& value)
{
  TPMResult result(PM_NO_ERROR);
  if (!_pOldValue || !_pCurValue) 
    result = PM_PROP_NOT_VALID;
  else if (_pOldValue->copy(*_pNewValue) != SA_NO_ERROR)
    result = PM_PROP_WRONG_TYPE;
  else if (_pNewValue->copy(value) != SA_NO_ERROR)
    result = PM_PROP_WRONG_TYPE;
  
  return result;
}

TPMResult GPMProperty::getValue(GCFPValue& value, bool curValue) const
{
  TPMResult result(PM_NO_ERROR);
  if (curValue && _pCurValue) 
  {
    if (value.copy(*_pCurValue) != SA_NO_ERROR)
      result = PM_PROP_WRONG_TYPE;
  }
  else if (!curValue && _pOldValue)
  {
    if (value.copy(*_pOldValue) != SA_NO_ERROR)
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

GCFPValue* GPMProperty::createValue(GCFPValue::TMACValueType type) const
{
  GCFPValue* pResult(0);
  
  switch (macValue.getType())
  {
    case GCFPValue::BOOL_VAL:
      pResult = new GCFPVBool();
      break;
/*    case GCFPValue::BIT32_VAL:
      *pVar = new GCFPVBit32();
      break;
    case GCFPValue::CHAR_VAL:
      *pVar = new GCFPVChar();
      break;
    case GCFPValue::UNSIGNED_VAL:
      *pVar = new GCFPVUnsigned();
      break;
    case GCFPValue::INTEGER_VAL:
      *pVar = new GCFVPInteger();
      break;
    case GCFPValue::FLOAT_VAL:
      *pVar = new GCFPVFloat();
      break;
    case GCFPValue::STRING_VAL:
      *pVar = new GCFPVString();
      break;
    case GCFPValue::REF_VAL:
      *pVar = new GCFPVRef();
      break;
    case GCFPValue::BLOB_VAL:
      *pVar = new GCFPVBlob();
      break;
    case GCFPValue::DATETIME_VAL:
      *pVar = new GCFPVDateTime();
      break;*/
    default:
      LOFAR_LOG_ERROR(PML_STDOUT_LOGGER, (
          "Type of MAC value is unknown or not supported yet: '%d'", 
          macValue.getType()));
      break;
  }  
  
  return pResult;
}
