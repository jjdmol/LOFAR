//#  GPI_PropertySet.cc: 
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

#include "GPI_PropertySet.h"
#include "GPI_SupervisoryServer.h"
#include <GCF/GCF_PValue.h>
#include <Utils.h>
#include <PI_Protocol.ph>

TPAResult convertPIToPAResult(TPIResult result)
{
  switch (result)
  {
    case PI_NO_ERROR: return PA_NO_ERROR;
    case PI_MISSING_PROPS: return PA_MISSING_PROPS;
    default: return PA_UNKNOWN_ERROR;
  }
}

TPIResult convertPAToPIResult(TPAResult result)
{
  switch (result)
  {
    case PA_NO_ERROR: return PI_NO_ERROR;
    case PA_MISSING_PROPS: return PI_MISSING_PROPS;
    default: return PI_UNKNOWN_ERROR;
  }
}

void GPIPropertySet::propSubscribed(const string& /*propName*/)
{
  _counter--;
  if (_counter == 0 && _tmpSubsList.size() == 0)
  {
    PAPropertiesLinkedEvent responseOut;
    responseOut.result = convertPIToPAResult(_tmpPIResult);
    responseOut.scope = _scope;
    _state = REGISTERED;
    _ss.getPAPort().send(responseOut);
  }
}

void GPIPropertySet::propValueChanged(const string& propName, const GCFPValue& value)
{
  PIValueChangedEvent indicationOut;
  indicationOut.scopeLength = _scope.length();
  indicationOut.name = propName;
  indicationOut.value._pValue = &value;
  _ss.getPort().send(indicationOut);
}


void GPIPropertySet::registerScope(PIRegisterScopeEvent& requestIn)
{
  PARegisterScopeEvent requestOut;
  requestOut.scope = requestIn.scope;
  forwardMsgToPA(requestOut);
}

void GPIPropertySet::registerCompleted(TPAResult result)
{
  PIScopeRegisteredEvent responseOut;
  responseOut.scope = _scope;
  responseOut.result = convertPAToPIResult(result);
  assert(_ss.getPort().isConnected());
  _ss.getPort().send(responseOut);
  if (result == PA_NO_ERROR)
  {    
    _state = REGISTERED;
  }
}

void GPIPropertySet::unregisterScope(PIUnregisterScopeEvent& requestIn)
{
  PAUnregisterScopeEvent requestOut;
  requestOut.scope = requestIn.scope;
  _state = UNREGISTERING;  
  forwardMsgToPA(requestOut);
}

void GPIPropertySet::unregisterCompleted(TPAResult result)
{
  assert(_state == UNREGISTERING);
  PIScopeUnregisteredEvent responseOut;
  responseOut.scope = _scope;
  responseOut.result = convertPAToPIResult(result);
  assert(_ss.getPort().isConnected());
  _ss.getPort().send(responseOut);
}

void GPIPropertySet::linkProperties(PALinkPropertiesEvent& e)
{
  assert(_state == REGISTERED || _state == UNREGISTERING);
  
  if (_state == REGISTERED)
  {
    PILinkPropertiesEvent requestOut;
    requestOut.scope = e.scope;
    requestOut.propList = e.propList;
    _state = LINKING;
    _ss.getPort().send(requestOut);    
  }
  else
  {
    LOFAR_LOG_TRACE(PI_STDOUT_LOGGER, ( 
        "Property set with scope %d is deleting in the meanwhile", 
        _scope.c_str()));
  }
}

bool GPIPropertySet::propertiesLinked(PIPropertiesLinkedEvent& responseIn)
{
  if (_state == LINKING)
  {    
    Utils::getPropertyListFromString(_tmpSubsList, responseIn.propList);
    assert(_counter == 0);
    if (responseIn.result != PI_PROP_SET_GONE)
    {
      _tmpPIResult = responseIn.result;
      return retrySubscriptions();
    }
  }
  return false;
}

bool GPIPropertySet::retrySubscriptions()
{  
  bool retry(true);
  if (_tmpSubsList.size() > 0 && _state == LINKING)
  {
    list<string>::iterator iter = _tmpSubsList.begin(); 
    while (iter != _tmpSubsList.end())
    {
      string fullName;
      if (_scope.length() > 0)
      {
        fullName = _scope + GCF_PROP_NAME_SEP + *iter;
      }
      else
      {
        fullName = *iter;
      }
      if (exists(fullName))
      {
        TGCFResult result = subscribeProp(fullName);
        _counter++;
        assert(result == GCF_NO_ERROR);        
        iter = _tmpSubsList.erase(iter);        
      }
      else
      {
        break;
      }
    }
    if (_counter == 0 && _tmpSubsList.size() == 0)
    {
      // no more asyncronous subscription responses will be expected and 
      // no more properties needed to be subscribed
      // so we can return a response to the PA      
      PAPropertiesLinkedEvent responseOut;
      responseOut.result = PA_NO_ERROR;
      responseOut.scope = _scope;
      _state = REGISTERED;
      _ss.getPAPort().send(responseOut);
      retry = false;
    }
  }
  else 
  {
    retry = false;
  }
  return retry;
}

void GPIPropertySet::unlinkProperties(PAUnlinkPropertiesEvent& requestIn)
{
  assert(_state == REGISTERED || _state == UNREGISTERING);
  
  if (_state == REGISTERED)
  {
    PIUnlinkPropertiesEvent requestOut;
    requestOut.scope = requestIn.scope;
    requestOut.propList = requestIn.propList;
    _state = UNLINKING;
    _ss.getPort().send(requestOut);
  }
  else
  {
    LOFAR_LOG_TRACE(PI_STDOUT_LOGGER, ( 
        "Property set with scope %d is deleting in the meanwhile", 
        _scope.c_str()));
  }
}

void GPIPropertySet::propertiesUnlinked(PIPropertiesUnlinkedEvent& responseIn)
{
  if (_state == UNLINKING)
  {
    Utils::getPropertyListFromString(_tmpSubsList, responseIn.propList);

    for (list<string>::iterator iter = _tmpSubsList.begin(); 
         iter != _tmpSubsList.end(); ++iter)
    {
      string fullName;
      if (_scope.length() > 0)
      {
        fullName = _scope + GCF_PROP_NAME_SEP + *iter;
      }
      else
      {
        fullName = *iter;
      }
      if (exists(fullName))
      {
        TGCFResult gcfResult = unsubscribeProp(fullName);
        assert(gcfResult == GCF_NO_ERROR);
      }
    }
    PAPropertiesUnlinkedEvent responseOut;    
    responseOut.result = convertPIToPAResult(responseIn.result);
    responseOut.scope = responseIn.scope;
    _state = REGISTERED;
    _ss.getPAPort().send(responseOut);
  }
}

void GPIPropertySet::forwardMsgToPA(GCFEvent& msg)
{
  if (_ss.getPAPort().isConnected())
  {
    _ss.getPAPort().send(msg);
  }
}
