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

#include <SAL/GCF_PVBool.h>
#include <SAL/GCF_PVChar.h>
#include <SAL/GCF_PVInteger.h>
#include <SAL/GCF_PVUnsigned.h>
#include <SAL/GCF_PVDouble.h>
#include <SAL/GCF_PVString.h>
#include <SAL/GCF_PVDynArr.h>

#define DECLARE_SIGNAL_NAMES
#include "F_Supervisory_Protocol.ph"
#include <PA/PA_Protocol.ph>

static string sSSTaskName("SS");

GPISupervisoryServer::GPISupervisoryServer(GPIController& controller) : 
  GCFTask((State)&GPISupervisoryServer::initial, sSSTaskName),
  _controller(controller),
  _propProxy(*this),
  _isBusy(false)
{
  // register the protocols for debugging purposes only
  registerProtocol(F_SUPERVISORY_PROTOCOL, F_SUPERVISORY_PROTOCOL_signalnames);
  registerProtocol(PA_PROTOCOL, PA_PROTOCOL_signalnames);

  // initialize the port to the real supervisory server 
  _ssPort.init(*this, "server", GCFPortInterface::SPP, F_SUPERVISORY_PROTOCOL);
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
      if (_ssPort.isConnected() && _propertyAgent.isConnected())
        TRAN(GPISupervisoryServer::connected);
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

GCFEvent::TResult GPISupervisoryServer::connected(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      break;
  
    case F_DISCONNECTED_SIG:      
      if (&p == &_ssPort)
      {
        TRAN(GPISupervisoryServer::closing);
      }
      break;
     
    case F_SV_NAME:
    {
      char* pResponseData = ((char*)(&e)) + sizeof(GCFEvent);
      unsigned int ssNameLength(e.length - sizeof(GCFEvent));
      _name.assign(pResponseData, ssNameLength);
      registerScope(_name);
      break;
    }
    
    case PA_SCOPEREGISTERED:
    {
      PAScoperegisteredEvent* pResponse = static_cast<PAScoperegisteredEvent*>(&e);
      if (pResponse)
      {
        if (pResponse->result == 1) // no error
        {
          TRAN(GPISupervisoryServer::operational);
          break;
        }
      }
      _ssPort.close();
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GPISupervisoryServer::operational(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {      
    case F_DISCONNECTED_SIG:      
      if (&p == &_ssPort)
      {
        TRAN(GPISupervisoryServer::closing);
      }
      break;

    case PA_LINKPROPERTIES:
    {
      PALinkpropertiesEvent* pRequest = static_cast<PALinkpropertiesEvent*>(&e);
      char* pScopeData = ((char*)pRequest) + sizeof(PALinkpropertiesEvent);
      unsigned int scopeNameLength(0);
      sscanf(pScopeData, "%03x", &scopeNameLength);
      string scope(pScopeData + 3, scopeNameLength);
      if (scope == _name)
      {
        subscribe(pScopeData + 3 + scopeNameLength, true);
      }
      break;
    }
    case PA_UNLINKPROPERTIES:
    {
      PAUnlinkpropertiesEvent* response = static_cast<PAUnlinkpropertiesEvent*>(&e);
      char* pScopeData = ((char*)response) + sizeof(PAUnlinkpropertiesEvent);
      unsigned int scopeNameLength(0);
      sscanf(pScopeData, "%03x", &scopeNameLength);
      string scope(pScopeData + 3, scopeNameLength);
      if (scope == _name)
      {
        subscribe(pScopeData + 3 + scopeNameLength, false);
      }
      break;
    }
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

void GPISupervisoryServer::registerScope(const string& scope)
{
  PARegisterscopeEvent e(0);
  unsigned short bufLength(scope.size() + 3);
  char* buffer = new char[bufLength + 1];
  sprintf(buffer, "%03x%s", scope.size(), scope.c_str());
  e.length += bufLength;
  _propertyAgent.send(e, buffer, bufLength);
  delete [] buffer;
}

void GPISupervisoryServer::subscribe(char* data, bool onOff)
{
  F_SVSubscribeEvent e;
  unsigned int datalength(0);
  sscanf(data, "%03x", &datalength);
  e.onOff = onOff;
  e.seqNr = 0;
  e.length += datalength;
  _ssPort.send(e, data + 3, datalength);
}

void GPISupervisoryServer::unpackPropertyList(
    char* pListData,
    unsigned int listDataLength,
    list<string>& propertyList)
{
  propertyList.clear();
  if (listDataLength > 0)
  {
    string propName;
    char* pPropName = strtok(pListData, "|");
    while (pPropName && listDataLength > 0)
    {
      propName = pPropName;
      listDataLength -= (propName.size() + 1);
      propertyList.push_front(propName);
      pPropName = strtok(NULL, "|");
    }
  }
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
      propName = _name + "_" + *iter;
      if (onOff)
      {
        result = (_propProxy.subscribe(propName) == GCF_NO_ERROR ?
                  PI_NO_ERROR :
                  PI_SCADA_ERROR);
      }
      else
      {
        result = (_propProxy.unsubscribe(propName) == GCF_NO_ERROR ?
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
    PAPropertieslinkedEvent e(0, 1);
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
    PAPropertiesunlinkedEvent e(0, 1);
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
    case GCFPValue::BOOL_VAL:
      pVal = new FPBoolValue(((GCFPVBool*)&value)->getValue());
      break;
    case GCFPValue::CHAR_VAL:
      pVal = new FPCharValue(((GCFPVChar*)&value)->getValue());
      break;
    case GCFPValue::INTEGER_VAL:
      pVal = new FPIntegerValue(((GCFPVInteger*)&value)->getValue());
      break;
    case GCFPValue::UNSIGNED_VAL:
      pVal = new FPUnsignedValue(((GCFPVUnsigned*)&value)->getValue());
      break;
    case GCFPValue::DOUBLE_VAL:
      pVal = new FPDoubleValue(((GCFPVDouble*)&value)->getValue());
      break;
    case GCFPValue::STRING_VAL:
      pVal = new FPStringValue(((GCFPVString*)&value)->getValue());
      break;

    default: 
      if (value.getType() > GCFPValue::DYNARR_VAL && 
          value.getType() <= (GCFPValue::DYNARR_VAL & GCFPValue::STRING_VAL))
      {
        FPValueArray arrayTo;
        FPValue* pItemValue;
        FPValue::ValueType type(FPValue::NO_VAL);
        // the type for the new FPValue must be determined 
        // separat, because the array could be empty
        switch (value.getType())
        {
          case GCFPValue::DYNBOOL_VAL:
            type = FPValue::DYNBOOL_VAL;
            break;
          case GCFPValue::DYNCHAR_VAL:
            type = FPValue::DYNCHAR_VAL;
            break;
          case GCFPValue::DYNINTEGER_VAL:
            type = FPValue::DYNINTEGER_VAL;
            break;
          case GCFPValue::DYNUNSIGNED_VAL:
            type = FPValue::DYNUNSIGNED_VAL;
            break;
          case GCFPValue::DYNDOUBLE_VAL:
            type = FPValue::DYNDOUBLE_VAL;
            break;
          case GCFPValue::DYNSTRING_VAL:
            type = FPValue::DYNSTRING_VAL;
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
            case GCFPValue::BOOL_VAL:
              pItemValue  = new FPBoolValue(((GCFPVBool*)pValue)->getValue());
              break;
            case GCFPValue::CHAR_VAL:
              pItemValue  = new FPCharValue(((GCFPVChar*)pValue)->getValue());
              break;
            case GCFPValue::INTEGER_VAL:
              pItemValue  = new FPIntegerValue(((GCFPVInteger*)pValue)->getValue());
              break;
            case GCFPValue::UNSIGNED_VAL:
              pItemValue  = new FPUnsignedValue(((GCFPVUnsigned*)pValue)->getValue());
              break;
            case GCFPValue::DOUBLE_VAL:
              pItemValue  = new FPDoubleValue(((GCFPVDouble*)pValue)->getValue());
              break;
            case GCFPValue::STRING_VAL:
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
    case FPValue::BOOL_VAL:
      pGCFVal = new GCFPVBool(((FPBoolValue*)pVal)->getValue());
      break;
    case FPValue::INTEGER_VAL:
      pGCFVal = new GCFPVInteger(((FPIntegerValue*)pVal)->getValue());
      break;
    case FPValue::UNSIGNED_VAL:
      pGCFVal = new GCFPVUnsigned(((FPUnsignedValue*)pVal)->getValue());
      break;
    case FPValue::DOUBLE_VAL:
      pGCFVal = new GCFPVDouble(((FPDoubleValue*)pVal)->getValue());
      break;
    case FPValue::STRING_VAL:
      pGCFVal = new GCFPVString(((FPStringValue*)pVal)->getValue());
      break;
    case FPValue::CHAR_VAL:
      pGCFVal = new GCFPVChar(((FPCharValue*)pVal)->getValue());
      break;
    default:
      if (pVal->getType() > FPValue::DYNARR_VAL && 
          pVal->getType() <= FPValue::DYNDATETIME_VAL)
      {
        GCFPValueArray arrayTo;
        GCFPValue* pItemValue(0);
        GCFPValue::TMACValueType type(GCFPValue::DYNARR_VAL);
        // the type for the new FPValue must be determined 
        // separat, because the array could be empty
        switch (pVal->getType())
        {
          case FPValue::DYNBOOL_VAL:
            type = GCFPValue::DYNBOOL_VAL;
            break;
          case FPValue::DYNCHAR_VAL:
            type = GCFPValue::DYNCHAR_VAL;
            break;
          case FPValue::DYNINTEGER_VAL:
            type = GCFPValue::DYNINTEGER_VAL;
            break;
          case FPValue::DYNUNSIGNED_VAL:
            type = GCFPValue::DYNUNSIGNED_VAL;
            break;
          case FPValue::DYNDOUBLE_VAL:
            type = GCFPValue::DYNDOUBLE_VAL;
            break;
          case FPValue::DYNSTRING_VAL:
            type = GCFPValue::DYNSTRING_VAL;
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
            case FPValue::BOOL_VAL:
              pItemValue  = new GCFPVBool(((FPBoolValue*)pValue)->getValue());
              break;
            case FPValue::CHAR_VAL:
              pItemValue  = new GCFPVChar(((FPCharValue*)pValue)->getValue());
              break;
            case FPValue::INTEGER_VAL:
              pItemValue  = new GCFPVInteger(((FPIntegerValue*)pValue)->getValue());
              break;
            case FPValue::UNSIGNED_VAL:
              pItemValue  = new GCFPVUnsigned(((FPUnsignedValue*)pValue)->getValue());
              break;
            case FPValue::DOUBLE_VAL:
              pItemValue  = new GCFPVDouble(((FPDoubleValue*)pValue)->getValue());
              break;
            case FPValue::STRING_VAL:
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
    if (_propProxy.set(propName, *pGCFVal) != GCF_NO_ERROR)
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
