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
#include "GPI_CEPServer.h"
#include "GPI_RTCServer.h"
#include "PI_Protocol.ph"
#include <GCF/ParameterSet.h>

static string sPITaskName("GCF-PI");

GPIController::GPIController() : 
  GCFTask((State)&GPIController::initial, sPITaskName)
{
  // register the protocol for debugging purposes
  registerProtocol(PI_PROTOCOL, PI_PROTOCOL_signalnames);

  // initialize the port provider
  _rtcClientPortProvider.init(*this, "rtc-provider", GCFPortInterface::MSPP, PI_PROTOCOL);
  _cepClientPortProvider.init(*this, "cep-provider", GCFPortInterface::MSPP, PI_PROTOCOL);
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
      if (!_rtcClientPortProvider.isConnected())
      {
        _rtcClientPortProvider.open();
      }
      if (!_cepClientPortProvider.isConnected())
      {
        _cepClientPortProvider.open();
      }
      break;

    case F_CONNECTED:
      if (_cepClientPortProvider.isConnected() && _rtcClientPortProvider.isConnected())
      {
        TRAN(GPIController::connected);
      }
      break;

    case F_DISCONNECTED:
      if (&p == &_rtcClientPortProvider)
        _rtcClientPortProvider.setTimer(1.0); // try again after 1 second
      if (&p == &_cepClientPortProvider)
        _cepClientPortProvider.setTimer(1.0); // try again after 1 second
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
      if (&p == &_rtcClientPortProvider || &p == &_cepClientPortProvider)
      {
        // TODO: find out this implies problems for the concrete ports too or not
      }
      break;

    case F_ACCEPT_REQ:
    {
      GPIPMLlightServer* pNewPLS;
      if (&p == &_rtcClientPortProvider)
      {
        pNewPLS = new GPIRTCServer(*this);
        _rtcClientPortProvider.accept(pNewPLS->getClientPort());
      }
      else
      {
        pNewPLS = new GPICEPServer(*this);
        _cepClientPortProvider.accept(pNewPLS->getClientPort());
      }
      pNewPLS->start();
      _pmlLightServers.push_back(pNewPLS);
      break;
    }
     
    case F_TIMER:
    {
      GCFTimerEvent* pTimer = (GCFTimerEvent*)(&e);
      if (pTimer->arg)
      {
        GPIPMLlightServer* pPLS = (GPIPMLlightServer*)(pTimer->arg);
        if (pPLS)
        {
          for (TPMLlightServers::iterator iter = _pmlLightServers.begin();
               iter != _pmlLightServers.end(); ++iter)
          {
            if ((*iter) == pPLS)
            {
              _pmlLightServers.erase(iter);
              delete pPLS;
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

void GPIController::close(GPIPMLlightServer& pls)
{
  _rtcClientPortProvider.setTimer(0, 0, 0, 0, (void*)&pls);
}
