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

#include <PI_Protocol.ph>
#include <PA_Protocol.ph>

static string sPLSTaskName("PI");

GPIPMLlightServer::GPIPMLlightServer(GPIController& controller) : 
  GCFTask((State)&GPIPMLlightServer::initial, sPLSTaskName),
  _controller(controller)
{
  // register the protocols for debugging purposes only
  registerProtocol(PI_PROTOCOL, PI_PROTOCOL_signalnames);
  registerProtocol(PA_PROTOCOL, PA_PROTOCOL_signalnames);

  // initialize the port to the real RTC 
  _plsPort.init(*this, "server", GCFPortInterface::SPP, PI_PROTOCOL);
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
      _controller.getPortProvider().accept(_plsPort);
      // intential fall through
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

GCFEvent::TResult GPIPMLlightServer::operational(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static GPIPropertySet* pPropertySet = 0;
  static int timerID = -1;
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
        PARegisterScopeEvent requestOut;
        for (TPropSetRegister::iterator iter = _propSetRegister.begin();
               iter != _propSetRegister.end(); ++iter)
        {
          iter->second->retryEnable();
        }
      }  
      break;

    case F_DISCONNECTED:
      if (&_propertyAgent == &p)
      {
        assert(!propertyAgentConnected);
        p.setTimer(1.0); // try again after 1 second        
      }
      else
      {
        p.close(); // to avoid more DISCONNECTED signals for this port
        TRAN(GPIPMLlightServer::closing);
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
        for (TPropSetRegister::iterator iter = _propSetRegister.begin();
             iter != _propSetRegister.end(); ++iter)
        {
          if (!iter->second->trySubscribing())
          {
            retryAgain = true;
          }
        }
        if (retryAgain) _plsPort.setTimer(0.0);
        else            timerID = -1;
      }
      break;
     
    case PI_REGISTER_SCOPE:
    {
      PIRegisterScopeEvent requestIn(e);
      if (!findPropertySet(requestIn.scope))
      {
        pPropertySet = new GPIPropertySet(*this);
        
        _propSetRegister[requestIn.scope] = pPropertySet;
        _actionSeqList[requestIn.seqnr] = pPropertySet;
        pPropertySet->enable(requestIn);
      }
      else
      {
        PIScopeRegisteredEvent responseOut;
        responseOut.result = PI_PROP_SET_ALLREADY_EXISTS;
        responseOut.seqnr = requestIn.seqnr;
        _plsPort.send(responseOut);
      }      

      LOG_INFO(LOFAR::formatString ( 
          "SS-REQ: Register scope %s",
          requestIn.scope.c_str()));
      break;
    }  
    case PA_SCOPE_REGISTERED:
    {      
      PAScopeRegisteredEvent responseIn(e);

      pPropertySet = findPropertySet(responseIn.seqnr);

      LOG_INFO(LOFAR::formatString ( 
          "PA-RESP: Scope %s is registered", 
          pPropertySet->getScope().c_str()));

      assert(pPropertySet);
      _actionSeqList.erase(responseIn.seqnr);
      pPropertySet->enabled(responseIn.result);
      break;
    }

    case PI_UNREGISTER_SCOPE:
    {
      PIUnregisterScopeEvent requestIn(e);
      pPropertySet = findPropertySet(requestIn.scope);

      LOG_INFO(LOFAR::formatString ( 
          "SS-REQ: Unregister scope %s",
          requestIn.scope.c_str()));

      assert(pPropertySet);
      pPropertySet->disable(requestIn);
      break;
    }  

    case PA_SCOPE_UNREGISTERED:
    {
      PAScopeUnregisteredEvent responseIn(e);
      pPropertySet = findPropertySet(responseIn.seqnr);

      LOG_INFO(LOFAR::formatString ( 
          "PA-RESP: Scope %s is unregistered", 
          pPropertySet->getScope().c_str()));
          
      assert(pPropertySet);

      pPropertySet->disabled(responseIn.result);
      _actionSeqList.erase(responseIn.seqnr);
      _propSetRegister.erase(pPropertySet->getScope());
      delete pPropertySet;

      break;
    }

    case PA_LINK_PROP_SET:
    {
      PALinkPropSetEvent requestIn(e);
      pPropertySet = findPropertySet(requestIn.scope);
      LOG_INFO(LOFAR::formatString ( 
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
        replyMsgToPA(responseOut);
        LOG_DEBUG(LOFAR::formatString ( 
            "Property set with scope %d was deleted in the meanwhile", 
            responseOut.scope.c_str()));
      }
      break;
    }
    case PI_PROP_SET_LINKED:
    {
      PIPropSetLinkedEvent responseIn(e);
      pPropertySet = findPropertySet(responseIn.scope);
      if (pPropertySet)
      {
        bool mustRetry = !pPropertySet->propSetLinkedInRTC(responseIn);
        if (mustRetry && timerID == -1)
        {
          timerID = _plsPort.setTimer(0.0);
        }        
      }
      break;
    }
    case PA_UNLINK_PROP_SET:
    {
      PAUnlinkPropSetEvent requestIn(e);
      pPropertySet = findPropertySet(requestIn.scope);
      LOG_INFO(LOFAR::formatString ( 
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
        replyMsgToPA(responseOut);
        LOG_DEBUG(LOFAR::formatString ( 
            "Property set with scope %d was deleted in the meanwhile", 
            responseOut.scope.c_str()));
      }
      break;
    }

    case PI_PROP_SET_UNLINKED:
    {
      PIPropSetUnlinkedEvent responseIn(e);
      pPropertySet = findPropertySet(responseIn.scope);
      if (pPropertySet)
      {
        pPropertySet->propSetUnlinkedInRTC(responseIn); 
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

GPIPropertySet* GPIPMLlightServer::findPropertySet(string& scope)
{
  TPropSetRegister::iterator iter = _propSetRegister.find(scope);  
  return (iter != _propSetRegister.end() ? iter->second : 0);
}

GPIPropertySet* GPIPMLlightServer::findPropertySet(unsigned int seqnr)
{
  TActionSeqList::iterator iter = _actionSeqList.find(seqnr);  
  return (iter != _actionSeqList.end() ? iter->second : 0);
}

void GPIPMLlightServer::replyMsgToPA(GCFEvent& e)
{
  if (_propertyAgent.isConnected())
  {    
    _propertyAgent.send(e);
  }
}
