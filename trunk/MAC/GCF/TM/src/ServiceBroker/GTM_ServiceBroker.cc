//#  GTM_ServiceBroker.cc: 
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

#include "GTM_ServiceBroker.h"
#include <SB_Protocol.ph>
#include <GCF/ParameterSet.h>

using namespace GCF;

#include <unistd.h>

static string sSBTaskName("GCF-SB");
GTMSBHandler* GTMSBHandler::_pInstance = 0;

extern void logResult(TSBResult result, const string& servicename);

GTMSBHandler::GTMSBHandler()
{
  GCFTask::registerHandler(*this);
}

GTMServiceBroker::GTMServiceBroker() :
  GCFTask((State)&GTMServiceBroker::initial, sSBTaskName)
{
  // register the protocol for debugging purposes
  registerProtocol(SB_PROTOCOL, SB_PROTOCOL_signalnames);

  // initialize the port
  _serviceBroker.init(*this, "client", GCFPortInterface::SAP, SB_PROTOCOL);
  ParameterSet::instance()->adoptFile("ServiceBroker.conf");
}

GTMServiceBroker::~GTMServiceBroker()
{
}

GTMServiceBroker* GTMServiceBroker::instance(bool temporary)
{
  if (0 == GTMSBHandler::_pInstance)
  {    
    GTMSBHandler::_pInstance = new GTMSBHandler();
    assert(!GTMSBHandler::_pInstance->mayDeleted());
    GTMSBHandler::_pInstance->_controller.start();
  }
  if (!temporary) GTMSBHandler::_pInstance->use();
  return &GTMSBHandler::_pInstance->_controller;
}

void GTMServiceBroker::release()
{
  assert(GTMSBHandler::_pInstance);
  assert(!GTMSBHandler::_pInstance->mayDeleted());
  GTMSBHandler::_pInstance->leave(); 
  if (GTMSBHandler::_pInstance->mayDeleted())
  {
    delete GTMSBHandler::_pInstance;
    assert(!GTMSBHandler::_pInstance);
  }
}

void GTMServiceBroker::registerService(GCFTCPPort& servicePort)
{
  string servicename = formatString("%s:%s", 
      servicePort.getTask()->getName().c_str(), 
      servicePort.getRealName().c_str());
  char hostName[200];
  gethostname(hostName, 200);

  SBRegisterServiceEvent request;

  TAction action;
  action.action = request.signal;
  action.pPort = &servicePort;
  action.servicename = servicename;
  
  request.seqnr = registerAction(action);
  request.host = hostName;
  request.servicename = servicename;
    
  if (_serviceBroker.isConnected())
  {
    _serviceBroker.send(request);
  }
}

void GTMServiceBroker::unregisterService(GCFTCPPort& servicePort)
{
  string servicename = formatString("%s:%s", 
      servicePort.getTask()->getName().c_str(), 
      servicePort.getRealName().c_str());

  SBUnregisterServiceEvent request;

  request.servicename = servicename;
    
  if (_serviceBroker.isConnected())
  {
    _serviceBroker.send(request);
  }
}

void GTMServiceBroker::getServiceinfo(GCFTCPPort& clientPort, const string& remoteServiceName)
{  
  SBGetServiceinfoEvent request;
  TAction action;
  action.action = request.signal;
  action.pPort = &clientPort;
  action.servicename = remoteServiceName;
  request.seqnr = registerAction(action);
  request.servicename = remoteServiceName;
    
  if (_serviceBroker.isConnected())
  {
    _serviceBroker.send(request);
  }
}


void GTMServiceBroker::deletePort(GCFTCPPort& port)
{
  TAction* pAction;
  unregisterService(port);
  for (TActionSeqList::iterator iter = _actionSeqList.begin();
       iter != _actionSeqList.end(); ++iter)
  {
    pAction = &iter->second;
    if (pAction->pPort == &port)
    {
      _actionSeqList.erase(iter);
      break;
    }
  }
  for (TServiceClients::iterator iter = _serviceClients.begin();
       iter != _serviceClients.end(); ++iter)
  {
    list<GCFTCPPort*>* pClientPorts = &iter->second;
    pClientPorts->remove(&port);
  }
}

unsigned short GTMServiceBroker::registerAction(TAction action)
{
  unsigned short seqnr(1); // 0 is reserved for internal msg. in SB
  TActionSeqList::const_iterator iter;
  do   
  {
    seqnr++;
    iter = _actionSeqList.find(seqnr);
  } while (iter != _actionSeqList.end());

  _actionSeqList[seqnr] = action; 

  return seqnr;
}

GCFEvent::TResult GTMServiceBroker::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      _serviceBroker.open();
      break;

    case F_CONNECTED:
      TRAN(GTMServiceBroker::operational);
      break;

    case F_DISCONNECTED:
      _serviceBroker.setTimer(1.0); // try again after 1 second
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GTMServiceBroker::operational(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_DISCONNECTED:
      LOG_FATAL(LOFAR::formatString ( 
          "Connection lost to Service Broker deamon"));
      p.close();
      break;
    case F_CLOSED:
      TRAN(GTMServiceBroker::initial);
      break;
    case F_ENTRY:
    {      
      TAction* pAction(0);
      TActionSeqList tmpSeqList(_actionSeqList);
      _actionSeqList.clear();
      for (TActionSeqList::iterator iter = tmpSeqList.begin();
           iter != tmpSeqList.end(); ++iter)
      {
        pAction = &iter->second;
        switch (pAction->action)
        {
          case SB_REGISTER_SERVICE: registerService(*pAction->pPort); break;
          case SB_UNREGISTER_SERVICE: unregisterService(*pAction->pPort); break;
          case SB_GET_SERVICEINFO: getServiceinfo(*pAction->pPort, pAction->servicename); break;
          default: assert(0);
        }
      }
      
      break;
    }  
    case SB_SERVICE_REGISTERED:
    {
      SBServiceRegisteredEvent response(e);
      TAction* pAction = &_actionSeqList[response.seqnr];
      if (pAction)
      {
        logResult(response.result, pAction->servicename);
        _actionSeqList.erase(response.seqnr);
        pAction->pPort->serviceRegistered(response.result, response.portnumber);
      }
      break;
    }

    case SB_SERVICE_INFO:
    {
      SBServiceInfoEvent response(e);
      TAction* pAction = &_actionSeqList[response.seqnr];
      if (pAction)
      {
        logResult(response.result, pAction->servicename);
        if (response.result == SB_NO_ERROR)
        {
          _serviceClients[pAction->servicename].push_back(pAction->pPort);
        }
        _actionSeqList.erase(response.seqnr);
        pAction->pPort->serviceInfo(response.result, response.portnumber, response.host);
      }
      break;
    }

    case SB_SERVICE_GONE:
    {
      SBServiceGoneEvent indication(e);
      TServiceClients::iterator iter = _serviceClients.find(indication.servicename);
      if (iter != _serviceClients.end())
      {
        list<GCFTCPPort*>* pClientPorts = &iter->second;
        GCFTCPPort* pClientPort;
        for (list<GCFTCPPort*>::iterator cIter = pClientPorts->begin();
             cIter != pClientPorts->end(); ++cIter)
        {
          pClientPort = *cIter;
          pClientPort->serviceGone();
        }
      }
      _serviceClients.erase(indication.servicename);
      
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void logResult(TSBResult result, const string& servicename)
{
  switch (result)
  {
    case SB_NO_ERROR:
      break;
    case SB_UNKNOWN_ERROR:
      LOG_FATAL(LOFAR::formatString ( 
          "Unknown error"));      
      break;
    case SB_SERVICE_ALREADY_EXIST:
      LOG_ERROR(LOFAR::formatString ( 
          "Service %s already exist",
          servicename.c_str()));
      break;
    case   SB_NO_FREE_PORTNR:
      LOG_ERROR(LOFAR::formatString ( 
          "No free portnumber for this service: %s)",
          servicename.c_str()));
      break;
    case SB_UNKNOWN_SERVICE:
      LOG_FATAL(LOFAR::formatString ( 
          "Unkown remote service: %s",
          servicename.c_str()));
      break;
    default:
      break;
  }
}
