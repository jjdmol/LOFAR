//#  GPI_Controller.cc: 
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

#include "GPI_Controller.h"
#include "GPI_PMLlightServer.h"
#include "PI_Protocol.ph"
#include <GCF/ParameterSet.h>

static string sPITaskName("GCF-PI");

GPIController::GPIController() : 
  GCFTask((State)&GPIController::initial, sPITaskName),
  _isBusy(false)
{
  // register the protocol for debugging purposes
  registerProtocol(PI_PROTOCOL, PI_PROTOCOL_signalnames);

  // initialize the port provider
  _rtcClientPortProvider.init(*this, "provider", GCFPortInterface::MSPP, PI_PROTOCOL);
  ParameterSet::instance()->adoptFile("PropertyAgent.conf");  
}

GPIController::~GPIController()
{
  LOG_INFO("Deleting PropertyInterface");
}

GCFEvent::TResult GPIController::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      _rtcClientPortProvider.open();
      break;

    case F_CONNECTED:
      TRAN(GPIController::connected);
      break;

    case F_DISCONNECTED:
      if (&p == &_rtcClientPortProvider)
        _rtcClientPortProvider.setTimer(1.0); // try again after 1 second
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GPIController::connected(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_DISCONNECTED:      
      if (&p == &_rtcClientPortProvider)
      {
        // TODO: find out this implies problems for the concrete ports too or not
      }
      break;

    case F_ACCEPT_REQ:
    {
      GPIPMLlightServer* pNewSS = new GPIPMLlightServer(*this);
      _rtcClientPortProvider.accept(pNewSS->getPort());
      pNewSS->start();
      _pmlLightServers.push_back(pNewSS);
      break;
    }
     
    case F_TIMER:
    {
      GCFTimerEvent* pTimer = (GCFTimerEvent*)(&e);
      if (pTimer->arg)
      {
        GPIPMLlightServer* pSS = (GPIPMLlightServer*)(pTimer->arg);
        if (pSS)
        {
          for (TPMLlightServers::iterator iter = _pmlLightServers.begin();
               iter != _pmlLightServers.end(); ++iter)
          {
            if ((*iter) == pSS)
            {
              _pmlLightServers.erase(iter);
              delete pSS;
              break;
            }
          }
        }
      }      
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void GPIController::close(GPIPMLlightServer& ss)
{
  _rtcClientPortProvider.setTimer(0, 0, 0, 0, (void*)&ss);
}
