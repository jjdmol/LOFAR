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
  registerProtocol(F_SUPERVISORY_PROTOCOL, F_SUPERVISORY_PROTOCOL_signalnames, 0);

  // initialize the port provider
  _ssPortProvider.init(*this, "pi", GCFPortInterface::MSPP, PI_PROTOCOL);
}

GPIController::~GPIController()
{
}

int GPIController::initial(GCFEvent& e, GCFPortInterface& p)
{
  int status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
    case F_TIMER_SIG:
      _ssPortProvider.open();
      break;

    case F_CONNECTED_SIG:
      TRAN(&GPIController::connected);
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

int GPIController::connected(GCFEvent& e, GCFPortInterface& p)
{
  int status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_DISCONNECTED_SIG:      
      if (&p == &_ssPortProvider)
      {
        // TODO: find out this implies problems for the concrete ports too or not
      }
      break;

    case F_ACCEPT_REQ:
    {
      GPISupervisoryServer* newSS = new GPISupervisoryServer(*this);
      newSS->start();
      _supervisoryServers.push_back(newSS);
      _ssPortProvider.accept(*newSS);
      break;
    }
     
    case F_TIMER_SIG:
    {
      GCFTimerEvent* pTimer = static_cast<GCFTimerEvent*>(&e);
      if (pTimer->arg)
      {
        GPISupervisoryServer* pSS = static_cast<GPISupervisoryServer*>(pTimer->arg);
        if (pSS)
        {
          _supervisoryServers.remove(pSS);
          delete pSS;
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