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
#include "GPI_PValue.h"

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
    case F_INIT:
      break;

    case F_ENTRY:
      _controller.getPortProvider().accept(_ssPort);
      // intential fall through
    case F_TIMER:
      _propertyAgent.open();
      break;

    case F_CONNECTED:
      if (_ssPort.isConnected())
        TRAN(GPISupervisoryServer::operational);
      break;

    case F_DISCONNECTED:
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
  static GPIPropertySet* pPropertySet = 0;
  static int timerID = -1;

  switch (e.signal)
  {      
    case F_CONNECTED:
      if (&p == &_propertyAgent)
      {
        PARegisterScopeEvent requestOut;
        for (TScopeRegister::iterator iter = _scopeRegister.begin();
               iter != _scopeRegister.end(); ++iter)
        {
          requestOut.scope = iter->first;
          _propertyAgent.send(requestOut);
        }
      }  
      break;

    case F_DISCONNECTED:
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

    case F_TIMER:
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
     
    case PI_REGISTER_SCOPE:
    {
      PIRegisterScopeEvent requestIn(e);
      if (!findPropertySet(requestIn.scope))
      {
        pPropertySet = new GPIPropertySet(*this, requestIn.scope);
        _scopeRegister[requestIn.scope] = pPropertySet;
        pPropertySet->registerScope(requestIn);
      }
      else
      {
        PIScopeRegisteredEvent responseOut;
        responseOut.result = PI_SCOPE_ALREADY_REGISTERED;
        responseOut.scope = requestIn.scope;
        _ssPort.send(responseOut);
      }      

      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "SS-REQ: Register scope %s",
          requestIn.scope.c_str()));
      break;
    }  
    case PA_SCOPE_REGISTERED:
    {      
      PAScopeRegisteredEvent responseIn(e);

      pPropertySet = findPropertySet(responseIn.scope);

      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "PA-RESP: Scope %s is registered", 
          responseIn.scope.c_str()));

      if (pPropertySet)
      {
        pPropertySet->registerCompleted(responseIn.result);
      }
      else
      {
        // in case a scope is gone due to a crash in a controller application on 
        // the RTC
        if (responseIn.result == PA_NO_ERROR)
        {
          PAUnregisterScopeEvent requestOut;
          requestOut.scope = responseIn.scope;
          _propertyAgent.send(requestOut);
        }        
        LOFAR_LOG_TRACE(PI_STDOUT_LOGGER, ( 
            "Property set with scope %d was deleted in the meanwhile", 
            responseIn.scope.c_str()));
      }
      break;
    }

    case PI_UNREGISTER_SCOPE:
    {
      PIUnregisterScopeEvent requestIn(e);
      pPropertySet = findPropertySet(requestIn.scope);

      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "SS-REQ: Unregister scope %s",
          requestIn.scope.c_str()));

      assert(pPropertySet);
      pPropertySet->unregisterScope(requestIn);
      break;
    }  

    case PA_SCOPE_UNREGISTERED:
    {
      PAScopeUnregisteredEvent responseIn(e);
      pPropertySet = findPropertySet(responseIn.scope);

      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "PA-RESP: Scope %s is unregistered", 
          responseIn.scope.c_str()));
          
      if (pPropertySet)
      {
        pPropertySet->unregisterCompleted(responseIn.result);
        delete pPropertySet;
      }
      _scopeRegister.erase(responseIn.scope);
      break;
    }

    case PA_LINK_PROPERTIES:
    {
      PALinkPropertiesEvent requestIn(e);
      pPropertySet = findPropertySet(requestIn.scope);
      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "PA-REQ: Link properties on scope %s", 
          requestIn.scope.c_str()));
      if (pPropertySet)
      {
        pPropertySet->linkProperties(requestIn);
      }
      else
      {
        PAPropertiesLinkedEvent responseOut;
        responseOut.result = PA_PROP_SET_GONE;
        responseOut.scope = requestIn.scope;
        replyMsgToPA(responseOut);
        LOFAR_LOG_TRACE(PI_STDOUT_LOGGER, ( 
            "Property set with scope %d was deleted in the meanwhile", 
            responseOut.scope.c_str()));
      }
      break;
    }
    case PI_PROPERTIES_LINKED:
    {
      PIPropertiesLinkedEvent responseIn(e);
      pPropertySet = findPropertySet(responseIn.scope);
      if (pPropertySet)
      {
        bool mustRetry = pPropertySet->propertiesLinked(responseIn);
        if (mustRetry && timerID == -1)
        {
          timerID = _ssPort.setTimer(0.0);
        }        
      }
      break;
    }
    case PA_UNLINK_PROPERTIES:
    {
      PAUnlinkPropertiesEvent requestIn(e);
      pPropertySet = findPropertySet(requestIn.scope);
      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "PA-REQ: Unlink properties on scope %s", 
          requestIn.scope.c_str()));
      if (pPropertySet)
      {
        pPropertySet->unlinkProperties(requestIn);
      }
      else
      {
        PAPropertiesUnlinkedEvent responseOut;
        responseOut.result = PA_PROP_SET_GONE;
        responseOut.scope = requestIn.scope;
        replyMsgToPA(responseOut);
        LOFAR_LOG_TRACE(PI_STDOUT_LOGGER, ( 
            "Property set with scope %d was deleted in the meanwhile", 
            responseOut.scope.c_str()));
      }
      break;
    }

    case PI_PROPERTIES_UNLINKED:
    {
      PIPropertiesUnlinkedEvent responseIn(e);
      pPropertySet = findPropertySet(responseIn.scope);
      if (pPropertySet)
      {
        pPropertySet->propertiesUnlinked(responseIn); 
      }
      break;
    }
    
    case PI_VALUE_SET:
    {
      PIValueSetEvent requestIn(e);
      const GCFPValue* pValue = requestIn.value._pValue;
      if (pValue)
      {
        _controller.getPropertyProxy().setPropValue(requestIn.name, *pValue);
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
    case F_ENTRY:
      _propertyAgent.close();
      break;
      
    case F_DISCONNECTED:      
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

GPIPropertySet* GPISupervisoryServer::findPropertySet(string& scope)
{
  TScopeRegister::iterator iter = _scopeRegister.find(scope);  
  return (iter != _scopeRegister.end() ? iter->second : 0);
}

void GPISupervisoryServer::replyMsgToPA(GCFEvent& e)
{
  if (_propertyAgent.isConnected())
  {    
    _propertyAgent.send(e);
  }
}
