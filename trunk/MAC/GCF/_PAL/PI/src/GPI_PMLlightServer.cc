//#  GPI_PMLlightServer.cc: 
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

#include <GPI_PMLlightServer.h>
#include <GPI_Controller.h>
#include <GPI_PropertySet.h>
#include <GCF/Utils.h>
#include <GPI_PValue.h>

GPIPMLlightServer::GPIPMLlightServer(GPIController& controller, const string& name, bool transportRawData) : 
  GCFTask((State)&GPIPMLlightServer::initial, name),
  _controller(controller),
  _timerID(-1)
{
  // register the protocols for debugging purposes only
  registerProtocol(PI_PROTOCOL, PI_PROTOCOL_signalnames);
  registerProtocol(PA_PROTOCOL, PA_PROTOCOL_signalnames);

  // initialize the port to the real Client 
  _plsPort.init(*this, "server", GCFPortInterface::SPP, PI_PROTOCOL, transportRawData);
  // initialize the port to the the Property Agent (acts as a PML)
  _propertyAgent.init(*this, "client", GCFPortInterface::SAP, PA_PROTOCOL);
}

GPIPMLlightServer::~GPIPMLlightServer()
{
  for (TPropSetRegister::iterator iter = _propSetRegister.begin();
       iter != _propSetRegister.end(); ++iter)
  {
    delete iter->second;
  }         
}

GCFEvent::TResult GPIPMLlightServer::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      _propertyAgent.open();
      break;

    case F_CONNECTED:
      if (_plsPort.isConnected())
        TRAN(GPIPMLlightServer::operational);
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

void GPIPMLlightServer::registerPropSet(const PIRegisterScopeEvent& requestIn)
{
  if (!findPropertySet(requestIn.scope))
  {
    GPIPropertySet* pPropertySet = new GPIPropertySet(*this);
    
    _propSetRegister[requestIn.scope] = pPropertySet;
    _actionSeqList[requestIn.seqnr] = pPropertySet;
    pPropertySet->enable(requestIn);
  }
  else
  {
    PIScopeRegisteredEvent responseOut;
    responseOut.result = PI_PROP_SET_ALLREADY_EXISTS;
    responseOut.seqnr = requestIn.seqnr;
    sendMsgToClient(responseOut);
  }      

  LOG_INFO(formatString ( 
      "PL-REQ: Register scope %s",
      requestIn.scope.c_str()));  
}

void GPIPMLlightServer::propSetRegistered(const PAScopeRegisteredEvent& responseIn)
{
  GPIPropertySet* pPropertySet = findPropertySet(responseIn.seqnr);

  LOG_INFO(formatString ( 
      "PA-RESP: Scope %s is registered", 
      pPropertySet->getScope().c_str()));

  assert(pPropertySet);
  _actionSeqList.erase(responseIn.seqnr);
  pPropertySet->enabled(responseIn.result);
}

void GPIPMLlightServer::unregisterPropSet(const PIUnregisterScopeEvent& requestIn)
{
  GPIPropertySet* pPropertySet = findPropertySet(requestIn.scope);

  LOG_INFO(formatString ( 
      "PL-REQ: Unregister scope %s",
      requestIn.scope.c_str()));

  assert(pPropertySet);
  pPropertySet->disable(requestIn);
}

void GPIPMLlightServer::propSetUnregistered(const PAScopeUnregisteredEvent& responseIn)
{
  GPIPropertySet* pPropertySet = findPropertySet(responseIn.seqnr);

  LOG_INFO(formatString ( 
      "PA-RESP: Scope %s is unregistered", 
      pPropertySet->getScope().c_str()));
      
  assert(pPropertySet);

  pPropertySet->disabled(responseIn.result);
  _actionSeqList.erase(responseIn.seqnr);
  _propSetRegister.erase(pPropertySet->getScope());
  delete pPropertySet;
}

void GPIPMLlightServer::linkPropSet(const PALinkPropSetEvent& requestIn)
{
  GPIPropertySet* pPropertySet = findPropertySet(requestIn.scope);
  LOG_INFO(formatString ( 
      "PA-REQ: Link properties on scope %s", 
      requestIn.scope.c_str()));
  if (pPropertySet)
  {
    pPropertySet->linkPropSet(requestIn);
  }
  else
  {
    PAPropSetLinkedEvent responseOut;
    responseOut.result = PA_PS_GONE;
    responseOut.scope = requestIn.scope;
    sendMsgToPA(responseOut);
    LOG_DEBUG(formatString ( 
        "Property set with scope %d was deleted in the meanwhile", 
        responseOut.scope.c_str()));
  }
}

void GPIPMLlightServer::propSetLinked(const PIPropSetLinkedEvent& responseIn)
{
  GPIPropertySet* pPropertySet = findPropertySet(responseIn.scope);
  if (pPropertySet)
  {
    bool mustRetry = !pPropertySet->propSetLinkedInClient(responseIn);
    if (mustRetry && _timerID == -1)
    {
      _timerID = _plsPort.setTimer(0.0);
    }        
  }
}

void GPIPMLlightServer::unlinkPropSet(const PAUnlinkPropSetEvent& requestIn)
{
  GPIPropertySet* pPropertySet = findPropertySet(requestIn.scope);
  LOG_INFO(formatString ( 
      "PA-REQ: Unlink properties on scope %s", 
      requestIn.scope.c_str()));
  if (pPropertySet)
  {
    pPropertySet->unlinkPropSet(requestIn);
  }
  else
  {
    PAPropSetUnlinkedEvent responseOut;
    responseOut.result = PA_PS_GONE;
    responseOut.scope = requestIn.scope;
    sendMsgToPA(responseOut);
    LOG_DEBUG(formatString ( 
        "Property set with scope %d was deleted in the meanwhile", 
        responseOut.scope.c_str()));
  }
}

void GPIPMLlightServer::propSetUnlinked(const PIPropSetUnlinkedEvent& responseIn)
{
  GPIPropertySet* pPropertySet = findPropertySet(responseIn.scope);
  if (pPropertySet)
  {
    pPropertySet->propSetUnlinkedInClient(responseIn); 
  }
}
void GPIPMLlightServer::valueSet(const PIValueSetEvent& indication)
{
  const GCFPValue* pValue = indication.value._pValue;
  if (pValue)
  {
    _controller.getPropertyProxy().setPropValue(indication.name, *pValue);
  }
}

GCFEvent::TResult GPIPMLlightServer::operational(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  static int propertyAgentConnected = false;

  switch (e.signal)
  {      
    case F_ENTRY:
      if (_propertyAgent.isConnected()) propertyAgentConnected = true;
      break;
      
    case F_CONNECTED:
      if (&p == &_propertyAgent)
      {
        propertyAgentConnected = true;        
        for (TPropSetRegister::iterator iter = _propSetRegister.begin();
               iter != _propSetRegister.end(); ++iter)
        {
          iter->second->retryEnable();
        }
      }  
      break;

    case F_DISCONNECTED:
      if (&_propertyAgent != &p)
      {
        p.close(); // to avoid more DISCONNECTED signals for this port        
      }
      TRAN(GPIPMLlightServer::closing);
      break;

    case F_TIMER:
      if (&_propertyAgent != &p)
      {          
        p.open(); // try again
      }
      else
      {
        bool retryAgain(false);
        for (TPropSetRegister::iterator iter = _propSetRegister.begin();
             iter != _propSetRegister.end(); ++iter)
        {
          if (!iter->second->trySubscribing())
          {
            retryAgain = true;
          }
        }
        if (retryAgain) _plsPort.setTimer(0.0);
        else            _timerID = -1;
      }
      break;
     
    case PI_REGISTER_SCOPE:
    {
      PIRegisterScopeEvent requestIn(e);
      registerPropSet(requestIn);
      break;
    }  
    case PA_SCOPE_REGISTERED:
    {      
      PAScopeRegisteredEvent responseIn(e);
      propSetRegistered(responseIn);
      break;
    }
    case PI_UNREGISTER_SCOPE:
    {
      PIUnregisterScopeEvent requestIn(e);
      unregisterPropSet(requestIn);
      break;
    }
    case PA_SCOPE_UNREGISTERED:
    {
      PAScopeUnregisteredEvent responseIn(e);
      propSetUnregistered(responseIn);
      break;
    }
    case PA_LINK_PROP_SET:
    {
      PALinkPropSetEvent requestIn(e);
      linkPropSet(requestIn);
      break;
    }
    case PI_PROP_SET_LINKED:
    {
      PIPropSetLinkedEvent responseIn(e);
      propSetLinked(responseIn);
      break;
    }
    case PA_UNLINK_PROP_SET:
    {
      PAUnlinkPropSetEvent requestIn(e);
      unlinkPropSet(requestIn);
      break;
    }
    case PI_PROP_SET_UNLINKED:
    {
      PIPropSetUnlinkedEvent responseIn(e);
      propSetUnlinked(responseIn);
      break;
    }    
    case PI_VALUE_SET:
    {
      PIValueSetEvent indication(e);
      valueSet(indication);
      break;                                          
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GPIPMLlightServer::closing(GCFEvent& e, GCFPortInterface& p)
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

GPIPropertySet* GPIPMLlightServer::findPropertySet(const string& scope) const
{
  TPropSetRegister::const_iterator citer = _propSetRegister.find(scope);  
  return (citer != _propSetRegister.end() ? citer->second : 0);
}

GPIPropertySet* GPIPMLlightServer::findPropertySet(unsigned int seqnr) const
{
  TActionSeqList::const_iterator citer = _actionSeqList.find(seqnr);  
  return (citer != _actionSeqList.end() ? citer->second : 0);
}

void GPIPMLlightServer::sendMsgToPA(GCFEvent& e)
{
  if (_propertyAgent.isConnected())
  {    
    _propertyAgent.send(e);
  }
}

void GPIPMLlightServer::sendMsgToClient(GCFEvent& msg)
{
  if (_plsPort.isConnected())
  {    
    _plsPort.send(msg);
  }
}
