//#  GCF_PropertyProxy.cc: 
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

#include <GCF/PAL/GCF_PropertyProxy.h>
#include <GPM_PropertyProxy.h>
#include <GCF/PAL/GCF_PVSSInfo.h>

namespace LOFAR {
 namespace GCF {
using namespace Common;
  namespace PAL {

//
// GCFPropertyProxy()
//
GCFPropertyProxy::GCFPropertyProxy() :
	_pPMProxy(0)
{
	_pPMProxy = new GPMPropertyProxy(*this);
}


//
// ~GCFPropertyProxy()
//
GCFPropertyProxy::~GCFPropertyProxy()
{
	delete _pPMProxy;
	_pPMProxy = 0;
}

//
// subscribeProp(propName)
//
TGCFResult GCFPropertyProxy::subscribeProp(const string& propName)
{
	return (_pPMProxy->subscribePM(propName) == SA_NO_ERROR ? 
													GCF_NO_ERROR : GCF_PML_ERROR);
}

//
// unsubscribeProp(propName)
//
TGCFResult GCFPropertyProxy::unsubscribeProp(const string& propName)
{
	return (_pPMProxy->unsubscribePM(propName) == SA_NO_ERROR ? 
													GCF_NO_ERROR : GCF_PML_ERROR);
}

//
// requestPropValue(propName)
//
TGCFResult GCFPropertyProxy::requestPropValue(const string& propName)
{
	return (_pPMProxy->getPM(propName) == SA_NO_ERROR ? GCF_NO_ERROR : GCF_PML_ERROR);
}

//
// setPropValueTimed(propName,value, timestamp, wantAnswer)
//
TGCFResult GCFPropertyProxy::setPropValueTimed(const string& propName, 
                                               const GCFPValue& value, 
                                               double timestamp, 
                                               bool wantAnswer)
{
	return (_pPMProxy->setPM(propName, value, timestamp, wantAnswer) == SA_NO_ERROR ? 
														GCF_NO_ERROR : GCF_PML_ERROR);
}

//
// setPropValueTimed(propName,valueStr, timestamp, wantAnswer)
//
TGCFResult GCFPropertyProxy::setPropValueTimed(const string& propName, 
                                               const string& value, 
                                               double timestamp, 
                                               bool wantAnswer)
{
	GCFPValue* pValue = GCFPValue::createMACTypeObject(GCFPVSSInfo::getMACTypeId(propName));
	if (!pValue) {  
		return GCF_PML_ERROR;
	}

	pValue->setValue(value);

	return (_pPMProxy->setPM(propName, *pValue, timestamp, wantAnswer) == SA_NO_ERROR ?
															GCF_NO_ERROR : GCF_PML_ERROR);
}

//
// dpQuerySubscribeSingle(queryWhere, queryFrom)
//
TGCFResult GCFPropertyProxy::dpQuerySubscribeSingle(const string& queryWhere, const string& queryFrom)
{
	return (_pPMProxy->dpQuerySubscribeSinglePM(queryWhere, queryFrom) == SA_NO_ERROR ? 
														GCF_NO_ERROR : GCF_PML_ERROR);
}

//
// GCFPropertyProxy::dpQueryUnsubscribe(queryID)
//
TGCFResult GCFPropertyProxy::dpQueryUnsubscribe(uint32 queryId)
{
	return (_pPMProxy->dpQueryUnsubscribePM(queryId) == SA_NO_ERROR ? 
														GCF_NO_ERROR : GCF_PML_ERROR);
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
