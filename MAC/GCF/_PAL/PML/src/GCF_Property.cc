//#  GCF_Property.cc: 
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

#include <GCF/PAL/GCF_Property.h>
#include <GCF/PAL/GCF_PropertySet.h>
#include <GCF/PAL/GCF_Answer.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GPM_PropertyService.h>

namespace LOFAR 
{
 namespace GCF 
 {
using namespace Common;
using namespace TM;  
  namespace PAL
  {
GCFProperty::GCFProperty (const TPropertyInfo& propInfo, GCFPropertySet* pPropertySet) : 
  _isBusy(false),
  _pPropertySet(pPropertySet),
  _pAnswerObj(0),
  _pPropService(0),
  _propInfo(propInfo),
  _isIndependedProp(pPropertySet == 0)
{
  _pPropService = new GPMPropertyService(*this);
}


GCFProperty::~GCFProperty()
{
  assert (_isIndependedProp);
  if (exists()) // can already be deleted by the Property Agent
  {      
    unsubscribe();
  }
  delete _pPropService;
  _pPropService = 0;
}

const string GCFProperty::getFullName () const
{
  string fullName;
  if (_pPropertySet == 0)
  {
    if (_propInfo.propName.find(':') < _propInfo.propName.length())
    {
      fullName = _propInfo.propName;
    }
    else
    { 
      fullName = GCFPVSSInfo::getLocalSystemName() + ":" + _propInfo.propName;
    }
  }
  else
  {
    string scope = _pPropertySet->getFullScope();
    assert(scope.length() > 0);
    fullName = scope + GCF_PROP_NAME_SEP + _propInfo.propName;      
  }
  return fullName;
}

TGCFResult GCFProperty::requestValue ()
{ 
  assert(_pPropService);
  return _pPropService->requestPropValue(getFullName()); 
}

TGCFResult GCFProperty::setValue(const GCFPValue& value, bool wantAnswer)
{ 
  assert(_pPropService);
  return _pPropService->setPropValue(getFullName(), value, wantAnswer); 
}

TGCFResult GCFProperty::setValue(const string& value, bool wantAnswer)
{ 
  assert(_pPropService);
  GCFPValue* pValue = GCFPValue::createMACTypeObject(_propInfo.type);
  if (pValue) 
  { 
    pValue->setValue(value);
    return _pPropService->setPropValue(getFullName(), *pValue, wantAnswer); 
  }
  else 
  {
    return GCF_PROP_WRONG_TYPE;
  }
}

bool GCFProperty::exists () 
{ 
  assert(_pPropService);
  return GCFPVSSInfo::propExists(getFullName()); 
}
            
TGCFResult GCFProperty::subscribe ()
{ 
  assert(_pPropService);
  return _pPropService->subscribeProp(getFullName()); 
}
 
TGCFResult GCFProperty::unsubscribe ()
{ 
  assert(_pPropService);
  return _pPropService->unsubscribeProp(getFullName()); 
}
            
void GCFProperty::dispatchAnswer(GCFEvent& answer)
{
  if (_pAnswerObj != 0)
  {
    _pAnswerObj->handleAnswer(answer);
  }  
}

void GCFProperty::subscribed ()
{
  GCFPropAnswerEvent e(F_SUBSCRIBED);
  string fullName(getFullName());
  e.pPropName = fullName.c_str();
  dispatchAnswer(e);
}

void GCFProperty::valueChanged (const GCFPValue& value)
{
  GCFPropValueEvent e(F_VCHANGEMSG);
  e.pValue = &value;
  string fullName(getFullName());
  e.pPropName = fullName.c_str();
  e.internal = false;
  dispatchAnswer(e);
}

void GCFProperty::valueGet (const GCFPValue& value)
{
  GCFPropValueEvent e(F_VGETRESP);
  e.pValue = &value;
  string fullName(getFullName());
  e.pPropName = fullName.c_str();
  dispatchAnswer(e);
}

void GCFProperty::valueSet ()
{
  GCFPropAnswerEvent e(F_VSETRESP);
  string fullName(getFullName());
  e.pPropName = fullName.c_str();
  dispatchAnswer(e);
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
