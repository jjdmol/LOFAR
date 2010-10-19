//# GSS_Controller.cc: the supervisory server for each ERTC board,
//#                    intermedier between ERTC controllers and LCU
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$
//

#include "GSS_Controller.h"
#include "GSS_Defines.h"
#include <GCF/GCF_Control.h>
#include <Utils.h>
#include <PI_Protocol.ph>

static string ssName = "SS";
 
GSSController::GSSController() :
  GCFTask((State)&GSSController::initial_state, ssName)
{
  // register the protocol for debugging purposes
  registerProtocol(PI_PROTOCOL, PI_PROTOCOL_signalnames);
  for (unsigned int i = 0; i < MAX_NR_OF_CLIENTS; i++)
  {
    _supClientPorts[i] = 0;
  }

  _propertyInterface.init(*this, "client", GCFPortInterface::SAP, PI_PROTOCOL);
  _scPortProvider.init(*this, "server", GCFPortInterface::MSPP, PI_PROTOCOL);
}

GSSController::~GSSController()
{
  for (unsigned int i = 0; i < MAX_NR_OF_CLIENTS; i++)
  {
    if (_supClientPorts[i]) delete _supClientPorts[i];
  }
}

GCFEvent::TResult GSSController::initial_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      _scPortProvider.open();
      // intentional fall through
    case F_TIMER:
      _propertyInterface.open();
      break;

    case F_CONNECTED:
      if (_scPortProvider.isConnected())
      {
        TRAN(GSSController::operational_state);
      }
      break;

    case F_DISCONNECTED:
      if (&p == &_scPortProvider) assert(0);
      else if (&p == &_propertyInterface)  p.setTimer(1.0); // try again after 1 second        
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GSSController::operational_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_CONNECTED:
		  if (&p == &_propertyInterface)
			{
        PIRegisterScopeEvent requestOut;
        for (TScopeRegister::iterator iter = _scopeRegister.begin();
               iter != _scopeRegister.end(); ++iter)
        {
          requestOut.scope = iter->first;
          _propertyInterface.send(requestOut);
        }
      }  
      break;
  
    case F_DISCONNECTED:
      assert(&_scPortProvider != &p);
      if (&_propertyInterface == &p)
      {
        p.setTimer(1.0); // try again after 1 second
      }
      else
      {
        // force context switch to be able to detele the port object
        _scPortProvider.setTimer(0, 0, 0, 0, &p); 
        // cleanup scope-port relations and raport it to te PI
        list<string> scopesToBeDeleted;
        PIUnregisterScopeEvent requestOut;
        for (TScopeRegister::iterator iter = _scopeRegister.begin();
             iter != _scopeRegister.end(); ++iter)
        {
          if (iter->second == &p)
          {
            scopesToBeDeleted.push_back(iter->first);
            requestOut.scope = iter->first;
            _propertyInterface.send(requestOut);
          }
        }
        for (list<string>::iterator iter = scopesToBeDeleted.begin();
             iter != scopesToBeDeleted.end(); ++iter)
        {
          _scopeRegister.erase(*iter);
        }
        p.close(); // to avoid more DISCONNECTED signals for this port
      }
      break;
    
    case F_ACCEPT_REQ:
    {
      GCFTCPPort* pNewPMLPort = new GCFTCPPort();
      pNewPMLPort->init(*this, "ss", GCFPortInterface::SPP, PI_PROTOCOL);
      _scPortProvider.accept(*pNewPMLPort);
      for (unsigned int i = 0; i < MAX_NR_OF_CLIENTS; i++)
      {
        if (_supClientPorts[i] == 0) 
        {
          _supClientPorts[i] = pNewPMLPort;
          break;
        }
      }
      break;
    }

    case F_TIMER:
      if (&_propertyInterface == &p)
      {          
        p.open(); // try again
      }
      else if (&_scPortProvider == &p)
      {
        GCFTimerEvent* pTimer = static_cast<GCFTimerEvent*>(&e);
        GCFPortInterface* pPort = (GCFPortInterface*)(pTimer->arg);
        if (pPort)
        {
          for (unsigned int i = 0; i < MAX_NR_OF_CLIENTS; i++)
          {
            if (_supClientPorts[i] == pPort) 
            {
              delete _supClientPorts[i];
              _supClientPorts[i] = 0;
              break;
            }
          }
        }
      }
      break;

    case PI_REGISTER_SCOPE:
    {      
      PIRegisterScopeEvent requestIn(e);
      if (!findScope(requestIn.scope))
      {
        sendMsgToPI(requestIn);
        // creates a new scope entry
        _scopeRegister[requestIn.scope] = &p;
      }
      else
      {
        PIScopeRegisteredEvent responseOut;
        responseOut.result = PI_SCOPE_ALREADY_REGISTERED;
        responseOut.scope = requestIn.scope;
        replyMsgToPMLlite(responseOut, p);
      }      

      LOFAR_LOG_INFO(SS_STDOUT_LOGGER, ( 
          "PMLlite-REQ: Register scope %s",
          requestIn.scope.c_str()));
      break;
    }
    case PI_SCOPE_REGISTERED:
    {
      PIScopeRegisteredEvent responseIn(e);
      
      char logMsg[] = "PI-RESP: Scope %s is registered";
      if (!forwardMsgToPMLlite(responseIn, responseIn.scope, logMsg) && 
          responseIn.result == PI_NO_ERROR)
      {
        PIUnregisterScopeEvent requestOut;
        requestOut.scope = responseIn.scope;
        _propertyInterface.send(requestOut);
      }      
      if (responseIn.result != PI_NO_ERROR)
      {
        _scopeRegister.erase(responseIn.scope);
      }
      break;
    }
    case PI_UNREGISTER_SCOPE:
    {
      PIUnregisterScopeEvent requestIn(e);
      if (!findScope(requestIn.scope))
      {
        sendMsgToPI(requestIn);
      }
      else
      {
        PIScopeUnregisteredEvent responseOut;
        responseOut.result = PI_PROP_SET_GONE;
        responseOut.scope = requestIn.scope;
        replyMsgToPMLlite(responseOut, p);
      }      

      LOFAR_LOG_INFO(SS_STDOUT_LOGGER, ( 
          "PMLlite-REQ: Unregister scope %s",
          requestIn.scope.c_str()));
      break;
    } 
    case PI_SCOPE_UNREGISTERED:
    {
      PIScopeUnregisteredEvent responseIn(e);
      char logMsg[] = "PI-RESP: Scope %s is unregistered";
      forwardMsgToPMLlite(responseIn, responseIn.scope, logMsg);
      // deletes the scope entry        
      _scopeRegister.erase(responseIn.scope);
      break;
    }
    case PI_LINK_PROPERTIES:
    {
      PILinkPropertiesEvent requestIn(e);
      char logMsg[] = "PI-REQ: Link properties on scope %s";  
      if (!forwardMsgToPMLlite(requestIn, requestIn.scope, logMsg))
      {
        PIPropertiesLinkedEvent responseOut;
        responseOut.result = PI_PROP_SET_GONE;
        responseOut.scope = requestIn.scope;
        sendMsgToPI(responseOut);
      }
      break;
    }
    case PI_PROPERTIES_LINKED:
    {
      PIPropertiesLinkedEvent responseIn(e);  
      sendMsgToPI(responseIn);
      break;
    }
    case PI_UNLINK_PROPERTIES:
    {
      PIUnlinkPropertiesEvent requestIn(e);
      char logMsg[] = "PI-REQ: Unlink properties on scope %s";
      if (!forwardMsgToPMLlite(requestIn, requestIn.scope, logMsg))
      {
        PIPropertiesLinkedEvent responseOut;
        responseOut.result = PI_PROP_SET_GONE;
        responseOut.scope = requestIn.scope;
        sendMsgToPI(responseOut);
      }
      break;
    }

    case PI_PROPERTIES_UNLINKED:      
    {
      PIPropertiesUnlinkedEvent responseIn(e);  
      sendMsgToPI(responseIn);
      break;
    }
    case PI_VALUE_SET:
    { 
      PIValueSetEvent indicationIn(e);
      sendMsgToPI(indicationIn);
      break;
    }
    case PI_VALUE_CHANGED:
    {
      PIValueChangedEvent indicationIn(e);
      string scope;
      scope.assign(indicationIn.name, 0, indicationIn.scopeLength);

      LOFAR_LOG_INFO(SS_STDOUT_LOGGER, ( 
          "PI-MSG: Property %s changed", 
          indicationIn.name.c_str()));
    
      GCFPortInterface* pPort = _scopeRegister[scope];
      if (pPort)
      {
        assert(pPort->isConnected());
        pPort->send(indicationIn);    
      }
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

bool GSSController::findScope(string& scope)
{
  TScopeRegister::iterator iter = _scopeRegister.find(scope);
  return (iter != _scopeRegister.end());
}

void GSSController::sendMsgToPI(GCFEvent& e)
{
  if (_propertyInterface.isConnected())
  {
    _propertyInterface.send(e);
  }
}

bool GSSController::forwardMsgToPMLlite(GCFEvent& e, string& scope, char* logMsg)
{
  bool result(false);
  
  LOFAR_LOG_INFO(SS_STDOUT_LOGGER, ( 
      logMsg, 
      scope.c_str()));

  GCFPortInterface* pPort = _scopeRegister[scope];
  if (pPort)
  {
    assert(pPort->isConnected());
    pPort->send(e);    
    result = true;
  }
  else
  {
    LOFAR_LOG_TRACE(SS_STDOUT_LOGGER, ( 
        "Property set with scope %s was deleted in the meanwhile", 
        scope.c_str()));
  }
  return result;
}

void GSSController::replyMsgToPMLlite(GCFEvent& e, GCFPortInterface& p)
{
  assert(p.isConnected());
  p.send(e);
}
