//#  GPA_PropSetSession.cc:
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

#include "GPA_PropSetSession.h"
#include "GPA_Controller.h"
#include "GPA_PropertySet.h"
#include <GCF/GCF_PVString.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <strings.h>
#include <stdio.h>
#include <unistd.h>

namespace LOFAR 
{
 namespace GCF 
 {
using namespace TM;
using namespace Common;
  namespace PAL 
  {

#define RETURN_WRONG_STATE_MSG(_request_, _response_) \
    { \
      _request_##Event& request((_request_##Event&) e); \
      _response_##Event response; \
      response.seqnr = request.seqnr; \
      response.result = PA_WRONG_STATE; \
      p.send(response); \
      break; \
    }
#define IN_WRONG_STATE wrongState(e, p, __func__);

#define QUEUE_REQUEST(e, p)  \
  if (!mayContinue(e, p)) \
  { \
    status = GCFEvent::NOT_HANDLED;  \
    break; \
  }

bool operator == (const GPAPropSetSession::TPSUser& a, const GPAPropSetSession::TPSUser& b)
{
  return (a.pPSUserPort == b.pPSUserPort);
}


GPAPropSetSession::GPAPropSetSession(GPAController& controller, GCFPortInterface& psProviderPort) :
  GCFTask((State)&GPAPropSetSession::enabling_state, PA_PS_SESSION_TASK_NAME_EXT),
  _controller(controller),
  _usecount(0),
  _psProviderPort(psProviderPort),
  _propSet(*this),
  _savedResult(PA_NO_ERROR),
  _savedSeqnr(0),
  _pSavedEvent(0),
  _pSavedPort(0),
  _isBusy(false)
{
}

GPAPropSetSession::~GPAPropSetSession()
{
  _requestQueue.deleteAllRequests();
}

GCFEvent::TResult GPAPropSetSession::enabling_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case PA_REGISTER_SCOPE:
    {
      QUEUE_REQUEST(e, p)      
      PAScopeRegisteredEvent response;
      PARegisterScopeEvent& request((PARegisterScopeEvent&) e);
      
      LOG_DEBUG(formatString(
          "Handle request to enable prop. set '%s' of type '%s'",
          request.scope.c_str(),
          request.type.c_str()));

      response.seqnr = request.seqnr;
      _savedSeqnr = request.seqnr;

      ASSERT(_psUsers.size() == 0);
      string newTaskName = request.scope + PA_PS_SESSION_TASK_NAME_EXT;
      GCFTask::setName(newTaskName);
      response.result = _propSet.enable(request.scope, request.type, request.category);
      
      if (response.result != PA_NO_ERROR)
      {
        // not enabled due to errors; respond with error
        _pSavedEvent = &response;
        TRAN(GPAPropSetSession::disabled_state);
      }
      //else enabled DP will be created asynchronous, so wait for PA_PROP_SET_DP_CREATED signal
      break;
    }
    case PA_PROP_SET_DP_CREATED:
    {
      ASSERT(_isBusy);
      PAScopeRegisteredEvent response;
      response.seqnr = _savedSeqnr;
      response.result = PA_NO_ERROR;
      
      LOG_DEBUG(formatString("Prop. set '%s' is enabled successful.",
           _propSet.name().c_str()));
            
      if (_psProviderPort.isConnected())
      {
        _psProviderPort.send(response);          
        if (PS_IS_AUTOLOAD(_propSet.category()))
        {
          LOG_DEBUG(formatString("Prop. set '%s' is configured as autoloaded. So it will be loaded directly...",
               _propSet.name().c_str()));
          
          PALoadPropSetEvent autoRequest;
          autoRequest.seqnr = 0; // sequence number 0 is necessary to handle load response as an internal message
          autoRequest.scope = _propSet.name();
          _pSavedEvent = &autoRequest;
          _pSavedPort = &_psProviderPort;
          TRAN(GPAPropSetSession::linking_state);
        }
        else
        {
          TRAN(GPAPropSetSession::enabled_state);
        }
      }
      break;
    }
    case PA_PROP_SET_DP_DELETED:
    case PA_UNLOAD_PROP_SET:
    case PA_PROP_SET_LINKED:
    case PA_PROP_SET_UNLINKED:
      IN_WRONG_STATE
      break;
      
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

GCFEvent::TResult GPAPropSetSession::enabled_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
      ASSERT(_isBusy);
      _controller.propSetIdle(*this);
      break;
      
    case PA_UNREGISTER_SCOPE:
      QUEUE_REQUEST(e, p)      
      _pSavedEvent = &e;
      TRAN(GPAPropSetSession::disabling_state);
      break;

    case PA_LOAD_PROP_SET:
      QUEUE_REQUEST(e, p)      
      _pSavedEvent = &e;
      _pSavedPort = &p;
      TRAN(GPAPropSetSession::linking_state);
      break;
      
    case PA_CONF_PROP_SET:
      configure(e, p);
      break;
      
    case F_CLOSED:
    {
      QUEUE_REQUEST(e, p)      
      LOG_DEBUG(formatString(
          "Handles closed port event in session '%s'...",
          getName().c_str()));
      if (&p == &_psProviderPort)
      {
        // only if closed port is the port of the application, which manages the property set (prop. set provider)
        LOG_DEBUG(formatString("Reset state of session %s from %s to 'IDLE'.", getName().c_str(), (_isBusy ? "'BUSY'" : "'IDLE'")));
        _isBusy = false; // to avoid the following action will be placed in the queue
        PAUnregisterScopeEvent request;  
        request.seqnr = 0;
        status = GCFFsm::dispatch(request, p); // to this state
      }
      else
      {
        LOG_DEBUG(formatString(
            "Closed port event has no consequences for session '%s'. Ignored!",
            getName().c_str()));
        _controller.closingPortFinished(*this, p);
        ASSERT(_isBusy);
        _controller.propSetIdle(*this);
        // has no further consequences for this session
      }
      break;
    }
    case PA_REGISTER_SCOPE:
    case PA_PROP_SET_DP_CREATED:
    case PA_PROP_SET_DP_DELETED:
    case PA_UNLOAD_PROP_SET:
    case PA_PROP_SET_LINKED:
    case PA_PROP_SET_UNLINKED:
      IN_WRONG_STATE
      break;
      
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

GCFEvent::TResult GPAPropSetSession::disabling_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
    {      
      ASSERT(_pSavedEvent);
      PAUnregisterScopeEvent& request((PAUnregisterScopeEvent&) *_pSavedEvent);
      PAScopeUnregisteredEvent response;
      response.seqnr = request.seqnr;
      response.result = PA_NO_ERROR;
      
      LOG_DEBUG(formatString(
          "Handle request to disable prop. set '%s' of type '%s'",
          _propSet.name().c_str(),
          _propSet.type().c_str()));
      
      bool mustWait;
      response.result = _propSet.disable(mustWait);
      
      _savedSeqnr = request.seqnr;
      _pSavedEvent = 0;    
      _usecount = 0;
      if (mustWait) // asynchronous?
      {
        _savedResult = response.result; // wait for PA_PROP_SET_DP_DELETED signal
      }
      else
      {
        _pSavedEvent = &response;
        TRAN(GPAPropSetSession::disabled_state);
      }
      break;
    }
    case PA_PROP_SET_DP_DELETED:
    {
      ASSERT(_isBusy);
      PAScopeUnregisteredEvent response;
      response.seqnr = _savedSeqnr;
      response.result = _savedResult;
      _pSavedEvent = &response;
      TRAN(GPAPropSetSession::disabled_state);
      break;
    } 
    case PA_REGISTER_SCOPE:
    case PA_UNREGISTER_SCOPE:
    case PA_PROP_SET_DP_CREATED:
    case PA_LOAD_PROP_SET:
    case PA_UNLOAD_PROP_SET:
    case PA_CONF_PROP_SET:
      IN_WRONG_STATE
      break;
      
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

GCFEvent::TResult GPAPropSetSession::disabled_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
    {    
      LOG_DEBUG(formatString("Prop. set '%s' is disabled successful.",
           _propSet.name().c_str()));
        
      ASSERT(_pSavedEvent);
      if (_savedSeqnr > 0) // was it a internal unregister request or not? 
      {
        // not internal, so a response can be send
        _psProviderPort.send(*_pSavedEvent);
      }
      _pSavedEvent = 0;    
      _controller.mayDeleted(*this);
      break;
    }
    case PA_REGISTER_SCOPE:
    case PA_UNREGISTER_SCOPE:
    case PA_PROP_SET_DP_CREATED:
    case PA_PROP_SET_DP_DELETED:
    case PA_LOAD_PROP_SET:
    case PA_UNLOAD_PROP_SET:
    case PA_CONF_PROP_SET:
    case PA_PROP_SET_LINKED:
    case PA_PROP_SET_UNLINKED:
    case F_CLOSED:
      IN_WRONG_STATE
      break;
      
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}
      
GCFEvent::TResult GPAPropSetSession::linking_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
    {
      ASSERT(_pSavedEvent);
      ASSERT(_pSavedPort);
      ASSERT(_usecount == 0);

      LOG_DEBUG(formatString(
          "Handle request to load prop. set '%s' of type '%s'",
          _propSet.name().c_str(),
          _propSet.type().c_str()));

      PALoadPropSetEvent& request = (PALoadPropSetEvent&) *_pSavedEvent;
      PAPropSetLoadedEvent response;
      response.seqnr = request.seqnr;
      bool mustWait;
      response.result = _propSet.load(mustWait);
      _pSavedEvent = 0;    
      if (response.result != PA_NO_ERROR)
      {
        _pSavedPort->send(response);
        TRAN(GPAPropSetSession::enabled_state);
      }
      else
      {
        // In case a loaded property set will be disabled by the provider (owner, server MCA),
        // we need a mechanism to inform all users (client MCA's, which has loaded 
        // this property set). 
        // That's why we remember the port of the requestor (client MCA). The 
        // combination of the following two reasons explains the need of extending 
        // the port with a load counter:
        //    - a requestor can has loaded the same property set (same scope) 
        //      more than once (on different places) at the same time and 
        //    - we need to know when this requestor may be deleted from the 
        //      clientport list and thus not needed to be informed about disabling 
        //      the prop. set anymore. 
        // If client has already loaded a property set with this scope (_propSet.name()),
        // only the counter needed to be increased (see state handling linked_state->PA_LOAD_PROP_SET below).
        // On unloading the counter will be decreased and if counter 
        // is 0 the requestor can be deleted from the list. Then the requestor
        // not needed to be and also will not be informed about disabling the 
        // property set.
        ASSERT(_psUsers.size() == 0);
        TPSUser psUser;
        psUser.pPSUserPort = _pSavedPort;
        psUser.count = 1;
        _psUsers.push_back(psUser);
        _usecount++;
        
        // on permanent prop. sets no DP's needed to be created
        // so it can be linked immediately
        _savedSeqnr = request.seqnr;
        if (!mustWait)
        {
          link();
        }        
        // else wait for PA_PROP_SET_DP_CREATED signal
      }
      break;
    }
    case PA_UNREGISTER_SCOPE:
      QUEUE_REQUEST(e, p)      
      _pSavedEvent = &e;
      TRAN(GPAPropSetSession::disabling_state);
      break;

    case PA_PROP_SET_DP_CREATED:
      ASSERT(_isBusy);
      link();
      break;
      
    case PA_PROP_SET_LINKED:
      _pSavedEvent = &e;
      TRAN(GPAPropSetSession::linked_state);
      break;

    case F_CLOSED:
    {
      ASSERT(_isBusy);
      if (&p == &_psProviderPort)
      {
        LOG_DEBUG(formatString(
            "Handles closed port event during linking in session '%s'...",
            getName().c_str()));
        // the linked message will be received anymore because of the closed port,
        // so pretend the unregister prop. set event was received to clean up this prop. set (session)
        LOG_DEBUG(formatString("Resets state of session %s from %s to 'IDLE'.", getName().c_str(), (_isBusy ? "'BUSY'" : "'IDLE'")));
        _isBusy = false;
        PAUnregisterScopeEvent request;  
        request.seqnr = 0; // seqnr == 0 -> internal action
        status = GCFFsm::dispatch(request, p); // to this state        
      }
      else
      {
        status = defaultHandling(e, p);
      }      
      break;
    }
    case PA_REGISTER_SCOPE:
    case PA_PROP_SET_DP_DELETED:
    case PA_PROP_SET_UNLINKED:
      IN_WRONG_STATE
      break;
      
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

GCFEvent::TResult GPAPropSetSession::linked_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
    {
      ASSERT(_pSavedEvent);
      PAPropSetLinkedEvent& response((PAPropSetLinkedEvent&) *_pSavedEvent);
      PAPropSetLoadedEvent loadedResponse;
      loadedResponse.seqnr = _savedSeqnr;
      loadedResponse.result = response.result;
      _pSavedEvent = 0;    
      if (_savedSeqnr > 0) // 0 == auto load
      {
        ASSERT(_pSavedPort);
        _pSavedPort->send(loadedResponse);
      }        
      if (response.result != PA_NO_ERROR)
      {
        TRAN(GPAPropSetSession::enabled_state);
      }
      else        
      { 
        LOG_DEBUG(formatString("Prop. set '%s' is loaded successful.",
           _propSet.name().c_str()));
        ASSERT(_isBusy);
        _controller.propSetIdle(*this);
      }        
      break;
    }
    case PA_UNREGISTER_SCOPE:
    {
      QUEUE_REQUEST(e, p)      
      LOG_DEBUG(formatString("Inform all prop. set users, which have loaded this prop. set (%s), about disabling it...", 
          _propSet.name().c_str()));
      PAPropSetGoneEvent indication;
      indication.scope = GCFPVSSInfo::getLocalSystemName() + ":" + _propSet.name();
      GCFPortInterface* pPSUserPort;
      for (TPSUsers::iterator iter = _psUsers.begin();
           iter != _psUsers.end(); ++iter)
      {
        pPSUserPort = iter->pPSUserPort;
        ASSERT(pPSUserPort);
        
        if (pPSUserPort->isConnected())
        {          
          pPSUserPort->send(indication);
        }
      }
      _pSavedEvent = &e;
      TRAN(GPAPropSetSession::disabling_state);
      break;
    }     
    case PA_LOAD_PROP_SET:
    {
      QUEUE_REQUEST(e, p)      
      ASSERT(_usecount > 0);
      PALoadPropSetEvent& request = (PALoadPropSetEvent&) e;
      PAPropSetLoadedEvent response;
      response.seqnr = request.seqnr;
      response.result = PA_NO_ERROR;
      TPSUser* pPSUser = findUser(p);
      if (pPSUser)
      {
        pPSUser->count++;
      }
      else // client not known yet
      {
        TPSUser psUser;
        psUser.pPSUserPort = &p;
        psUser.count = 1;
        _psUsers.push_back(psUser);
      }
      _usecount++;
      LOG_DEBUG(formatString(
          "Prop. set already loaded! So only increase the usecount to %d.",
          _usecount));
      if (response.seqnr > 0) // 0 == auto load
      {
        p.send(response);
      }           
      ASSERT(_isBusy);
      _controller.propSetIdle(*this);
      break;
    }
    case PA_UNLOAD_PROP_SET:
    {
      QUEUE_REQUEST(e, p)      
      PAUnloadPropSetEvent& request((PAUnloadPropSetEvent&) e);
      PAPropSetUnloadedEvent response;
      response.seqnr = request.seqnr;
      response.result = PA_NO_ERROR;
      _savedSeqnr = request.seqnr;
      _savedResult = PA_NO_ERROR;
      
      LOG_DEBUG(formatString(
          "Handle request to unload prop. set '%s' of type '%s'",
          _propSet.name().c_str(),
          _propSet.type().c_str()));
      // decrease the load counter and remove the client (if counter == 0),
      // see also 'linking_state->F_ENTRY'
      TPSUser* pPSUser = findUser(p);
      if (pPSUser)
      {
        if (!PS_IS_AUTOLOAD(_propSet.category()) || _usecount > 1)
        {
          _usecount--;
          LOG_DEBUG(formatString(
              "Decreased the usecount to %d",          
              _usecount));
          pPSUser->count--;
        }
        if (pPSUser->count == 0)
        {
          _psUsers.remove(*pPSUser);
        }        
      }

      if (_usecount == 0)
      {  
        _pSavedEvent = &response; // will be used in the F_ENTRY handling
        _pSavedPort = &p;
        TRAN(GPAPropSetSession::unlinking_state);
      }
      else
      {
        LOG_DEBUG(formatString("Prop. set '%s' is unloaded successful.",
             _propSet.name().c_str()));
        _pSavedPort = &p;
        unloaded(&response);
        ASSERT(_isBusy);
        _controller.propSetIdle(*this);
      }         
      break;
    }
    case PA_CONF_PROP_SET:
      configure(e, p);
      break;
      
    case F_CLOSED:
    {
      QUEUE_REQUEST(e, p)      
      LOG_DEBUG(formatString(
          "Handles closed port event in session '%s'...",
          getName().c_str()));
      if (&p == &_psProviderPort)
      {
        LOG_DEBUG(formatString("Resets state of session %s from %s to 'IDLE'.", getName().c_str(), (_isBusy ? "'BUSY'" : "'IDLE'")));
        _isBusy = false; // to avoid that the actions in one of the following 'dispatches' will be queued
        PAUnregisterScopeEvent request;  
        request.seqnr = 0;
        status = GCFFsm::dispatch(request, p); // to this state        
      }
      else
      {
        TPSUser* pPSUser = findUser(p);
        if (pPSUser)
        {
          // this manipulation of the _usecount and the load counter pretends 
          // that this is the last unload request of the client 'p' for this
          // property set
          _usecount -= (pPSUser->count - 1);
          pPSUser->count = 1;
      
          LOG_DEBUG(formatString("Resets state of session %s from %s to 'IDLE'.", getName().c_str(), (_isBusy ? "'BUSY'" : "'IDLE'")));
          _isBusy = false; // to avoid that the actions in one of the following 'dispatches' will be queued
          PAUnloadPropSetEvent request;
          request.scope = _propSet.name();
          request.seqnr = 0;
          status = GCFFsm::dispatch(request, p); // to this state
        } 
        else
        {
          LOG_DEBUG(formatString(
              "This 'closed' port event has no consequences for session '%s'. Ignored!",
              getName().c_str()));
          _controller.closingPortFinished(*this, p);
          ASSERT(_isBusy);
          _controller.propSetIdle(*this);
        } 
      }
      break;
    }
    case PA_REGISTER_SCOPE:
    case PA_PROP_SET_DP_CREATED:
    case PA_PROP_SET_DP_DELETED:
    case PA_PROP_SET_LINKED:
    case PA_PROP_SET_UNLINKED:
      IN_WRONG_STATE
      break;
      
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

GCFEvent::TResult GPAPropSetSession::unlinking_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
    {
      ASSERT(_pSavedEvent);
      ASSERT(_pSavedPort);
      bool mustWait;
      PAPropSetUnloadedEvent& response((PAPropSetUnloadedEvent&) *_pSavedEvent);
      _savedResult = response.result = _propSet.unload(mustWait);
      _pSavedEvent = 0;    
      if (!mustWait)
      {
        if (response.result == PA_NO_ERROR)
        {
          unlink(); 
        }
        else
        {
          unloaded(&response);
          TRAN(GPAPropSetSession::enabled_state);
        }
      }
      break; 
    }
    case PA_PROP_SET_UNLINKED:
    {
      PAPropSetUnlinkedEvent& response((PAPropSetUnlinkedEvent&) e);
      PAPropSetUnloadedEvent unloadedResponse;
      unloadedResponse.seqnr = _savedSeqnr;
      unloadedResponse.result = (_savedResult != PA_NO_ERROR ? _savedResult : response.result);
      LOG_DEBUG(formatString("Prop. set '%s' is unloaded successful.",
           _propSet.name().c_str()));
      unloaded(&unloadedResponse);
      TRAN(GPAPropSetSession::enabled_state);  
      break;
    }
    case PA_PROP_SET_DP_DELETED:
      ASSERT(_isBusy);
      unlink();
      break;
      
    case F_CLOSED:
    {
      ASSERT(_isBusy);
      if (&p == &_psProviderPort)
      {
        LOG_DEBUG(formatString(
            "Handles closed port event during unlinking in session '%s'...",
            getName().c_str()));
        
        // the unlinked message will never be received because of the closed port,
        // so pretend the prop. set is unlinked and unregister prop. set event was received 
        // to clean up this prop. set (session)
        PAPropSetUnloadedEvent unloadedResponse;
        unloadedResponse.seqnr = _savedSeqnr;
        unloadedResponse.result = _savedResult;
        LOG_DEBUG(formatString("Prop. set '%s' is unloaded successful.",
           _propSet.name().c_str()));
        unloaded(&unloadedResponse);
        
        PAUnregisterScopeEvent request;  
        request.seqnr = 0;
        _pSavedEvent = &request;
        TRAN(GPAPropSetSession::disabling_state);
      }
      else
      {
        status = defaultHandling(e, p);
      }      
      break;
    }
    case PA_REGISTER_SCOPE:
    case PA_PROP_SET_DP_CREATED:
    case PA_UNLOAD_PROP_SET:
    case PA_CONF_PROP_SET:
    case PA_PROP_SET_LINKED:
      IN_WRONG_STATE
      break;
      
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

void GPAPropSetSession::link()
{
  ASSERT(GCFPVSSInfo::propExists(_propSet.name()));
  if (_psProviderPort.isConnected())
  {
    LOG_DEBUG(formatString(
        "Prop. set '%s' will be linked",
        _propSet.name().c_str()));

    PALinkPropSetEvent request;
    request.scope = _propSet.name();
    _psProviderPort.send(request);
  }
  else
  {
    TRAN(GPAPropSetSession::enabled_state);
    // user has already received the SERVER GONE message and does not wait for a nack anymore    
  }
}

void GPAPropSetSession::unlink()
{
  if (_psProviderPort.isConnected())
  {
    LOG_DEBUG(formatString(
        "Prop. set '%s' will be unlinked",
        _propSet.name().c_str()));

    PAUnlinkPropSetEvent request;
    request.scope = _propSet.name();
    _psProviderPort.send(request);
  }
  else
  {
    unloaded(0);
    TRAN(GPAPropSetSession::enabled_state);  
    // user has already received the SERVER GONE message and does not wait for a nack anymore    
  }
}

void GPAPropSetSession::unloaded(GCFEvent* pResponse)
{
  ASSERT(_pSavedPort);
  if (_savedSeqnr > 0) 
  {
    if (_pSavedPort->isConnected() && pResponse)
    {
      _pSavedPort->send(*pResponse);
    }
  }
  else
  {
    // closing port sequence finished    
    _controller.closingPortFinished(*this, *_pSavedPort);
  }
}

void GPAPropSetSession::configure(GCFEvent& e, GCFPortInterface& p)
{
  PAConfPropSetEvent& request((PAConfPropSetEvent&) e);
  TPAResult paResult(PA_NO_ERROR);
  LOG_DEBUG(formatString(
      "Handle request to configure prop. set '%s' with '%s.apc'",
      _propSet.name().c_str(),
      request.apcName.c_str()));
  if (GCFPVSSInfo::propExists(_propSet.name()))
  {
    int result = system("chmod 777 loadAPC"); // execute rights are gone after check-out with eclipse
    if (result == -1)
    {
      LOG_ERROR("System call cannot execute: chmod 777 loadAPC");
      paResult = PA_INTERNAL_ERROR;
    }
    else
    {
      string loadAPCcmd = formatString("loadAPC %s %s",
           _propSet.name().c_str(),
           request.apcName.c_str());
      result = system(loadAPCcmd.c_str());
      switch (result)
      {
        case -1:
          LOG_ERROR(formatString(
              "System call cannot execute: %s",
              loadAPCcmd.c_str()));
          paResult = PA_INTERNAL_ERROR;
          break;
        case 256:
          LOG_ERROR(formatString("Apc '%s' file does not exists.", 
              request.apcName.c_str()));
          paResult = PA_APC_NOT_EXISTS;
          break;
      }
    }
  }
  else
  {
    paResult = PA_PROP_SET_NOT_EXISTS;
  }   
  LOG_DEBUG(formatString(
      "Ready with configuring prop. set '%s' (Error code: %d - see GPA_Defines.h)",
      _propSet.name().c_str(),
      paResult));
  PAPropSetConfEvent response;
  response.seqnr = request.seqnr;
  response.apcName = request.apcName;
  response.result = paResult;
  p.send(response);
}

GPAPropSetSession::TPSUser* GPAPropSetSession::findUser(const GCFPortInterface& p) 
{
  TPSUser* pPSUser(0);
  for (TPSUsers::iterator iter = _psUsers.begin();
       iter != _psUsers.end(); ++iter)
  {
    if (iter->pPSUserPort == &p)
    {
      pPSUser = &(*iter);
      break;
    }
  } 
  return pPSUser;
}

bool GPAPropSetSession::mayContinue(GCFEvent& e, GCFPortInterface& p)
{
  bool maycontinue(false); 
  GCFEvent* pEvent(&e); 
  if (_isBusy)
  {    
    if (e.signal == F_CLOSED)
    {
      // will be deleted by the RequestManager.      
      pEvent = new GCFEvent(F_CLOSED);
    }
    _requestQueue.registerRequest(*pEvent, p);    
    LOG_DEBUG(formatString("Session '%s' is busy! Request is queued!",
        getName().c_str()));    
  }
  else
  {  
    LOG_DEBUG(formatString("Session %s goes from %s to 'BUSY'.", getName().c_str(), (_isBusy ? "'BUSY'" : "'IDLE'")));
    _isBusy = true; 
    maycontinue = true;
  }
  return maycontinue;
}

void GPAPropSetSession::doNextRequest()
{
  GCFPortInterface* pPort = _requestQueue.getOldestRequestPort();
  GCFEvent* pEvent = _requestQueue.getOldestRequest();

  LOG_DEBUG(formatString("0 timer for session '%s' was expired. State changes from %s to 'IDLE'.%s",
      getName().c_str(),
      (_isBusy ? "'BUSY'" : "'IDLE'"), 
      (pPort ? "" : " No message in queue to handle.")));
  _isBusy = false;
  if (pPort) // new action available ?
  {
    ASSERT(pEvent);
    LOG_DEBUG("There is a queued message! Will be handled now!");
    if (GCFFsm::dispatch(*pEvent, *pPort) == GCFEvent::HANDLED)
    {
      if (pEvent->signal == F_CLOSED)
      {
        // closing port sequence finished, in that case the request was an internal one, so no response will/could be returned
        // and all still queued messages will be deleted, which were received from the closed port
        _requestQueue.deleteRequestsOfPort(*pPort);    
      }
      else
      {
        _requestQueue.deleteOldestRequest();      
      }
    }
  }
}

GCFEvent::TResult GPAPropSetSession::defaultHandling(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  switch (e.signal)
  {
    case F_EXIT:
    case F_ENTRY:
    case F_INIT:
      status = GCFEvent::HANDLED;
      break;
      
    case PA_REGISTER_SCOPE:
    case PA_UNREGISTER_SCOPE:
    case PA_LOAD_PROP_SET:
    case PA_UNLOAD_PROP_SET:
    case PA_CONF_PROP_SET:
    case PA_PROP_SET_LINKED:
    case PA_PROP_SET_UNLINKED:
      LOG_DEBUG(formatString("Session '%s' is busy! Request is queued!",
          getName().c_str()));    
      _requestQueue.registerRequest(e, p);
      break;

    case F_CLOSED:
    case PA_PROP_SET_DP_CREATED:
    case PA_PROP_SET_DP_DELETED:
    {
      // will be deleted by the RequestManager.
      GCFEvent* pEvent = new GCFEvent(e.signal);
      LOG_DEBUG(formatString("Session '%s' is busy! Request is queued!",
          getName().c_str()));    
      _requestQueue.registerRequest(*pEvent, p);
      break;
    }
  }

  return status;
}

void GPAPropSetSession::wrongState(GCFEvent& e, GCFPortInterface& p, const char* state)
{
  string msg;   
  switch (e.signal)
  {
    case PA_REGISTER_SCOPE:
      msg = "Enable";
      RETURN_WRONG_STATE_MSG(PARegisterScope, PAScopeRegistered)
    case PA_UNREGISTER_SCOPE:
      msg = "Disable";
      RETURN_WRONG_STATE_MSG(PAUnregisterScope, PAScopeUnregistered)
    case PA_LOAD_PROP_SET:
      msg = "Load";
      RETURN_WRONG_STATE_MSG(PALoadPropSet, PAPropSetLoaded)
    case PA_UNLOAD_PROP_SET:
      msg = "Unload";
      RETURN_WRONG_STATE_MSG(PAUnloadPropSet, PAPropSetUnloaded)
    case PA_CONF_PROP_SET:
      msg = "Configure";
      RETURN_WRONG_STATE_MSG(PAConfPropSet, PAPropSetConf)
    case F_CLOSED:
      msg = "Closed";
      break;
      
    case PA_PROP_SET_DP_CREATED:
      msg = "DP created";
      break;
      
    case PA_PROP_SET_DP_DELETED:
      msg = "DP deleted";
      break;
      
    case PA_PROP_SET_LINKED:
      msg = "Linked";
      break;
      
    case PA_PROP_SET_UNLINKED:
      msg = "Unlinked";
      break;
  }

  LOG_WARN(formatString ( 
        "Could not perform '%s' on property set '%s'. Wrong state: %s",
        msg.c_str(),
        _propSet.name().c_str(),
        state));
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
