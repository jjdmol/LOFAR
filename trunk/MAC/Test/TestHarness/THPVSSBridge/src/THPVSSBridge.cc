//
//  THPVSSBridge.cc: Implementation of the THPVSSBridge task class.
//
//  Copyright (C) 2003
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/GCF_PValue.h>


#include "THPVSSBridge.h"
#include "THPVSSBridge_Protocol.ph"

using namespace LOFAR;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace LOFAR::APLCommon;
using namespace LOFAR::MACTest;

INIT_TRACER_CONTEXT(LOFAR::MACTest::THPVSSBridge,LOFARLOGGER_PACKAGE);

string stripProperty(string in)
{
  return in.substr(0,in.rfind('.'));
}

string getProperty(string in)
{
  string property("");
  string::size_type dotPos = in.rfind('.');
  if(dotPos != string::npos)
  {
    property = in.substr(dotPos+1);
  }
  return property;
}

THPVSSBridge::THPVSSBridge(string name) : 
  GCFTask((State)&THPVSSBridge::initial, name),
  PropertySetAnswerHandlerInterface(),
  m_propertySetAnswer(*this),
  m_myPropertySets(),
  m_extPropertySets(),
  m_serverPortName(string("server")),
  m_serverPort(*this, m_serverPortName, GCFPortInterface::SPP, THPVSSBRIDGE_PROTOCOL),
  m_propertyProxy(*this),
  m_proxySubscriptions()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

  // register the protocol for debugging purposes
  registerProtocol(THPVSSBRIDGE_PROTOCOL, THPVSSBRIDGE_PROTOCOL_signalnames);
}

THPVSSBridge::~THPVSSBridge()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  _flushSubscriptions();
}

void THPVSSBridge::handlePropertySetAnswer(GCFEvent& answer)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  switch(answer.signal)
  {
    case F_SUBSCRIBED:
    {
      GCFPropAnswerEvent* pPropAnswer=static_cast<GCFPropAnswerEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s)","F_SUBSCRIBED",pPropAnswer->pPropName));
      
      THPVSSBridgeSubscribeExtPropertyResponseEvent pvssBridgeEvent;
      pvssBridgeEvent.scope = stripProperty(string(pPropAnswer->pPropName));
      pvssBridgeEvent.property = getProperty(string(pPropAnswer->pPropName));
      pvssBridgeEvent.response = GCF_NO_ERROR;
      m_serverPort.send(pvssBridgeEvent);
      break;
    }
    
    case F_UNSUBSCRIBED:
    {
      GCFPropAnswerEvent* pPropAnswer=static_cast<GCFPropAnswerEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s)","F_UNSUBSCRIBED",pPropAnswer->pPropName));

      THPVSSBridgeUnsubscribeExtPropertyResponseEvent pvssBridgeEvent;
      pvssBridgeEvent.scope = stripProperty(string(pPropAnswer->pPropName));
      pvssBridgeEvent.property = getProperty(string(pPropAnswer->pPropName));
      pvssBridgeEvent.response = GCF_NO_ERROR;
      m_serverPort.send(pvssBridgeEvent);
      break;
    }
    
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s=%s)","F_VCHANGEMSG",pPropAnswer->pPropName,pPropAnswer->pValue->getValueAsString().c_str()));
      
      THPVSSBridgeValueChangeResponseEvent pvssBridgeEvent;
      pvssBridgeEvent.scope = stripProperty(string(pPropAnswer->pPropName));
      pvssBridgeEvent.property = getProperty(string(pPropAnswer->pPropName));
      pvssBridgeEvent.value = pPropAnswer->pValue->getValueAsString();
      m_serverPort.send(pvssBridgeEvent);
      break;
    }  
    
    case F_VGETRESP:
    {
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s=%s)","F_VGETRESP",pPropAnswer->pPropName,pPropAnswer->pValue->getValueAsString().c_str()));
      
      THPVSSBridgeValueChangeResponseEvent pvssBridgeEvent;
      pvssBridgeEvent.scope = stripProperty(string(pPropAnswer->pPropName));
      pvssBridgeEvent.property = getProperty(string(pPropAnswer->pPropName));
      pvssBridgeEvent.value = pPropAnswer->pValue->getValueAsString();
      m_serverPort.send(pvssBridgeEvent);
      break;
    }  
    
    case F_VSETRESP:
    {
      GCFPropAnswerEvent* pPropAnswer=static_cast<GCFPropAnswerEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s)","F_VSETRESP",pPropAnswer->pPropName));

      THPVSSBridgeExtSetValueResponseEvent pvssBridgeEvent;
      pvssBridgeEvent.scope = stripProperty(string(pPropAnswer->pPropName));
      pvssBridgeEvent.property = getProperty(string(pPropAnswer->pPropName));
      pvssBridgeEvent.response = GCF_NO_ERROR;
      m_serverPort.send(pvssBridgeEvent);
      break;
    }
    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s,%d)","F_EXTPS_LOADED",pPropAnswer->pScope,pPropAnswer->result));

      THPVSSBridgeLoadExtPropertySetResponseEvent pvssBridgeEvent;
      pvssBridgeEvent.scope = string(pPropAnswer->pScope);
      pvssBridgeEvent.response = pPropAnswer->result;
      m_serverPort.send(pvssBridgeEvent);

      break;
    }
    
    case F_EXTPS_UNLOADED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s,%d)","F_EXTPS_UNLOADED",pPropAnswer->pScope,pPropAnswer->result));

      THPVSSBridgeUnloadExtPropertySetResponseEvent pvssBridgeEvent;
      pvssBridgeEvent.scope = string(pPropAnswer->pScope);
      pvssBridgeEvent.response = pPropAnswer->result;
      m_serverPort.send(pvssBridgeEvent);
      break;
    }
    
    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s,%d)","F_MYPS_ENABLED",pPropAnswer->pScope,pPropAnswer->result));

      THPVSSBridgeEnableMyPropertySetResponseEvent pvssBridgeEvent;
      pvssBridgeEvent.scope = string(pPropAnswer->pScope);
      pvssBridgeEvent.response = pPropAnswer->result;
      m_serverPort.send(pvssBridgeEvent);
      break;
    }
    
    case F_MYPS_DISABLED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s,%d)","F_MYPS_DISABLED",pPropAnswer->pScope,pPropAnswer->result));

      THPVSSBridgeDisableMyPropertySetResponseEvent pvssBridgeEvent;
      pvssBridgeEvent.scope = string(pPropAnswer->pScope);
      pvssBridgeEvent.response = pPropAnswer->result;
      m_serverPort.send(pvssBridgeEvent);
      break;
    }
    
    case F_SERVER_GONE:
      LOG_DEBUG(formatString("%s - ignored","F_SERVER_GONE"));
      break;

    case F_PS_CONFIGURED:
      LOG_DEBUG(formatString("%s - ignored","F_PS_CONFIGURED"));
      break;

    default:
      LOG_DEBUG(formatString("%s - ignored","default"));
      break;
  }  
}

GCFEvent::TResult THPVSSBridge::initial(GCFEvent& e, GCFPortInterface& p)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(e)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      _flushSubscriptions();
      m_serverPort.open();
      break;

    case F_CONNECTED:
      TRAN(THPVSSBridge::connected);
      break;

    case F_DISCONNECTED:
      p.setTimer(5.0); // try again after 5 seconds
      break;

    case F_TIMER:
      p.open(); // try again
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult THPVSSBridge::connected(GCFEvent& e, GCFPortInterface& p)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(e)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_DISCONNECTED:
      p.close();
      break;

    case F_CLOSED:
      TRAN(THPVSSBridge::initial);
      break;

    case THPVSSBRIDGE_ENABLE_MY_PROPERTY_SET:
    {
      THPVSSBridgeEnableMyPropertySetEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s,%d)","THPVSSBRIDGE_ENABLE_MY_PROPERTY_SET",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.type.c_str(),pvssBridgeEvent.category));
      
      TMyPropertySetPtr pMyPropSet(new GCFMyPropertySet(pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.type.c_str(),pvssBridgeEvent.category,&m_propertySetAnswer));
      m_myPropertySets[pvssBridgeEvent.scope] = pMyPropSet;
      TGCFResult res = pMyPropSet->enable();
      if(GCF_NO_ERROR != res)
      {
        THPVSSBridgeEnableMyPropertySetResponseEvent pvssBridgeResponseEvent;
        pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
        pvssBridgeResponseEvent.response = res;
        m_serverPort.send(pvssBridgeResponseEvent);
      }
      break;
    }

    case THPVSSBRIDGE_DISABLE_MY_PROPERTY_SET:
    {
      THPVSSBridgeDisableMyPropertySetEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s)","THPVSSBRIDGE_DISABLE_MY_PROPERTY_SET",pvssBridgeEvent.scope.c_str()));

      TMyPropertySetMap::iterator it = m_myPropertySets.find(pvssBridgeEvent.scope);
      if(it != m_myPropertySets.end())
      {
        TGCFResult res = it->second->disable();
        if(GCF_NO_ERROR != res)
        {
          THPVSSBridgeDisableMyPropertySetResponseEvent pvssBridgeResponseEvent;
          pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
          pvssBridgeResponseEvent.response = res;
          m_serverPort.send(pvssBridgeResponseEvent);
        }
      }
      else
      {
        LOG_FATAL(formatString("PropertySet %s not found. Did you enable it?",pvssBridgeEvent.scope.c_str()));
        
        THPVSSBridgeDisableMyPropertySetResponseEvent pvssBridgeResponseEvent;
        pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
        pvssBridgeResponseEvent.response = GCF_MYPS_DISABLE_ERROR;
        m_serverPort.send(pvssBridgeResponseEvent);
      }
      break;
    }

    case THPVSSBRIDGE_MY_GET_VALUE:
    {
      THPVSSBridgeMyGetValueEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s)","THPVSSBRIDGE_MY_GET_VALUE",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.property.c_str()));

      THPVSSBridgeMyGetValueResponseEvent pvssBridgeResponseEvent;
      pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
      pvssBridgeResponseEvent.property = pvssBridgeEvent.property;
      pvssBridgeResponseEvent.value = string("");
      
      TMyPropertySetMap::iterator it = m_myPropertySets.find(pvssBridgeEvent.scope);
      if(it != m_myPropertySets.end())
      {
        GCFPValue* pValue = it->second->getValue(pvssBridgeEvent.property);
        pvssBridgeResponseEvent.value = pValue->getValueAsString();
      }
      else
      {
        LOG_FATAL(formatString("PropertySet %s not found. Did you enable it?",pvssBridgeEvent.scope.c_str()));
      }
      m_serverPort.send(pvssBridgeResponseEvent);
      break;
    }

    case THPVSSBRIDGE_MY_SET_VALUE:
    {
      THPVSSBridgeMySetValueEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s,%s)","THPVSSBRIDGE_MY_SET_VALUE",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.property.c_str(),pvssBridgeEvent.value.c_str()));

      TMyPropertySetMap::iterator it = m_myPropertySets.find(pvssBridgeEvent.scope);
      if(it != m_myPropertySets.end())
      {
        TGCFResult res = it->second->setValue(pvssBridgeEvent.property,pvssBridgeEvent.value);
        if(GCF_NO_ERROR != res)
        {
          THPVSSBridgeMySetValueResponseEvent pvssBridgeResponseEvent;
          pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
          pvssBridgeResponseEvent.property = pvssBridgeEvent.property;
          pvssBridgeResponseEvent.response = res;
          m_serverPort.send(pvssBridgeResponseEvent);
        }
      }
      else
      {
        LOG_FATAL(formatString("PropertySet %s not found. Did you enable it?",pvssBridgeEvent.scope.c_str()));

        THPVSSBridgeMySetValueResponseEvent pvssBridgeResponseEvent;
        pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
        pvssBridgeResponseEvent.property = pvssBridgeEvent.property;
        pvssBridgeResponseEvent.response = GCF_MYPS_ENABLE_ERROR;
        m_serverPort.send(pvssBridgeResponseEvent);
      }
      break;
    }

    case THPVSSBRIDGE_LOAD_EXT_PROPERTY_SET:
    {
      THPVSSBridgeLoadExtPropertySetEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s)","THPVSSBRIDGE_LOAD_EXT_PROPERTY_SET",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.type.c_str()));

      TExtPropertySetPtr pExtPropSet(new GCFExtPropertySet(pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.type.c_str(),&m_propertySetAnswer));
      m_extPropertySets[pvssBridgeEvent.scope] = pExtPropSet;
      TGCFResult res = pExtPropSet->load();
      if(GCF_NO_ERROR != res)
      {
        THPVSSBridgeLoadExtPropertySetResponseEvent pvssBridgeResponseEvent;
        pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
        pvssBridgeResponseEvent.response = res;
        m_serverPort.send(pvssBridgeResponseEvent);
      }
      break;
    }

    case THPVSSBRIDGE_UNLOAD_EXT_PROPERTY_SET:
    {
      THPVSSBridgeUnloadExtPropertySetEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s)","THPVSSBRIDGE_UNLOAD_EXT_PROPERTY_SET",pvssBridgeEvent.scope.c_str()));

      TExtPropertySetMap::iterator it = m_extPropertySets.find(pvssBridgeEvent.scope);
      if(it != m_extPropertySets.end())
      {
        TGCFResult res = it->second->unload();
        if(GCF_NO_ERROR != res)
        {
          THPVSSBridgeUnloadExtPropertySetResponseEvent pvssBridgeResponseEvent;
          pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
          pvssBridgeResponseEvent.response = res;
          m_serverPort.send(pvssBridgeResponseEvent);
        }
      }
      else
      {
        LOG_FATAL(formatString("PropertySet %s not found. Did you load it?",pvssBridgeEvent.scope.c_str()));

        THPVSSBridgeUnloadExtPropertySetResponseEvent pvssBridgeResponseEvent;
        pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
        pvssBridgeResponseEvent.response = GCF_EXTPS_UNLOAD_ERROR;
        m_serverPort.send(pvssBridgeResponseEvent);
      }
      break;
    }

    case THPVSSBRIDGE_SUBSCRIBE_EXT_PROPERTY:
    {
      THPVSSBridgeSubscribeExtPropertyEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s)","THPVSSBRIDGE_SUBSCRIBE_EXT_PROPERTY_SET",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.property.c_str()));

      TExtPropertySetMap::iterator it = m_extPropertySets.find(pvssBridgeEvent.scope);
      if(it != m_extPropertySets.end())
      {
        TGCFResult res = it->second->subscribeProp(pvssBridgeEvent.property);
        if(GCF_NO_ERROR != res)
        {
          THPVSSBridgeSubscribeExtPropertyResponseEvent pvssBridgeResponseEvent;
          pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
          pvssBridgeResponseEvent.property = pvssBridgeEvent.property;
          pvssBridgeResponseEvent.response = res;
          m_serverPort.send(pvssBridgeResponseEvent);
        }
      }
      else
      {
        LOG_FATAL(formatString("PropertySet %s not found. Did you load it?",pvssBridgeEvent.scope.c_str()));
        
        THPVSSBridgeSubscribeExtPropertyResponseEvent pvssBridgeResponseEvent;
        pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
        pvssBridgeResponseEvent.property = pvssBridgeEvent.property;
        pvssBridgeResponseEvent.response = GCF_EXTPS_LOAD_ERROR;
        m_serverPort.send(pvssBridgeResponseEvent);
      }
      break;
    }

    case THPVSSBRIDGE_UNSUBSCRIBE_EXT_PROPERTY:
    {
      THPVSSBridgeUnsubscribeExtPropertyEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s)","THPVSSBRIDGE_UNSUBSCRIBE_EXT_PROPERTY_SET",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.property.c_str()));

      THPVSSBridgeUnsubscribeExtPropertyResponseEvent pvssBridgeResponseEvent;
      pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
      pvssBridgeResponseEvent.property = pvssBridgeEvent.property;

      TExtPropertySetMap::iterator it = m_extPropertySets.find(pvssBridgeEvent.scope);
      if(it != m_extPropertySets.end())
      {
        pvssBridgeResponseEvent.response = it->second->unsubscribeProp(pvssBridgeEvent.property);
      }
      else
      {
        LOG_FATAL(formatString("PropertySet %s not found. Did you load it?",pvssBridgeEvent.scope.c_str()));
        pvssBridgeResponseEvent.response = GCF_EXTPS_LOAD_ERROR;
      }
      m_serverPort.send(pvssBridgeResponseEvent);
      break;
    }

    case THPVSSBRIDGE_EXT_GET_VALUE:
    {
      THPVSSBridgeExtGetValueEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s)","THPVSSBRIDGE_EXT_GET_VALUE",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.property.c_str()));

      TExtPropertySetMap::iterator it = m_extPropertySets.find(pvssBridgeEvent.scope);
      if(it != m_extPropertySets.end())
      {
        TGCFResult res = it->second->requestValue(pvssBridgeEvent.property);
        if(GCF_NO_ERROR != res)
        {
          THPVSSBridgeExtGetValueResponseEvent pvssBridgeResponseEvent;
          pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
          pvssBridgeResponseEvent.property = pvssBridgeEvent.property;
          pvssBridgeResponseEvent.value = string("");
          m_serverPort.send(pvssBridgeResponseEvent);
        }
      }
      else
      {
        LOG_FATAL(formatString("PropertySet %s not found. Did you load it?",pvssBridgeEvent.scope.c_str()));

        THPVSSBridgeExtGetValueResponseEvent pvssBridgeResponseEvent;
        pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
        pvssBridgeResponseEvent.property = pvssBridgeEvent.property;
        pvssBridgeResponseEvent.value = string("");
        m_serverPort.send(pvssBridgeResponseEvent);
      }      
      break;
    }

    case THPVSSBRIDGE_EXT_SET_VALUE:
    {
      THPVSSBridgeExtSetValueEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s,%s)","THPVSSBRIDGE_EXT_SET_VALUE",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.property.c_str(),pvssBridgeEvent.value.c_str()));

      TGCFResult res;
      TExtPropertySetMap::iterator it = m_extPropertySets.find(pvssBridgeEvent.scope);
      if(it != m_extPropertySets.end())
      {
        res = it->second->setValue(pvssBridgeEvent.property,pvssBridgeEvent.value);
      }
      else
      {
        LOG_FATAL(formatString("PropertySet %s not found. Did you load it?",pvssBridgeEvent.scope.c_str()));
        res = GCF_EXTPS_LOAD_ERROR;
      }
      THPVSSBridgeExtSetValueResponseEvent pvssBridgeResponseEvent;
      pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
      pvssBridgeResponseEvent.property = pvssBridgeEvent.property;
      pvssBridgeResponseEvent.response = res;
      m_serverPort.send(pvssBridgeResponseEvent);
      break;
    }

    case THPVSSBRIDGE_SUBSCRIBE_PROPERTY:
    {
      THPVSSBridgeSubscribePropertyEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s)","THPVSSBRIDGE_SUBSCRIBE_PROPERTY_SET",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.property.c_str()));
      
      TGCFResult res = m_propertyProxy.subscribeProp(pvssBridgeEvent.scope+string(".")+pvssBridgeEvent.property);
      if(GCF_NO_ERROR != res)
      {
        THPVSSBridgeSubscribePropertyResponseEvent pvssBridgeResponseEvent;
        pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
        pvssBridgeResponseEvent.property = pvssBridgeEvent.property;
        pvssBridgeResponseEvent.response = res;
        m_serverPort.send(pvssBridgeResponseEvent);
      }
      break;
    }

    case THPVSSBRIDGE_UNSUBSCRIBE_PROPERTY:
    {
      THPVSSBridgeUnsubscribePropertyEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s)","THPVSSBRIDGE_UNSUBSCRIBE_PROPERTY_SET",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.property.c_str()));
      
      _removeProxySubscription(pvssBridgeEvent.scope+string(".")+pvssBridgeEvent.property);

      THPVSSBridgeUnsubscribePropertyResponseEvent pvssBridgeResponseEvent;
      pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
      pvssBridgeResponseEvent.property = pvssBridgeEvent.property;
      pvssBridgeResponseEvent.response = m_propertyProxy.unsubscribeProp(pvssBridgeEvent.scope+string(".")+pvssBridgeEvent.property);
      m_serverPort.send(pvssBridgeResponseEvent);
      break;
    }

    case THPVSSBRIDGE_GET_VALUE:
    {
      THPVSSBridgeGetValueEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s)","THPVSSBRIDGE_GET_VALUE",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.property.c_str()));

      TGCFResult res = m_propertyProxy.requestPropValue(pvssBridgeEvent.scope+string(".")+pvssBridgeEvent.property);
      if(GCF_NO_ERROR != res)
      {
        THPVSSBridgeGetValueResponseEvent pvssBridgeResponseEvent;
        pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
        pvssBridgeResponseEvent.property = pvssBridgeEvent.property;
        pvssBridgeResponseEvent.value = string("");
        m_serverPort.send(pvssBridgeResponseEvent);
      }
      break;
    }

    case THPVSSBRIDGE_SET_VALUE:
    {
      THPVSSBridgeSetValueEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s,%s)","THPVSSBRIDGE_SET_VALUE",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.property.c_str(),pvssBridgeEvent.value.c_str()));

      THPVSSBridgeSetValueResponseEvent pvssBridgeResponseEvent;
      pvssBridgeResponseEvent.scope = pvssBridgeEvent.scope;
      pvssBridgeResponseEvent.property = pvssBridgeEvent.property;
      pvssBridgeResponseEvent.response = m_propertyProxy.setPropValue(pvssBridgeEvent.scope+string(".")+pvssBridgeEvent.property,pvssBridgeEvent.value);
      m_serverPort.send(pvssBridgeResponseEvent);
      break;
    }

    case THPVSSBRIDGE_FLUSH_SUBSCRIPTIONS:
    {
      LOG_DEBUG(formatString("%s","THPVSSBRIDGE_FLUSH_SUBSCRIPTIONS"));
      _flushSubscriptions();
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

void THPVSSBridge::proxyPropSubscribed(const string& propName)
{
  LOG_DEBUG(formatString("%s(%s)","proxyPropSubscribed",propName.c_str()));
  
  m_proxySubscriptions.push_back(propName);
  
  THPVSSBridgeSubscribePropertyResponseEvent pvssBridgeEvent;
  pvssBridgeEvent.scope = stripProperty(propName);
  pvssBridgeEvent.property = getProperty(propName);
  pvssBridgeEvent.response = GCF_NO_ERROR;
  m_serverPort.send(pvssBridgeEvent);
}

void THPVSSBridge::proxyPropSubscriptionLost(const string& propName)
{
  LOG_DEBUG(formatString("%s(%s)","proxyPropSubscriptionLost",propName.c_str()));
  
  _removeProxySubscription(propName);
}

void THPVSSBridge::proxyPropUnsubscribed(const string& propName)
{
  LOG_DEBUG(formatString("%s(%s)","proxyPropUnsubscribed",propName.c_str()));
  
  _removeProxySubscription(propName);

  THPVSSBridgeUnsubscribePropertyResponseEvent pvssBridgeEvent;
  pvssBridgeEvent.scope = stripProperty(propName);
  pvssBridgeEvent.property = getProperty(propName);
  pvssBridgeEvent.response = GCF_NO_ERROR;
  m_serverPort.send(pvssBridgeEvent);
}

void THPVSSBridge::proxyPropValueGet(const string& propName,const string& value)
{
  LOG_DEBUG(formatString("%s(%s,%s)","proxyPropValueGet",propName.c_str(),value.c_str()));

  THPVSSBridgeGetValueResponseEvent pvssBridgeEvent;
  pvssBridgeEvent.scope = stripProperty(propName);
  pvssBridgeEvent.property = getProperty(propName);
  pvssBridgeEvent.value = value;
  m_serverPort.send(pvssBridgeEvent);
}

void THPVSSBridge::proxyPropValueChanged(const string& propName,const string& value)
{
  LOG_DEBUG(formatString("%s(%s,%s)","proxyPropValueChanged",propName.c_str(),value.c_str()));

  THPVSSBridgeValueChangeResponseEvent pvssBridgeEvent;
  pvssBridgeEvent.scope = stripProperty(propName);
  pvssBridgeEvent.property = getProperty(propName);
  pvssBridgeEvent.value = value;
  m_serverPort.send(pvssBridgeEvent);
}

void THPVSSBridge::proxyPropValueSet(const string& propName)
{
  LOG_DEBUG(formatString("%s(%s)","proxyPropValueSet",propName.c_str()));

  THPVSSBridgeSetValueResponseEvent pvssBridgeEvent;
  pvssBridgeEvent.scope = stripProperty(propName);
  pvssBridgeEvent.property = getProperty(propName);
  pvssBridgeEvent.response = GCF_NO_ERROR;
  m_serverPort.send(pvssBridgeEvent);
}

void THPVSSBridge::_flushSubscriptions()
{
  // unsubscribe from all properties subscribed through the proxy
  while(m_proxySubscriptions.size() > 0)
  {
    vector<string>::reference ref=m_proxySubscriptions.back();
    m_propertyProxy.unsubscribeProp(ref);
    m_proxySubscriptions.pop_back();
  }
  
  // unload all ext propsets
  TExtPropertySetMap::iterator extIt=m_extPropertySets.begin();
  while(extIt != m_extPropertySets.end())
  {
    extIt->second->unload();
    m_extPropertySets.erase(extIt);
    extIt=m_extPropertySets.begin();
  }
  
  // disable all my propsets
  TMyPropertySetMap::iterator myIt=m_myPropertySets.begin();
  while(myIt != m_myPropertySets.end())
  {
    myIt->second->disable();
    m_myPropertySets.erase(myIt);
    myIt=m_myPropertySets.begin();
  }
}

void THPVSSBridge::_removeProxySubscription(const string& propName)
{
  bool found(false);
  vector<string>::iterator it=m_proxySubscriptions.begin();
  while(!found && it!=m_proxySubscriptions.end())
  {
    found = ((*it) == propName);
    if(found)
    {
      m_proxySubscriptions.erase(it);
    }
    else
    {
      ++it;
    }
  }
}
