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

#include "RSP_Protocol.ph"
#include "EPA_Protocol.ph"

#include "RSPDriverTask.h"
#include "RSPConfig.h"
#include "Command.h"
#include "SetWeightsCmd.h"
#include "GetWeightsCmd.h"
#include "SetSubbandsCmd.h"
#include "GetSubbandsCmd.h"
#include "GetStatusCmd.h"
#include "BWWrite.h"
#include "BWRead.h"
#include "SSWrite.h"
#include "SSRead.h"
#include "RCUWrite.h"
#include "RCURead.h"
#include "StatusRead.h"
#include "StatsRead.h"
#include "WGWrite.h"
#include "WGRead.h"
#include "VersionsRead.h"
#include "WriteReg.h"
#include "Cache.h"
#include "RawEvent.h"
#include "MEPHeader.h"
#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#define ETHERTYPE_EPA 0x10FA

using namespace RSP;
using namespace std;
using namespace LOFAR;
using namespace blitz;

RSPDriverTask::RSPDriverTask(string name)
  : GCFTask((State)&RSPDriverTask::initial, name), m_board(0), m_scheduler()
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
  registerProtocol(EPA_PROTOCOL, EPA_PROTOCOL_signalnames);

  m_clock.init(*this, "spid", GCFPortInterface::SPP, 0 /*don't care*/, true /*raw*/);

  m_acceptor.init(*this, "acceptor", GCFPortInterface::MSPP, RSP_PROTOCOL);

  m_board = new GCFETHRawPort[GET_CONFIG("N_RSPBOARDS", i)];

  for (int boardid = 0; boardid < GET_CONFIG("N_RSPBOARDS", i); boardid++)
  {
    char name[64] = "board";
    char macaddrstr[64] = "";
      
    snprintf(name, 64, "board%d", boardid);
    snprintf(macaddrstr, 64, "00:00:00:00:00:%02x", boardid + 1);

    m_board[boardid].init(*this, name, GCFPortInterface::SAP, EPA_PROTOCOL,true /*raw*/);
    m_board[boardid].setAddr(GET_CONFIG_STRING("IF_NAME"), macaddrstr);

    // set ethertype to 0x10FA so Ethereal can decode EPA messages
    m_board[boardid].setEtherType(ETHERTYPE_EPA);
  }

  addAllSyncActions();
}


RSPDriverTask::~RSPDriverTask()
{
  delete [] m_board;
}

bool RSPDriverTask::isEnabled()
{
  bool enabled = true;
  for (int boardid = 0; boardid < GET_CONFIG("N_RSPBOARDS", i); boardid++)
  {
    if (!m_board[boardid].isConnected())
    {
      enabled = false;
      break;
    }
  }

  enabled = enabled && m_clock.isConnected();
  
  return enabled;
}

/**
 * Add all synchronization actions per board.
 * Order is:
 * - BF:     write beamformer weights          // BWWrite
 * - SS:     write subband selection settings  // SSWrite
 * - RCU:    write RCU control settings        // RCUWrite
 * - STATUS (RSP Status): read RSP status info // StatusRead
 * - ST:     read statistics                   // StatsRead
 * - WG:     write waveform generator settings // WGWrite
 * - STATUS (Version): read version info       // VersionsSync
 *
 * For testing purposes, read back register that have just been written
 * - BF:  read beamformer weights          // BWRead
 * - SS:  read subbands selection settings // SSRead
 * - RCU: read RCU control settings        // RCURead
 * - WG:  read waveform generator settings // WGRead
 */
void RSPDriverTask::addAllSyncActions()
{
  /**
   * For each board a separate BWSync instance is created which handles
   * the synchronization of data between the board and the cache for that board.
   */
  for (int boardid = 0; boardid < GET_CONFIG("N_RSPBOARDS", i); boardid++)
  {
    if (1 == GET_CONFIG("WRITE_BF", i))
    {
      BWWrite* bwsync = 0;

      bwsync = new BWWrite(m_board[boardid], boardid, MEPHeader::BFXRE);
      m_scheduler.addSyncAction(bwsync);
      bwsync = new BWWrite(m_board[boardid], boardid, MEPHeader::BFXIM);
      m_scheduler.addSyncAction(bwsync);
      bwsync = new BWWrite(m_board[boardid], boardid, MEPHeader::BFYRE);
      m_scheduler.addSyncAction(bwsync);
      bwsync = new BWWrite(m_board[boardid], boardid, MEPHeader::BFYIM);
      m_scheduler.addSyncAction(bwsync);
    }

    if (1 == GET_CONFIG("WRITE_SS", i))
    {
      SSWrite* sswrite = new SSWrite(m_board[boardid], boardid);
      m_scheduler.addSyncAction(sswrite);
    }
    
    if (1 == GET_CONFIG("WRITE_RCU", i))
    {
      RCUWrite* rcuwrite = new RCUWrite(m_board[boardid], boardid);
      m_scheduler.addSyncAction(rcuwrite);
    }
    
    if (1 == GET_CONFIG("READ_STATUS", i))
    {
      StatusRead* statusread = new StatusRead(m_board[boardid], boardid);
      m_scheduler.addSyncAction(statusread);
    }
    
    if (1 == GET_CONFIG("READ_ST", i))
    {
      StatsRead* statsread = new StatsRead(m_board[boardid], boardid);
      m_scheduler.addSyncAction(statsread);
    }

#if 0    
    if (1 == GET_CONFIG("WRITE_WG", i))
    {
      WriteReg* writereg = new WriteReg(m_board[boardid], boardid,
					MEPHeader::DST_BLPS,
					MEPHeader::WG,
					MEPHeader::WGSETTINGS,
					MEPHeader::WGSETTINGS_SIZE);
      writereg->setSrcAddress(&(Cache::getInstance().getBack().getWGSettings()()(0)));
      m_scheduler.addSyncAction(writereg);
    }
#else
    if (1 == GET_CONFIG("WRITE_WG", i))
    {
      WGWrite* wgwrite = new WGWrite(m_board[boardid], boardid);
      m_scheduler.addSyncAction(wgwrite);
    }
#endif

    if (1 == GET_CONFIG("READ_VERSION", i))
    {
      VersionsRead* versionread = new VersionsRead(m_board[boardid], boardid);
      m_scheduler.addSyncAction(versionread);
    }

    if (1 == GET_CONFIG("READ_BF", i))
    {
      BWRead* bwsync = 0;

      bwsync = new BWRead(m_board[boardid], boardid, MEPHeader::BFXRE);
      m_scheduler.addSyncAction(bwsync);
      bwsync = new BWRead(m_board[boardid], boardid, MEPHeader::BFXIM);
      m_scheduler.addSyncAction(bwsync);
      bwsync = new BWRead(m_board[boardid], boardid, MEPHeader::BFYRE);
      m_scheduler.addSyncAction(bwsync);
      bwsync = new BWRead(m_board[boardid], boardid, MEPHeader::BFYIM);
      m_scheduler.addSyncAction(bwsync);
    }

    if (1 == GET_CONFIG("READ_SS", i))
    {
      SSRead* ssread = new SSRead(m_board[boardid], boardid);
      m_scheduler.addSyncAction(ssread);
    }

    if (1 == GET_CONFIG("READ_RCU", i))
    {
      RCURead* rcuread = new RCURead(m_board[boardid], boardid);
      m_scheduler.addSyncAction(rcuread);
    }
    if (1 == GET_CONFIG("READ_WG", i))
    {
      WGRead* wgread = new WGRead(m_board[boardid], boardid);
      m_scheduler.addSyncAction(wgread);
    }
  }
}

void RSPDriverTask::openBoards()
{
  for (int boardid = 0; boardid < GET_CONFIG("N_RSPBOARDS", i); boardid++)
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
      if (!m_clock.isConnected()) m_clock.open();
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
      if (&port == &m_clock)
      {
	/**
	 * We don't need the clock here yet, simply read the value
	 * and ignore
	 */
	uint8 count = 0;
	(void)port.recv(&count, sizeof(uint8));
      }
      else
      {
	// ignore in this state
	static char buf[ETH_DATA_LEN];
	(void)port.recv(buf, ETH_DATA_LEN);
      }
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
  for (list<GCFPortInterface*>::iterator it = m_garbage_list.begin();
       it != m_garbage_list.end();
       it++)
  {
    delete (*it);
  }
  m_garbage_list.clear();
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

      if (1 == GET_CONFIG("SW_CLOCK", i))
      {
	/* Start the update timer after 1 second */
	m_board[0].setTimer(1.0,
			    GET_CONFIG("SYNC_INTERVAL", f)); // update SYNC_INTERVAL seconds
      }
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

    case F_DATAIN:
    {
      if (&port == &m_clock)
      {
	status = clock_tick(port);
      }
      else
      {
	status = RawEvent::dispatch(*this, port);
      }
    }
    break;
    
    case RSP_SETWEIGHTS:
      rsp_setweights(event, port);
      break;

    case RSP_GETWEIGHTS:
      rsp_getweights(event, port);
      break;

    case RSP_SETSUBBANDS:
      rsp_setsubbands(event, port);
      break;

    case RSP_SETRCU:
      rsp_setrcu(event, port);
      break;

    case RSP_GETRCU:
      rsp_getrcu(event, port);
      break;
      
    case RSP_SETWG:
      rsp_setwg(event, port);
      break;
      
    case RSP_GETWG:
      rsp_getwg(event, port);
      break;
      
    case RSP_SUBSTATUS:
      rsp_substatus(event, port);
      break;
      
    case RSP_UNSUBSTATUS:
      rsp_unsubstatus(event, port);
      break;

    case RSP_GETSTATUS:
      rsp_getstatus(event, port);
      break;
      
    case RSP_SUBSTATS:
      rsp_substats(event, port);
      break;
      
    case RSP_UNSUBSTATS:
      rsp_unsubstats(event, port);
      break;
      
    case RSP_GETSTATS:
      rsp_getstats(event, port);
      break;

    case RSP_GETVERSION:
      rsp_getversions(event, port);
      break;

    case F_TIMER:
    {
      if (&port == &m_board[0])
      {
	if (1 == GET_CONFIG("SW_CLOCK", i))
	{
	  /**
	   * Trigger a clock signal by sending
	   * an 's' character on the clock port.
	   */
	  EPATriggerClockEvent trigger;
	  trigger.value = 's';
	  m_clock.send(trigger);
	}
      }
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
	/* cancel all commands for this port */
	m_scheduler.cancel(port);

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
      if (isBoardPort(port))
      {
	status = m_scheduler.dispatch(event, port);
      }
      break;
  }

  return status;
}

bool RSPDriverTask::isBoardPort(GCFPortInterface& port)
{
#if 1
  if (   &port >= &m_board[0]
      && &port <= &m_board[GET_CONFIG("N_RSPBOARDS", i)])
    return true;
#else
  for (int i = 0; i < GET_CONFIG("N_RSPBOARDS", i); i++)
  {
    if (&port == &m_board[i]) return true;
  }
#endif
  
  return false;
}

GCFEvent::TResult RSPDriverTask::clock_tick(GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  uint8 count = 0;

  if (1 == GET_CONFIG("SOFTPPS", i))
  {
    // Send SoftPPS signal to all boards
    EPAWgsoftppsEvent softpps;
    MEP_WGSOFTPPS(softpps.hdr);

    for (int i = 0; i < GET_CONFIG("N_RSPBOARDS", i); i++)
    {
      m_board[i].send(softpps);
    }
  }
	
  if (port.recv(&count, sizeof(uint8)) != 1)
  {
    LOG_FATAL("We got a signal, but there is no clock pulse!");
    exit(EXIT_FAILURE);
  }
  else
  {
    count -= '0'; // convert to integer
    if (count > 1)
    {
      LOG_WARN("Got more than one clock pulse: missed real-time deadline");
    }
	  
    struct timeval now;
    (void)gettimeofday(&now, 0);

    // print time, ugly
    char timestr[32];
    strftime(timestr, 32, "%T", localtime(&now.tv_sec));
    LOG_INFO(formatString("time=%s.%d", timestr, now.tv_usec));

    /* construct a timer event */
    GCFTimerEvent timer;
    timer.sec  = now.tv_sec;
    timer.usec = now.tv_usec;
    timer.id   = 0;
    timer.arg  = 0;
	  
    /* run the scheduler with the timer event */
    status = m_scheduler.run(timer, port);
  }

  return status;
}

void RSPDriverTask::rsp_setweights(GCFEvent& event, GCFPortInterface& port)
{
  /**
   * Create a separate command for each timestep for which
   * weights are contained in the event.
   */

  /* unpack the event */
  RSPSetweightsEvent* sw_event = new RSPSetweightsEvent(event);

  /* range check on parameters */
  if ((sw_event->weights().dimensions() != BeamletWeights::NDIM)
      || (sw_event->weights().extent(thirdDim) != MAX_N_BEAMLETS)
      || (sw_event->weights().extent(secondDim) > GET_CONFIG("N_RCU", i)))
  {
    delete sw_event;
    
    RSPSetweightsackEvent ack;
    ack.timestamp = Timestamp(0,0);
    ack.status = FAILURE;
    port.send(ack);
    return;
  }

  for (int timestep = 0; timestep < sw_event->weights().extent(firstDim); timestep++)
  {
    SetWeightsCmd* command = new SetWeightsCmd(*sw_event, port, Command::WRITE, timestep);

    if (0 == timestep)
    {
      command->ack(Cache::getInstance().getFront());
    }
	
    command->setWeights(sw_event->weights()(Range(timestep, timestep),
					    Range::all(),
					    Range::all()));
	
    (void)m_scheduler.enter(command);
  }

  /* cleanup the event */
  delete sw_event;
}

void RSPDriverTask::rsp_getweights(GCFEvent& event, GCFPortInterface& port)
{
  GetWeightsCmd* command = new GetWeightsCmd(event, port, Command::READ);

  if (!command->validate())
  {
    delete command;
    
    RSPGetweightsackEvent ack;
    ack.timestamp = Timestamp(0,0);
    ack.status = FAILURE;
    port.send(ack);
    return;
  }

  // if null timestamp get value from the cache and acknowledge immediately
  if ( (Timestamp(0,0) == command->getTimestamp())
       && (true == command->readFromCache()))
  {
    command->setTimestamp(Cache::getInstance().getFront().getTimestamp());
    command->ack(Cache::getInstance().getFront());
    delete command;
  }
  else
  {
    (void)m_scheduler.enter(command);
  }
}


void RSPDriverTask::rsp_setsubbands(GCFEvent& event, GCFPortInterface& port)
{
  SetSubbandsCmd* command = new SetSubbandsCmd(event, port, Command::WRITE);

  if (!command->validate())
  {
    delete command;
    
    RSPSetsubbandsackEvent ack;
    ack.timestamp = Timestamp(0,0);
    ack.status = FAILURE;
    port.send(ack);
    return;
  }

  (void)m_scheduler.enter(command);
  command->ack(Cache::getInstance().getFront());
}

void RSPDriverTask::rsp_getsubbands(GCFEvent& event, GCFPortInterface& port)
{
  GetSubbandsCmd* command = new GetSubbandsCmd(event, port, Command::READ);

  if (!command->validate())
  {
    delete command;
    
    RSPGetsubbandsackEvent ack;
    ack.timestamp = Timestamp(0,0);
    ack.status = FAILURE;
    port.send(ack);
    return;
  }
  
  // if null timestamp get value from the cache and acknowledge immediately
  if ( (Timestamp(0,0) == command->getTimestamp())
       && (true == command->readFromCache()))
  {
    command->setTimestamp(Cache::getInstance().getFront().getTimestamp());
    command->ack(Cache::getInstance().getFront());
    delete command;
  }
  else
  {
    (void)m_scheduler.enter(command);
  }
}

void RSPDriverTask::rsp_setrcu(GCFEvent& /*event*/, GCFPortInterface& /*port*/)
{
  /* not implemented yet, ignore event */
}

void RSPDriverTask::rsp_getrcu(GCFEvent& /*event*/, GCFPortInterface& /*port*/)
{
  /* not implemented yet, ignore event */
}

void RSPDriverTask::rsp_setwg(GCFEvent& /*event*/, GCFPortInterface& /*port*/)
{
  /* not implemented yet, ignore event */
}

void RSPDriverTask::rsp_getwg(GCFEvent& /*event*/, GCFPortInterface& /*port*/)
{
  /* not implemented yet, ignore event */
}

void RSPDriverTask::rsp_substatus(GCFEvent& /*event*/, GCFPortInterface& /*port*/)
{
  /* not implemented yet, ignore event */
}

void RSPDriverTask::rsp_unsubstatus(GCFEvent& /*event*/, GCFPortInterface& /*port*/)
{
  /* not implemented yet, ignore event */
}

void RSPDriverTask::rsp_getstatus(GCFEvent& event, GCFPortInterface& port)
{
  GetStatusCmd* command = new GetStatusCmd(event, port, Command::READ);

  if (!command->validate())
  {
    command->ack_fail();
    delete command;
    return;
  }

  // if null timestamp get value from the cache and acknowledge immediately
  if (Timestamp(0,0) == command->getTimestamp())
  {
    command->setTimestamp(Cache::getInstance().getFront().getTimestamp());
    command->ack(Cache::getInstance().getFront());

    delete command;
  }
  else
  {
    (void)m_scheduler.enter(command);
  }
}

void RSPDriverTask::rsp_substats(GCFEvent& /*event*/, GCFPortInterface& /*port*/)
{
  /* not implemented yet, ignore event */
}

void RSPDriverTask::rsp_unsubstats(GCFEvent& /*event*/, GCFPortInterface& /*port*/)
{
  /* not implemented yet, ignore event */
}

void RSPDriverTask::rsp_getstats(GCFEvent& /*event*/, GCFPortInterface& /*port*/)
{
  /* not implemented yet, ignore event */
}

void RSPDriverTask::rsp_getversions(GCFEvent& /*event*/, GCFPortInterface& /*port*/)
{
  /* not implemented yet, ignore event */
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

  RSPConfig::getInstance().load("./rsp.cfg");

  RSPDriverTask rsp("RSP");

  rsp.start(); // make initial transition

  GCFTask::run();

  LOG_INFO("Normal termination of program");

  return 0;
}
