//#  GCF_RTMyProperty.cc: 
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

#include <GCF/PALlight/GCF_RTMyProperty.h>
#include <GCF/PALlight/GCF_RTMyPropertySet.h>
#include <GCF/PALlight/GCF_RTAnswer.h>

namespace LOFAR 
{
 namespace GCF 
 {
using namespace Common;
using namespace TM;
  namespace RTCPMLlight 
  {
GCFRTMyProperty::GCFRTMyProperty(const TPropertyInfo& propertyFields,
                             GCFRTMyPropertySet& propertySet) :
  _name(propertyFields.propName), 
  _propertySet(propertySet),
  _accessMode(GCF_READWRITE_PROP),
  _pCurValue(0),
  _pOldValue(0),
  _isLinked(false),
  _isBusy(false),
  _pAnswerObj(0)  
{
  _pCurValue = Common::GCFPValue::createMACTypeObject((TMACValueType) propertyFields.type);
  ASSERT(_pCurValue);
  _pOldValue = _pCurValue->clone();
}

GCFRTMyProperty::GCFRTMyProperty(GCFRTMyPropertySet& propertySet) :
  _name(""),
  _propertySet(propertySet),
  _accessMode(0),
  _pCurValue(0),
  _pOldValue(0),
  _isLinked(false),
  _isBusy(false),
  _pAnswerObj(0)  
{
}

GCFRTMyProperty::~GCFRTMyProperty()
{
  if (_pOldValue)
    delete _pOldValue;
    
  if (_pCurValue)
    delete _pCurValue;
    
  _pOldValue = 0;
  _pCurValue = 0;
}

TGCFResult GCFRTMyProperty::setValue(const string& value)
{
  TGCFResult result(GCF_NO_ERROR);
  if (!_pOldValue || !_pCurValue) 
    result = GCF_PROP_NOT_VALID;
  else if (_pOldValue->copy(*_pCurValue) != GCF_NO_ERROR)
    result = GCF_PROP_WRONG_TYPE;
  else result = _pCurValue->setValue(value);

  if ((_accessMode & GCF_READABLE_PROP) && _isLinked && result == GCF_NO_ERROR)
  {
    ASSERT(_pCurValue);
    _propertySet.valueSet(getFullName(), *_pCurValue);
  }
  
  return result;
}

TGCFResult GCFRTMyProperty::setValue(const GCFPValue& value)
{
  TGCFResult result(GCF_NO_ERROR);
  if (!_pOldValue || !_pCurValue) 
    result = GCF_PROP_NOT_VALID;
  else if (_pOldValue->copy(*_pCurValue) != GCF_NO_ERROR)
    result = GCF_PROP_WRONG_TYPE;
  else if (_pCurValue->copy(value) != GCF_NO_ERROR)
    result = GCF_PROP_WRONG_TYPE;

  if ((_accessMode & GCF_READABLE_PROP) && _isLinked && result == GCF_NO_ERROR)
  {
    ASSERT(_pCurValue);
    _propertySet.valueSet(getFullName(), *_pCurValue);
  }
  
  return result;
}

GCFPValue* GCFRTMyProperty::getValue() const
{
  if (_pCurValue) 
    return _pCurValue->clone();
  else 
    return 0;
}

GCFPValue* GCFRTMyProperty::getOldValue() const
{
  if (_pOldValue) 
    return _pOldValue->clone();
  else 
    return 0;
}

void GCFRTMyProperty::link()
{
  ASSERT(!_isLinked);  

  if (_accessMode & GCF_READABLE_PROP)
  {
    ASSERT(_pCurValue);
    _propertySet.valueSet(getFullName(), *_pCurValue);
  }
  _isLinked = true;
}

void GCFRTMyProperty::unlink()
{
  ASSERT(_isLinked);
  ASSERT(!_isBusy);
  _isLinked = false;
}

void GCFRTMyProperty::setAccessMode(TAccessMode mode, bool on)
{
  TAccessMode oldAccessMode(_accessMode);
  if (on)
    _accessMode |= mode;
  else
    _accessMode &= ~mode;
  
  if ((~_accessMode & GCF_WRITABLE_PROP) && 
      (~oldAccessMode & GCF_READABLE_PROP) &&
      (_accessMode & GCF_READABLE_PROP) &&
      _isLinked)
  {
    ASSERT(_pCurValue);
    _propertySet.valueSet(getFullName(), *_pCurValue);    
  }  
}

bool GCFRTMyProperty::testAccessMode(TAccessMode mode) const
{
  return (_accessMode & mode); 
}

void GCFRTMyProperty::valueChanged (const GCFPValue& value)
{
  if (_accessMode & GCF_WRITABLE_PROP )
  {
    TGCFResult result;
    ASSERT(_pOldValue && _pCurValue);
    result = _pOldValue->copy(*_pCurValue);
    ASSERT(result == GCF_NO_ERROR);
    result = _pCurValue->copy(value);
    ASSERT(result == GCF_NO_ERROR);
    
    GCFPropValueEvent e(F_VCHANGEMSG);
    e.pValue = &value;
    string fullName(getFullName());
    e.pPropName = fullName.c_str();
    e.internal = true;
    dispatchAnswer(e);
  }
}

const string GCFRTMyProperty::getFullName () const
{
  string scope = _propertySet.getScope();
  if (scope.length() == 0)
  {
    return _name;
  }
  else
  {
    string fullName = scope + GCF_PROP_NAME_SEP + _name;
    return fullName;
  }
}

void GCFRTMyProperty::dispatchAnswer(GCFEvent& answer)
{
  if (_pAnswerObj != 0)
  {
    _pAnswerObj->handleAnswer(answer);
  }  
}
  } // namespace RTCPMLlight
 } // namespace GCF
} // namespace LOFAR
