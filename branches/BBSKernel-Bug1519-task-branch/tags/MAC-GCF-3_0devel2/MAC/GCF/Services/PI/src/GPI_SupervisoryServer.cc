//#  GPI_SupervisoryServer.cc: 
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

#include "GPI_SupervisoryServer.h"
#include "GPI_Controller.h"
#include "GPI_PropertySet.h"
#include <Utils.h>
#include <GCF/GCF_PValue.h>

#define DECLARE_SIGNAL_NAMES
#include "PI_Protocol.ph"
#include <PA_Protocol.ph>

static string sSSTaskName("PI");

GPISupervisoryServer::GPISupervisoryServer(GPIController& controller) : 
  GCFTask((State)&GPISupervisoryServer::initial, sSSTaskName),
  _controller(controller)
{
  // register the protocols for debugging purposes only
  registerProtocol(PI_PROTOCOL, PI_PROTOCOL_signalnames);
  registerProtocol(PA_PROTOCOL, PA_PROTOCOL_signalnames);

  // initialize the port to the real supervisory server 
  _ssPort.init(*this, "server", GCFPortInterface::SPP, PI_PROTOCOL);
  // initialize the port to the the Property Agent (acts as a PML)
  _propertyAgent.init(*this, "client", GCFPortInterface::SAP, PA_PROTOCOL);
}

GPISupervisoryServer::~GPISupervisoryServer()
{
}

GCFEvent::TResult GPISupervisoryServer::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      _controller.getPortProvider().accept(_ssPort);
      // intential fall through
    case F_TIMER_SIG:
      _propertyAgent.open();
      break;

    case F_CONNECTED_SIG:
      if (_ssPort.isConnected())
        TRAN(GPISupervisoryServer::operational);
      break;

    case F_DISCONNECTED_SIG:
      p.setTimer(1.0); // try again after 1 second
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GPISupervisoryServer::operational(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static string scope = "";
  static char* pData = 0;
  static GPIPropertySet* pPropertySet = 0;
  static int timerID = -1;

  switch (e.signal)
  {      
    case F_CONNECTED_SIG:
      if (&p == &_propertyAgent)
      {
        PARegisterscopeEvent rse(0);
        for (TScopeRegister::iterator iter = _scopeRegister.begin();
               iter != _scopeRegister.end(); ++iter)
        {
          rse.length = sizeof(GCFEvent) + Utils::packString(iter->first, _buffer, MAX_BUF_SIZE);
          _propertyAgent.send(rse, _buffer, rse.length - sizeof(GCFEvent));
        }
      }  
      break;

    case F_DISCONNECTED_SIG:
      if (&_propertyAgent == &p)
      {
        p.setTimer(1.0); // try again after 1 second
      }
      else
      {
        p.close(); // to avoid more DISCONNECTED signals for this port
        TRAN(GPISupervisoryServer::closing);
      }
      break;

    case F_TIMER_SIG:
      if (&_propertyAgent == &p)
      {          
        p.open(); // try again
      }
      else
      {
        bool retryAgain(false);
        for (TScopeRegister::iterator iter = _scopeRegister.begin();
             iter != _scopeRegister.end(); ++iter)
        {
          if (iter->second->retrySubscriptions())
          {
            retryAgain = true;
          }
        }
        if (retryAgain) _ssPort.setTimer(0.0);
        else            timerID = -1;
      }
      break;
     
    case PI_REGISTERSCOPE:
    {
      pData = ((char*)&e) + sizeof(GCFEvent);
      if (!findPropertySet(pData, scope))
      {
        pPropertySet = new GPIPropertySet(*this, scope);
        _scopeRegister[scope] = pPropertySet;
        pPropertySet->registerScope(e);
      }
      else
      {
        PIScoperegisteredEvent response(PI_SCOPE_ALREADY_REGISTERED);
        unsigned int scopeDataLength = Utils::packString(scope, _buffer, MAX_BUF_SIZE);
        response.length += scopeDataLength;
        _ssPort.send(response, _buffer, scopeDataLength);
      }      

      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "SS-REQ: Register scope %s",
          scope.c_str()));
      break;
    }  
    case PA_SCOPEREGISTERED:
    {      
      PAScoperegisteredEvent* pResponse = static_cast<PAScoperegisteredEvent*>(&e);
      assert(pResponse);
      pData = ((char*)&e) + sizeof(PAScoperegisteredEvent);
      pPropertySet = findPropertySet(pData, scope);

      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "PA-RESP: Scope %s is registered", 
          scope.c_str()));

      if (pPropertySet)
      {
        pPropertySet->registerCompleted(pResponse->result);
      }
      else
      {
        // in case a scope is gone due to a crash in a controller application on 
        // the RTC
        if (pResponse->result == PA_NO_ERROR)
        {
          GCFEvent paUrsE(PA_UNREGISTERSCOPE);
          unsigned int scopeDataLength = scope.length() + Utils::SLEN_FIELD_SIZE;
          paUrsE.length += scopeDataLength;
          _propertyAgent.send(paUrsE, pData, scopeDataLength);
        }        
        LOFAR_LOG_TRACE(PI_STDOUT_LOGGER, ( 
            "Property set with scope %d was deleted in the meanwhile", 
            scope.c_str()));
      }
      break;
    }

    case PI_UNREGISTERSCOPE:
    {
      pData = ((char*)&e) + sizeof(GCFEvent);
      pPropertySet = findPropertySet(pData, scope);

      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "SS-REQ: Unregister scope %s",
          scope.c_str()));

      assert(pPropertySet);
      pPropertySet->unregisterScope(e);
      break;
    }  

    case PA_SCOPEUNREGISTERED:
    {
      PAScopeunregisteredEvent* pResponse = static_cast<PAScopeunregisteredEvent*>(&e);
      assert(pResponse);
      pData = ((char*)&e) + sizeof(PAScopeunregisteredEvent);
      pPropertySet = findPropertySet(pData, scope);

      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "PA-RESP: Scope %s is unregistered", 
          scope.c_str()));
          
      if (pPropertySet)
      {
        pPropertySet->unregisterCompleted(pResponse->result);
        delete pPropertySet;
      }
      _scopeRegister.erase(scope);
      break;
    }

    case PA_LINKPROPERTIES:
    {
      PALinkpropertiesEvent* pRequest = static_cast<PALinkpropertiesEvent*>(&e);
      assert(pRequest);
      pData = ((char*)&e) + sizeof(PALinkpropertiesEvent);
      pPropertySet = findPropertySet(pData, scope);
      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "PA-REQ: Link properties on scope %s", 
          scope.c_str()));
      if (pPropertySet)
      {
        pPropertySet->linkProperties(*pRequest);
      }
      else
      {
        PAPropertieslinkedEvent response(0, PA_PROP_SET_GONE);
        replyMsgToPA(response, pData);
        LOFAR_LOG_TRACE(PI_STDOUT_LOGGER, ( 
            "Property set with scope %d was deleted in the meanwhile", 
            scope.c_str()));
      }
      break;
    }
    case PI_PROPERTIESLINKED:
    {
      PIPropertieslinkedEvent* pResponse = static_cast<PIPropertieslinkedEvent*>(&e);
      assert(pResponse);
      pData = ((char*)&e) + sizeof(PIPropertieslinkedEvent);
      pPropertySet = findPropertySet(pData, scope);
      pData += Utils::getStringDataLength(pData);
      if (pPropertySet)
      {
        bool mustRetry = pPropertySet->propertiesLinked(pResponse->result, pData);
        if (mustRetry && timerID == -1)
        {
          timerID = _ssPort.setTimer(0.0);
        }        
      }
      break;
    }
    case PA_UNLINKPROPERTIES:
    {
      PAUnlinkpropertiesEvent* pRequest = static_cast<PAUnlinkpropertiesEvent*>(&e);
      assert(pRequest);
      pData = ((char*)&e) + sizeof(PAUnlinkpropertiesEvent);
      pPropertySet = findPropertySet(pData, scope);
      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "PA-REQ: Unlink properties on scope %s", 
          scope.c_str()));
      if (pPropertySet)
      {
        pPropertySet->unlinkProperties(*pRequest);
      }
      else
      {
        PAPropertiesunlinkedEvent response(0, PA_PROP_SET_GONE);
        replyMsgToPA(response, pData);
        LOFAR_LOG_TRACE(PI_STDOUT_LOGGER, ( 
            "Property set with scope %d was deleted in the meanwhile", 
            scope.c_str()));
      }
      break;
    }

    case PI_PROPERTIESUNLINKED:
    {
      PIPropertiesunlinkedEvent* pResponse = static_cast<PIPropertiesunlinkedEvent*>(&e);
      assert(pResponse);
      pData = ((char*)&e) + sizeof(PIPropertiesunlinkedEvent);
      pPropertySet = findPropertySet(pData, scope);
      pData += Utils::getStringDataLength(pData);
      if (pPropertySet)
      {
        pPropertySet->propertiesUnlinked(pResponse->result, pData); 
      }
      break;
    }
    
    case PI_VALUESET:
    {
      string propName;
      pData = ((char*)&e) + sizeof(GCFEvent);
      unsigned int propDataLength = Utils::unpackString(pData, propName);
      unsigned int valueBufLength = e.length - propDataLength - sizeof(GCFEvent);
      GCFPValue* pValue = GCFPValue::unpackValue(pData + propDataLength, valueBufLength); 
      if (pValue)
      {
        _controller.getPropertyProxy().setPropValue(propName, *pValue);
        delete pValue;
      }
      break;                                          
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GPISupervisoryServer::closing(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      _propertyAgent.close();
      break;
      
    case F_DISCONNECTED_SIG:      
      if (&p == &_propertyAgent)
      {
        _controller.close(*this);
      }
      break;
     
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GPIPropertySet* GPISupervisoryServer::findPropertySet(char* pScopeData, string& scope)
{
  if (pScopeData != 0)
  {
    Utils::unpackString(pScopeData, scope);
  }
  TScopeRegister::iterator iter = _scopeRegister.find(scope);  
  return (iter != _scopeRegister.end() ? iter->second : 0);
}

void GPISupervisoryServer::replyMsgToPA(GCFEvent& e, const string& scope)
{
  if (_propertyAgent.isConnected())
  {    
    unsigned int scopeDataLength = Utils::packString(scope, _buffer, MAX_BUF_SIZE);
    e.length += scopeDataLength;
    _propertyAgent.send(e, _buffer, scopeDataLength);
  }
}
