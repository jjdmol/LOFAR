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

#define DEBUG_SIGNAL
#include "GSS_Controller.h"
#include <Common/lofar_vector.h>
#include <Utils.h>
#define DECLARE_SIGNAL_NAMES
#include "PI_protocol.ph"
#include <GCF/GCF_Control.h>

GSSController::GSSController(const char* boardName) :
  GCFTask((State)&GSSController::initial_state, boardName)
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
  static unsigned int connectedCount = 0;
  int status = GCFEvent::HANDLED;

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
  int status = GCFEvent::HANDLED;
  static string scope = "";
  static char* pScopeData = 0;

  switch (e.signal)
  {
    case F_CONNECTED_SIG:
		  if (&p == &_propertyInterface)
			{
        GCFEvent rse(PI_REGISTERSCOPE);
        for (TScopeRegister::iterator iter = _scopeRegister.begin();
               iter != _scopeRegister.end(); ++iter)
        {
          rse.length = sizeof(GCFEvent) + Utils::packString(iter->first, _buffer, MAX_BUF_LENGTH);
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
            urse.length = sizeof(GCFEvent) + Utils::packString(iter->first, _buffer, MAX_BUF_LENGTH);
            _propertyInterface.send(urse, _buffer, urse.length - sizeof(GCFEvent));
          }
        }
        for (list<string>::iterator iter = scopesToBeDeleted.begin();
             iter != scopesToBeDeleted.end(); ++iter)
        {
          _scopeRegister.erase(iter);
        }
        p.close(); // to avoid more DISCONNECTED signals for this port
      }
      break;
    
    case F_ACCEPT_REQ_SIG:
    {
      GCFTCPPort* pNewPMLPort = new GCFTCPPort();
      pNewPMLPort->init(*this, "ss", GCFPortInterface::SPP, PI_PROTOCOL);
      _supClientPorts.accept(*pNewPMLPort);
      for (unsigned int i = 0; i < MAX_NR_OF_CLIENTS; i++)
      {
        if (_supClientPorts[i] == 0) 
        {
          _supClientPorts[i] = pNewPMLPort;
        }
      }
      break;
    }

    case F_TIMER_SIG:
      if (&_propertyInterface == &p)
      {          
        p.open(); // try again
      }
      else if (&_pmlPortProvider == &p)
      {
        GCFTimerEvent* pTimer = static_cast<GCFTimerEvent*>(&e);
        GCFPortInterface* pPort = (GCFPortInterface*)(pTimer->arg);
        if (pPort)
        {
          for (unsigned int i = 0; i < MAX_NR_OF_CLIENTS; i++)
          {
            if (_supClientPorts[i] == &p) 
            {
              delete _supClientPorts[i];
              _supClientPorts[i] = 0;
            }
          }
        }
      }
      break;

    case PI_REGISTERSCOPE:
    {      
      pScopeData = ((char*)&e) + sizeof(GCFEvent);
      if (!findScope(pScopeData, scope))
      {
        forwardMsgToPI(e);
        // creates a new scope entry
        _scopeRegister[scope] = &p;
      }
      else
      {
        PIScoperegisteredEvent response(PI_SCOPE_ALREADY_REGISTERED);
        replyMsgToPMLlite(response, p, pScopeData);
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
      pScopeData = ((char*)&e) + sizeof(PIScoperegisteredEvent);      
      char logMsg[] = "PI-RESP: Scope %s is registered";
      if (!forwardMsgToPMLlite(e, pScopeData, scope, logMsg) && 
          pResponse->result == PI_NO_ERROR)
      {
        GCFEvent urse(PI_UNREGISTERSCOPE);
        unsigned short scopeDataLength = scope.length() + Utils::SLENGTH_FIELD_SIZE;
        urse.length += scopeDataLength;
        _propertyInterface.send(urse, pScopeData, scopeDataLength);
      }      
      if (pResponse->result != PI_NO_ERROR)
      {
        _scopeRegister.erase(scope);
      }
      break;
    }
    case PI_UNREGISTERSCOPE:
    {
      pScopeData = ((char*)&e) + sizeof(GCFEvent);
      if (!findScope(pScopeData, scope))
      {
        forwardMsgToPI(e);
      }
      else
      {
        PIScopeunregisteredEvent response(PI_PROP_SET_GONE);
        replyMsgToPMLlite(response, p, pScopeData);
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
      pScopeData = ((char*)&e) + sizeof(PIScopeunregisteredEvent);      
      char logMsg[] = "PI-RESP: Scope %s is unregistered";
      forwardMsgToPMLlite(e, pScopeData, scope, logMsg);
      // deletes the scope entry        
      _scopeRegister.erase(scope);
      break;
    }
    case PI_LINKPROPERTIES:
    {
      pScopeData = ((char*)&e) + sizeof(GCFEvent);
      char logMsg[] = "PI-REQ: Link properties on scope %s";  
      if (!forwardMsgToPMLlite(e, pScopeData, scope, logMsg))
      {
        PIPropertieslinkedEvent response(PI_PROP_SET_GONE);
        replyMsgToPI(response, pScopeData);
      }
      break;
    }
    case PI_PROPERTIESLINKED:
      forwardMsgToPI(e);
      break;
    case PA_UNLINKPROPERTIES:
    {
      pScopeData = ((char*)&e) + sizeof(GCFEvent);
      char logMsg[] = "PI-REQ: Unlink properties on scope %s";
      if (!forwardMsgToPMLlite(e, pScopeData, scope, logMsg))
      {
        PIPropertieslinkedEvent response(PI_PROP_SET_GONE);
        replyMsgToPI(response, pScopeData);
      }
      break;
    }

    case PI_PROPERTIESUNLINKED:      
      forwardMsgToPI(e);
      break;

    case PI_PROPERTYCHANGED:
      forwardMsgToPI(e);
      break;

    case PI_PROPERTYWRITTEN:
    {
      PIPropertywrittenEvent* pMsg = static_cast<PIPropertywrittenEvent*>(&e);
      assert(pMsg);
      char* pPropNameData = ((char*)&e) + sizeof(PIPropertywrittenEvent);
      scope.assign(pPropNameData + 3, pMsg->scopeLength);
      string propName;
      Utils::unpackString(pPropNameData, propName);

      LOFAR_LOG_INFO(SS_STDOUT_LOGGER, ( 
          "PI-MSG: Property %s written", 
          pPropNameData.c_str()));
    
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

bool GSSController::findScope(char* pScopeData, string& scope)
{
  Utils::unpackString(pScopeData, scope);
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

bool GSSController::forwardMsgToPMLlite(GCFEvent& e, char* pScopeData, string& scope, const char* logMsg)
{
  bool result(false);
  Utils::unpackString(pScopeData, scope);
  
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
        "Property set with scope %d was deleted in the meanwhile", 
        scope.c_str()));
  }
  return result;
}

void GSSController::replyMsgToPMLlite(GCFEvent& e, GCFPortInterface& p, char* pScopeData)
{
  assert(p.isConnected());
  unsigned short scopeDataLength = Utils::getStringDataLength(pScopeData);
  e.length += scopeDataLength;
  p.send(e, pScopeData, scopeDataLength);
}

//
// Program usage
//
void usage(const char* progname)
{
  fprintf(stderr, "usage: %s\n"
		  "\t-locname <name of the location, e.g.:LCU or BOARD1 (unique on in a station)>\n"
		  "\t[-help]\n"
		  progname);
}

int main(int argc, char* argv[])
{
  GCFTask::init(argc, argv);

  string locName;
  
  if (GCFTask::_argv != 0)
  {
    CCmdLine cmdLine;
    // parse argc,argv 
    if (cmdLine.SplitLine(GCFTask::_argc, GCFTask::_argv) > 0)
    {
      locName = cmdLine.GetSafeArgument("-locname", 0, "");
      if (cmdLine.HasSwitch("-help"))
      {
        usage(argv[0]);
        exit(0);
      }      
    }            
  }
  if (locName.length() == 0)
  {
    usage(argv[0]);
    exit(0);
  }

  GSSController supervisoryServer(locName);

  supervisoryServer.start(); // make initial transition

  GCFTask::run();

  return 0;
}