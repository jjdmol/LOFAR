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
#include "GPI_SupervisoryServer.h"
#include "PI_Protocol.ph"

static string sPITaskName("PI");

GPIController::GPIController() : 
  GCFTask((State)&GPIController::initial, sPITaskName),
  _isBusy(false)
{
  // register the protocol for debugging purposes
  registerProtocol(PI_PROTOCOL, PI_PROTOCOL_signalnames);

  // initialize the port provider
  _ssPortProvider.init(*this, "server", GCFPortInterface::MSPP, PI_PROTOCOL);
}

GPIController::~GPIController()
{
}

GCFEvent::TResult GPIController::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
    case F_TIMER_SIG:
      _ssPortProvider.open();
      break;

    case F_CONNECTED_SIG:
      TRAN(GPIController::connected);
      break;

    case F_DISCONNECTED_SIG:
      if (&p == &_ssPortProvider)
        _ssPortProvider.setTimer(1.0); // try again after 1 second
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
    case F_DISCONNECTED_SIG:      
      if (&p == &_ssPortProvider)
      {
        // TODO: find out this implies problems for the concrete ports too or not
      }
      break;

    case F_ACCEPT_REQ_SIG:
    {
      GPISupervisoryServer* pNewSS = new GPISupervisoryServer(*this);
      pNewSS->start();
      _supervisoryServers.push_back(pNewSS);
      break;
    }
     
    case F_TIMER_SIG:
    {
      GCFTimerEvent* pTimer = static_cast<GCFTimerEvent*>(&e);
      if (pTimer->arg)
      {
        const GPISupervisoryServer* pSS = static_cast<const GPISupervisoryServer*>(pTimer->arg);
        if (pSS)
        {
          for (TSupervisoryServers::iterator iter = _supervisoryServers.begin();
               iter != _supervisoryServers.end(); ++iter)
          {
            if ((*iter) == pSS)
            {
              _supervisoryServers.erase(iter);
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

void GPIController::close(GPISupervisoryServer& ss)
{
  _ssPortProvider.setTimer(0, 0, 0, 0, (void*)&ss);
}
