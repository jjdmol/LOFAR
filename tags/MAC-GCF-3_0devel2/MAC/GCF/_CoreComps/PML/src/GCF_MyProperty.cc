//#  GCF_MyProperty.cc: 
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

#include <GCF/GCF_MyProperty.h>
#include <GCF/GCF_MyPropertySet.h>
#include <GCF/GCF_Answer.h>

GCFMyProperty::GCFMyProperty(const TProperty& propertyFields,
                             GCFMyPropertySet& propertySet) :
  GCFPropertyBase(propertyFields.propName, &propertySet),
  _accessMode(propertyFields.accessMode),
  _changingAccessMode(false),
  _pCurValue(0),
  _pOldValue(0),
  _isLinked(false),
  _propertySet(propertySet),
  _isBusy(false)  
{
  assert(!exists());
  _pCurValue = GCFPValue::createMACTypeObject((GCFPValue::TMACValueType) propertyFields.type);
  assert(_pCurValue);
  _pOldValue = _pCurValue->clone();
  if (propertyFields.defaultValue)
  {
    _pCurValue->setValue(propertyFields.defaultValue);
  }
}

GCFMyProperty::~GCFMyProperty()
{
  if (_pOldValue)
    delete _pOldValue;
    
  if (_pCurValue)
    delete _pCurValue;
    
  _pOldValue = 0;
  _pCurValue = 0;
}

TGCFResult GCFMyProperty::setValue(const string value)
{
  TGCFResult result(GCF_NO_ERROR);
  if (!_pOldValue || !_pCurValue) 
    result = GCF_PROP_NOT_VALID;
  else if (_pOldValue->copy(*_pCurValue) != GCF_NO_ERROR)
    result = GCF_PROP_WRONG_TYPE;
  else result = _pCurValue->setValue(value);

  if ((_accessMode & GCF_READABLE_PROP) && exists() && result == GCF_NO_ERROR)
  {
    assert(_pCurValue);
    result = GCFPropertyBase::setValue(*_pCurValue);    
    assert(result == GCF_NO_ERROR);
  }
  
  return result;
}

TGCFResult GCFMyProperty::setValue(const GCFPValue& value)
{
  TGCFResult result(GCF_NO_ERROR);
  if (!_pOldValue || !_pCurValue) 
    result = GCF_PROP_NOT_VALID;
  else if (_pOldValue->copy(*_pCurValue) != GCF_NO_ERROR)
    result = GCF_PROP_WRONG_TYPE;
  else if (_pCurValue->copy(value) != GCF_NO_ERROR)
    result = GCF_PROP_WRONG_TYPE;

  if ((_accessMode & GCF_READABLE_PROP) && exists() && result == GCF_NO_ERROR)
  {
    assert(_pCurValue);
    result = GCFPropertyBase::setValue(*_pCurValue);    
    assert(result == GCF_NO_ERROR);
  }
  
  return result;
}

GCFPValue* GCFMyProperty::getValue() const
{
  if (_pCurValue) 
    return _pCurValue->clone();
  else 
    return 0;
}

GCFPValue* GCFMyProperty::getOldValue() const
{
  if (_pOldValue) 
    return _pOldValue->clone();
  else 
    return 0;
}

bool GCFMyProperty::link()
{
  bool isAsync(false);
  assert(!_isLinked);  

  TGCFResult result(GCF_NO_ERROR);
  assert(exists());
  assert(!_isBusy);
  if (_accessMode & GCF_READABLE_PROP)
  {
    assert(_pCurValue);
    result = GCFPropertyBase::setValue(*_pCurValue);    
    assert(result == GCF_NO_ERROR);
  }
  if (_accessMode & GCF_WRITABLE_PROP )
  {
    _changingAccessMode = false;
    result = subscribe();
    assert(result == GCF_NO_ERROR);
    isAsync = true;
    _isBusy = true;
  }
  if (!isAsync)
    _isLinked = true;
  return isAsync;
}

void GCFMyProperty::unlink()
{
  assert(_isLinked);
  assert(!_isBusy);
  if (_accessMode & GCF_WRITABLE_PROP )
  {
    if (exists()) // can already be deleted by the Property Agent
    {      
      unsubscribe();
    }
  }
  _isLinked = false;
}

void GCFMyProperty::setAccessMode(TAccessMode mode, bool on)
{
  TAccessMode oldAccessMode(_accessMode);
  if (on)
    _accessMode |= mode;
  else
    _accessMode &= ~mode;
  
  TGCFResult result(GCF_NO_ERROR);
  if ((_accessMode & GCF_WRITABLE_PROP) && 
      (~oldAccessMode & GCF_WRITABLE_PROP) &&
      _isLinked)
  {
    assert(!_isBusy);
    _isBusy = true;
    _changingAccessMode = true;
    result = subscribe();
    assert(result == GCF_NO_ERROR);    
  }
  else if ((oldAccessMode & GCF_WRITABLE_PROP) &&
      (~_accessMode & GCF_WRITABLE_PROP) && 
      _isLinked)
  {
    result = unsubscribe();
    assert(result == GCF_NO_ERROR);    
  }
  
  if ((~_accessMode & GCF_WRITABLE_PROP) && 
      (~oldAccessMode & GCF_READABLE_PROP) &&
      (_accessMode & GCF_READABLE_PROP) &&
      _isLinked)
  {
    assert(_pCurValue);
    result = GCFPropertyBase::setValue(*_pCurValue);    
    assert(result == GCF_NO_ERROR);
  }  
}

bool GCFMyProperty::testAccessMode(TAccessMode mode) const
{
  return (_accessMode & mode); 
}

void GCFMyProperty::subscribed ()
{
  assert(_isBusy);
  _isBusy = false;  
  if (!_changingAccessMode)
  {
    assert(!_isLinked);
    _isLinked = true;
    _propertySet.linked();
  }
}

void GCFMyProperty::valueGet (const GCFPValue& value)
{
  valueChanged(value);
}
                               
void GCFMyProperty::valueChanged (const GCFPValue& value)
{
  if (_accessMode & GCF_WRITABLE_PROP )
  {
    TGCFResult result;
    assert(_pOldValue && _pCurValue);
    result = _pOldValue->copy(*_pCurValue);
    assert(result == GCF_NO_ERROR);
    result = _pCurValue->copy(value);
    assert(result == GCF_NO_ERROR);
    
    GCFPropValueEvent e(F_VCHANGEMSG_SIG);
    e.pValue = &value;
    string fullName(getFullName());
    e.pPropName = fullName.c_str();
    e.internal = true;
    dispatchAnswer(e);
  }
}
