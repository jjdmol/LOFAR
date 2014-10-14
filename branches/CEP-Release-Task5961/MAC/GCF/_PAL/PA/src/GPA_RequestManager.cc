//#  GPA_RequestManager.cc: 
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

#include <lofar_config.h>

#include "GPA_RequestManager.h"
#include <GCF/TM/GCF_Event.h>

namespace LOFAR 
{
 namespace GCF 
 {
using namespace TM;
  namespace PAL 
  {

GPARequestManager::GPARequestManager()
{
}

GPARequestManager::~GPARequestManager()
{
  deleteAllRequests();
}

void GPARequestManager::registerRequest(const GCFEvent& e, GCFPortInterface& requestPort)
{
  TRequest* pRequest;
  bool alreadyRegistered(false);
  for (list<TRequest>::iterator iter = _requests.begin();
       iter != _requests.end(); ++iter)
  {
    pRequest = &(*iter);
    if (pRequest)
    {
      if (pRequest->pRPort == &requestPort && (GCFEvent*) (pRequest->pEvent) == &e)
      {
        alreadyRegistered = true;
        break;
      }
    }
  }

  if (!alreadyRegistered)
  {
    TRequest newRequest;
    newRequest.pRPort = &requestPort;
    newRequest.pEvent = &e;
    _requests.push_back(newRequest);
  }
}

GCFEvent* GPARequestManager::getOldestRequest()
{
  GCFEvent* pRequestEvent(0);
  if (_requests.size() > 0)
  {
    TRequest* pRequest = &_requests.front();
    pRequestEvent = (GCFEvent*) (pRequest->pEvent);
  }
  return pRequestEvent;  
}

GCFPortInterface* GPARequestManager::getOldestRequestPort()
{
  GCFPortInterface* pRequestPort(0);
  if (_requests.size() > 0)
  {
    TRequest* pRequest = &_requests.front();
    pRequestPort = pRequest->pRPort;
  }
  return pRequestPort;  
}

void GPARequestManager::deleteOldestRequest()
{
  if (_requests.size() > 0)
  {
    TRequest* pRequest = &_requests.front();
    if (pRequest->pEvent)
      // constructed in GPAController::waiting_state or 
      // in case of F_CLOSED: GPAPropSetSession::mayContinue or GPAPropSetSession::defaultHandling 
      delete pRequest->pEvent;
  }
  
  _requests.pop_front();
}

void GPARequestManager::deleteRequestsOfPort(const GCFPortInterface& requestPort)
{
  TRequest* pRequest;
  for (list<TRequest>::iterator iter = _requests.begin();
       iter != _requests.end(); ++iter)
  {
    pRequest = &(*iter);
    if (pRequest)
    {
      if (pRequest->pRPort == &requestPort)
      {
        if (pRequest->pEvent)
          // constructed in GPAController::waiting_state or 
          // in case of F_CLOSED: GPAPropSetSession::mayContinue or GPAPropSetSession::defaultHandling 
          delete pRequest->pEvent; 
        iter = _requests.erase(iter);
        --iter;
      }
    }
  }
}

void GPARequestManager::deleteAllRequests()
{
  for (list<TRequest>::iterator pRequest = _requests.begin();
       pRequest != _requests.end(); ++pRequest)
  {
    if (pRequest->pEvent)
      // constructed in GPAController::waiting_state or 
      // in case of F_CLOSED: GPAPropSetSession::mayContinue or GPAPropSetSession::defaultHandling 
      delete pRequest->pEvent;
  }
  _requests.clear();
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
