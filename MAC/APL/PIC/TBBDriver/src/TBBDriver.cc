//#  TBBDriver.cc: one line description
//# 
//#  Copyright (C) 2006
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <APS/ParameterSet.h>
#include <GCF/GCF_ServiceInfo.h>
//#include <string>

#include "TBBDriver.h"
#include "RawEvent.h" 

// include all cmd and msg classes
#include "AllocCmd.h"
#include "FreeCmd.h"
#include "RecordCmd.h"
#include "StopCmd.h"
#include "TrigReleaseCmd.h"
#include "TrigGenCmd.h"
#include "TrigSetupCmd.h"
#include "TrigCoefCmd.h"
#include "TrigInfoCmd.h"
#include "ReadCmd.h"
#include "UdpCmd.h"
#include "PageperiodCmd.h"
#include "VersionCmd.h"
#include "SizeCmd.h"
#include "StatusCmd.h"
#include "ClearCmd.h"
#include "ResetCmd.h"
#include "ConfigCmd.h"
#include "ErasefCmd.h"
#include "ReadfCmd.h"
#include "WritefCmd.h"
#include "ImageInfoCmd.h"
#include "ReadwCmd.h"
#include "WritewCmd.h"
#include "ReadrCmd.h"
#include "WriterCmd.h"
#include "ReadxCmd.h"


#define ETHERTYPE_TP 0x7BB0			// letters of TBB

using namespace LOFAR;
using namespace GCFCommon;
using namespace ACC::APS;
using namespace TBB;

static int32    g_instancenr = -1;

static const long ALIVECHECKTIME = 60;

/*
//
// parseOptions
//
void parseOptions(int argc, char** argv)
{

	static struct option long_options[] = {
		{ "instance",   required_argument, 0, 'I' },
		{ "daemon",     no_argument,       0, 'd' },
		{ 0, 0, 0, 0 }
	};

	optind = 0; // reset option parsing
	for(;;)
	{
		int option_index = 0;
		int c = getopt_long(argc, argv, "dI:", long_options, &option_index);

		if (c == -1)
		{
			break;
		}

		switch (c)
		{
			case 'I':   // --instance
				g_instancenr = atoi(optarg);
				break;
			case 'd':   // --daemon
				g_daemonize = true;
				break;
			default:
				LOG_FATAL (formatString("Unknown option %c", c));
				ASSERT(false);
		}
	}

}
*/

//-----------------------------------------------------------------------------		
TBBDriver::TBBDriver(string name)
  : GCFTask((State)&TBBDriver::init_state, name)
{
	// use TS->getXXX() tot get settings of the driver
	TS	= TbbSettings::instance();
	
	// get settings adopted from config file
  TS->getTbbSettings();
  
	cmd = 0;
	itsActiveBoards = 0;
	itsAliveCheck = false;
	itsActiveBoardsChange = false;
	itsResetCount = 0;
  
  // tell broker we are here
  LOG_DEBUG("Registering protocols");
  registerProtocol(TBB_PROTOCOL, TBB_PROTOCOL_signalnames);
  registerProtocol(TP_PROTOCOL, TP_PROTOCOL_signalnames);

  // open client port
  LOG_DEBUG("Opening listener for clients");
  string  acceptorID;
  if (g_instancenr>=0) {
	 acceptorID = formatString("(%d)", g_instancenr);
  }
  itsAcceptor.init(*this, MAC_SVCMASK_TBBDRIVER + acceptorID, GCFPortInterface::MSPP, TBB_PROTOCOL);

  // open port with TBB board
  LOG_DEBUG("Connecting to TBB boards");
	itsBoard = new GCFETHRawPort[TS->maxBoards()];
  ASSERT(itsBoard);
	
  char boardname[64];
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		snprintf(boardname, 64, "board%d", boardnr);

		LOG_DEBUG_STR("initializing board " << boardname << ":" << TS->getSrcMac(boardnr).c_str());
		itsBoard[boardnr].init(*this, boardname, GCFPortInterface::SAP, TP_PROTOCOL,true /*raw*/);
		
		LOG_DEBUG_STR("setAddr " << boardname << ":" << TS->getSrcMac(boardnr).c_str());
		itsBoard[boardnr].setAddr(TS->getIfName().c_str(), TS->getSrcMac(boardnr).c_str());
		
		// set ethertype to 0x7BB0 so Ethereal can decode TBB messages
		itsBoard[boardnr].setEtherType(ETHERTYPE_TP);
	}
	
	for (int bn = 0; bn < TS->maxBoards();bn++) {
		TS->setBoardPorts(bn,&itsBoard[bn]);	
	}
	
	itsAliveTimer = new GCFTimerPort(*this, "AliveTimer");
	
	 // create cmd & msg handler
	LOG_DEBUG_STR("initializing handlers");
	cmdhandler = new BoardCmdHandler();
	msghandler = new MsgHandler();
	
	itsResetCount = new int32[TS->maxBoards()];
	memset(itsResetCount,0,sizeof(int32)*TS->maxBoards());
	 	 	 
	// set Tbb queue
	LOG_DEBUG_STR("initializing TbbQueue");
	itsTbbQueue = new deque<TbbEvent>(100);
	itsTbbQueue->clear();
	 
	itsAlive					= new TPAliveEvent();
 	itsAlive->opcode	= TPALIVE;
 	itsAlive->status	= 0;
}

//-----------------------------------------------------------------------------
TBBDriver::~TBBDriver()
{
	delete cmdhandler; 
	delete msghandler;
	delete itsAlive;
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TBBDriver::init_state(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	
	switch(event.signal) {
		case F_INIT: {
			if (itsAcceptor.isConnected())
				itsAcceptor.close();
		} break;
        
		case F_ENTRY:	{
			openBoards();
		}	break;
		
		case F_CONNECTED: {
      LOG_INFO_STR(formatString("CONNECTED: port '%s'", port.getName().c_str()));
			
			if (isEnabled() && !itsAcceptor.isConnected()) {
 			/*	
 				// free all inputs on all boards
 				TPFreeEvent free;
 				free.opcode = TPFREE;
 				free.status = 0;
 				free.channel = 0xFFFFFFFF;  // send channel = -1 to free all inputs
 				for (int32 bnr = 0; bnr < TS->maxBoards(); bnr++) {
 					itsBoard[bnr].send(free);	
 					LOG_DEBUG(formatString("FREE -1 is send to port '%s'", itsBoard[bnr].getName().c_str()));
 				}
 				  
 				// clear all boards(FPGA register are set to 0 and firmware is maintained)
 				TPClearEvent clear;
 				clear.opcode = TPCLEAR;
 				clear.status = 0;
 				for (int32 bnr = 0; bnr < TS->maxBoards(); bnr++) {
 					itsBoard[bnr].send(clear);	
 					LOG_DEBUG(formatString("CLEAR is send to port '%s'", itsBoard[bnr].getName().c_str()));
 				}
 			*/
				itsAcceptor.open();
			}	      			
			if (itsAcceptor.isConnected()) {
				TRAN(TBBDriver::idle_state);
      }
      
    } break;
		
		case F_TIMER: {
    } break;
		
		case F_DATAIN: {
		}	break;
				 
		default: {
			status = GCFEvent::NOT_HANDLED;
		}	break;
	}
	return(status);
}
	

//-----------------------------------------------------------------------------
// idle(event, port)
//
GCFEvent::TResult TBBDriver::idle_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("idle:" << evtstr(event) << "@" << port.getName());
	
	GCFEvent::TResult status = GCFEvent::HANDLED;
  undertaker();
  
	switch(event.signal) {
		case F_INIT: {
		} break;
        
		case F_ENTRY:	{
			LOG_DEBUG_STR("Entering Idle State");
			// look if there is an Tbb command in queue
			if (!itsTbbQueue->empty()) {
				LOG_DEBUG_STR("The queue is NOT empty");
				
				GCFEvent event;
				memcpy(&event, itsTbbQueue->front().event, itsTbbQueue->front().length); 
				SetTbbCommand(event.signal);
				status = cmdhandler->dispatch(event,*itsTbbQueue->front().port);
				
				itsTbbQueue->pop_front();
				TRAN(TBBDriver::busy_state);
			}
			// if no tbb boards start check-alive
			if (itsActiveBoards == 0) {
				itsAliveTimer->setTimer((long)0);
			} 
		}	break;
        
		case F_CONNECTED:	{
			LOG_DEBUG_STR(formatString("CONNECTED: port '%s'", port.getName().c_str()));
		}	break;
		
		case F_DISCONNECTED: {
			
			LOG_DEBUG_STR(formatString("DISCONNECTED: port '%s'", port.getName().c_str()));
      port.close();
      		
			if (&port == &itsAcceptor) {
        LOG_FATAL("Failed to start listening for client connections.");
        exit(EXIT_FAILURE);
      } else {
				itsClientList.remove(&port);
        itsDeadClients.push_back(&port);
      }
		} break;
		
		case F_ACCEPT_REQ: {
			GCFTCPPort* client = new GCFTCPPort();
			client->init(*this, "client", GCFPortInterface::SPP, TBB_PROTOCOL);
      itsAcceptor.accept(*client);
      itsClientList.push_back(client);

      LOG_DEBUG_STR(formatString("NEW CLIENT CONNECTED: %d clients connected", itsClientList.size()));
		} break;
		
		case F_DATAIN: {
			status = RawEvent::dispatch(*this, port);	
		}	break;
		
		case F_TIMER: {
			if (&port == itsAliveTimer) {
				CheckAlive(event, port);
			}
			
		} break;
		
		case TP_ALIVE_ACK: {
			LOG_DEBUG("TP_ALIVE_ACK received");
			CheckAlive(event, port);
		} break;
				
		case TBB_GET_CONFIG: {
			TBBGetConfigAckEvent ack;
			ack.max_boards = TS->maxBoards();
			ack.active_boards_mask = TS->activeBoardsMask();
			port.send(ack); 
		} break;
		
		case TBB_RCU_INFO: {
			TBBRcuInfoAckEvent ack;
			int rcu;
			for (int32 ch = 0; ch < TS->maxChannels(); ch++) {
				rcu = TS->getChRcuNr(ch);
				//LOG_INFO(formatString("info for ch %d rcu.%d", ch, rcu));
				ack.rcu_status[rcu] = (uint16)TS->getChStatus(ch);
				ack.rcu_state[rcu] = TS->getChState(ch);
				ack.rcu_start_addr[rcu] = TS->getChStartAddr(ch);
				ack.rcu_pages[rcu] = TS->getChPageSize(ch);
				ack.rcu_on_board[rcu] = (uint8)TS->getChBoardNr(ch);
				ack.rcu_on_input[rcu] = (uint8)TS->getChInputNr(ch);
				
				//LOG_DEBUG_STR(formatString("Channel %d ,Rcu %d = status[0x%04X] state[%c] addr[%u] pages[%u]"
				//	,ch,TS->getChRcuNr(ch), TS->getChStatus(ch), TS->getChState(ch),TS->getChStartAddr(ch),TS->getChPageSize(ch)));
			}
			port.send(ack); 
		} break;
		
		case TBB_SUBSCRIBE: {
			msghandler->addClient(port)	;
			TBBSubscribeAckEvent subscribeack;
			port.send(subscribeack);
			
		} break;
		
		case TBB_UNSUBSCRIBE: {
			msghandler->removeClient(port);
			TBBUnsubscribeAckEvent unsubscribeack;
			port.send(unsubscribeack);
		} break;
						
		case TP_TRIGGER: {
			for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
				if (&port == &TS->boardPort(boardnr)) {
					msghandler->sendTrigger(event,boardnr);
					break;
				}
			}
		} break;

		case TP_ERROR: {
			msghandler->sendError(event);
		} break;					
		
		default: {
			// look if the event is a Tbb event
			if (SetTbbCommand(event.signal)) {
				itsAliveTimer->cancelAllTimers();
				//itsAliveTimer->setTimer(ALIVECHECKTIME);
				itsAliveTimer->setTimer((long)10);
				status = cmdhandler->dispatch(event,port);
				TRAN(TBBDriver::busy_state);
			} else {
			// if not a Tbb event, return not-handled 
				status = GCFEvent::NOT_HANDLED;
			}
		}	break;
	}
	LOG_DEBUG_STR("leaving??");
	return(status);
}

//-----------------------------------------------------------------------------
// enabled(event, port)
//
GCFEvent::TResult TBBDriver::busy_state(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;    
	
	LOG_DEBUG_STR(formatString("busy_state signal = [%d]",event.signal));
	switch(event.signal) {
		case F_INIT: {
		} break;
		
		case F_ENTRY: {
		}	break;
		
		case F_ACCEPT_REQ: {
		}	break;
		
		case F_CONNECTED:	{
		}	break;
		
		case F_TIMER: {
			if (&port == itsAliveTimer) { 
				if (itsAliveCheck) {
					CheckAlive(event, port);// if true, then all boards are checked
				} else {
					itsAliveTimer->setTimer(ALIVECHECKTIME);
				}
			}

			status = cmdhandler->dispatch(event,port); // dispatch time-out event	
		} break;
		
		case F_DISCONNECTED: {
			itsAcceptor.setTimer((long)1);
			TRAN(TBBDriver::idle_state);	
		}	break;
		
		case F_EXIT: {
		} break;
		
		case F_DATAIN: {
			status = RawEvent::dispatch(*this, port);
		}	break;
		
		case TP_ALIVE_ACK: {
			LOG_DEBUG_STR("TP_ALIVE_ACK received");
			CheckAlive(event, port);
		} break;
		
		case TBB_GET_CONFIG: {
			TBBGetConfigAckEvent ack;
			ack.max_boards = TS->maxBoards();
			ack.active_boards_mask = itsActiveBoards;
			port.send(ack); 
		
		} break;
		
		case TBB_SUBSCRIBE: {
			msghandler->addClient(port)	;
		} break;
		
		case TBB_UNSUBSCRIBE: {
			msghandler->removeClient(port);
		} break;
		
		case TP_TRIGGER: {
			for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
				if (&port == &TS->boardPort(boardnr)) {
					msghandler->sendTrigger(event,boardnr);
					break;
				}
			}
		} break;

		case TP_ERROR: {
			msghandler->sendError(event);
		} break;		
		
		case TP_ALLOC_ACK:	
		case TP_FREE_ACK:
		case TP_RECORD_ACK: 
		case TP_STOP_ACK:
		case TP_TRIG_RELEASE_ACK:
		case TP_TRIG_GENERATE_ACK:
		case TP_TRIG_SETUP_ACK:
		case TP_TRIG_COEF_ACK:
		case TP_TRIG_INFO_ACK:				
		case TP_READ_ACK:
		case TP_UDP_ACK:
		case TP_PAGEPERIOD_ACK:	
		case TP_VERSION_ACK:
		case TP_STATUS_ACK:
		case TP_CLEAR_ACK:
		case TP_SIZE_ACK:	
		case TP_RESET_ACK:
		case TP_CONFIG_ACK:
		case TP_ERASEF_ACK:
		case TP_READF_ACK:
		case TP_WRITEF_ACK:
		case TP_READW_ACK:
		case TP_WRITEW_ACK:
		case TP_READR_ACK:
		case TP_WRITER_ACK:
		case TP_READX_ACK:	
		{
			status = cmdhandler->dispatch(event,port); // dispatch ack from boards
			
			if (cmdhandler->tpCmdDone() == true) {
				itsAliveTimer->setTimer((long)2);
				itsAliveCheck = false;
				TRAN(TBBDriver::idle_state);
			}
		}	break;	
										
		default: {
			LOG_DEBUG("DEFAULT");
			if (cmdhandler->tpCmdDone() == true){
				// set ALIVE timer, and check for resets
				//if (!itsAliveCheck) {
				//	itsAliveTimer->setTimer((long)1);
				//	itsAliveCheck = false;
				//}
				TRAN(TBBDriver::idle_state);
			} else {
			
				// put event on the queue
				TbbEvent tbbevent;
				tbbevent.length = event.length;
				tbbevent.event = new uint8[tbbevent.length];
				memcpy(tbbevent.event, &event, tbbevent.length);			
				//tbbevent.signal = event.signal;
				tbbevent.port = &port;
				itsTbbQueue->push_back(tbbevent);
			}
			status = GCFEvent::NOT_HANDLED;
		}	break;
	}
	return(status);
}

//-----------------------------------------------------------------------------
void TBBDriver::undertaker()
{
  for (list<GCFPortInterface*>::iterator it = itsDeadClients.begin();
       it != itsDeadClients.end();
       it++)
  {
    delete (*it);
  }
  itsDeadClients.clear();
}

//-----------------------------------------------------------------------------
// openBoards()
//
void TBBDriver::openBoards()
{
	LOG_DEBUG_STR("opening boards");
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		if (itsBoard[boardnr].isConnected())
			itsBoard[boardnr].close();
		itsBoard[boardnr].open();
  }
}

//-----------------------------------------------------------------------------
// isEnabled()
//
bool TBBDriver::isEnabled()
{
  bool enabled = true;
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++)
	{
    if (!itsBoard[boardnr].isConnected())
    {
      enabled = false;
      break;
    }
  }
  return(enabled);
}

//-----------------------------------------------------------------------------
bool TBBDriver::CheckAlive(GCFEvent& event, GCFPortInterface& port)
{
	//bool done = false;
	static int32 boardnr;
	static uint32 activeboards;
	static uint32 checkmask;
	
	itsAliveTimer->cancelAllTimers();
	if (!itsAliveCheck) {
		itsAliveCheck	= true;
		boardnr				= 0;
		checkmask			= 0;
		activeboards	= 0;
		
		for(int nr = 0; nr < TS->maxBoards(); nr++) {
			checkmask |= (1 << nr);
		}
		itsBoard[boardnr].send(*itsAlive);
		itsAliveTimer->setTimer(TS->timeout());
	} else {
		checkmask &= ~(1 << boardnr);
				
		if (event.signal == TP_ALIVE_ACK){
			// new board, send free and clear cmd 
			if ((itsActiveBoards & (1 << boardnr)) == 0) {  // is it a new board ??
 				// free all inputs on all boards
 				TPFreeEvent free;
 				free.opcode = TPFREE;
 				free.status = 0;
 				free.channel = 0xFFFFFFFF;  // send channel = -1 to free all inputs
				itsBoard[boardnr].send(free);	
				LOG_DEBUG_STR(formatString("FREE -1 is send to port '%s'", itsBoard[boardnr].getName().c_str()));
				
				// clear all boards(FPGA register are set to 0 and firmware is maintained)
 				TPClearEvent clear;
 				clear.opcode = TPCLEAR;
 				clear.status = 0;
				itsBoard[boardnr].send(clear);	
 				LOG_DEBUG(formatString("CLEAR is send to port '%s'", itsBoard[boardnr].getName().c_str()));
			}
			
			activeboards |= (1 << boardnr);
			TPAliveAckEvent ack(event);
			if (ack.resetflag == 0){
				itsResetCount[boardnr]++;
				TS->clearRcuSettings(boardnr);
				LOG_INFO_STR(formatString("=BOARD-RESET=, TBB board %d has been reset %d times",boardnr,itsResetCount[boardnr]));
			}
		}
		boardnr++;
		if (boardnr < TS->maxBoards()) {
			itsBoard[boardnr].send(*itsAlive);
			itsAliveTimer->setTimer(TS->timeout());
		}
	}
	
	if (checkmask == 0) {
		if (activeboards != itsActiveBoards) {
			itsActiveBoards = activeboards;
			TS->setActiveBoards(itsActiveBoards);
			itsActiveBoardsChange = true;
		
			string boardstr;
			for (int i = 0; i < 12; i++) {
				boardstr += " ";
				if (activeboards & (1 << i)) {
					if (i >= 9) {
						boardstr += "1";
						boardstr += i+38;
					} else {			
						boardstr += i+48;
					}
				} else {
					boardstr += ".";
				}
			}
			LOG_INFO_STR("Available TBB boards changed:" + boardstr);	
		}
		LOG_DEBUG_STR("Active TBB boards check");
		if (itsActiveBoards == 0) {
			itsAliveTimer->setTimer((long)5);
		} else {
			itsAliveTimer->setTimer(ALIVECHECKTIME);
		}
		itsAliveCheck = false;
		//done = true;
	}
	return(!itsAliveCheck);
}

//-----------------------------------------------------------------------------
bool TBBDriver::SetTbbCommand(unsigned short signal)
{
	if (cmd) delete cmd;
	switch(signal)
	{
		
		case TBB_ALLOC:	{
			AllocCmd *cmd;
			cmd = new AllocCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_FREE: {
			FreeCmd *cmd;
			cmd = new FreeCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_RECORD: 	{
			RecordCmd *cmd;
			cmd = new RecordCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_STOP:	{
			StopCmd *cmd;
			cmd = new StopCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_TRIG_RELEASE:	{
			TrigReleaseCmd *cmd;
			cmd = new TrigReleaseCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_TRIG_GENERATE:	{
			TrigGenCmd *cmd;
			cmd = new TrigGenCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_TRIG_SETUP:	{
			TrigSetupCmd *cmd;
			cmd = new TrigSetupCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_TRIG_COEF:	{
			TrigCoefCmd *cmd;
			cmd = new TrigCoefCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_TRIG_INFO:	{
			TrigInfoCmd *cmd;
			cmd = new TrigInfoCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_READ:	{
			ReadCmd *cmd;
			cmd = new ReadCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_MODE: {
			UdpCmd *cmd;
			cmd = new UdpCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_PAGEPERIOD: {
			PageperiodCmd *cmd;
			cmd = new PageperiodCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_VERSION: {
			VersionCmd *cmd;
			cmd = new VersionCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_SIZE:	{
			SizeCmd *cmd;
			cmd = new SizeCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_STATUS:	{
			StatusCmd *cmd;
			cmd = new StatusCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_CLEAR:	{
			ClearCmd *cmd;
			cmd = new ClearCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_RESET:	{
			ResetCmd *cmd;
			cmd = new ResetCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_CONFIG:	{
			ConfigCmd *cmd;
			cmd = new ConfigCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_ERASE_IMAGE:	{
			ErasefCmd *cmd;
			cmd = new ErasefCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_READ_IMAGE:	{
			ReadfCmd *cmd;
			cmd = new ReadfCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_WRITE_IMAGE:
		{
			WritefCmd *cmd;
			cmd = new WritefCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_IMAGE_INFO:
		{
			ImageInfoCmd *cmd;
			cmd = new ImageInfoCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_READW: {
			ReadwCmd *cmd;
			cmd = new ReadwCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_WRITEW:	{
			WritewCmd *cmd;
			cmd = new WritewCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_READR:	{
			ReadrCmd *cmd;
			cmd = new ReadrCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_WRITER:	{
			WriterCmd *cmd;
			cmd = new WriterCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_READX:	{
			ReadxCmd *cmd;
			cmd = new ReadxCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		default: {
			return (false);		
		} break;
	}
	
	itsAcceptor.cancelAllTimers();
	return (true);
}

  //} // end namespace TBB
//} // end namespace LOFAR

        
//-----------------------------------------------------------------------------
// main (argc, argv)
//
int main(int argc, char** argv)
{
  LOFAR::GCF::TM::GCFTask::init(argc, argv);    // initializes log system
  
	/*
	LOG_INFO(formatString("Starting up %s", argv[0]));
  
  // adopt commandline switches
  LOG_DEBUG("Parsing options");
  parseOptions (argc, argv);
  
  // daemonize if required 
  if (g_daemonize) {
	 if (0 != daemonize(false)) {
		cerr << "Failed to background this process: " << strerror(errno) << endl;
		exit(EXIT_FAILURE);
	 }
  }
	*/
  LOG_DEBUG ("Reading configuration files");
  try {
  	LOFAR::ConfigLocator cl;
		//LOFAR::ACC::APS::globalParameterSet()->adoptFile(cl.locate("RemoteStation.conf"));
		LOFAR::ACC::APS::globalParameterSet()->adoptFile(cl.locate("TBBDriver.conf"));
	}
	catch (LOFAR::Exception e) {
		LOG_ERROR_STR("Failed to load configuration files: " << e.text());
		//exit(EXIT_FAILURE);
	}
  
	LOFAR::TBB::TBBDriver tbb("TBBDriver");
  
	tbb.start(); // make initialsition
  
  try {
		LOFAR::GCF::TM::GCFTask::run();
	}
	catch (LOFAR::Exception e) {
		LOG_ERROR_STR("Exception: " << e.text());
		exit(EXIT_FAILURE);
	}
  
  LOG_INFO("Normal termination of program");
  
  return(0);
}

// Remove lines or remove comments for copy constructor and assignment.
///TBBDriver::TBBDriver (const TBBDriver& that)
///{}
///TBBDriver& TBBDriver::operator= (const TBBDriver& that)
///{
///  if (this != &that) {
///    ... copy members ...
///  }
///  return *this;
///}

