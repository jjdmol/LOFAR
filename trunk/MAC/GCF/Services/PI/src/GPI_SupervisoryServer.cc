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
#include <stdio.h>

#include "SS/FPBoolValue.h"
#include "SS/FPCharValue.h"
#include "SS/FPStringValue.h"
#include "SS/FPIntegerValue.h"
#include "SS/FPUnsignedValue.h"
#include "SS/FPDoubleValue.h"
#include "SS/FPDynArrValue.h"
#include "SS/FProperty.h"

#include <GCF/GCF_PVBool.h>
#include <GCF/GCF_PVChar.h>
#include <GCF/GCF_PVInteger.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDynArr.h>

#define DECLARE_SIGNAL_NAMES
#include "PI_Protocol.ph"
#include <PA_Protocol.ph>

static string sSSTaskName("SS");

GPISupervisoryServer::GPISupervisoryServer(GPIController& controller) : 
  GCFTask((State)&GPISupervisoryServer::initial, sSSTaskName),
  _controller(controller),
  _propProxy(*this),
  _isBusy(false)
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
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      _controller.getPortProvider().accept(_ssPort);
      // intential fall through
    case F_TIMER_SIG:
      _propertyAgent.open();
      break;

    case F_CONNECTED_SIG:
      if (_ssPort.isConnected())
        TRAN(GPISupervisoryServer::operational);
      break;

    case F_DISCONNECTED_SIG:
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
  static string scope = "";
  static char* pData = 0;

  switch (e.signal)
  {      
    case F_CONNECTED_SIG:
      if (&p == &_propertyAgent)
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

    case F_TIMER_SIG:
      if (&_propertyAgent == &p)
      {          
        p.open(); // try again
      }
      break;
     
    case PI_REGISTERSCOPE:
    {
      pData = ((char*)&e) + sizeof(GCFEvent);
      if (!findScope(pData, scope))
      {
        PARegisterscopeEvent pae(0);
        forwardMsgToPA(pae, e);
        // creates a new scope entry
        _scopeRegister[scope] = SST_REGISTERING;
      }
      else
      {
        PIScoperegisteredEvent response(PI_SCOPE_ALREADY_REGISTERED);
        replyMsgToSS(response, pData);
      }      

      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "SS-REQ: Register scope %s",
          scope.c_str()));
      break;
    }  
    case PA_SCOPEREGISTERED:
    {      
      PAScoperegisteredEvent* pResponse = static_cast<PAScoperegisteredEvent*>(&e);
      assert(pResponse);
      pData = ((char*)&e) + sizeof(PAScoperegisteredEvent);
      bool scopeFound(false);
      scopeFound = findScope(pData, scope);

      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "PA-RESP: Scope %s is registered", 
          scope.c_str()));

      unsigned int scopeDataLength = scope.length() + Utils::SLENGTH_FIELD_SIZE;

      if (scopeFound)
      {
        PIScoperegisteredEvent pirse(PI_NO_ERROR);
        assert(_ssPort.isConnected());
        pirse.result = 
        pirse.length += scopeDataLength;
        _ssPort.send(pirse, pData, scopeDataLength);
        if (pResponse->result == PA_NO_ERROR)
        {
          // creates a new scope entry
          _scopeRegister[scope] = SST_REGISTERED;
        }
      }
      else
      {
        if (pResponse->result == PA_NO_ERROR)
        {
          GCFEvent urse(PA_UNREGISTERSCOPE);
          urse.length += scopeDataLength;
          _propertyAgent.send(urse, pData, scopeDataLength);
        }        
        LOFAR_LOG_TRACE(PI_STDOUT_LOGGER, ( 
            "Property set with scope %d was deleted in the meanwhile", 
            scope.c_str()));
      }
      break;
    }

    case PI_UNREGISTERSCOPE:
    {
      pData = ((char*)&e) + sizeof(GCFEvent);
      if (findScope(pData, scope))
      {
        PAUnregisterscopeEvent pae(0);
        forwardMsgToPA(pae, e);
        _scopeRegister[scope] = SST_UNREGISTERING;
      }
      else
      {
        PIScoperunegisteredEvent response(PI_PROP_SET_GONE);
        replyMsgToSS(response, pData);
      }      

      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "SS-REQ: Unregister scope %s",
          scope.c_str()));
      break;
    }  

    case PA_SCOPEUNREGISTERED:
    {
      PAScopeunregisteredEvent* pResponse = static_cast<PAScopeunregisteredEvent*>(&e);
      assert(pResponse);
      pData = ((char*)&e) + sizeof(PAScopeunregisteredEvent);
      unsigned int scopeDataLength = Utils::unpackString(pData, scope);

      assert(_scopeRegister[scope] == SST_UNREGISTERING);

      LOFAR_LOG_INFO(PI_STDOUT_LOGGER, ( 
          "PA-RESP: Scope %s is unregistered", 
          scope.c_str()));

      PIScopeunregisteredEvent piurse(PI_NO_ERROR);
      assert(_ssPort.isConnected());
      piurse.result = 
      piurse.length += scopeDataLength;
      _ssPort.send(piurse, pData, scopeDataLength);
      // creates a new scope entry
      _scopeRegister.erase(scope);
      break;
    }

    case PA_LINKPROPERTIES:
    {
      PALinkpropertiesEvent* pRequest = static_cast<PALinkpropertiesEvent*>(&e);
      assert(pRequest);
      pData = ((char*)&e) + sizeof(PALinkpropertiesEvent);
      char logMsg[] = "PA-REQ: Link properties on scope %s";
      GCFEvent pilpe(PI_LINKPROPERTIES);
      pilpe.length += e.length - sizeof(PALinkpropertiesEvent);
      if (!forwardMsgToSS(pilpe, pData, scope, logMsg))
      {
        PAPropertieslinkedEvent response(0, PA_PROP_SET_GONE);
        replyMsgToPA(response, pData);
      }
      else
      {
        _scopeRegister[scope] = SST_LINKING;
      }
      break;
    }
    case PI_PROPERTIESLINKED:
      break;
    
    case PA_UNLINKPROPERTIES:
    {
      PAUnlinkpropertiesEvent* pRequest = static_cast<PAUnlinkpropertiesEvent*>(&e);
      assert(pRequest);
      pData = ((char*)&e) + sizeof(PAUnlinkpropertiesEvent);
      char logMsg[] = "PA-REQ: Unlink properties on scope %s";
      GCFEvent piulpe(PI_LINKPROPERTIES);
      piulpe.length += e.length - sizeof(PALinkpropertiesEvent);      
      if (!forwardMsgToPMLlite(piulpe, pData, scope, logMsg))
      {
        PIPropertieslinkedEvent response(0, PA_PROP_SET_GONE);
        replyMsgToPI(response, pData);
      }
      else
      {
        _scopeRegister[scope] = SST_UNLINKING;
      }
      break;
    }

    case PI_PROPERTIESUNLINKED:
      break;
      
    case F_SV_SUBSCRIBED:
    {
      F_SVSubscribedEvent *pResponse = static_cast<F_SVSubscribedEvent*>(&e);
      if (pResponse->result >= NO_ERROR)
      {
        list<string> propertyList;
        unpackPropertyList(
            ((char*)pResponse) + sizeof(F_SVSubscribedEvent), 
            e.length - sizeof(F_SVSubscribedEvent),
            propertyList);
        unLinkProperties(propertyList, (pResponse->result != UNSUBSCRIBED));
      }
      break;
    }
    case F_SV_VALUESET:
    {
      F_SVValuesetEvent *pEvent = static_cast<F_SVValuesetEvent*>(&e);
      switch (pEvent->result)
      {
        case NO_ERROR:
        {
          cerr << "Value was set succesfully" << endl;
        }
      }
      break;
    }
    case F_SV_VALUECHANGED:
    {
      if (&p == &_ssPort)
        localValueChanged(e);
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
    case F_ENTRY_SIG:
      _propertyAgent.close();
      break;
      
    case F_DISCONNECTED_SIG:      
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

bool GPISupervisoryServer::findScope(char* pScopeData, string& scope)
{
  if (pScopeData != 0)
  {
    Utils::unpackString(pScopeData, scope);
  }
  for (TScopeRegister::iterator iter = _scopeRegister.begin();
       iter != _scopeRegister.end(); ++iter)
  {
    if (iter->first == scope) return true;
  }       
  return false;
}

void GPISupervisoryServer::forwardMsgToPA(GCFEvent& pae, GCFEvent& pie)
{
  if (_propertyAgent.isConnected())
  {
    unsigned int pieDataLength = pie.length - sizeof(GCFEvent);
    pae.length += pieDataLength;
    _propertyAgent.send(pae, ((char*)pie) + sizeof(GCFEvent), pieDataLength);
  }
}

void GPISupervisoryServer::replyMsgToPA(GCFEvent& e, const string& scope)
{
  if (_propertyAgent.isConnected())
  {    
    unsigned int scopeDataLength = Utils::packString(scope, _buffer, MAX_BUF_SIZE);
    e.length += scopeDataLength;
    _propertyAgent.send(e, _buffer, scopeDataLength);
  }
}

bool GSSController::forwardMsgToSS(GCFEvent& e, char* pData, string& scope, const char* logMsg)
{
  bool result(false);
  Utils::unpackString(pData, scope);
  
  LOFAR_LOG_INFO(SS_STDOUT_LOGGER, ( 
      logMsg, 
      scope.c_str()));

  TScopeRegister::iterator iter = _scopeRegister.find(scope);
  if (iter != _scopeRegister.end() && iter != SST_UNREGISTERING)
  {
    assert(_ssPort.isConnected());
    _ssPort.send(e, pData, e.length - sizeof(GCFEvent));    
    result = true;
  }
  else
  {
    LOFAR_LOG_TRACE(PI_STDOUT_LOGGER, ( 
        "Property set with scope %d was deleted/is deleting in the meanwhile", 
        scope.c_str()));
  }
  return result;
}

void GPISupervisoryServer::replyMsgToSS(GCFEvent& e, char* pScopeData)
{
  assert(_ssPort.isConnected());
  unsigned short scopeDataLength = Utils::getStringDataLength(pScopeData);
  e.length += scopeDataLength;
  _ssPort.send(e, pScopeData, scopeDataLength);
}

TPIResult GPISupervisoryServer::unLinkProperties(list<string>& properties, bool onOff)
{
  
  TPIResult result(PI_NO_ERROR);
  if (_counter > 0)
  {
    result = PI_SS_BUSY;
  }
  else 
  {
    string propName;
    for (list<string>::iterator iter = properties.begin(); 
         iter != properties.end(); ++iter)
    {
      propName = _name + GCF_PROP_NAME_SEP + *iter;
      if (onOff)
      {
        result = (_propProxy.subscribeProp(propName) == GCF_NO_ERROR ?
                  PI_NO_ERROR :
                  PI_SCADA_ERROR);
      }
      else
      {
        result = (_propProxy.unsubscribeProp(propName) == GCF_NO_ERROR ?
                  PI_NO_ERROR :
                  PI_SCADA_ERROR);
      }
      if (result == PI_NO_ERROR)
        _counter++;        
    }
  }
    
  return result;
}

void GPISupervisoryServer::propSubscribed(const string& /*propName*/)
{
  _counter--;
  if (_counter == 0 && _propertyAgent.isConnected())
  {
    PAPropertieslinkedEvent e(0, PA_NO_ERROR);
    string allPropNames;
    // PA does expect but not use a property name list (for now)
    unsigned short bufLength(_name.size() + allPropNames.size() + 6);
    char* buffer = new char[bufLength + 1];
    sprintf(buffer, "%03x%s%03x%s", _name.size(), _name.c_str(), 
                                    allPropNames.size(), allPropNames.c_str());
    e.length += bufLength;
    _propertyAgent.send(e, buffer, bufLength);
    delete [] buffer;
  }
}

void GPISupervisoryServer::propUnsubscribed(const string& /*propName*/)
{
  _counter--;
  if (_counter == 0 && _propertyAgent.isConnected())
  {
    PAPropertiesunlinkedEvent e(0, PA_NO_ERROR);
    string allPropNames;
    // PA does expect but not use a property name list (for now)
    unsigned short bufLength(_name.size() + allPropNames.size() + 6);
    char* buffer = new char[bufLength + 1];
    sprintf(buffer, "%03x%s%03x%s", _name.size(), _name.c_str(), 
                                    allPropNames.size(), allPropNames.c_str());
    e.length += bufLength;
    _propertyAgent.send(e, buffer, bufLength);
    delete [] buffer;
  }
}

void GPISupervisoryServer::propValueChanged(const string& propName, const GCFPValue& value)
{
  unsigned short bufLength(0);
  FPValue* pVal(0);
  static char buffer[1024];
  
  switch (value.getType())
  {
    case GCFPValue::LPT_BOOL:
      pVal = new FPBoolValue(((GCFPVBool*)&value)->getValue());
      break;
    case GCFPValue::LPT_CHAR:
      pVal = new FPCharValue(((GCFPVChar*)&value)->getValue());
      break;
    case GCFPValue::LPT_INTEGER:
      pVal = new FPIntegerValue(((GCFPVInteger*)&value)->getValue());
      break;
    case GCFPValue::LPT_UNSIGNED:
      pVal = new FPUnsignedValue(((GCFPVUnsigned*)&value)->getValue());
      break;
    case GCFPValue::LPT_DOUBLE:
      pVal = new FPDoubleValue(((GCFPVDouble*)&value)->getValue());
      break;
    case GCFPValue::LPT_STRING:
      pVal = new FPStringValue(((GCFPVString*)&value)->getValue());
      break;

    default: 
      if (value.getType() > GCFPValue::LPT_DYNARR && 
          value.getType() <= (GCFPValue::LPT_DYNARR & GCFPValue::LPT_STRING))
      {
        FPValueArray arrayTo;
        FPValue* pItemValue;
        FPValue::ValueType type(FPValue::NO_LPT);
        // the type for the new FPValue must be determined 
        // separat, because the array could be empty
        switch (value.getType())
        {
          case GCFPValue::LPT_DYNBOOL:
            type = FPValue::LPT_DYNBOOL;
            break;
          case GCFPValue::LPT_DYNCHAR:
            type = FPValue::LPT_DYNCHAR;
            break;
          case GCFPValue::LPT_DYNINTEGER:
            type = FPValue::LPT_DYNINTEGER;
            break;
          case GCFPValue::LPT_DYNUNSIGNED:
            type = FPValue::LPT_DYNUNSIGNED;
            break;
          case GCFPValue::LPT_DYNDOUBLE:
            type = FPValue::LPT_DYNDOUBLE;
            break;
          case GCFPValue::LPT_DYNSTRING:
            type = FPValue::LPT_DYNSTRING;
            break;
        }
        GCFPValue* pValue;
        const GCFPValueArray& arrayFrom(((GCFPVDynArr*)&value)->getValue());
        for (GCFPValueArray::const_iterator iter = arrayFrom.begin();
             iter != arrayFrom.end(); ++iter)
        {
          pValue = (*iter);
          switch (pValue->getType())
          {
            case GCFPValue::LPT_BOOL:
              pItemValue  = new FPBoolValue(((GCFPVBool*)pValue)->getValue());
              break;
            case GCFPValue::LPT_CHAR:
              pItemValue  = new FPCharValue(((GCFPVChar*)pValue)->getValue());
              break;
            case GCFPValue::LPT_INTEGER:
              pItemValue  = new FPIntegerValue(((GCFPVInteger*)pValue)->getValue());
              break;
            case GCFPValue::LPT_UNSIGNED:
              pItemValue  = new FPUnsignedValue(((GCFPVUnsigned*)pValue)->getValue());
              break;
            case GCFPValue::LPT_DOUBLE:
              pItemValue  = new FPDoubleValue(((GCFPVDouble*)pValue)->getValue());
              break;
            case GCFPValue::LPT_STRING:
              pItemValue  = new FPStringValue(((GCFPVString*)pValue)->getValue());
              break;
          }
          arrayTo.push_back(pItemValue);
        }
        pVal = new FPDynArrValue(type, arrayTo);
      }
      break;
  }
  if (pVal)
  {
    FProperty property(propName.c_str(), *pVal);
    bufLength = property.pack(buffer);
    delete pVal;
  }
  F_SVSetvalueEvent e;
  e.seqNr = 0;
  e.length += bufLength;
  _ssPort.send(e, buffer, bufLength);
}

void GPISupervisoryServer::localValueChanged(GCFEvent& e)
{
  char* buffer = ((char*)&e) + sizeof(GCFEvent);
  unsigned char propNameLength(0);
  unsigned char taskNameLength(0);
  unsigned char bytesRead(0);
  static char totalPropName[512];
  static char locPropName[256];
  static char taskName[256];
  string propName;

  memcpy((void*) &propNameLength, buffer, sizeof(unsigned char));
  buffer += sizeof(unsigned char);
  memcpy(locPropName, buffer, propNameLength);
  locPropName[propNameLength] = 0;
  buffer += propNameLength;
  FPValue* pVal = FPValue::createValueObject((FPValue::ValueType) buffer[0]);
  if (pVal)
    bytesRead = pVal->unpack(buffer);
  buffer += bytesRead;

  memcpy((void *) &taskNameLength, buffer, sizeof(unsigned char));
  buffer += sizeof(unsigned char);
  memcpy(taskName, buffer, taskNameLength);
  taskName[taskNameLength] = 0;

  sprintf(totalPropName, "%s_%s_%s", _name.c_str(), taskName, locPropName);
  propName = locPropName;

  GCFPValue *pGCFVal(0);
   
  switch (pVal->getType())
  {
    case FPValue::LPT_BOOL:
      pGCFVal = new GCFPVBool(((FPBoolValue*)pVal)->getValue());
      break;
    case FPValue::LPT_INTEGER:
      pGCFVal = new GCFPVInteger(((FPIntegerValue*)pVal)->getValue());
      break;
    case FPValue::LPT_UNSIGNED:
      pGCFVal = new GCFPVUnsigned(((FPUnsignedValue*)pVal)->getValue());
      break;
    case FPValue::LPT_DOUBLE:
      pGCFVal = new GCFPVDouble(((FPDoubleValue*)pVal)->getValue());
      break;
    case FPValue::LPT_STRING:
      pGCFVal = new GCFPVString(((FPStringValue*)pVal)->getValue());
      break;
    case FPValue::LPT_CHAR:
      pGCFVal = new GCFPVChar(((FPCharValue*)pVal)->getValue());
      break;
    default:
      if (pVal->getType() > FPValue::LPT_DYNARR && 
          pVal->getType() <= FPValue::LPT_DYNDATETIME)
      {
        GCFPValueArray arrayTo;
        GCFPValue* pItemValue(0);
        GCFPValue::TMACValueType type(GCFPValue::LPT_DYNARR);
        // the type for the new FPValue must be determined 
        // separat, because the array could be empty
        switch (pVal->getType())
        {
          case FPValue::LPT_DYNBOOL:
            type = GCFPValue::LPT_DYNBOOL;
            break;
          case FPValue::LPT_DYNCHAR:
            type = GCFPValue::LPT_DYNCHAR;
            break;
          case FPValue::LPT_DYNINTEGER:
            type = GCFPValue::LPT_DYNINTEGER;
            break;
          case FPValue::LPT_DYNUNSIGNED:
            type = GCFPValue::LPT_DYNUNSIGNED;
            break;
          case FPValue::LPT_DYNDOUBLE:
            type = GCFPValue::LPT_DYNDOUBLE;
            break;
          case FPValue::LPT_DYNSTRING:
            type = GCFPValue::LPT_DYNSTRING;
            break;
        }
        FPValue* pValue(0);
        const FPValueArray& arrayFrom(((FPDynArrValue*)pVal)->getValue());
        for (FPValueArray::const_iterator iter = arrayFrom.begin();
             iter != arrayFrom.end(); ++iter)
        {
          pValue = (*iter);
          switch (pValue->getType())
          {
            case FPValue::LPT_BOOL:
              pItemValue  = new GCFPVBool(((FPBoolValue*)pValue)->getValue());
              break;
            case FPValue::LPT_CHAR:
              pItemValue  = new GCFPVChar(((FPCharValue*)pValue)->getValue());
              break;
            case FPValue::LPT_INTEGER:
              pItemValue  = new GCFPVInteger(((FPIntegerValue*)pValue)->getValue());
              break;
            case FPValue::LPT_UNSIGNED:
              pItemValue  = new GCFPVUnsigned(((FPUnsignedValue*)pValue)->getValue());
              break;
            case FPValue::LPT_DOUBLE:
              pItemValue  = new GCFPVDouble(((FPDoubleValue*)pValue)->getValue());
              break;
            case FPValue::LPT_STRING:
              pItemValue  = new GCFPVString(((FPStringValue*)pValue)->getValue());
              break;
          }
          arrayTo.push_back(pItemValue);
        }
        pGCFVal = new GCFPVDynArr(type, arrayTo);
      }
      break;
  }
  
  if (pGCFVal)
  {
    if (_propProxy.setPropValue(propName, *pGCFVal) != GCF_NO_ERROR)
    {
      LOFAR_LOG_ERROR(PI_STDOUT_LOGGER, (
        "Value of property '%s' could not be set in the SCADA DB", 
        propName.c_str()));
    }   
    delete pGCFVal;
  }
  if (pVal)
    delete pVal;
}
