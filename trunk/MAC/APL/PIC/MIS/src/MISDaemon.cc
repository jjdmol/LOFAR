//#  MISDaemon.cc: 
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

#include "MISDaemon.h"
#include <GCF/LogSys/GCF_KeyValueLogger.h>
#include <GCF/Utils.h>
#include <MISSession.h>
#include <MIS_Protocol.ph>
#include <MISDefines.h>
#include <GCF/ParameterSet.h>

namespace LOFAR 
{
using namespace GCF::Common;
using namespace GCF::TM;
 namespace AMI
 {

MISDaemon::MISDaemon() :
  GCFTask((State)&MISDaemon::initial, MISD_TASK_NAME)
{
  // register the protocol for debugging purposes
  registerProtocol(MIS_PROTOCOL, MIS_PROTOCOL_signalnames);

  // initialize the port
  _misdPortProvider.init(*this, MISD_PORT_NAME, GCFPortInterface::MSPP, MIS_PROTOCOL);
  try
  {
    GCF::ParameterSet::instance()->adoptFile("./RemoteStation.conf");
  }
  catch (Exception e)
  {
    LOG_FATAL_STR("Error: failed to load configuration files: " << e.text());
    exit(EXIT_FAILURE);
  }  
}

GCFEvent::TResult MISDaemon::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      if (!_misdPortProvider.isConnected())
      {
        _misdPortProvider.open();
      }
      break;

    case F_CONNECTED:
      TRAN(MISDaemon::accepting);
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

GCFEvent::TResult MISDaemon::accepting(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static unsigned long garbageTimerID = 0;
  static unsigned long rereadPolicyTimerID = 0;

  switch (e.signal)
  {
    case F_ENTRY:
      garbageTimerID = _misdPortProvider.setTimer(5.0, 5.0); 
      rereadPolicyTimerID = _misdPortProvider.setTimer(60.0, 60.0); 
      break;

    case F_DISCONNECTED:
      DBGFAILWHEN(&_misdPortProvider == &p && "MISD port provider may not be disconnected."); 
      break;
      
    case F_TIMER:
    {
      GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(e);
      
      if (timerEvent.id == garbageTimerID)
      {
        // cleanup the garbage of closed ports to master clients
        MISSession* pClient;
        for (TSessions::iterator iter = _sessionsGarbage.begin();
             iter != _sessionsGarbage.end(); ++iter)
        {
          pClient = *iter;
          delete pClient;
        }
        _sessionsGarbage.clear();
      }
      else if (timerEvent.id == rereadPolicyTimerID)
      {
        _policyHandler.rereadPolicyFile();
      }      
      break;
    }  
    case F_CLOSED:
      DBGFAILWHEN(&_misdPortProvider == &p);
      break;
      
    case F_CONNECTED:
      DBGFAILWHEN(&_misdPortProvider == &p);
      break;
      
    case F_ACCEPT_REQ:
    {
      LOG_INFO("New MIS client accepted!");
      MISSession* miss = new MISSession(*this);
      miss->start();
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void MISDaemon::clientClosed(MISSession& client)
{
  _sessionsGarbage.push_back(&client);  
}

 } // namespace AMI
} // namespace LOFAR
