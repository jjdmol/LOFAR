//#  GPA_PropertySet.cc:
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

#include "GPA_PropertySet.h"
#include "GPA_Controller.h"
#include <strings.h>
#include <stdio.h>
#include <unistd.h>

bool operator == (const GPAPropertySet::TPSClient& a, const GPAPropertySet::TPSClient& b)
{
  return (a.pPSClientPort == b.pPSClientPort);
}


GPAPropertySet::GPAPropertySet(GPAController& controller, GCFPortInterface& serverPort) :
  _controller(controller),
  _usecount(0),
  _name(""),
  _type(""),
  _serverPort(serverPort),
  _isTemporary(true),
  _state(S_DISABLED),
  _counter(0),
  _savedResult(PA_NO_ERROR),
  _savedSeqnr(0)  
{
}

GPAPropertySet::~GPAPropertySet()
{
}

void GPAPropertySet::enable(PARegisterScopeEvent& request)
{
  PAScopeRegisteredEvent response;
  response.seqnr = request.seqnr;
  response.result = PA_NO_ERROR;
  switch (_state)
  {
    case S_DISABLED:
    {
      _state = S_ENABLING;
      assert(_psClients.size() == 0);
      _name = request.scope;
      _type = request.type;
      _isTemporary = request.isTemporary;
      TSAResult saResult(SA_NO_ERROR);
      if (!typeExists(_type))
      {
        LOG_INFO(LOFAR::formatString (
            "Type %s is not knwon in the PVSS DB!",
            _type.c_str()));
        response.result = PA_DPTYPE_UNKNOWN;
      }
      else if (dpeExists(_name) && _isTemporary)
      {
        LOG_INFO(LOFAR::formatString (
            "Temporary DP '%s' already exists!",
            _type.c_str()));
        response.result = PA_PS_ALLREADY_EXISTS;
      }
      else if (!dpeExists(_name) && !_isTemporary)
      {
        LOG_INFO(LOFAR::formatString (
            "Permanent DP '%s' does not exists!",
            _type.c_str()));
        response.result = PA_PS_NOT_EXISTS;
      }
      else if (_isTemporary)
      {
        if ((saResult = dpCreate(_name + string("_temp"), string("GCFTempRef"))) != SA_NO_ERROR)
        {
          if (saResult == SA_DPTYPE_UNKNOWN)
          {
            LOG_FATAL("Please check the existens of dpType 'GCFTempRef' in PVSS DB!!!");
          }
          response.result = PA_INTERNAL_ERROR;
        }
      }
      if (response.result != PA_NO_ERROR)
      {
        // not enabled due to errors; respond with error
        _state = S_DISABLED;
        _controller.sendAndNext(response);
      }
      break;
    }
    default:
      wrongState("enable");
      response.result = PA_WRONG_STATE;
      _controller.sendAndNext(response);     
      break;
  }
}

void GPAPropertySet::disable(PAUnregisterScopeEvent& request)
{
  PAScopeUnregisteredEvent response;
  response.seqnr = request.seqnr;
  response.result = PA_NO_ERROR;
  _savedSeqnr = request.seqnr;
  switch (_state)
  {
    case S_LINKED:
    {
      PAPropSetGoneEvent indication;
      indication.scope = _name;
      GCFPortInterface* pPSClientPort;
      for (TPSClients::iterator iter = _psClients.begin();
           iter != _psClients.end(); ++iter)
      {
        pPSClientPort = iter->pPSClientPort;
        assert(pPSClientPort);
        
        if (pPSClientPort->isConnected())
        {          
          pPSClientPort->send(indication);
        }
      }
      // list will be automatically cleaned up on destruction of this class
    }
    // intentional fall through
    case S_ENABLED:
    {
      _state = S_DISABLING;
      _counter = 0;
      if (_isTemporary)
      {
        if (dpeExists(_name + string("_temp")))
        {
          LOG_INFO(LOFAR::formatString (
              "%s_temp still exists! Will be removed too!",
              _name.c_str()));
          if (dpDelete(_name + string("_temp")) != SA_NO_ERROR)
          {
            response.result = PA_INTERNAL_ERROR;
          }
          else
          {
            _counter += 1;
          }
        }
        if (dpeExists(_name))
        {
          LOG_INFO(LOFAR::formatString (
              "%s still exists! Will be removed too!",
              _name.c_str()));
          if (dpDelete(_name) != SA_NO_ERROR)
          {
            response.result = PA_INTERNAL_ERROR;
          }          
          else
          {
            _counter += 1;
          }
        }
      }
      
      if (_counter == 0)
      {
        _state = S_DISABLED;
        _controller.sendAndNext(response);
      }
      else
      {
        _savedResult = response.result;
      }
      _usecount = 0;
      break;
    }
    default:
      wrongState("disable");
      response.result = PA_WRONG_STATE;
      _controller.sendAndNext(response);     
      break;
  }
}

void GPAPropertySet::load(PALoadPropSetEvent& request, GCFPortInterface& p)
{
  PAPropSetLoadedEvent response;
  response.seqnr = request.seqnr;
  response.result = PA_NO_ERROR;
  _savedSeqnr = request.seqnr;
  switch (_state)
  {
    case S_ENABLED:
    {
      _state = S_LINKING;
      assert(_usecount == 0);
      if (_isTemporary)
      {
        if (dpCreate(_name, _type) != SA_NO_ERROR)
        {
          response.result = PA_INTERNAL_ERROR;        
        }                 
      }
      else
      {
        linkPropSet();
      }
      if (response.result != PA_NO_ERROR)
      {
        _controller.sendAndNext(response);
        _state = S_ENABLED;
      }
      else
      {
        // In case a loaded property set will be disabled by the owner (server MCA),
        // we need a mechanism to inform all client MCA's (which has loaded 
        // this property set). 
        // That's why we remember the port of the requestor (client MCA). The 
        // combination of the following two reasons explains the need of extending 
        // the port with a load counter:
        //    - a requestor can has loaded the same property set (same scope) 
        //      more than once (on different places) at the same time and 
        //    - we need to know when this requestor may be deleted from the 
        //      clientport list and thus not needed to be informed about disabling 
        //      the prop. set anymore. 
        // If client has allready loaded a property set with this scope (_name),
        // only the counter needed to be increased.
        // On unloading the counter will be decreased and if counter 
        // is 0 the requestor can be deleted from the list. Then the requestor
        // not needed to be and also will not be informed about disabling the 
        // property set.
        TPSClient* pPSClient = findClient(p);
        if (pPSClient)
        {
          pPSClient->count++;
        }
        else // client not known yet
        {
          TPSClient psClient;
          psClient.pPSClientPort = &p;
          psClient.count = 1;
          _psClients.push_back(psClient);
        }
      }
      break;
    }
    case S_LINKED:
      assert(dpeExists(_name));
      _usecount++;
      _controller.sendAndNext(response);
      break;
      
    default:
      wrongState("load");
      response.result = PA_WRONG_STATE;
      _controller.sendAndNext(response);     
      break;
  }  
}

void GPAPropertySet::unload(PAUnloadPropSetEvent& request, const GCFPortInterface& p)
{
  PAPropSetUnloadedEvent response;
  response.seqnr = request.seqnr;
  response.result = PA_NO_ERROR;
  _savedSeqnr = request.seqnr;
  _savedResult = PA_NO_ERROR;
  switch (_state)
  {
    case S_LINKED:
    {
      _usecount--;
      if (_usecount == 0)
      {         
        _state = S_UNLINKING;        
        if (_isTemporary)
        {
          _counter = 0;
          if (dpDelete(_name) != SA_NO_ERROR)
          {
            response.result = PA_INTERNAL_ERROR;        
          }            
          else
          {
            _counter += 1;
          }     
          if (dpDelete(_name + string("_temp")) != SA_NO_ERROR)
          {
            response.result = PA_INTERNAL_ERROR;        
          }
          else
          {
            _counter += 1;
          }     
        }
        else
        {
          unlinkPropSet();
        }
      }
      else
      {
        _controller.sendAndNext(response);
      }

      if (response.result != PA_NO_ERROR)
      {
        if (_counter == 0)
        {
          _state = S_ENABLED;
          _controller.sendAndNext(response);
        }
        else
        {
          _savedResult = response.result;
        }
      }
      // decrease the load counter and remove the client (if counter == 0),
      // see also 'load'
      TPSClient* pPSClient = findClient(p);
      if (pPSClient)
      {
        pPSClient->count--;
        if (pPSClient->count == 0)
        {
          _psClients.remove(*pPSClient);
        }
      }
      break;
    }
      
    default:
      wrongState("unload");
      response.result = PA_WRONG_STATE;
      _controller.sendAndNext(response);     
      break;
  }  
}

void GPAPropertySet::configure(PAConfPropSetEvent& request)
{
  //TODO: implement configure
}

void GPAPropertySet::linked(PAPropSetLinkedEvent& response)
{
  PAPropSetLoadedEvent loadedResponse;
  loadedResponse.seqnr = _savedSeqnr;
  if (_state == S_LINKING)
  {
    _state = S_LINKED;
    loadedResponse.result = response.result;
  }
  else
  {
    _state = S_ENABLED;
    wrongState("linked");
    loadedResponse.result = PA_WRONG_STATE;
  }
  _controller.sendAndNext(loadedResponse);
}

void GPAPropertySet::unlinked(PAPropSetUnlinkedEvent& response)
{
  PAPropSetUnloadedEvent unloadedResponse;
  unloadedResponse.seqnr = _savedSeqnr;
  if (_state == S_UNLINKING)
  {
    unloadedResponse.result = (_savedResult != PA_NO_ERROR ? _savedResult : response.result);
  }
  else
  {
    wrongState("unlinked");
    unloadedResponse.result = PA_WRONG_STATE;
  }
  _state = S_ENABLED;
  _controller.sendAndNext(unloadedResponse);
}

void GPAPropertySet::deleteClient(const GCFPortInterface& p)
{
  if (&p == &_serverPort)
  { 
    // This property set must be disabled because the server does not
    // exists anymore.
    // So pretend a disable request is received
    PAUnregisterScopeEvent request;
    request.scope = _name;
    request.seqnr = 0;
    disable(request);
  }
  else
  {
    // This means that all load requests of client 'p' have to be undone.
    // So pretend a unload request is received
    TPSClient* pPSClient = findClient(p);
    if (pPSClient)
    {
      // this manipulation of the _usecount and the load counter pretends 
      // that this is the last unload request of the client 'p' for this
      // property set
      _usecount -= (pPSClient->count - 1);
      pPSClient->count = 1;

      PAUnloadPropSetEvent request;
      request.scope = _name;
      request.seqnr = 0;
      unload(request, p);
    }  
  }
}

void GPAPropertySet::dpCreated(const string& dpName)
{
  switch (_state)
  {
    case S_ENABLING:
      assert(dpName == (_name + string("_temp")));
      _state = S_ENABLED;
      break;
    case S_LINKING:
      assert(dpName == _name);
      linkPropSet();
      break;
    default:
      wrongState("dpCreated");
      break;      
  }
}

void GPAPropertySet::dpDeleted(const string& /*dpName*/)
{
  switch (_state)
  {
    case S_DISABLING:
      _counter--;
      if (_counter == 0)
      {
        _state = S_DISABLED;
        PAScopeUnregisteredEvent response;
        response.seqnr = _savedSeqnr;
        response.result = _savedResult;
        _controller.sendAndNext(response);
      }
      break;
    case S_UNLINKING:
      _counter--;
      if (_counter == 0)
      {
        unlinkPropSet();
      }
      break;
    default:
      wrongState("dpDeleted");
      break;      
  }
}

void GPAPropertySet::linkPropSet()
{
  assert(dpeExists(_name));
  //TODO: what if connection is realy disabled 
  assert(_serverPort.isConnected()); 
  PALinkPropSetEvent linkRequest;
  linkRequest.scope = _name;
  _serverPort.send(linkRequest);
}

void GPAPropertySet::unlinkPropSet()
{
  assert(dpeExists(_name));
  //TODO: what if connection is realy disabled 
  assert(_serverPort.isConnected());
  PAUnlinkPropSetEvent unlinkRequest;
  unlinkRequest.scope = _name;
  _serverPort.send(unlinkRequest);
}

GPAPropertySet::TPSClient* GPAPropertySet::findClient(const GCFPortInterface& p) 
{
  TPSClient* pPSClient(0);
  for (TPSClients::iterator iter = _psClients.begin();
       iter != _psClients.end(); ++iter)
  {
    if (iter->pPSClientPort == &p)
    {
      pPSClient = &(*iter);
      break;
    }
  } 
  return pPSClient;
}

void GPAPropertySet::wrongState(const char* request)
{
  char* stateString(0);
  switch (_state)
  {
    case S_DISABLED:
      stateString = "DISABLED";
      break;
    case S_DISABLING:
      stateString = "DISABLING";
      break;
    case S_ENABLED:
      stateString = "ENABLED";
      break;
    case S_ENABLING:
      stateString = "ENABLING";
      break;
    case S_LINKING:
      stateString = "LINKING";
      break;
    case S_UNLINKING:
      stateString = "UNLINKING";
      break;
    case S_LINKED:
      stateString = "LINKED";
      break;
  }
  LOG_WARN(LOFAR::formatString ( 
        "Could not perform '%s' on property set '%s'. Wrong state: %s",
        request,
        stateString));  
}
