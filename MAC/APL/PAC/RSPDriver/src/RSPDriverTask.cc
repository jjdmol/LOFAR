//#
//#  RSPDriverTask.cc: implementation of RSPDriverTask class
//#
//#  Copyright (C) 2002-2004
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

// this include needs to be first!
#define DECLARE_SIGNAL_NAMES
#include "RSP_Protocol.ph"
#include "EPA_Protocol.ph"

#include "RSPDriverTask.h"
#include "Command.h"
#include "BWCommand.h"
#include "BWSync.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace std;
using namespace LOFAR;

RSPDriverTask::RSPDriverTask(string name)
  : GCFTask((State)&RSPDriverTask::initial, name), m_scheduler()
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
  registerProtocol(EPA_PROTOCOL, EPA_PROTOCOL_signalnames);

  m_client.init(*this, "client", GCFPortInterface::SPP, RSP_PROTOCOL);

  for (int boardid = 0; boardid < N_RSPBOARDS; boardid++)
  {
    char name[64] = "board";
    snprintf(name, 64, "board%d", boardid);
    m_board[boardid].init(*this, name, GCFPortInterface::SAP, EPA_PROTOCOL /*, true*/);
    //m_board[boardid].setAddr("eth0", "aa:bb:cc:dd:ee:ff");
  }

  /**
   * Create synchronization action classes.
   */
  BWSync* bwsync[N_RSPBOARDS];
  for (int boardid = 0; boardid < N_RSPBOARDS; boardid++)
  {
    bwsync[boardid] = new BWSync(m_board[boardid], boardid);
    bwsync[boardid]->setPriority(1);

    m_scheduler.addSyncAction(bwsync[boardid]);
  }
}

RSPDriverTask::~RSPDriverTask()
{
}

bool RSPDriverTask::isEnabled()
{
  bool enabled = true;
  for (int boardid = 0; boardid < N_RSPBOARDS; boardid++)
  {
    if (!m_board[boardid].isConnected())
    {
      enabled = false;
      break;
    }
  }
  
  return enabled;
}

void RSPDriverTask::openBoards()
{
  for (int boardid = 0; boardid < N_RSPBOARDS; boardid++)
  {
    if (!m_board[boardid].isConnected()) m_board[boardid].open();
  }
}

GCFEvent::TResult RSPDriverTask::initial(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch(event.signal)
  {
    case F_INIT:
    {
    }
    break;

    case F_ENTRY:
    {
      openBoards();
    }
    break;

    case F_CONNECTED:
    {
      LOG_DEBUG(formatString("port '%s' connected", port.getName().c_str()));
      if (isEnabled())
      {
	TRAN(RSPDriverTask::enabled);
      }
    }
    break;

    case F_DISCONNECTED:
    {
      port.setTimer((long)3); // try again in 3 seconds
      LOG_WARN(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
      port.close();
    }
    break;

    case F_TIMER:
    {
      LOG_INFO(formatString("port '%s' retry of open...", port.getName().c_str()));
      port.open();
    }
    break;

    case F_DATAIN:
    {
    }
    break;

    case F_EXIT:
    {
      // cancel timers
      m_board[0].cancelAllTimers();
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult RSPDriverTask::enabled(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      // start waiting for clients
      if (!m_client.isConnected()) m_client.open();

      /* Start the update timer */
      m_board[0].setTimer((long)1,0,1,0); // update every second
    }
    break;

#if 0
    case F_ACCEPT_REQ:
      m_client.getPortProvider().accept();
      break;
#else
    case F_CONNECTED:
    {
      LOG_DEBUG(formatString("port '%s' connected", port.getName().c_str()));
    }
    break;
#endif

    case RSP_SETWEIGHTS:
    {
      /* enter the command in the scheduler's queue */
      BWCommand* command = new BWCommand(event, port, Command::WRITE);

      /* acknowledgement */
      RSPSetweightsackEvent ack;

      /* enter into the scheduler's queue */
      ack.timestamp = m_scheduler.enter(command);

      ack.status = SUCCESS;

      m_client.send(ack);
    }
    break;

    case F_TIMER:
    {
      GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&event);
      LOG_DEBUG(formatString("timer=(%d,%d)", timer->sec, timer->usec));

      if (&port == &m_board[0])
      {
	/* run the scheduler */
	status = m_scheduler.run(event,port);
      }
      else
      {
	m_scheduler.dispatch(event, port);
      }
    }
    break;

    case F_DISCONNECTED:
    {
      LOG_DEBUG(formatString("port %s disconnected", port.getName().c_str()));
      port.close();

      if (&port != &m_client)
      {
	m_client.close();
	TRAN(RSPDriverTask::initial);
      }
      else port.open(); // re-open for next client
    }
    break;

    case F_EXIT:
    {
      // cancel timers
      m_client.cancelAllTimers();
      m_board[0].cancelAllTimers();
    }
    break;

    default:
      status = m_scheduler.dispatch(event, port);
      break;
  }

  return status;
}

int main(int argc, char** argv)
{
#if 0
  char prop_path[PATH_MAX];
  const char* mac_config = getenv("MAC_CONFIG");

  snprintf(prop_path, PATH_MAX-1,
	   "%s/%s", (mac_config?mac_config:"."),
	   "log4cplus.properties");
  INIT_LOGGER(prop_path);
#endif

  LOG_INFO(formatString("Program %s has started", argv[0]));

  GCFTask::init(argc, argv);

  RSPDriverTask rsp("RSP");

  rsp.start(); // make initial transition

  GCFTask::run();

  LOG_INFO("Normal termination of program");

  return 0;
}
