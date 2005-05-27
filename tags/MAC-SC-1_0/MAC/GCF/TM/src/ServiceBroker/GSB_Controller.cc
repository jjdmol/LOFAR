//#  GSB_Controller.cc: 
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

#include "GSB_Controller.h"
#include <../SB_Protocol.ph>
#include <GCF/ParameterSet.h>
#include <GSB_Defines.h>

namespace LOFAR 
{
 namespace GCF 
 {
  using namespace TM;
  namespace SB 
  {

static string sSBTaskName("GCF-SB");

GSBController::GSBController() : 
  GCFTask((State)&GSBController::initial, sSBTaskName),
  _counter(0)
{
  // register the protocol for debugging purposes
  registerProtocol(SB_PROTOCOL, SB_PROTOCOL_signalnames);

  // initialize the port
  _brokerProvider.init(*this, "provider", GCFPortInterface::MSPP, SB_PROTOCOL);
  readRanges();
}

GSBController::~GSBController()
{
  cleanupGarbage();
  LOG_INFO("Deleting ServiceBroker");
}

GCFEvent::TResult GSBController::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      _brokerProvider.open();
      break;

    case F_CONNECTED:
      TRAN(GSBController::operational);
      break;

    case F_DISCONNECTED:
      if (&p == &_brokerProvider)
        _brokerProvider.setTimer(1.0); // try again after 1 second
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GSBController::operational(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_CONNECTED:   
      cleanupGarbage();
      _brokerClients.push_back(&p);
      break;

    case F_ACCEPT_REQ:
      acceptConnectRequest();
      break;

    case F_DISCONNECTED:      
      if (&p != &_brokerProvider) p.close();
      // else //TODO: find out this can realy happend
      break;

    case F_CLOSED:
      clientGone(p);
      break;
 
    case SB_REGISTER_SERVICE:
    {
      SBRegisterServiceEvent request(e);
      SBServiceRegisteredEvent response;
      response.seqnr = request.seqnr;
      if (findService(request.servicename))
      {
        LOG_ERROR(formatString(
            "Service %s already exist", 
            request.servicename.c_str()));
        response.result = SB_SERVICE_ALREADY_EXIST;
      }
      else
      {
        TServiceInfo serviceInfo;
        serviceInfo.portNumber = claimPortNumber(request.host);
        if (serviceInfo.portNumber > 0)
        {
          LOG_INFO(formatString(
              "Service %s registered with portnumber %d",
              request.servicename.c_str(),
              serviceInfo.portNumber));
          serviceInfo.host = request.host;
          serviceInfo.pPortToOwner = &p;
          _services[request.servicename] = serviceInfo;
  
          response.result = SB_NO_ERROR;
          response.portnumber = serviceInfo.portNumber;
        }
        else
        {
          LOG_ERROR(formatString(
              "All available port numbers are claimed (%s)", 
              request.servicename.c_str()));
          response.result = SB_NO_FREE_PORTNR;
        }
      }
      p.send(response);
      break;
    }
    case SB_UNREGISTER_SERVICE:
    {
      SBUnregisterServiceEvent request(e);
      TServiceInfo* pServiceInfo = findService(request.servicename);
      if (pServiceInfo)
      {
        LOG_INFO(formatString(
            "Service %s unregistered", 
            request.servicename.c_str()));
        freePort(request.servicename, pServiceInfo);
        _services.erase(request.servicename);
      }
      else
      {
        LOG_ERROR(formatString(
            "Unknown service: %s", 
            request.servicename.c_str()));
      }
      break;
    }
    case SB_GET_SERVICEINFO:
    {
      SBGetServiceinfoEvent request(e);
      SBServiceInfoEvent response;
      response.seqnr = request.seqnr;
      TServiceInfo* pServiceInfo = findService(request.servicename);
      if (pServiceInfo)
      {
        LOG_INFO(formatString(
            "Service %s requested on %s:%d", 
            request.servicename.c_str(),
            pServiceInfo->host.c_str(),
            pServiceInfo->portNumber));
        response.host = pServiceInfo->host;
        response.portnumber = pServiceInfo->portNumber;
        response.result = SB_NO_ERROR;
      }
      else
      {
        LOG_ERROR(formatString(
            "Unknown service: %s", 
            request.servicename.c_str()));
        response.result = SB_UNKNOWN_SERVICE;        
      }
      p.send(response);
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void GSBController::acceptConnectRequest()
{
  GTMSBTCPPort* pNewSBClientPort = new GTMSBTCPPort();
  assert(pNewSBClientPort);
  pNewSBClientPort->init(*this, "server", GCFPortInterface::SPP, SB_PROTOCOL);
  _brokerProvider.accept(*pNewSBClientPort);
  LOG_INFO("A new SB client tries to connect. Accept!!!");
}

void GSBController::clientGone(GCFPortInterface& p)
{
  // first step of what must be done before the closed port can be deleted too
  LOG_INFO("A SB client is gone, so start the closing sequence!");  

  TServiceInfo* pServiceInfo;
  list<string> servicesGarbage;
  // search for all services registered via the closed port
  for (TServices::iterator sIter = _services.begin();
       sIter != _services.end(); ++sIter)
  {
    pServiceInfo = &sIter->second;
    if (pServiceInfo->pPortToOwner == &p)
    {
      servicesGarbage.push_back(sIter->first);
      // the involved and claimed portnumbers must be freed
      freePort(sIter->first, pServiceInfo);
    }
  }
  
  // delete found services
  for (list<string>::iterator iter = servicesGarbage.begin();
       iter != servicesGarbage.end(); ++iter)
  {
    _services.erase(*iter);
  }
  
  // move closed port to garbage list
  _brokerClients.remove(&p);
  _brokerClientsGarbage.push_back(&p);
  LOG_INFO("Closing sequence finished!");  
}

void GSBController::freePort(const string& servicename, TServiceInfo* pServiceInfo)
{
  SBServiceGoneEvent indication;

  TPortStates* pPortStates = findHost(pServiceInfo->host);
  if (pPortStates)
  {
    // 1: mark port as not in use
    (*pPortStates)[pServiceInfo->portNumber] = false;
    LOG_INFO(formatString(
          "Portnumber %d freed for use on host '%s'.", 
         pServiceInfo->portNumber,
         pServiceInfo->host.c_str()));

    // 2: send to each service client the indication that the service is gone 
    GCFPortInterface* pPort;
    indication.servicename = servicename;
    
    for (list<GCFPortInterface*>::iterator pIter = _brokerClients.begin();
         pIter != _brokerClients.end(); ++pIter)
    {
      pPort = (*pIter);
      if (pPort->isConnected())
      {
        pPort->send(indication);
      }
    }
  }
}

void GSBController::cleanupGarbage()
{
  GCFPortInterface* pPort;
  for (list<GCFPortInterface*>::iterator iter = _brokerClientsGarbage.begin();
       iter != _brokerClientsGarbage.end(); ++iter)
  {
    pPort = *iter;
    delete pPort;    
  }
  _brokerClientsGarbage.clear();
}

void GSBController::readRanges()
{
  ParameterSet rangeSet = ParameterSet::instance()->makeSubset("mac.gcf.sb.range");
  unsigned short rangeNr(1);
  unsigned int firstValue(0);
  unsigned int lastValue(0);
  string host;
    
  string hostParam = formatString("%d.host", rangeNr);  
  string firstPortNumberParam;  
  string lastPortNumberParam;
  while (rangeSet.isDefined(hostParam))
  {    
    firstPortNumberParam = formatString("%d.firstPortNumber", rangeNr);  
    lastPortNumberParam = formatString("%d.lastPortNumber", rangeNr);  
    host = rangeSet.getString(hostParam);

    try
    {
      firstValue = rangeSet.getInt(firstPortNumberParam);
    }
    catch (...)
    {
      LOG_FATAL(formatString (
          "No firstPortNumbers defined for range %d in ServiceBroker.conf file.",
          rangeNr));
      exit(0);      
    }
    
    try 
    {
      lastValue = rangeSet.getInt(lastPortNumberParam);
    }
    catch (...)
    {
      // if only firstValue is set than one value should be add to the available list
      lastValue = firstValue;
    }
    
    // swap values if first is higher then lastValue
    if (firstValue > lastValue) 
    {
      unsigned int tmpValue = lastValue;
      lastValue = firstValue;
      firstValue = tmpValue;
    }
    
    TPortStates* pPortStates = &_availableHosts[host];
    for (unsigned int i = firstValue; i <= lastValue; i++)
    {
      (*pPortStates)[i] = false;
    }
    rangeNr++;
    hostParam = formatString("%d.host", rangeNr);  
  }
}

unsigned int GSBController::claimPortNumber(const string& host)
{
  TPortStates* pPortStates = &_availableHosts[host];
  for (TPortStates::iterator portIter = pPortStates->begin();
       portIter != pPortStates->end(); ++portIter)
  {
    if (!portIter->second)
    {
      portIter->second = true; // portNumber is now in use
      LOG_INFO(formatString(
            "Portnumber %d claimed on host '%s'.", 
           portIter->first,
           host.c_str()));
      return portIter->first;
    }
  }
  return 0; // no portNumber could be claimed
}

GSBController::TServiceInfo* GSBController::findService(const string& servicename)
{
  TServices::iterator iter = _services.find(servicename);  
  return (iter != _services.end() ? &iter->second : 0);
}

GSBController::TPortStates* GSBController::findHost(const string& host)
{
  // 1: find the host
  TPortHosts::iterator iter = _availableHosts.find(host);
  return (iter != _availableHosts.end() ? &iter->second : 0);
}
  } // namespace SB
 } // namespace GCF
} // namespace LOFAR
