//#  NodeManagerDaemon.cc: 
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

#include "NodeManagerDaemon.h"
#include <GCF/LogSys/GCF_KeyValueLogger.h>
#include <GCF/Utils.h>
#include <NodeManagerClient.h>
#include <NM_Protocol.ph>

namespace LOFAR 
{
using namespace GCF::Common;
using namespace GCF::TM;
 namespace ANM 
 {

NodeManagerDaemon::NodeManagerDaemon() :
  GCFTask((State)&NodeManagerDaemon::initial, "APL-NMD")
{
  // register the protocol for debugging purposes
  registerProtocol(NM_PROTOCOL, NM_PROTOCOL_signalnames);

  // initialize the port
  _nmdPortProvider.init(*this, "server", GCFPortInterface::MSPP, NM_PROTOCOL);
}

GCFEvent::TResult NodeManagerDaemon::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      if (!_nmdPortProvider.isConnected())
      {
        _nmdPortProvider.open();
      }
      break;

    case F_CONNECTED:
      TRAN(NodeManagerDaemon::operational);
      break;

    case F_DISCONNECTED:
      p.setTimer(1.0); // try again after 1 second
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult NodeManagerDaemon::operational(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static unsigned long garbageTimerID = 0;

  switch (e.signal)
  {
    case F_DISCONNECTED:
      DBGFAILWHEN(&_nmdPortProvider == &p && "NMD port provider may not be disconnected."); 
      break;
      
    case F_TIMER:
    {
      // cleanup the garbage of closed ports to master clients
      NodeManagerClient* pClient;
      for (TClients::iterator iter = _clientsGarbage.begin();
           iter != _clientsGarbage.end(); ++iter)
      {
        pClient = *iter;
        delete pClient;
      }
      _clientsGarbage.clear();
      break;
    }  
    case F_CLOSED:
      DBGFAILWHEN(&_nmdPortProvider == &p);
      break;
      
    case F_CONNECTED:
      DBGFAILWHEN(&_nmdPortProvider == &p);
      break;
      
    case F_ACCEPT_REQ:
    {
      LOG_INFO("New NodeManger client accepted!");
      NodeManagerClient* nmc = new NodeManagerClient(*this);
      nmc->start();
      break;
    }
    case F_ENTRY:
      garbageTimerID = _nmdPortProvider.setTimer(1.0, 5.0); 
      break;

    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void NodeManagerDaemon::clientConnected(NodeManagerClient& client)
{
  _clients.push_back(&client);
}

void NodeManagerDaemon::clientClosed(NodeManagerClient& client)
{
  _clients.remove(&client);
  _clientsGarbage.push_back(&client);  
}

 } // namespace ANM
} // namespace LOFAR
