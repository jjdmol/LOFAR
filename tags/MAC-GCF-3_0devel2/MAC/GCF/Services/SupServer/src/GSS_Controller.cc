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
#define DEBUG_SIGNAL
#define DECLARE_SIGNAL_NAMES
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
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      _scPortProvider.open();
      // intentional fall through
    case F_TIMER_SIG:
      _propertyInterface.open();
      break;

    case F_CONNECTED_SIG:
      if (_scPortProvider.isConnected())
      {
        TRAN(GSSController::operational_state);
      }
      break;

    case F_DISCONNECTED_SIG:
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
  static string scope = "";
  static char* pData = 0;

  switch (e.signal)
  {
    case F_CONNECTED_SIG:
		  if (&p == &_propertyInterface)
			{
        GCFEvent rse(PI_REGISTERSCOPE);
        for (TScopeRegister::iterator iter = _scopeRegister.begin();
               iter != _scopeRegister.end(); ++iter)
        {
          rse.length = sizeof(GCFEvent) + Utils::packString(iter->first, _buffer, MAX_BUF_SIZE);
          _propertyInterface.send(rse, _buffer, rse.length - sizeof(GCFEvent));
        }
      }  
      break;
  
    case F_DISCONNECTED_SIG:
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
        GCFEvent urse(PI_UNREGISTERSCOPE);
        for (TScopeRegister::iterator iter = _scopeRegister.begin();
             iter != _scopeRegister.end(); ++iter)
        {
          if (iter->second == &p)
          {
            scopesToBeDeleted.push_back(iter->first);
            urse.length = sizeof(GCFEvent) + Utils::packString(iter->first, _buffer, MAX_BUF_SIZE);
            _propertyInterface.send(urse, _buffer, urse.length - sizeof(GCFEvent));
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
    
    case F_ACCEPT_REQ_SIG:
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

    case F_TIMER_SIG:
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

    case PI_REGISTERSCOPE:
    {      
      pData = ((char*)&e) + sizeof(GCFEvent);
      if (!findScope(pData, scope))
      {
        forwardMsgToPI(e);
        // creates a new scope entry
        _scopeRegister[scope] = &p;
      }
      else
      {
        PIScoperegisteredEvent response(PI_SCOPE_ALREADY_REGISTERED);
        replyMsgToPMLlite(response, p, pData);
      }      

      LOFAR_LOG_INFO(SS_STDOUT_LOGGER, ( 
          "PMLlite-REQ: Register scope %s",
          scope.c_str()));
      break;
    }
    case PI_SCOPEREGISTERED:
    {
      PIScoperegisteredEvent* pResponse = static_cast<PIScoperegisteredEvent*>(&e);
      assert(pResponse);
      pData = ((char*)&e) + sizeof(PIScoperegisteredEvent);      
      char logMsg[] = "PI-RESP: Scope %s is registered";
      if (!forwardMsgToPMLlite(e, pData, scope, logMsg) && 
          pResponse->result == PI_NO_ERROR)
      {
        GCFEvent urse(PI_UNREGISTERSCOPE);
        unsigned short scopeDataLength = scope.length() + Utils::SLEN_FIELD_SIZE;
        urse.length += scopeDataLength;
        _propertyInterface.send(urse, pData, scopeDataLength);
      }      
      if (pResponse->result != PI_NO_ERROR)
      {
        _scopeRegister.erase(scope);
      }
      break;
    }
    case PI_UNREGISTERSCOPE:
    {
      pData = ((char*)&e) + sizeof(GCFEvent);
      if (!findScope(pData, scope))
      {
        forwardMsgToPI(e);
      }
      else
      {
        PIScopeunregisteredEvent response(PI_PROP_SET_GONE);
        replyMsgToPMLlite(response, p, pData);
      }      

      LOFAR_LOG_INFO(SS_STDOUT_LOGGER, ( 
          "PMLlite-REQ: Unregister scope %s",
          scope.c_str()));
      break;
    } 
    case PI_SCOPEUNREGISTERED:
    {
      PIScopeunregisteredEvent* pResponse = static_cast<PIScopeunregisteredEvent*>(&e);
      assert(pResponse);
      pData = ((char*)&e) + sizeof(PIScopeunregisteredEvent);      
      char logMsg[] = "PI-RESP: Scope %s is unregistered";
      forwardMsgToPMLlite(e, pData, scope, logMsg);
      // deletes the scope entry        
      _scopeRegister.erase(scope);
      break;
    }
    case PI_LINKPROPERTIES:
    {
      pData = ((char*)&e) + sizeof(GCFEvent);
      char logMsg[] = "PI-REQ: Link properties on scope %s";  
      if (!forwardMsgToPMLlite(e, pData, scope, logMsg))
      {
        PIPropertieslinkedEvent response(PI_PROP_SET_GONE);
        replyMsgToPI(response, pData);
      }
      break;
    }
    case PI_PROPERTIESLINKED:
      forwardMsgToPI(e);
      break;

    case PI_UNLINKPROPERTIES:
    {
      pData = ((char*)&e) + sizeof(GCFEvent);
      char logMsg[] = "PI-REQ: Unlink properties on scope %s";
      if (!forwardMsgToPMLlite(e, pData, scope, logMsg))
      {
        PIPropertieslinkedEvent response(PI_PROP_SET_GONE);
        replyMsgToPI(response, pData);
      }
      break;
    }

    case PI_PROPERTIESUNLINKED:      
      forwardMsgToPI(e);
      break;

    case PI_VALUESET:
      forwardMsgToPI(e);
      break;

    case PI_VALUECHANGED:
    {
      PIValuechangedEvent* pMsg = static_cast<PIValuechangedEvent*>(&e);
      assert(pMsg);
      char* pPropNameData = ((char*)&e) + sizeof(PIValuechangedEvent);
      string propName;
      Utils::unpackString(pPropNameData, propName);
      scope.assign(propName, 0, pMsg->scopeLength);

      LOFAR_LOG_INFO(SS_STDOUT_LOGGER, ( 
          "PI-MSG: Property %s changed", 
          propName.c_str()));
    
      GCFPortInterface* pPort = _scopeRegister[scope];
      if (pPort)
      {
        assert(pPort->isConnected());
        pPort->send(e);    
      }
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

bool GSSController::findScope(char* pData, string& scope)
{
  Utils::unpackString(pData, scope);
  TScopeRegister::iterator iter = _scopeRegister.find(scope);
  return (iter != _scopeRegister.end());
}

void GSSController::forwardMsgToPI(GCFEvent& e)
{
  if (_propertyInterface.isConnected())
  {
    _propertyInterface.send(e);
  }
}

void GSSController::replyMsgToPI(GCFEvent& e, const string& scope)
{
  if (_propertyInterface.isConnected())
  {    
    unsigned int scopeDataLength = Utils::packString(scope, _buffer, MAX_BUF_SIZE);
    e.length += scopeDataLength;
    _propertyInterface.send(e, _buffer, scopeDataLength);
  }
}

bool GSSController::forwardMsgToPMLlite(GCFEvent& e, char* pData, string& scope, char* logMsg)
{
  bool result(false);
  Utils::unpackString(pData, scope);
  
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

void GSSController::replyMsgToPMLlite(GCFEvent& e, GCFPortInterface& p, char* pData)
{
  assert(p.isConnected());
  unsigned short scopeDataLength = Utils::getStringDataLength(pData);
  e.length += scopeDataLength;
  p.send(e, pData, scopeDataLength);
}
