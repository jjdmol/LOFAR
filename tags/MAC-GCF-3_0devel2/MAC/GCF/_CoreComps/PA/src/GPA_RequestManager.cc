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

#include "GPA_RequestManager.h"
#include <GCF/GCF_Event.h>

GPARequestManager::GPARequestManager()
{
}

GPARequestManager::~GPARequestManager()
{
  deleteAllRequests();
}

void GPARequestManager::registerRequest(GCFPortInterface& requestPort, const GCFEvent& e)
{
  TRequest request;
  request.pRPort = &requestPort;
  char* buffer = new char[e.length];
  memcpy(buffer, (const char*) &e, e.length);
  request.pEvent = (GCFEvent*) buffer;

  _requests.push_back(request);
}

GCFEvent* GPARequestManager::getOldestRequest()
{
  if (_requests.size() > 0)
  {
    TRequest* pRequest = &_requests.front();
    return pRequest->pEvent;
  }
  else
    return 0;  
}

GCFPortInterface* GPARequestManager::getOldestRequestPort()
{
  if (_requests.size() > 0)
  {
    TRequest* pRequest = &_requests.front();
    return pRequest->pRPort;
  }
  else
    return 0;  
}

void GPARequestManager::deleteOldestRequest()
{
  if (_requests.size() > 0)
  {
    TRequest* pRequest = &_requests.front();
    if (pRequest->pEvent)
      delete [] pRequest->pEvent;
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
          delete [] pRequest->pEvent;
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
      delete [] pRequest->pEvent;
  }
  _requests.clear();
}
