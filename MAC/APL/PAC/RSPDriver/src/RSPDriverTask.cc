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

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#define SYNC_INTERVAL 10

using namespace RSP;
using namespace std;
using namespace LOFAR;

RSPDriverTask::RSPDriverTask(string name)
    : GCFTask((State)&RSPDriverTask::initial, name)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  m_client.init(*this, "client", GCFPortInterface::SPP, RSP_PROTOCOL);
  m_board.init(*this, "board", GCFPortInterface::SAP, EPA_PROTOCOL /*, true*/);
}

RSPDriverTask::~RSPDriverTask()
{
}

bool RSPDriverTask::isEnabled()
{
  return m_client.isConnected() && m_board.isConnected();
}

GCFEvent::TResult RSPDriverTask::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch(e.signal)
  {
      case F_INIT:
      {
      }
      break;

      case F_ENTRY:
      {
	  if (!m_client.isConnected()) m_client.open(); // need this otherwise GTM_Sockethandler is not called
	  //m_board.setAddr("eth0", "aa:bb:cc:dd:ee:ff");
	  if (!m_board.isConnected()) m_board.open();
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
	m_client.cancelAllTimers();
	m_board.cancelAllTimers();
      }
      break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }

  return status;
}

GCFEvent::TResult RSPDriverTask::enabled(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  static int period = 0;

  switch (e.signal)
    {
#if 0
    case F_ACCEPT_REQ:
      m_client.getPortProvider().accept();
      break;
#endif
    case F_ENTRY:
      {
      }
      break;

    case F_TIMER:
      {
	GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&e);

	LOG_DEBUG(formatString("timer=(%d,%d)", timer->sec, timer->usec));

	if (0 == (period % SYNC_INTERVAL))
	{
	    period = 0;

	    // perform sync
	}

	period++;
      }
      break;

    case F_DATAIN:
      LOG_DEBUG("F_DATAIN");
      break;

    case F_DATAOUT:
      LOG_DEBUG("F_DATAOUT");
      break;

    case F_DISCONNECTED:
      {
	LOG_DEBUG(formatString("port %s disconnected", port.getName().c_str()));
	port.close();

	TRAN(RSPDriverTask::initial);
      }
      break;

    case F_EXIT:
      {
	// cancel timers
	m_client.cancelAllTimers();
	m_board.cancelAllTimers();
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
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
