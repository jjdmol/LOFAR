//#
//#  RSPDriver.cc: implementation of RSPDriver class
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

#include "RSPDriver.h"
#include "Command.h"
#include "SetWeightsCmd.h"
#include "GetWeightsCmd.h"
#include "SetSubbandsCmd.h"
#include "GetSubbandsCmd.h"
#include "GetStatusCmd.h"
#include "UpdStatusCmd.h"
#include "SetRCUCmd.h"
#include "GetRCUCmd.h"
#include "SetWGCmd.h"
#include "GetWGCmd.h"
#include "GetVersionsCmd.h"
#include "GetStatsCmd.h"
#include "UpdStatsCmd.h"

#include "BWWrite.h"
#include "BWRead.h"
#include "SSWrite.h"
#include "SSRead.h"
#include "RCUWrite.h"
#include "RCURead.h"
#include "StatusRead.h"
#include "SstRead.h"
#include "BstRead.h"
#include "WGWrite.h"
#include "WGRead.h"
#include "VersionsRead.h"
#include "WriteReg.h"

#include "Cache.h"
#include "RawEvent.h"
#include "MEPHeader.h"

#include "netraw.h"

#include <PSAccess.h>
#include <GCF/ParameterSet.h>

#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#ifndef RSP_SYSCONF
#define RSP_SYSCONF "."
#endif

#define ETHERTYPE_EPA 0x10FA

using namespace RSP;
using namespace std;
using namespace LOFAR;
using namespace blitz;

RSPDriver::RSPDriver(string name)
  : GCFTask((State)&RSPDriver::initial, name), m_board(0), m_scheduler()
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);
  registerProtocol(EPA_PROTOCOL, EPA_PROTOCOL_signalnames);

  m_clock.init(*this, "spid", GCFPortInterface::SAP, 0 /*don't care*/, true /*raw*/);

  m_acceptor.init(*this, "acceptor", GCFPortInterface::MSPP, RSP_PROTOCOL);

  m_board = new GCFETHRawPort[GET_CONFIG("RS.N_RSPBOARDS", i)];

  //
  // Attempt access of RSPDriver.MAC_BASE, if it fails use the RSPDriver.ADDR0
  // parameters.
  //
  bool bUseMAC_BASE = true;
  try         { (void)GET_CONFIG("RSPDriver.MAC_BASE",i); }
  catch (...) { bUseMAC_BASE = false;                     }

  char boardname[64];
  char paramname[64];
  char macaddrstr[64];
  for (int boardid = 0; boardid < GET_CONFIG("RS.N_RSPBOARDS", i); boardid++)
  {
    snprintf(boardname, 64, "board%d", boardid);

    if (bUseMAC_BASE)
    {
      snprintf(macaddrstr, 64, "00:00:00:00:00:%02x", boardid + GET_CONFIG("RSPDriver.MAC_BASE", i));
    }
    else
    {
      snprintf(paramname, 64, "RSPDriver.MAC_ADDR_%d", boardid);
      strncpy(macaddrstr, GET_CONFIG_STRING(paramname), 64);
    }

    m_board[boardid].init(*this, boardname, GCFPortInterface::SAP, EPA_PROTOCOL,true /*raw*/);
    m_board[boardid].setAddr(GET_CONFIG_STRING("RSPDriver.IF_NAME"), macaddrstr);

    // set ethertype to 0x10FA so Ethereal can decode EPA messages
    m_board[boardid].setEtherType(ETHERTYPE_EPA);
  }

  addAllSyncActions();
}


RSPDriver::~RSPDriver()
{
  delete [] m_board;
}

bool RSPDriver::isEnabled()
{
  bool enabled = true;
  for (int boardid = 0; boardid < GET_CONFIG("RS.N_RSPBOARDS", i); boardid++)
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
 * - SST:    read subband statistics           // SstRead
 * - BST:    read beamlet statistics           // BstRead
 * - WG:     write waveform generator settings // WGWrite
 * - STATUS (Version): read version info       // VersionsSync
 *
 * For testing purposes, read back register that have just been written
 * - BF:  read beamformer weights          // BWRead
 * - SS:  read subbands selection settings // SSRead
 * - RCU: read RCU control settings        // RCURead
 * - WG:  read waveform generator settings // WGRead
 */
void RSPDriver::addAllSyncActions()
{
  /**
   * For each board a separate BWSync instance is created which handles
   * the synchronization of data between the board and the cache for that board.
   */
  for (int boardid = 0; boardid < GET_CONFIG("RS.N_RSPBOARDS", i); boardid++)
  {
    if (1 == GET_CONFIG("RSPDriver.READ_STATUS", i))
    {
      StatusRead* statusread = new StatusRead(m_board[boardid], boardid);
      m_scheduler.addSyncAction(statusread);
    }

    //
    // Depending on the value of RSPDriver.LOOPBACK_MODE either the 
    // WRITE is done first or the READ is done first.
    //
    // If LOOPBACK_MODE == 0, the WRITE is done first.
    // In this mode you can check with Ethereal that what was
    // written is correctly read back from the board. This can
    // be used to check that the RSP hardware or the EPAStub
    // function correctly.
    //
    // If LOOPBACK_MODE == 1, the READ is done first.
    // In this mode you can check with Ethereal that what was
    // read from the EPAStub is written back in the same way.
    // This is used to check whether the RSPDriver stores the
    // information at the correct location in its cache.
    //
    // This is done in the same way for all read/write registers.
    //
    for (int action = 0; action < 2; action++)
    {
      if (action == GET_CONFIG("RSPDriver.LOOPBACK_MODE", i))
      {
	if (1 == GET_CONFIG("RSPDriver.WRITE_BF", i))
	{
	  BWWrite* bwsync = 0;

	  bwsync = new BWWrite(m_board[boardid], boardid, MEPHeader::BF_XROUT);
	  m_scheduler.addSyncAction(bwsync);
	  bwsync = new BWWrite(m_board[boardid], boardid, MEPHeader::BF_XIOUT);
	  m_scheduler.addSyncAction(bwsync);
	  bwsync = new BWWrite(m_board[boardid], boardid, MEPHeader::BF_YROUT);
	  m_scheduler.addSyncAction(bwsync);
	  bwsync = new BWWrite(m_board[boardid], boardid, MEPHeader::BF_YIOUT);
	  m_scheduler.addSyncAction(bwsync);
	}
      }
      else
      {
	if (1 == GET_CONFIG("RSPDriver.READ_BF", i))
	{
	  BWRead* bwsync = 0;

	  bwsync = new BWRead(m_board[boardid], boardid, MEPHeader::BF_XROUT);
	  m_scheduler.addSyncAction(bwsync);
	  bwsync = new BWRead(m_board[boardid], boardid, MEPHeader::BF_XIOUT);
	  m_scheduler.addSyncAction(bwsync);
	  bwsync = new BWRead(m_board[boardid], boardid, MEPHeader::BF_YROUT);
	  m_scheduler.addSyncAction(bwsync);
	  bwsync = new BWRead(m_board[boardid], boardid, MEPHeader::BF_YIOUT);
	  m_scheduler.addSyncAction(bwsync);
	}
      }
    }
    
    for (int action = 0; action < 2; action++)
    {
      if (action == GET_CONFIG("RSPDriver.LOOPBACK_MODE", i))
      {
	if (1 == GET_CONFIG("RSPDriver.WRITE_SS", i))
	{
	  SSWrite* sswrite = new SSWrite(m_board[boardid], boardid);
	  m_scheduler.addSyncAction(sswrite);
	}
      }
      else
      {
	if (1 == GET_CONFIG("RSPDriver.READ_SS", i))
	{
	  SSRead* ssread = new SSRead(m_board[boardid], boardid);
	  m_scheduler.addSyncAction(ssread);
	}
      }
    }
    
    for (int action = 0; action < 2; action++)
    {
      if (action == GET_CONFIG("RSPDriver.LOOPBACK_MODE", i))
      {
	if (1 == GET_CONFIG("RSPDriver.WRITE_RCU", i))
	{
	  RCUWrite* rcuwrite = new RCUWrite(m_board[boardid], boardid);
	  m_scheduler.addSyncAction(rcuwrite);
	}
      }
      else
      {
	if (1 == GET_CONFIG("RSPDriver.READ_RCU", i))
	{
	  RCURead* rcuread = new RCURead(m_board[boardid], boardid);
	  m_scheduler.addSyncAction(rcuread);
	}
      }
    }

    if (1 == GET_CONFIG("RSPDriver.READ_SST", i))
    {
      SstRead* sstread = 0;

      sstread = new SstRead(m_board[boardid], boardid, Statistics::SUBBAND_MEAN);
      m_scheduler.addSyncAction(sstread);
      sstread = new SstRead(m_board[boardid], boardid, Statistics::SUBBAND_POWER);
      m_scheduler.addSyncAction(sstread);

    }

    if (1 == GET_CONFIG("RSPDriver.READ_BST", i))
    {
      BstRead* bstread = 0;

      bstread = new BstRead(m_board[boardid], boardid, Statistics::BEAMLET_MEAN);
      m_scheduler.addSyncAction(bstread);
      bstread = new BstRead(m_board[boardid], boardid, Statistics::BEAMLET_POWER);
      m_scheduler.addSyncAction(bstread);
    }

    for (int action = 0; action < 2; action++)
    {
      if (action == GET_CONFIG("RSPDriver.LOOPBACK_MODE", i))
      {
	if (1 == GET_CONFIG("RSPDriver.WRITE_WG", i))
	{
	  WGWrite* wgwrite = new WGWrite(m_board[boardid], boardid);
	  m_scheduler.addSyncAction(wgwrite);
	}
      }
      else
      {
	if (1 == GET_CONFIG("RSPDriver.READ_WG", i))
	{
	  WGRead* wgread = new WGRead(m_board[boardid], boardid);
	  m_scheduler.addSyncAction(wgread);
	}
      }
    }

    if (1 == GET_CONFIG("RSPDriver.READ_VERSION", i))
    {
      VersionsRead* versionread = new VersionsRead(m_board[boardid], boardid);
      m_scheduler.addSyncAction(versionread);
    }
  }

//
// Example of the use of WriteReg to write waveform generator settings.
//
//     if (1 == GET_CONFIG("RSPDriver.WRITE_WG", i))
//     {
//       WriteReg* writereg = new WriteReg(m_board[boardid], boardid,
// 					MEPHeader::DST_BLPS,
// 					MEPHeader::WG,
// 					MEPHeader::WGSETTINGS,
// 					MEPHeader::WGSETTINGS_SIZE);
//       writereg->setSrcAddress(&(Cache::getInstance().getBack().getWGSettings()()(0)));
//       m_scheduler.addSyncAction(writereg);
//     }
}

void RSPDriver::openBoards()
{
  for (int boardid = 0; boardid < GET_CONFIG("RS.N_RSPBOARDS", i); boardid++)
  {
    if (!m_board[boardid].isConnected()) m_board[boardid].open();
  }
}

GCFEvent::TResult RSPDriver::initial(GCFEvent& event, GCFPortInterface& port)
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
      LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
      if (isEnabled())
      {
	TRAN(RSPDriver::enabled);
      }
    }
    break;

    case F_DISCONNECTED:
    {
      port.setTimer((long)3); // try again in 3 seconds
      LOG_DEBUG(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
      port.close();
    }
    break;

    case F_TIMER:
    {
      LOG_DEBUG(formatString("port '%s' retry of open...", port.getName().c_str()));
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

void RSPDriver::collect_garbage()
{
  for (list<GCFPortInterface*>::iterator it = m_garbage_list.begin();
       it != m_garbage_list.end();
       it++)
  {
    delete (*it);
  }
  m_garbage_list.clear();
}

GCFEvent::TResult RSPDriver::enabled(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  collect_garbage();

  switch (event.signal)
  {
    case F_ENTRY:
    {
      // start waiting for clients
      if (!m_acceptor.isConnected()) m_acceptor.open();

      if (1 == GET_CONFIG("RSPDriver.SW_SYNC", i))
      {
	/* Start the update timer after 1 second */
	m_board[0].setTimer(1.0,
			    GET_CONFIG("RSPDriver.SYNC_INTERVAL", f)); // update SYNC_INTERVAL seconds
      }
    }
    break;

    case F_ACCEPT_REQ:
    {
      GCFTCPPort* client = new GCFTCPPort();
      client->init(*this, "client", GCFPortInterface::SPP, RSP_PROTOCOL);
      m_acceptor.accept(*client);
      m_client_list.push_back(client);

      LOG_INFO(formatString("NEW CLIENT CONNECTED: %d clients connected", m_client_list.size()));
    }
    break;

    case F_CONNECTED:
    {
      LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
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

    case RSP_GETSUBBANDS:
      rsp_getsubbands(event, port);
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
	if (1 == GET_CONFIG("RSPDriver.SW_SYNC", i))
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
      LOG_INFO(formatString("DISCONNECTED: port '%s'", port.getName().c_str()));
      port.close();

      if (&port == &m_board[0] || &port == &m_board[1])
      {
	m_acceptor.close();
	TRAN(RSPDriver::initial);
      }
      else
      {
	/* cancel all commands for this port */
	(void)m_scheduler.cancel(port);

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

bool RSPDriver::isBoardPort(GCFPortInterface& port)
{
  /**
   * The addresses of the elements of the m_board array
   * are consecutive in memory, therefor we can do a range
   * check on the address to determine whether it is a port
   * to a board.
   */
  if (   &port >= &m_board[0]
      && &port <= &m_board[GET_CONFIG("RS.N_RSPBOARDS", i)])
    return true;
  
  return false;
}

GCFEvent::TResult RSPDriver::clock_tick(GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  uint8 count = 0;

  if (1 == GET_CONFIG("RSPDriver.SOFTPPS", i))
  {
    // Send SoftPPS signal to all boards
    EPACrrSoftppsEvent softpps;
    softpps.hdr.m_fields = MEPHeader::CRR_SOFTRESET_HDR;

    for (int i = 0; i < GET_CONFIG("RS.N_RSPBOARDS", i); i++)
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
    LOG_INFO(formatString("TICK: time=%s.%d", timestr, now.tv_usec));

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

void RSPDriver::rsp_setweights(GCFEvent& event, GCFPortInterface& port)
{
  /**
   * Create a separate command for each timestep for which
   * weights are contained in the event.
   */

  /* unpack the event */
  RSPSetweightsEvent* sw_event = new RSPSetweightsEvent(event);

  /* range check on parameters */
  if ((sw_event->weights().dimensions() != BeamletWeights::NDIM)
      || (sw_event->weights().extent(firstDim) < 1)
      || (sw_event->weights().extent(secondDim) > GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i))
      || (sw_event->weights().extent(thirdDim) != MEPHeader::N_BEAMLETS)
      || (sw_event->weights().extent(fourthDim) != MEPHeader::N_POL))
  {
    LOG_ERROR("SETWEIGHTS: invalid parameter");
    
    delete sw_event;
    
    RSPSetweightsackEvent ack;
    ack.timestamp = Timestamp(0,0);
    ack.status = FAILURE;
    port.send(ack);
    return;
  }

  for (int timestep = 0; timestep < sw_event->weights().extent(firstDim); timestep++)
  {
    Ptr<SetWeightsCmd> command = new SetWeightsCmd(*sw_event, port, Command::WRITE, timestep);

    if (0 == timestep)
    {
      command->ack(Cache::getInstance().getFront());
    }
	
    command->setWeights(sw_event->weights()(Range(timestep, timestep),
					    Range::all(),
					    Range::all()));
	
    (void)m_scheduler.enter(Ptr<Command>(&(*command)));
  }

  /* cleanup the event */
  delete sw_event;
}

void RSPDriver::rsp_getweights(GCFEvent& event, GCFPortInterface& port)
{
  Ptr<GetWeightsCmd> command = new GetWeightsCmd(event, port, Command::READ);

  if (!command->validate())
  {
    LOG_ERROR("GETWEIGHTS: invalid parameter");
    
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
  }
  else
  {
    (void)m_scheduler.enter(Ptr<Command>(&(*command)));
  }
}


void RSPDriver::rsp_setsubbands(GCFEvent& event, GCFPortInterface& port)
{
  Ptr<SetSubbandsCmd> command = new SetSubbandsCmd(event, port, Command::WRITE);

  if (!command->validate())
  {
    LOG_ERROR("SETSUBBANDS: invalid parameter");
    
    RSPSetsubbandsackEvent ack;
    ack.timestamp = Timestamp(0,0);
    ack.status = FAILURE;
    port.send(ack);
    return;
  }

  (void)m_scheduler.enter(Ptr<Command>(&(*command)));
  command->ack(Cache::getInstance().getFront());
}

void RSPDriver::rsp_getsubbands(GCFEvent& event, GCFPortInterface& port)
{
  Ptr<GetSubbandsCmd> command = new GetSubbandsCmd(event, port, Command::READ);

  if (!command->validate())
  {
    LOG_ERROR("GETSUBBANDS: invalid parameter");
    
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
  }
  else
  {
    (void)m_scheduler.enter(Ptr<Command>(&(*command)));
  }
}

void RSPDriver::rsp_setrcu(GCFEvent& event, GCFPortInterface& port)
{
  Ptr<SetRCUCmd> command = new SetRCUCmd(event, port, Command::WRITE);

  if (!command->validate())
  {
    LOG_ERROR("SETRCU: invalid parameter");
    
    RSPSetrcuackEvent ack;
    ack.timestamp = Timestamp(0,0);
    ack.status = FAILURE;
    port.send(ack);
    return;
  }

  (void)m_scheduler.enter(Ptr<Command>(&(*command)));
  command->ack(Cache::getInstance().getFront());
}

void RSPDriver::rsp_getrcu(GCFEvent& event, GCFPortInterface& port)
{
  Ptr<GetRCUCmd> command = new GetRCUCmd(event, port, Command::READ);

  if (!command->validate())
  {
    LOG_ERROR("GETRCU: invalid parameter");
    
    RSPGetrcuackEvent ack;
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
  }
  else
  {
    (void)m_scheduler.enter(Ptr<Command>(&(*command)));
  }
}

void RSPDriver::rsp_setwg(GCFEvent& event, GCFPortInterface& port)
{
  Ptr<SetWGCmd> command = new SetWGCmd(event, port, Command::WRITE);

  if (!command->validate())
  {
    LOG_ERROR("SETWG: invalid parameter");
    
    RSPSetwgackEvent ack;
    ack.timestamp = Timestamp(0,0);
    ack.status = FAILURE;
    port.send(ack);
    return;
  }

  (void)m_scheduler.enter(Ptr<Command>(&(*command)));
  command->ack(Cache::getInstance().getFront());
}

void RSPDriver::rsp_getwg(GCFEvent& event, GCFPortInterface& port)
{
  Ptr<GetWGCmd> command = new GetWGCmd(event, port, Command::READ);

  if (!command->validate())
  {
    LOG_ERROR("GETWG: invalid parameter");
    
    RSPGetwgackEvent ack;
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
  }
  else
  {
    (void)m_scheduler.enter(Ptr<Command>(&(*command)));
  }
}

void RSPDriver::rsp_substatus(GCFEvent& event, GCFPortInterface& port)
{
  // subscription is done by entering a UpdStatusCmd in the periodic queue
  Ptr<UpdStatusCmd> command = new UpdStatusCmd(event, port, Command::READ);
  RSPSubstatusackEvent ack;

  if (!command->validate())
  {
    LOG_ERROR("SUBSTATUS: invalid parameter");
    
    ack.timestamp = m_scheduler.getCurrentTime();
    ack.status = FAILURE;
    ack.handle = 0;

    port.send(ack);
    return;
  }
  else
  {
    ack.timestamp = m_scheduler.getCurrentTime();
    ack.status = SUCCESS;
    ack.handle = (uint32)&(*command);
    port.send(ack);
  }

  (void)m_scheduler.enter(Ptr<Command>(&(*command)),
			  Scheduler::PERIODIC);
}

void RSPDriver::rsp_unsubstatus(GCFEvent& event, GCFPortInterface& port)
{
  RSPUnsubstatusEvent unsub(event);

  RSPUnsubstatusackEvent ack;
  ack.timestamp = m_scheduler.getCurrentTime();
  ack.status = FAILURE;
  ack.handle = unsub.handle;

  if (m_scheduler.remove_subscription(port, unsub.handle) > 0)
  {
    ack.status = SUCCESS;
  }
  else
  {
    LOG_ERROR("UNSUBSTATUS: failed to remove subscription");
  }

  port.send(ack);
}

void RSPDriver::rsp_getstatus(GCFEvent& event, GCFPortInterface& port)
{
  Ptr<GetStatusCmd> command = new GetStatusCmd(event, port, Command::READ);

  if (!command->validate())
  {
    command->ack_fail();
    return;
  }

  // if null timestamp get value from the cache and acknowledge immediately
  if ((Timestamp(0,0) == command->getTimestamp())
      && (true == command->readFromCache()))
  {
    command->setTimestamp(Cache::getInstance().getFront().getTimestamp());
    command->ack(Cache::getInstance().getFront());
  }
  else
  {
    (void)m_scheduler.enter(Ptr<Command>(&(*command)));
  }
}

void RSPDriver::rsp_substats(GCFEvent& event, GCFPortInterface& port)
{
  // subscription is done by entering a UpdStatsCmd in the periodic queue
  Ptr<UpdStatsCmd> command = new UpdStatsCmd(event, port, Command::READ);
  RSPSubstatsackEvent ack;

  if (!command->validate())
  {
    LOG_ERROR("SUBSTATS: invalid parameter");
    
    ack.timestamp = m_scheduler.getCurrentTime();
    ack.status = FAILURE;
    ack.handle = 0;

    port.send(ack);
    return;
  }
  else
  {
    ack.timestamp = m_scheduler.getCurrentTime();
    ack.status = SUCCESS;
    ack.handle = (uint32)&(*command);
    port.send(ack);
  }

  (void)m_scheduler.enter(Ptr<Command>(&(*command)),
			  Scheduler::PERIODIC);
}

void RSPDriver::rsp_unsubstats(GCFEvent& event, GCFPortInterface& port)
{
  RSPUnsubstatsEvent unsub(event);

  RSPUnsubstatsackEvent ack;
  ack.timestamp = m_scheduler.getCurrentTime();
  ack.status = FAILURE;
  ack.handle = unsub.handle;

  if (m_scheduler.remove_subscription(port, unsub.handle) > 0)
  {
    ack.status = SUCCESS;
  }
  else
  {
    LOG_ERROR("UNSUBSTATS: failed to remove subscription");
  }

  port.send(ack);
}

void RSPDriver::rsp_getstats(GCFEvent& event, GCFPortInterface& port)
{
  Ptr<GetStatsCmd> command = new GetStatsCmd(event, port, Command::READ);

  if (!command->validate())
  {
    command->ack_fail();
    return;
  }

  // if null timestamp get value from the cache and acknowledge immediately
  if ((Timestamp(0,0) == command->getTimestamp())
      && (true == command->readFromCache()))
  {
    command->setTimestamp(Cache::getInstance().getFront().getTimestamp());
    command->ack(Cache::getInstance().getFront());
  }
  else
  {
    (void)m_scheduler.enter(Ptr<Command>(&(*command)));
  }
}

void RSPDriver::rsp_getversions(GCFEvent& event, GCFPortInterface& port)
{
  Ptr<GetVersionsCmd> command = new GetVersionsCmd(event, port, Command::READ);

  if (!command->validate())
  {
    LOG_ERROR("GETVERSIONS: invalid parameter");
    
    RSPGetversionackEvent ack;
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
  }
  else
  {
    (void)m_scheduler.enter(Ptr<Command>(&(*command)));
  }
}

int main(int argc, char** argv)
{
#if defined(ENABLE_CAP_NET_RAW)
  //
  // Need to run as (setuid) root (geteuid()==0), but will limit
  // capabilities to cap_net_raw (and cap_net_admin only)
  // and setuid immediately.
  //
  if (!enable_cap_net_raw())
  {
    fprintf(stderr, "%s: error: failed to enable CAP_NET_RAW capability.",argv[0]);
    exit(EXIT_FAILURE);
  }
#endif
  
  GCFTask::init(argc, argv);
  
  LOG_INFO(formatString("Program %s has started", argv[0]));

  try
  {
    GCF::ParameterSet::instance()->adoptFile("RSPDriverPorts.conf");
    GCF::ParameterSet::instance()->adoptFile("RemoteStation.conf");
    GCF::ParameterSet::instance()->adoptFile("RSPDriver.conf");
  }
  catch (Exception e)
  {
    cerr << "Failed to load configuration files: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  RSPDriver rsp("RSPDriver");

  rsp.start(); // make initial transition

  try
  {
    GCFTask::run();
  }
  catch (Exception e)
  {
    cerr << "Exception: " << e.text();
    exit(EXIT_FAILURE);
  }

  LOG_INFO("Normal termination of program");

  return 0;
}
