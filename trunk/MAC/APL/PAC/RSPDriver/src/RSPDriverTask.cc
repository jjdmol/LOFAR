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
#include "SetWeightsCmd.h"
#include "GetWeightsCmd.h"
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

  m_acceptor.init(*this, "acceptor", GCFPortInterface::MSPP, RSP_PROTOCOL);

  for (int boardid = 0; boardid < N_RSPBOARDS; boardid++)
  {
    char name[64] = "board";
    snprintf(name, 64, "board%d", boardid);
    m_board[boardid].init(*this, name, GCFPortInterface::SAP, EPA_PROTOCOL /*, true*/);
    //m_board[boardid].setAddr("eth0", "aa:bb:cc:dd:ee:ff");
  }

  addAllSyncActions();
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

/**
 * Add all synchronization actions per board.
 * Order is:
 * - BWSync       // sync of beamformer weights info
 * - SSSync       // sync of subband selection parameters
 * - RCUSync      // sync of RCU control info
 * - StatusSync   // sync of system status info
 * - StatsSync    // sync of statistics info
 * - WGSync       // sync of WG control info
 * - VersionsSync // sync of version info
 */
void RSPDriverTask::addAllSyncActions()
{
  /**
   * For each board a separate BWSync instance is created which handles
   * the synchronization of data between the board and the cache for that board.
   */
  for (int boardid = 0; boardid < N_RSPBOARDS; boardid++)
  {
    BWSync* bwsync = 0;

    bwsync = new BWSync(m_board[boardid], boardid, MEPHeader::BFXRE);
    m_scheduler.addSyncAction(bwsync);
    bwsync = new BWSync(m_board[boardid], boardid, MEPHeader::BFXIM);
    m_scheduler.addSyncAction(bwsync);
    bwsync = new BWSync(m_board[boardid], boardid, MEPHeader::BFYRE);
    m_scheduler.addSyncAction(bwsync);
    bwsync = new BWSync(m_board[boardid], boardid, MEPHeader::BFYIM);
    m_scheduler.addSyncAction(bwsync);

#if 0
    SSSync* sssync = new SSSync(m_board[boardid], boardid);
    m_scheduler.addSyncAction(sssync);

    RCUSync* rcusync = new RCUSync(m_board[boardid], boardid);
    m_scheduler.addSyncAction(rcusync);

    StatusSync* statussync = new StatusSync(m_board[boardid], boardid);
    m_scheduler.addSyncAction(statussync);

    StatsSync* statssync = new StatsSync(m_board[boardid], boardid);
    m_scheduler.addSyncAction(statssync);

    WGSync* wgsync = new WGSync(m_board[boardid], boardid);
    m_scheduler.addSyncAction(wgsync);

    VersionSync* versionsync = new VersionSync(m_board[boardid], boardid);
    m_scheduler.addSyncAction(versionsync);
#endif
  }
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

void RSPDriverTask::collect_garbage()
{
}

GCFEvent::TResult RSPDriverTask::enabled(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  collect_garbage();

  switch (event.signal)
  {
    case F_ENTRY:
    {
      // start waiting for clients
      if (!m_acceptor.isConnected()) m_acceptor.open();

      /* Start the update timer */
      m_board[0].setTimer((long)1,0,1,0); // update every second
    }
    break;

    case F_ACCEPT_REQ:
    {
      GCFTCPPort* client = new GCFTCPPort();
      client->init(*this, "client", GCFPortInterface::SPP, RSP_PROTOCOL);
      m_acceptor.accept(*client);
      m_client_list.push_back(client);
    }
    break;

    case F_CONNECTED:
    {
      LOG_DEBUG(formatString("port '%s' connected", port.getName().c_str()));
    }
    break;

    case RSP_SETWEIGHTS:
    {
      /**
       * Create the command for this event.
       * Enter it in the scheduler queue.
       * Acknowledge the command.
       */
      SetWeightsCmd* command = new SetWeightsCmd(event, port, Command::WRITE);
      (void)m_scheduler.enter(command);
      command->ack(Cache::getInstance().getFront());
    }
    break;

    case RSP_GETWEIGHTS:
    {
      GetWeightsCmd* command = new GetWeightsCmd(event, port, Command::READ);

      // if null timestamp get value from the cache and acknowledge immediately
      if (Timestamp(0,0) == command->getTimestamp())
      {
	command->setTimestamp(m_scheduler.getCurrentTime() + -1);
	command->ack(Cache::getInstance().getFront());
      }
      else
      {
	(void)m_scheduler.enter(command);
      }
    }
    break;

    case F_TIMER:
    {
      GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&event);
      LOG_DEBUG(formatString("timer=(%d,%d)", timer->sec, timer->usec));

      if (&port == &m_board[0] || &port == &m_board[1])
      {
	/* run the scheduler */
	status = m_scheduler.run(event,port);
      }
#if 0
      else
      {
	m_scheduler.dispatch(event, port);
      }
#endif
    }
    break;

    case F_DISCONNECTED:
    {
      LOG_DEBUG(formatString("port %s disconnected", port.getName().c_str()));
      port.close();

      if (&port == &m_board[0] || &port == &m_board[1])
      {
	m_acceptor.close();
	TRAN(RSPDriverTask::initial);
      }
      else
      {
	m_client_list.remove(&port);
	m_garbage_list.push_back(&port);
      }
    }
    break;

    case F_EXIT:
    {
      // cancel timers
      m_acceptor.cancelAllTimers();
      m_board[0].cancelAllTimers();
    }
    break;

    default:
      if (&port == &m_board[0] || &port == &m_board[1])
      {
	status = m_scheduler.dispatch(event, port);
      }
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
