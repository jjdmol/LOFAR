//#  GPM_PropertyProxy.cc: 
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

#include "GPM_PropertyProxy.h"
#include "GCF_PropertyProxy.h"

GPMPropertyProxy::GPMPropertyProxy(GCFPropertyProxy& gcfProxy) : 
  _gcfProxy(gcfProxy) 
{}

GPMPropertyProxy::~GPMPropertyProxy() 
{}

TPMResult GPMPropertyProxy::subscribePM(const string& propName)
{
  return (GSAService::subscribe(propName) == SA_NO_ERROR ? PM_NO_ERROR : PM_SCADA_ERROR);
}

TPMResult GPMPropertyProxy::unsubscribePM(const string& propName)
{
  return (GSAService::unsubscribe(propName) == SA_NO_ERROR ? PM_NO_ERROR : PM_SCADA_ERROR);
}

TPMResult GPMPropertyProxy::getPM(const string& propName)
{
  return (GSAService::get(propName) == SA_NO_ERROR ? PM_NO_ERROR : PM_SCADA_ERROR);
}

TPMResult GPMPropertyProxy::setPM(const string& propName, const GCFPValue& value)
{
  return (GSAService::set(propName, value) == SA_NO_ERROR ? PM_NO_ERROR : PM_SCADA_ERROR);
}

bool GPMPropertyProxy::existsPM(const string& propName)
{
  return GSAService::exists(propName);
}

void GPMPropertyProxy::propSubscribed(const string& propName)
{
  _gcfProxy.propSubscribed(propName);
}

void GPMPropertyProxy::propUnsubscribed(const string& propName)
{
  _gcfProxy.propUnsubscribed(propName);
}

void GPMPropertyProxy::propValueGet(const string& propName, const GCFPValue& value)
{
  _gcfProxy.propValueGet(propName, value);
}

void GPMPropertyProxy::propValueChanged(const string& propName, const GCFPValue& value)
{
  _gcfProxy.propValueChanged(propName, value);
}

void GPMPropertyProxy::propCreated(const string& /*propName*/) 
{ }
void GPMPropertyProxy::propDeleted(const string& /*propName*/) 
{ }
