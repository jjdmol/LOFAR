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
    PAPropertieslinkedEvent paPlE(0, PA_NO_ERROR);
    unsigned int scopeDataLength = Utils::packString(_scope, _buffer, MAX_BUF_SIZE);
    paPlE.result = convertPIToPAResult(_tmpPIResult);
    paPlE.length += scopeDataLength;
    _state = REGISTERED;
    _ss.getPAPort().send(paPlE, _buffer, scopeDataLength);
  }
}

void GPIPropertySet::propValueChanged(const string& propName, const GCFPValue& value)
{
  PIValuechangedEvent e(_scope.length());
  unsigned int dataLength = Utils::packString(propName, _buffer, MAX_BUF_SIZE);
  dataLength += value.pack(_buffer + dataLength, MAX_BUF_SIZE - dataLength);
  e.length += dataLength;
  _ss.getPort().send(e, _buffer, dataLength);
}


void GPIPropertySet::registerScope(GCFEvent& e)
{
  PARegisterscopeEvent pae(0);
  forwardMsgToPA(pae, e);
}

void GPIPropertySet::registerCompleted(TPAResult result)
{
  PIScoperegisteredEvent piSrE(PI_NO_ERROR);
  assert(_ss.getPort().isConnected());
  unsigned int scopeDataLength = Utils::packString(_scope, _buffer, MAX_BUF_SIZE);
  piSrE.result = convertPAToPIResult(result);
  piSrE.length += scopeDataLength;
  _ss.getPort().send(piSrE, _buffer, scopeDataLength);
  if (result == PA_NO_ERROR)
  {    
    _state = REGISTERED;
  }
}

void GPIPropertySet::unregisterScope(GCFEvent& e)
{
  PAUnregisterscopeEvent pae(0);
  forwardMsgToPA(pae, e);
  _state = UNREGISTERING;  
}

void GPIPropertySet::unregisterCompleted(TPAResult result)
{
  PIScopeunregisteredEvent piSurE(PI_NO_ERROR);
  assert(_ss.getPort().isConnected());
  assert(_state == UNREGISTERING);
  unsigned int scopeDataLength = Utils::packString(_scope, _buffer, MAX_BUF_SIZE);
  piSurE.result = convertPAToPIResult(result);
  piSurE.length += scopeDataLength;
  _ss.getPort().send(piSurE, _buffer, scopeDataLength);  
}

void GPIPropertySet::linkProperties(PALinkpropertiesEvent& e)
{
  assert(_state == REGISTERED || _state == UNREGISTERING);
  
  if (_state == REGISTERED)
  {
    GCFEvent piLpE(PI_LINKPROPERTIES);
    char* pData = ((char*)&e) + sizeof(PALinkpropertiesEvent);
    unsigned int dataLength = e.length - sizeof(PALinkpropertiesEvent);
    piLpE.length += dataLength;
    _state = LINKING;
    _ss.getPort().send(piLpE, pData, dataLength);    
  }
  else
  {
    LOFAR_LOG_TRACE(PI_STDOUT_LOGGER, ( 
        "Property set with scope %d is deleting in the meanwhile", 
        _scope.c_str()));
  }
}

bool GPIPropertySet::propertiesLinked(TPIResult result, char* pData)
{
  if (_state == LINKING)
  {
    Utils::unpackPropertyList(pData, _tmpSubsList);
    assert(_counter == 0);
    if (result != PI_PROP_SET_GONE)
    {
      _tmpPIResult = result;
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
      PAPropertieslinkedEvent paPlE(0, PA_NO_ERROR);
      unsigned int scopeDataLength = Utils::packString(_scope, _buffer, MAX_BUF_SIZE);
      paPlE.result = convertPIToPAResult(_tmpPIResult);
      paPlE.length += scopeDataLength;
      _state = REGISTERED;
      _ss.getPAPort().send(paPlE, _buffer, scopeDataLength);
      retry = false;
    }
  }
  else 
  {
    retry = false;
  }
  return retry;
}

void GPIPropertySet::unlinkProperties(PAUnlinkpropertiesEvent& e)
{
  assert(_state == REGISTERED || _state == UNREGISTERING);
  
  if (_state == REGISTERED)
  {
    GCFEvent piUlpE(PI_UNLINKPROPERTIES);
    char* pData = ((char*)&e) + sizeof(PAUnlinkpropertiesEvent);
    unsigned int dataLength = e.length - sizeof(PAUnlinkpropertiesEvent);
    piUlpE.length += dataLength;
    _state = UNLINKING;
    _ss.getPort().send(piUlpE, pData, dataLength);    
  }
  else
  {
    LOFAR_LOG_TRACE(PI_STDOUT_LOGGER, ( 
        "Property set with scope %d is deleting in the meanwhile", 
        _scope.c_str()));
  }
}

void GPIPropertySet::propertiesUnlinked(TPIResult result, char* pData)
{
  if (_state == UNLINKING)
  {
    Utils::unpackPropertyList(pData, _tmpSubsList);
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
        TGCFResult result = unsubscribeProp(fullName);
        assert(result == GCF_NO_ERROR);
      }
    }
    PAPropertiesunlinkedEvent paPulE(0, PA_NO_ERROR);
    unsigned int scopeDataLength = Utils::packString(_scope, _buffer, MAX_BUF_SIZE);
    paPulE.result = convertPIToPAResult(result);
    paPulE.length += scopeDataLength;
    _state = REGISTERED;
    _ss.getPAPort().send(paPulE, _buffer, scopeDataLength);
  }
}

void GPIPropertySet::forwardMsgToPA(GCFEvent& pae, GCFEvent& pie)
{
  if (_ss.getPAPort().isConnected())
  {
    unsigned int pieDataLength = pie.length - sizeof(GCFEvent);
    pae.length += pieDataLength;
    _ss.getPAPort().send(pae, ((char*)&pie) + sizeof(GCFEvent), pieDataLength);
  }
}

void GPIPropertySet::replyMsgToSS(GCFEvent& e, char* pScopeData)
{
  assert(_ss.getPort().isConnected());
  unsigned short scopeDataLength = Utils::getStringDataLength(pScopeData);
  e.length += scopeDataLength;
  _ss.getPort().send(e, pScopeData, scopeDataLength);
}

