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
#define DECLARE_SIGNAL_NAMES
#include "F_Supervisory_Protocol.ph"
#include "PA_Protocol.ph"

static string sSSTaskName("SS");

GPISupervisoryServer::GPISupervisoryServer(GPIController& controller) : 
  GCFTask((State)&GPISupervisoryServer::initial, sSSTaskName),
  _controller(controller),
  _isBusy(false)
{
  // register the protocols for debugging purposes only
  registerProtocol(F_SUPERVISORY_PROTOCOL, F_SUPERVISORY_PROTOCOL_signalnames);
  registerProtocol(PA_PROTOCOL, PA_PROTOCOL_signalnames);

  // initialize the port to the real supervisory server 
  _ssPort.init(*this, "ss", GCFPortInterface::SPP, PI_PROTOCOL);
  // initialize the port to the the Property Agent (acts as a PML)
  _propertyAgent.init(*this, "pml", GCFPortInterface::SAP, PA_PROTOCOL);
}

GPISupervisoryServer::~GPISupervisoryServer()
{
}

int GPISupervisoryServer::initial(GCFEvent& e, GCFPortInterface& p)
{
  int status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      _controller.getPortProvider()->accept(_ssPort);
      // intential fall through
    case F_TIMER_SIG:
      _propertAgent.open();
      break;

    case F_CONNECTED_SIG:
      if (_ssPort.isConnected() && _propertAgent.isConnected())
        TRAN(&GPISupervisoryServer::connected);
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

int GPISupervisoryServer::connected(GCFEvent& e, GCFPortInterface& p)
{
  int status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
    {
      SSGetnameEvent e;
      _ssPort.send(e);
      break;
    }  
    case F_DISCONNECTED_SIG:      
      if (&p == &_ssPort)
      {
        TRAN(&GPISupervisoryServer::closing);
      }
      break;
     
    case SS_NAMEGET:
    {
      SSNamegetEvent* pResponse = static_cast<SSNamegetEvent*>(&e);
      if (pResponse)
      {
        if (pResponse->result == 0) // no error
        {
          char* pResponseData = ((char*)pResponse) + sizeof(SSNamegetEvent);
          unsigned int ssNameLength(0);
          sscanf(pResponseData, "%03x", &ssNameLength);
          _name.copy(pResponseData + 3, ssNameLength);
          registerScope(_name);
          break;
        }
      }
      _ssPort.close();
      break;
    }
    
    case PA_SCOPEREGISTERED:
    {
      PAScoperegisteredEvent* pResponse = static_cast<PAScoperegisteredEvent*>(&e);
      if (pResponse)
      {
        if (pResponse->result == 0) // no error
        {
          TRAN(&GPISupervisoryServer::operational);
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

int GPISupervisoryServer::operational(GCFEvent& e, GCFPortInterface& p)
{
  int status = GCFEvent::HANDLED;

  switch (e.signal)
  {      
    case F_DISCONNECTED_SIG:      
      if (&p == &_ssPort)
      {
        TRAN(&GPISupervisoryServer::closing);
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
      SubscribedEvent *pResponse = static_cast<SubscribedEvent*>(&e);
      if (pResponse->result >= NO_ERROR)
      {
        char* pListData = ((char*)pResponse) + sizeof(SubscribedEvent);
        list<string> propertyList;
        unpackPropertyList(pListData, propertyList);
        unSubscribeProperties(propertyList, (pResponse->result != UNSUBSCRIBED));
      }
      break;
    }
    case F_SV_VALUESET:
    {
      ValuesetEvent *pEvent = static_cast<ValuesetEvent*>(&e);
      switch (pEvent->result)
      {
        case NO_ERROR:
        {
         deregisterSequence(pEvent->seqNr);
         cerr << "Value was set succesfully" << endl;
        }
      }
      break;
    }
    case F_SV_VALUECHANGED:
    {
      localValueChanged(e, p);
      break;                                          
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

int GPISupervisoryServer::closing(GCFEvent& e, GCFPortInterface& p)
{
  int status = GCFEvent::HANDLED;

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

void GPIController::subscribe(char* data, bool onOff)
{
  SubscribeEvent e;
  unsigned int datalength(0);
  sscanf(data, "%03x", &datalength);
  e.onOff = onOff;
  e.seqNr = 0;
  e.length += datalength;
  _ssPort.send(e, data + 3, datalength);
}

void GPIController::unpackPropertyList(char* pListData, list<string>& propertyList)
{
  unsigned int dataLength;
  char* pPropertyData;
  sscanf(pListData, "%03x", &dataLength);
  pPropertyData = pListData + 3;
  propertyList.clear();
  if (dataLength > 0)
  {
    string propName;
    char* pPropName = strtok(pPropertyData, "|");
    while (pPropName && dataLength > 0)
    {
      propName = pPropName;      
      pPropName = strtok(NULL, "|");
      dataLength -= (propName.size() + 1);
      propertyList.push_front(propName);
    }
  }
}

TPIResult GPIController::unLinkProperties(list<string>& properties, bool onOff)
{
  
  TPIResult result(PI_NO_ERROR);
  if (_counter > 0)
  {
    result = PI_SS_BUSY;
  }
  else 
  {
    for (list<string>::iterator iter = properties.begin(); 
         iter != properties.end(); ++iter)
    {
      if (onOff)
      {
        result = (_propProxy.subscribe(_name + "_" + *iter) == GCF_NO_ERROR ?
                  PI_NO_ERROR :
                  PI_SCADA_ERROR);
      }
      else
      {
        result = (_propProxy.unsubscribe(_name + "_" + *iter) == GCF_NO_ERROR ?
                  PI_NO_ERROR :
                  PI_SCADA_ERROR);
      }
      _counter++;        
    }
  }
    
  return result;
}

void GPIController::propSubscribed(string& propName)
{
  _counter--;
  if (_counter == 0 && _propertyAgent.isConnected())
  {
    PAPropertieslinkedEvent e(0, PM_NO_ERROR);
    string allPropNames;
    unsigned short bufLength(scope.size() + allPropNames.size() + 6);
    char* buffer = new char[bufLength + 1];
    sprintf(buffer, "%03x%s%03x%s", scope.size(), scope.c_str(), 
                                    allPropNames.size(), allPropNames.c_str());
    e.length += bufLength;
    _propertyAgent.send(e, buffer, bufLength);
    delete [] buffer;
  }
}

void GPIController::propUnsubscribed(string& propName)
{
  _counter--;
  if (_counter == 0 && _propertyAgent.isConnected())
  {
    PAPropertiesunlinkedEvent e(0, NO_ERROR);
    string allPropNames;
    unsigned short bufLength(scope.size() + allPropNames.size() + 6);
    char* buffer = new char[bufLength + 1];
    sprintf(buffer, "%03x%s%03x%s", scope.size(), scope.c_str(), 
                                    allPropNames.size(), allPropNames.c_str());
    e.length += bufLength;
    _propertyAgent.send(e, buffer, bufLength);
    delete [] buffer;
  }
}

void GPIController::propValueChanged(string& propName, GCFPValue& value)
{
  unsigned short bufLength;
  FProperty* pFProperty(0);
  FPValue* pVal(0);
  static char* buffer[1024];
  
  switch (value.getType())
  {
    case GCFPValue::BOOL_VAL:
      pVal = new FPBoolValue(((GCFPVInteger *)&value)->getValue());
      pFProperty = new FProperty(propName, FPValue::BOOL_VAL);
      break;
   
    default:
      break;
  }
  if (pFProperty && pVal)
  {
    pSetProperty->setValue(*pVal);
    bufLength = pSetProperty->pack(buffer);
    delete pVal;
    delete pSetProperty;
  }
  e.length += bufLength;
  pPort->send(e, buffer, bufLength);
}

void GPIController::localValueChanged(GCFEvent& e, GCFPortInterface& p)
{
  char* buffer = ((char*)&e) + sizeof(GCFEvent);
  unsigned char propNameLength(0);
  unsigned char taskNameLength(0);
  unsigned char bytesRead(0);
  static char totalPropName[512];
  static char locPropName[256];
  static char taskName[256];
  string propName;

  memcpy((void *) &propNameLength, buffer, sizeof(unsigned char));
  buffer += sizeof(unsigned char);
  memcpy(locPropName, buffer, propNameLength);
  locPropName[propNameLength] = 0;
  buffer += propNameLength;
  FProperty newProperty(locPropName, (FPValue::ValueType) buffer[0]);
  bytesRead = newProperty.unpack(buffer);
  buffer += bytesRead;

  memcpy((void *) &taskNameLength, buffer, sizeof(unsigned char));
  buffer += sizeof(unsigned char);
  memcpy(taskName, buffer, taskNameLength);
  taskName[taskNameLength] = 0;

  sprintf(totalPropName, "%s_%s_%s", _name, taskName, locPropName);
  propName = locPropName;

  GCFPValue *pGCFVal(0);
  
  const FPValue *pVal = newProperty.getValue();
  
  switch (pVal->getType())
  {
    case FPValue::BOOL_VAL:
      pGCFVal = new GCFPVBool(((FPBoolValue *)pVal)->getValue());
      break;
      
    default:
      cerr << "Error not support this type at this moment" << endl;
      break;
  }
  
  if (_propProxy.set(propName, &pGCFVal) == GCF_NO_ERROR)
    cerr << "Set value error: " << pVar->formatValue(CharString()) << endl;
  if (pGCFVal)
    delete pGCFVal;
}


