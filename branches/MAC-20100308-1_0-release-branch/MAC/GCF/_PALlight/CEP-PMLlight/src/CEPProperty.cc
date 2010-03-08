//#  CEPProperty.cc: 
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

#include <GCF/PALlight/CEPProperty.h>
#include <GCF/PALlight/CEPPropertySet.h>

namespace LOFAR 
{
 namespace GCF 
 {
using namespace Common;
  namespace CEPPMLlight 
  {

CEPProperty::CEPProperty(const TPropertyInfo& propertyFields,
                         CEPPropertySet& propertySet) :
  _name(propertyFields.propName), 
  _propertySet(propertySet),
  _pValue(0)  
{
  _pValue = GCFPValue::createMACTypeObject((TMACValueType) propertyFields.type);
  ASSERT(_pValue);
}

CEPProperty::CEPProperty(CEPPropertySet& propertySet) :
  _name("dummy"),
  _propertySet(propertySet),
  _pValue(0)  
{
}

CEPProperty::~CEPProperty()
{
  if (_pValue)
    delete _pValue;
    
  _pValue = 0;
}

bool CEPProperty::setValue(const string& value)
{
  TGCFResult result(GCF_NO_ERROR);
  ASSERT(_pValue);

  result = _pValue->setValue(value);

  if (isMonitoringOn() && result == GCF_NO_ERROR)
  {
    _propertySet.valueSet(getFullName(), *_pValue);
  }
  
  return (result == GCF_NO_ERROR);
}

bool CEPProperty::setValue(const GCFPValue& value)
{
  TGCFResult result(GCF_NO_ERROR);
  ASSERT(_pValue);
  
  result = _pValue->copy(value);

  if (isMonitoringOn() && result == GCF_NO_ERROR)
  {
    _propertySet.valueSet(getFullName(), *_pValue);
  }
  
  return (result == GCF_NO_ERROR);
}

GCFPValue* CEPProperty::getValue() const
{
  if (_pValue) 
    return _pValue->clone();
  else 
    return 0;
}

const string CEPProperty::getFullName () const
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

bool CEPProperty::isMonitoringOn () const 
{ 
  return _propertySet.isMonitoringOn(); 
}

  } // namespace CEPPMLlight
 } // namespace GCF
} // namespace LOFAR
