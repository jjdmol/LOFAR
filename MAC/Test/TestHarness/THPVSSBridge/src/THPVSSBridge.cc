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

void string2array(char out[PVSSBRIDGE_STRINGLEN], string in)
{
  memset(out,0,PVSSBRIDGE_STRINGLEN);
  strncpy(out,in.c_str(),PVSSBRIDGE_STRINGLEN);
}

string stripProperty(string in)
{
  return in.substr(0,in.rfind('.'));
}

string getProperty(string in)
{
  string property("");
  unsigned int dotPos = in.rfind('.');
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
  m_serverPort(*this, m_serverPortName, GCFPortInterface::SPP, THPVSSBRIDGE_PROTOCOL)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

  // register the protocol for debugging purposes
  registerProtocol(THPVSSBRIDGE_PROTOCOL, THPVSSBRIDGE_PROTOCOL_signalnames);
}

THPVSSBridge::~THPVSSBridge()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
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
      string2array(pvssBridgeEvent.scope,stripProperty(string(pPropAnswer->pPropName)));
      pvssBridgeEvent.scopelen = strlen(pvssBridgeEvent.scope);
      string2array(pvssBridgeEvent.property,getProperty(string(pPropAnswer->pPropName)));
      pvssBridgeEvent.propertylen = strlen(pvssBridgeEvent.property);
      pvssBridgeEvent.response = GCF_NO_ERROR;
      m_serverPort.send(pvssBridgeEvent);
      break;
    }
    
    case F_UNSUBSCRIBED:
    {
      GCFPropAnswerEvent* pPropAnswer=static_cast<GCFPropAnswerEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s)","F_UNSUBSCRIBED",pPropAnswer->pPropName));

      THPVSSBridgeUnsubscribeExtPropertyResponseEvent pvssBridgeEvent;
      string2array(pvssBridgeEvent.scope,stripProperty(string(pPropAnswer->pPropName)));
      pvssBridgeEvent.scopelen = strlen(pvssBridgeEvent.scope);
      string2array(pvssBridgeEvent.property,getProperty(string(pPropAnswer->pPropName)));
      pvssBridgeEvent.propertylen = strlen(pvssBridgeEvent.property);
      pvssBridgeEvent.response = GCF_NO_ERROR;
      m_serverPort.send(pvssBridgeEvent);
      break;
    }
    
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s=%s)","F_VCHANGEMSG",pPropAnswer->pPropName,pPropAnswer->pValue->getValueAsString().c_str()));
      
      THPVSSBridgeValueChangeResponseEvent pvssBridgeEvent;
      string2array(pvssBridgeEvent.scope,stripProperty(string(pPropAnswer->pPropName)));
      pvssBridgeEvent.scopelen = strlen(pvssBridgeEvent.scope);
      string2array(pvssBridgeEvent.property,getProperty(string(pPropAnswer->pPropName)));
      pvssBridgeEvent.propertylen = strlen(pvssBridgeEvent.property);
      string2array(pvssBridgeEvent.value,pPropAnswer->pValue->getValueAsString());
      pvssBridgeEvent.valuelen = strlen(pvssBridgeEvent.value);
      m_serverPort.send(pvssBridgeEvent);
      break;
    }  
    
    case F_VGETRESP:
    {
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s=%s)","F_VGETRESP",pPropAnswer->pPropName,pPropAnswer->pValue->getValueAsString().c_str()));
      
      THPVSSBridgeValueChangeResponseEvent pvssBridgeEvent;
      string2array(pvssBridgeEvent.scope,stripProperty(string(pPropAnswer->pPropName)));
      pvssBridgeEvent.scopelen = strlen(pvssBridgeEvent.scope);
      string2array(pvssBridgeEvent.property,getProperty(string(pPropAnswer->pPropName)));
      pvssBridgeEvent.propertylen = strlen(pvssBridgeEvent.property);
      string2array(pvssBridgeEvent.value,pPropAnswer->pValue->getValueAsString());
      pvssBridgeEvent.valuelen = strlen(pvssBridgeEvent.value);
      m_serverPort.send(pvssBridgeEvent);
      break;
    }  
    
    case F_VSETRESP:
    {
      GCFPropAnswerEvent* pPropAnswer=static_cast<GCFPropAnswerEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s)","F_VSETRESP",pPropAnswer->pPropName));

      THPVSSBridgeExtSetValueResponseEvent pvssBridgeEvent;
      string2array(pvssBridgeEvent.scope,stripProperty(string(pPropAnswer->pPropName)));
      pvssBridgeEvent.scopelen = strlen(pvssBridgeEvent.scope);
      string2array(pvssBridgeEvent.property,getProperty(string(pPropAnswer->pPropName)));
      pvssBridgeEvent.propertylen = strlen(pvssBridgeEvent.property);
      pvssBridgeEvent.response = GCF_NO_ERROR;
      m_serverPort.send(pvssBridgeEvent);
      break;
    }
    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s,%d)","F_EXTPS_LOADED",pPropAnswer->pScope,pPropAnswer->result));

      THPVSSBridgeLoadExtPropertySetResponseEvent pvssBridgeEvent;
      string2array(pvssBridgeEvent.scope,string(pPropAnswer->pScope));
      pvssBridgeEvent.scopelen = strlen(pvssBridgeEvent.scope);
      pvssBridgeEvent.response = pPropAnswer->result;
      m_serverPort.send(pvssBridgeEvent);

      break;
    }
    
    case F_EXTPS_UNLOADED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s,%d)","F_EXTPS_UNLOADED",pPropAnswer->pScope,pPropAnswer->result));

      THPVSSBridgeUnloadExtPropertySetResponseEvent pvssBridgeEvent;
      string2array(pvssBridgeEvent.scope,string(pPropAnswer->pScope));
      pvssBridgeEvent.scopelen = strlen(pvssBridgeEvent.scope);
      pvssBridgeEvent.response = pPropAnswer->result;
      m_serverPort.send(pvssBridgeEvent);
      break;
    }
    
    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s,%d)","F_MYPS_ENABLED",pPropAnswer->pScope,pPropAnswer->result));

      THPVSSBridgeEnableMyPropertySetResponseEvent pvssBridgeEvent;
      string2array(pvssBridgeEvent.scope,string(pPropAnswer->pScope));
      pvssBridgeEvent.scopelen = strlen(pvssBridgeEvent.scope);
      pvssBridgeEvent.response = pPropAnswer->result;
      m_serverPort.send(pvssBridgeEvent);
      break;
    }
    
    case F_MYPS_DISABLED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      LOG_DEBUG(formatString("%s(%s,%d)","F_MYPS_DISABLED",pPropAnswer->pScope,pPropAnswer->result));

      THPVSSBridgeDisableMyPropertySetResponseEvent pvssBridgeEvent;
      string2array(pvssBridgeEvent.scope,string(pPropAnswer->pScope));
      pvssBridgeEvent.scopelen = strlen(pvssBridgeEvent.scope);
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
      p.setTimer(5.0); // try again after 5 seconds
      break;

    case F_TIMER:
      p.open(); // try again
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
        string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
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
          string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
          pvssBridgeResponseEvent.response = res;
          m_serverPort.send(pvssBridgeResponseEvent);
        }
      }
      else
      {
        THPVSSBridgeDisableMyPropertySetResponseEvent pvssBridgeResponseEvent;
        string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
        pvssBridgeResponseEvent.response = GCF_UNKNOWN_ERROR;
        m_serverPort.send(pvssBridgeResponseEvent);
      }
      break;
    }

    case THPVSSBRIDGE_MY_GET_VALUE:
    {
      THPVSSBridgeMyGetValueEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s)","THPVSSBRIDGE_MY_GET_VALUE",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.property.c_str()));

      THPVSSBridgeMyGetValueResponseEvent pvssBridgeResponseEvent;
      string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
      string2array(pvssBridgeResponseEvent.property,pvssBridgeEvent.property);
      string2array(pvssBridgeResponseEvent.value,string(""));
      
      TMyPropertySetMap::iterator it = m_myPropertySets.find(pvssBridgeEvent.scope);
      if(it != m_myPropertySets.end())
      {
        string fixTerminatingZerosString(pvssBridgeEvent.property.c_str());
        GCFPValue* pValue = it->second->getValue(fixTerminatingZerosString);
        string2array(pvssBridgeResponseEvent.value,pValue->getValueAsString());
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
        string fixTerminatingZerosString(pvssBridgeEvent.property.c_str());
        TGCFResult res = it->second->setValue(fixTerminatingZerosString,pvssBridgeEvent.value);
        if(GCF_NO_ERROR != res)
        {
          THPVSSBridgeMySetValueResponseEvent pvssBridgeResponseEvent;
          string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
          string2array(pvssBridgeResponseEvent.property,pvssBridgeEvent.property);
          pvssBridgeResponseEvent.response = res;
          m_serverPort.send(pvssBridgeResponseEvent);
        }
      }
      else
      {
        THPVSSBridgeMySetValueResponseEvent pvssBridgeResponseEvent;
        string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
        string2array(pvssBridgeResponseEvent.property,pvssBridgeEvent.property);
        pvssBridgeResponseEvent.response = GCF_UNKNOWN_ERROR;
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
        string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
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
          string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
          pvssBridgeResponseEvent.response = res;
          m_serverPort.send(pvssBridgeResponseEvent);
        }
      }
      else
      {
        THPVSSBridgeUnloadExtPropertySetResponseEvent pvssBridgeResponseEvent;
        string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
        pvssBridgeResponseEvent.response = GCF_UNKNOWN_ERROR;
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
        string fixTerminatingZerosString(pvssBridgeEvent.property.c_str());
        TGCFResult res = it->second->subscribeProp(fixTerminatingZerosString);
        if(GCF_NO_ERROR != res)
        {
          THPVSSBridgeSubscribeExtPropertyResponseEvent pvssBridgeResponseEvent;
          string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
          string2array(pvssBridgeResponseEvent.property,pvssBridgeEvent.property);
          pvssBridgeResponseEvent.response = res;
          m_serverPort.send(pvssBridgeResponseEvent);
        }
      }
      else
      {
        THPVSSBridgeSubscribeExtPropertyResponseEvent pvssBridgeResponseEvent;
        string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
        string2array(pvssBridgeResponseEvent.property,pvssBridgeEvent.property);
        pvssBridgeResponseEvent.response = GCF_UNKNOWN_ERROR;
        m_serverPort.send(pvssBridgeResponseEvent);
      }
      break;
    }

    case THPVSSBRIDGE_UNSUBSCRIBE_EXT_PROPERTY:
    {
      THPVSSBridgeUnsubscribeExtPropertyEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s)","THPVSSBRIDGE_UNSUBSCRIBE_EXT_PROPERTY_SET",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.property.c_str()));

      THPVSSBridgeUnsubscribeExtPropertyResponseEvent pvssBridgeResponseEvent;
      string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
      string2array(pvssBridgeResponseEvent.property,pvssBridgeEvent.property);
      pvssBridgeResponseEvent.response = GCF_UNKNOWN_ERROR;

      TExtPropertySetMap::iterator it = m_extPropertySets.find(pvssBridgeEvent.scope);
      if(it != m_extPropertySets.end())
      {
        string fixTerminatingZerosString(pvssBridgeEvent.property.c_str());
        pvssBridgeResponseEvent.response = it->second->unsubscribeProp(fixTerminatingZerosString);
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
        string fixTerminatingZerosString(pvssBridgeEvent.property.c_str());
        TGCFResult res = it->second->requestValue(fixTerminatingZerosString);
        if(GCF_NO_ERROR != res)
        {
          THPVSSBridgeExtGetValueResponseEvent pvssBridgeResponseEvent;
          string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
          string2array(pvssBridgeResponseEvent.property,pvssBridgeEvent.property);
          string2array(pvssBridgeResponseEvent.value,string(""));
          m_serverPort.send(pvssBridgeResponseEvent);
        }
      }
      else
      {
        THPVSSBridgeExtGetValueResponseEvent pvssBridgeResponseEvent;
        string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
        string2array(pvssBridgeResponseEvent.property,pvssBridgeEvent.property);
        string2array(pvssBridgeResponseEvent.value,string(""));
        m_serverPort.send(pvssBridgeResponseEvent);
      }      
      break;
    }

    case THPVSSBRIDGE_EXT_SET_VALUE:
    {
      THPVSSBridgeExtSetValueEvent pvssBridgeEvent(e);
      LOG_DEBUG(formatString("%s(%s,%s,%s)","THPVSSBRIDGE_EXT_SET_VALUE",pvssBridgeEvent.scope.c_str(),pvssBridgeEvent.property.c_str(),pvssBridgeEvent.value.c_str()));

      TGCFResult res = GCF_UNKNOWN_ERROR;
      TExtPropertySetMap::iterator it = m_extPropertySets.find(pvssBridgeEvent.scope);
      if(it != m_extPropertySets.end())
      {
        string fixTerminatingZerosString(pvssBridgeEvent.property.c_str());
        res = it->second->setValue(fixTerminatingZerosString,pvssBridgeEvent.value);
      }
      THPVSSBridgeExtSetValueResponseEvent pvssBridgeResponseEvent;
      string2array(pvssBridgeResponseEvent.scope,pvssBridgeEvent.scope);
      string2array(pvssBridgeResponseEvent.property,pvssBridgeEvent.property);
      pvssBridgeResponseEvent.response = res;
      m_serverPort.send(pvssBridgeResponseEvent);
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

