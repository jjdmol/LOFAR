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
#include <GPM_PropertyService.h>

GCFProperty::GCFProperty (const TProperty& propInfo, GCFPropertySet* pPropertySet) : 
  _isBusy(false),
  _pPropertySet(pPropertySet),
  _pAnswerObj(0),
  _pPropService(0),
  _propInfo(propInfo)
{
  _pPropService = new GPMPropertyService(*this);
}


GCFProperty::~GCFProperty()
{
  assert (_pPropertySet == 0);
  delete _pPropService;
  _pPropService = 0;
}

const string GCFProperty::getFullName () const
{
  if (_pPropertySet == 0)
  {
    return _name;
  }
  else
  {
    string scope = _pPropertySet->getScope();
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
}
TGCFResult GCFProperty::requestValue ()
{ 
  assert(_pPropService);
  return _pPropService->requestPropValue(getFullName()); 
}

TGCFResult GCFProperty::setValue(const GCFPValue& value)
{ 
  assert(_pPropService);
  return _pPropService->setPropValue(getFullName(), value); 
}

bool GCFProperty::exists () 
{ 
  assert(_pPropService);
  return _pPropService->existsProp(getFullName()); 
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
  e.pPropName = _name.c_str();
  dispatchAnswer(e);
}

void GCFProperty::valueChanged (const GCFPValue& value)
{
  GCFPropValueEvent e(F_VCHANGEMSG);
  e.pValue = &value;
  e.pPropName = _name.c_str();
  e.internal = false;
  dispatchAnswer(e);
}

void GCFProperty::valueGet (const GCFPValue& value)
{
  GCFPropValueEvent e(F_VGETRESP);
  e.pValue = &value;
  e.pPropName = _name.c_str();
  dispatchAnswer(e);
}
