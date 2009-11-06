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

#include <lofar_config.h>

#include <GCF/PAL/GCF_Property.h>
#include <GCF/PAL/GCF_PropertySet.h>
#include <GCF/PAL/GCF_Answer.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GPM_PropertyService.h>

namespace LOFAR {
 namespace GCF {
 using namespace Common;
 using namespace TM;  
  namespace PAL {

//
// GCFProperty(propInfo, PS*)
//
GCFProperty::GCFProperty (const TPropertyInfo& propInfo, GCFPropertySet* pPropertySet) : 
	_isBusy		 (false),
	_pPropertySet(pPropertySet),
	_pAnswerObj	 (0),
	_pPropService(0),
	_propInfo	 (propInfo),
	_isIndependedProp(pPropertySet == 0)
{
	_pPropService = new GPMPropertyService(*this);
}


//
// ~GCFProperty()
//
GCFProperty::~GCFProperty()
{
	ASSERT (_isIndependedProp);

	if (exists()) { // can already be deleted by the Property Agent
		unsubscribe();
	}
	delete _pPropService;
	_pPropService = 0;
}

//
// getFullName()
//
const string GCFProperty::getFullName () const
{
	string fullName;
	if (_pPropertySet == 0) {	// Prop not attached to PS?
		// Make sure the systemname is in the propName.
		if (_propInfo.propName.find(':') < _propInfo.propName.length()) {
			fullName = _propInfo.propName;
		}
		else { 
			fullName = GCFPVSSInfo::getLocalSystemName() + ":" + _propInfo.propName;
		}
	}
	else {
		string scope = _pPropertySet->getFullScope();
		ASSERT(scope.length() > 0);
		fullName = scope + GCF_PROP_NAME_SEP + _propInfo.propName;      
	}

	return fullName;
}

//
// requestValue()
//
TGCFResult GCFProperty::requestValue ()
{ 
	ASSERT(_pPropService);
	return _pPropService->requestPropValue(getFullName()); 
}

//
// setValueTimed(value, timestamp, wantAnswer)
//
TGCFResult GCFProperty::setValueTimed(const GCFPValue& value, double timestamp, bool wantAnswer)
{ 
	ASSERT(_pPropService);
	return _pPropService->setPropValue(getFullName(), value, timestamp, wantAnswer); 
}

//
// setValueTimed(string, timestamp, wantAnswer)
//
TGCFResult GCFProperty::setValueTimed(const string& value, double timestamp, bool wantAnswer)
{ 
	ASSERT(_pPropService);

	TGCFResult result(GCF_NO_ERROR);
	GCFPValue* pValue = GCFPValue::createMACTypeObject(_propInfo.type);
	if (!pValue) { 
		return (GCF_PROP_WRONG_TYPE);
	}

	if ((result = pValue->setValue(value)) == GCF_NO_ERROR) {
		result = _pPropService->setPropValue(getFullName(), *pValue, timestamp, wantAnswer);
	}
	return result;
}

//
// exists()
//
bool GCFProperty::exists () 
{ 
  ASSERT(_pPropService);
  return GCFPVSSInfo::propExists(getFullName()); 
}
            
//
// subscribe()
//
TGCFResult GCFProperty::subscribe ()
{ 
  ASSERT(_pPropService);
  return _pPropService->subscribeProp(getFullName()); 
}
 
//
// unsubscribe()
//
TGCFResult GCFProperty::unsubscribe ()
{ 
  ASSERT(_pPropService);
  return _pPropService->unsubscribeProp(getFullName()); 
}
            
//
// dispatchAnswer
//
void GCFProperty::dispatchAnswer(GCFEvent& answer)
{
	if (_pAnswerObj != 0) {
		_pAnswerObj->handleAnswer(answer);
	}  
}

//
// subscribed()
//
void GCFProperty::subscribed ()
{
	GCFPropAnswerEvent e(F_SUBSCRIBED);
	string fullName(getFullName());
	e.pPropName = fullName.c_str();
	dispatchAnswer(e);
}

//
// valueChanged()
//
void GCFProperty::valueChanged (const GCFPValue& value)
{
	GCFPropValueEvent e(F_VCHANGEMSG);
	e.pValue = &value;
	string fullName(getFullName());
	e.pPropName = fullName.c_str();
	e.internal = false;
	dispatchAnswer(e);
}

//
// valueGet()
//
void GCFProperty::valueGet (const GCFPValue& value)
{
	GCFPropValueEvent e(F_VGETRESP);
	e.pValue = &value;
	string fullName(getFullName());
	e.pPropName = fullName.c_str();
	dispatchAnswer(e);
}

//
// valueSet()
//
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
