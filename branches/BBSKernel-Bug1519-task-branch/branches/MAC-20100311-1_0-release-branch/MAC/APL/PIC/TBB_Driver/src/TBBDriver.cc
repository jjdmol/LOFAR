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
#include <APL/RTCCommon/daemonize.h>
#include <Common/ParameterSet.h>
#include <MACIO/MACServiceInfo.h>
#include <Common/hexdump.h>

#include <getopt.h>
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
#include "TrigSetupSameCmd.h"
#include "TrigCoefCmd.h"
#include "TrigCoefSameCmd.h"
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
#include "ArpCmd.h"
#include "ArpModeCmd.h"
#include "StopCepCmd.h"
#include "CepStatusCmd.h"
#include "CepDelayCmd.h"
#include "TempLimitCmd.h"
#include "WatchDogCmd.h"
#include "UdpIpTools.h"


#define ETHERTYPE_TP 0x7BB0     // letters of TBB

using namespace LOFAR;
using namespace GCF::TM;
using namespace MACIO;
using namespace TBB;

static bool   itsDaemonize  = false;
static int32  itsInstancenr = -1;
static bool  itsFirstAliveCheck = true;

static const double ALIVECHECKTIME = 30.0;


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
	for(;;) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "dI:", long_options, &option_index);

		if (c == -1) {
			break;
		}

		switch (c) {
			case 'I': {   // --instance
				itsInstancenr = atoi(optarg);
			} break;

			case 'd': {   // --daemon
				itsDaemonize = true;
			} break;

			default: {
				LOG_FATAL_STR("Unknown option " << c);
				ASSERT(false);
			}
		}
	}
}

//-----------------------------------------------------------------------------
TBBDriver::TBBDriver(string name)
	: GCFTask((State)&TBBDriver::init_state, name)
{
	// use TS->getXXX() tot get settings of the driver
	TS  = TbbSettings::instance();

	// get settings adopted from config file
	TS->getTbbSettings();

	itsCmd = 0;
	//itsNewBoards = 0;
	itsAliveCheck = false;
	itsResetCount = 0;

	// tell broker we are here
	LOG_DEBUG_STR("Registering protocols");

	registerProtocol (TBB_PROTOCOL,      TBB_PROTOCOL_STRINGS);
	registerProtocol (TP_PROTOCOL,      TP_PROTOCOL_STRINGS);

	// open client port
	LOG_DEBUG_STR("Opening listener for clients");

	string  acceptorID;
	if (itsInstancenr >= 0) {
	 acceptorID = formatString("(%d)", itsInstancenr);
	}

	itsAcceptor.init(*this, MAC_SVCMASK_TBBDRIVER + acceptorID, GCFPortInterface::MSPP, TBB_PROTOCOL);

	// open port with TBB board
	LOG_DEBUG_STR("Connecting to TBB boards");
	itsBoard = new GCFETHRawPort[TS->maxBoards()];
	ASSERT(itsBoard);

	char boardname[64];
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		snprintf(boardname, 64, "board%d", boardnr);

		LOG_DEBUG_STR("initializing board " << boardname << ":" << TS->getDstMac(boardnr).c_str());
		itsBoard[boardnr].init(*this, boardname, GCFPortInterface::SAP, TP_PROTOCOL,true /*raw*/);

		LOG_DEBUG_STR("setAddr " << boardname << ":" << TS->getDstMac(boardnr).c_str());
		itsBoard[boardnr].setAddr(TS->getIfName().c_str(), TS->getDstMac(boardnr).c_str());

		// set ethertype to 0x7BB0 so Ethereal can decode TBB messages
		itsBoard[boardnr].setEtherType(ETHERTYPE_TP);
	}

	for (int bn = 0; bn < TS->maxBoards();bn++) {
		TS->setBoardPorts(bn,&itsBoard[bn]);
	}

	TS->setActiveBoardsMask(0);

	itsAliveTimer = new GCFTimerPort(*this, "AliveTimer");
	itsSetupTimer = new GCFTimerPort(*this, "SetupTimer");
	itsCmdTimer = new GCFTimerPort(*this, "CmdTimer");
	//itsSaveTimer = new GCFTimerPort(*this, "SaveTimer");

	// create cmd & msg handler
	LOG_DEBUG_STR("initializing handlers");
	itsCmdHandler = new BoardCmdHandler(itsCmdTimer);
	itsMsgHandler = new MsgHandler();

	itsResetCount = new int32[TS->maxBoards()];
	memset(itsResetCount,0,sizeof(int32)*TS->maxBoards());

	// set Tbb queue
	LOG_DEBUG_STR("initializing TbbQueue");
	itsTbbQueue = new deque<TbbEvent*>(100);
	itsTbbQueue->clear();
}

//-----------------------------------------------------------------------------
TBBDriver::~TBBDriver()
{
	delete [] itsBoard;
	delete itsAliveTimer;
	delete itsSetupTimer;
	delete itsCmdHandler;
	delete itsMsgHandler;
	delete itsTbbQueue;
	delete [] itsResetCount;
	if (itsCmd) delete itsCmd;
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TBBDriver::init_state(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("init_state:" << eventName(event) << "@" << port.getName());

	switch(event.signal) {
		case F_INIT: {
			if (itsAcceptor.isConnected())
				itsAcceptor.close();
		} break;

		case F_EXIT: {
		} break;

		case F_ENTRY: {
			openBoards();
		} break;

		case F_CONNECTED: {
			LOG_DEBUG_STR("CONNECTED: port " << port.getName());

			if (boardsConnected() && !itsAcceptor.isConnected()) {
				itsAcceptor.open();
			}
			if (itsAcceptor.isConnected()) {
				// wait some time(10s for clock switching and 80s for watchdog), to avoid problems with clock board
				itsAliveTimer->setTimer(0.1);
				//if (TS->saveTriggersToFile()) { itsSaveTimer->setTimer(10.0, 10.0); }
				TRAN(TBBDriver::idle_state);
			}
		} break;

		default: {
			if (addTbbCommandToQueue(event, port)) {
				LOG_DEBUG_STR("init_state: received TBB cmd, and put on queue");
			} else {
				status = GCFEvent::NOT_HANDLED;
			}
		} break;
	}
	return(status);
}


//-----------------------------------------------------------------------------
GCFEvent::TResult TBBDriver::setup_state(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("setup_state:" << eventName(event) << "@" << port.getName());

	switch(event.signal) {
		case F_INIT: {
		} break;

		case F_EXIT: {
		} break;

		case F_ENTRY: {
			LOG_DEBUG_STR("Entering setup_state");
			bool setupDone = true;
			for (int board = 0; board < TS->maxBoards(); board++) {

				if (TS->getSetupWaitTime(board)) {
					setupDone = false;
					continue;
				} // board still busy, next board

				if ((TS->getBoardState(board) == noBoard) ||
					 (TS->getBoardState(board) == boardReady) ||
					 (TS->getBoardState(board) == boardError))
				{ continue; }
									
				if (TS->getBoardState(board) == setImage1) {
					TPConfigEvent config;
					config.opcode = oc_CONFIG;
					config.status = 0;
					config.imagenr = 1;
					//config.imagenr = 0;
					itsBoard[board].send(config);
					itsBoard[board].setTimer(TS->timeout());
					LOG_INFO_STR("CONFIG = 1  is send to port '" << itsBoard[board].getName() << "'");
					TS->setSetupWaitTime(board, 10);
					TS->setSetupCmdDone(board, false);
					setupDone = false;
					continue;
				}

				if ((TS->getBoardState(board) == enableWatchdog) ||
					 (TS->getBoardState(board) == boardCleared) ||
					 (TS->getBoardState(board) == image1Set)) {

					TPWatchdogEvent watchdog;
					watchdog.opcode = oc_WATCHDOG;
					watchdog.status = 0;
					watchdog.mode = 1; // watchdog is set to eth port
					//watchdog.mode = 0;
					itsBoard[board].send(watchdog);
					itsBoard[board].setTimer(TS->timeout());
					LOG_INFO_STR("WATCHDOG = ETH is send to port '" << itsBoard[board].getName() << "'");
					TS->setSetupWaitTime(board, 10);
					TS->setSetupCmdDone(board, false);
					setupDone = false;
					continue;
				}
				
				if ((TS->getBoardState(board) == setMode) ||
					(TS->getBoardState(board) == watchdogEnabled)) {

					TPUdpEvent udp;
					udp.opcode = oc_UDP;
					udp.status = 0;
					string2mac(TS->getSrcMacCep(board).c_str(),udp.srcmac);
					string2mac(TS->getDstMacCep(board).c_str(),udp.dstmac);
					setup_udpip_header( board, 
										TBB_MODE_TRANSIENT, 
										TS->getSrcIpCep(board).c_str(), 
										TS->getDstIpCep(board).c_str(), 
										udp.ip, 
										udp.udp );
					itsBoard[board].send(udp);
					itsBoard[board].setTimer(TS->timeout());
					LOG_INFO_STR("MODE = TRANSIENT is send to port '" << itsBoard[board].getName() << "'");
					TS->setSetupWaitTime(board, 10);
					TS->setSetupCmdDone(board, false);
					setupDone = false;
					continue;
				}
				
				if ((TS->getBoardState(board) == enableArp) ||
					(TS->getBoardState(board) == modeSet)) {

					TPArpModeEvent arp;
					arp.opcode = oc_ARPMODE;
					arp.status = 0;
					arp.mode=1; // set arp mode to auto
					itsBoard[board].send(arp);
					itsBoard[board].setTimer(TS->timeout());
					LOG_INFO_STR("ARP = AUTO is send to port '" << itsBoard[board].getName() << "'");
					TS->setSetupWaitTime(board, 10);
					TS->setSetupCmdDone(board, false);
					setupDone = false;
					continue;
				}
				
				if ((TS->getBoardState(board) == freeBoard) ||
					(TS->getBoardState(board) == arpEnabled)) {

					TPFreeEvent free;
					free.opcode = oc_FREE;
					free.status = 0;
					free.channel = 0xFFFFFFFF;  // send channel = -1 to free all inputs
					itsBoard[board].send(free);
					itsBoard[board].setTimer(TS->timeout());
					LOG_INFO_STR("FREE ALL is send to port '" << itsBoard[board].getName() << "'");
					TS->setSetupWaitTime(board, 10);
					TS->setSetupCmdDone(board, false);
					setupDone = false;
					continue;
				}

				if (TS->getBoardState(board) == boardFreed) {
					TPAliveEvent alive;
					alive.opcode = oc_ALIVE;
					alive.status = 0;
					itsBoard[board].send(alive);
					itsBoard[board].setTimer(TS->timeout());
					LOG_INFO_STR("ALIVE is send to port '" << itsBoard[board].getName() << "'");
					TS->setSetupWaitTime(board, 10);
					TS->setSetupCmdDone(board, false);
					setupDone = false;
					continue;
				}
			}
			if (setupDone) {
				LOG_INFO_STR("setting up boards done");
				for (int board = 0; board < TS->maxBoards(); board++) {
					TS->resetSetupRetries(board);
				}
				TS->clearBoardSetup();
				itsSetupTimer->cancelAllTimers();
				TRAN(TBBDriver::idle_state);
			}	
		} break;

		case F_CONNECTED: {
		} break;

		case F_DISCONNECTED: {
			LOG_DEBUG_STR("DISCONNECTED: port ''" << port.getName() << "'");
			port.close();

			if (&port == &itsAcceptor) {
					LOG_FATAL_STR("Failed to start listening for client connections.");
					exit(EXIT_FAILURE);
				} else {
				itsClientList.remove(&port);
				itsMsgHandler->removeTriggerClient(port);
				itsMsgHandler->removeHardwareClient(port);
				}
		} break;

		case F_ACCEPT_REQ: {
			GCFTCPPort* client = new GCFTCPPort();
			client->init(*this, "client", GCFPortInterface::SPP, TBB_PROTOCOL);
			itsAcceptor.accept(*client);
			itsClientList.push_back(client);

			LOG_DEBUG_STR("NEW CLIENT CONNECTED: " << itsClientList.size() << " clients connected");
		} break;

		case F_TIMER: {
			if (&port == itsSetupTimer) {
			}
			//else if (&port == itsSaveTimer) {
			//	itsMsgHandler->saveTriggerMessage();
			//}
			else if (&port == itsAliveTimer) {
				CheckAlive(event, port);
			} 
			else {
				// must be a board time-out
				int board = TS->port2Board(&port); // get board nr
				TS->setSetupWaitTime(board, 0);
				TS->incSetupRetries(board);
				TS->setSetupCmdDone(board, false);
				
				if (TS->getSetupRetries(board) >= TS->maxRetries()) {
					TS->resetActiveBoard(board);
					TS->setBoardState(board,boardError);
					TS->setSetupCmdDone(board, true);
				}
			}
		} break;

		case F_DATAIN: {
			status = RawEvent::dispatch(*this, port);
		} break;

		case TP_CONFIG_ACK: {
			int board = TS->port2Board(&port); // get board nr
			itsBoard[board].cancelAllTimers();
			TS->setBoardState(board, image1Set);
			TS->setImageNr(board, 1);
			TS->setSetupWaitTime(board, 15);
			TS->setSetupCmdDone(board, true);
		} break;

		case TP_WATCHDOG_ACK: {
			int board = TS->port2Board(&port); // get board nr
			itsBoard[board].cancelAllTimers();
			TS->setBoardState(board, watchdogEnabled);
			TS->setSetupWaitTime(board, 0);
			TS->setSetupCmdDone(board, true);
		} break;

		case TP_UDP_ACK: {
			int board = TS->port2Board(&port); // get board nr
			itsBoard[board].cancelAllTimers();
			TS->setBoardState(board,modeSet);
			TS->setSetupWaitTime(board, 0);
			TS->setSetupCmdDone(board, true);
		} break;

		case TP_ARP_MODE_ACK: {
			int board = TS->port2Board(&port); // get board nr
			itsBoard[board].cancelAllTimers();
			TS->setBoardState(board,arpEnabled);
			TS->setSetupWaitTime(board, 0);
			TS->setSetupCmdDone(board, true);
		} break;

		case TP_FREE_ACK: {
			int board = TS->port2Board(&port); // get board nr
			itsBoard[board].cancelAllTimers();
			TS->setBoardState(board,boardFreed);
			TS->setSetupWaitTime(board, 0);
			TS->setSetupCmdDone(board, true);
		} break;

		case TP_ALIVE_ACK: {
			int board = TS->port2Board(&port); // get board nr
			if (TS->getBoardState(board) == boardFreed) {
				itsBoard[board].cancelAllTimers();
				TS->setBoardState(board,boardReady);
				LOG_INFO_STR("'" << itsBoard[board].getName() << "' is Ready");
				TS->setSetupWaitTime(board, 0);
				TS->setSetupCmdDone(board, true);
			}
		} break;

		case TP_TRIGGER: {
			for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
				if (&port == &TS->boardPort(boardnr)) {
					itsMsgHandler->sendTrigger(event,boardnr);
					break;
				}
			}
		} break;

		default: {
			if (addTbbCommandToQueue(event, port)) {
				LOG_DEBUG_STR("setup_state: received TBB cmd, and put on queue");
			} else {
				status = GCFEvent::NOT_HANDLED;
			}
		} break;
	}
	
	bool done = true;
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		if (TS->isSetupCmdDone(boardnr) == false) { done = false; }
	}
	if (done) { TRAN(TBBDriver::idle_state); }
		
	return(status);
}

//-----------------------------------------------------------------------------
// idle(event, port)
//
GCFEvent::TResult TBBDriver::idle_state(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR("idle_state:" << eventName(event) << "@" << port.getName());

	static bool busy;

	switch(event.signal) {
		case F_INIT: {
		} break;

		case F_EXIT: {
		} break;

		case F_ENTRY: {
			LOG_DEBUG_STR("Entering idle_state");
			busy = false;
			
			if (TS->isRecording()) {
				itsMsgHandler->openTriggerFile();
			}
			else {
				itsMsgHandler->closeTriggerFile();
			}

		// look if there is an Tbb command in queue
			if (!itsTbbQueue->empty()) {
				LOG_DEBUG_STR("The queue is NOT empty");
				
				if (itsClientList.empty()) {
					itsTbbQueue->clear();
					LOG_DEBUG_STR("Client list is empty, the queue is cleared");
					break;
				}
#if 1
				GCFEvent*	e = itsTbbQueue->front()->event;
#else
				uint8* bufptr = new uint8[sizeof(GCFEvent) + itsTbbQueue->front()->length];

				GCFEvent* e = new (bufptr) GCFEvent;
				memcpy(e, &(itsTbbQueue->front()->event), itsTbbQueue->front()->length);
#endif

				LOG_DEBUG_STR("queue:" << eventName(*e) << "@" << (*itsTbbQueue->front()->port).getName());

				if (SetTbbCommand(e->signal)) {
					busy = true;
					status = itsCmdHandler->doEvent(*e,*itsTbbQueue->front()->port);

					TbbEvent* tmp = itsTbbQueue->front();
					itsTbbQueue->pop_front();
					delete e;
					delete tmp;
					TRAN(TBBDriver::busy_state);
				} else {
					sendInfo(*e, *itsTbbQueue->front()->port);

					TbbEvent* tmp = itsTbbQueue->front();
					itsTbbQueue->pop_front();
					delete e;
					delete tmp;
				}
			}
		} break;

		case F_CONNECTED: {
			LOG_DEBUG_STR("CONNECTED: port '" << port.getName() << "'");
		} break;

		case F_DISCONNECTED: {
			LOG_DEBUG_STR("DISCONNECTED: port ''" << port.getName() << "'");
			port.close();

			if (&port == &itsAcceptor) {
				LOG_FATAL_STR("Failed to start listening for client connections.");
				exit(EXIT_FAILURE);
			} else {
				itsClientList.remove(&port);
				itsMsgHandler->removeTriggerClient(port);
				itsMsgHandler->removeHardwareClient(port);
			}
		} break;

		case F_ACCEPT_REQ: {
			GCFTCPPort* client = new GCFTCPPort();
			client->init(*this, "client", GCFPortInterface::SPP, TBB_PROTOCOL);
			itsAcceptor.accept(*client);
			itsClientList.push_back(client);

			LOG_DEBUG_STR("NEW CLIENT CONNECTED: " << itsClientList.size() << " clients connected");
		} break;

		case F_DATAIN: {
			status = RawEvent::dispatch(*this, port);
		} break;

		case F_TIMER: {
			if (&port == itsSetupTimer) {
				LOG_DEBUG_STR("need boards setup");
				TRAN(TBBDriver::setup_state);
			}
			else if (&port == itsAliveTimer) {
				CheckAlive(event, port);
			}
		} break;

		case TP_ALIVE_ACK: {
			LOG_DEBUG_STR("TP_ALIVE_ACK received");
			if (itsAliveCheck) {
				 CheckAlive(event, port);
			}
		} break;

		case TBB_SUBSCRIBE: {
			TBBSubscribeEvent subscribe(event);
			if (subscribe.triggers) { itsMsgHandler->addTriggerClient(port); }
			if (subscribe.hardware) { itsMsgHandler->addHardwareClient(port); }

			TBBSubscribeAckEvent subscribeack;
			port.send(subscribeack);
		} break;

		case TBB_UNSUBSCRIBE: {
			itsMsgHandler->removeTriggerClient(port);
			itsMsgHandler->removeHardwareClient(port);
			TBBUnsubscribeAckEvent unsubscribeack;
			port.send(unsubscribeack);
		} break;

		case TP_TRIGGER: {
			for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
				if (&port == &TS->boardPort(boardnr)) {
					itsMsgHandler->sendTrigger(event,boardnr);
					break;
				}
			}
		} break;

		case TP_ERROR: {
			itsMsgHandler->sendError(event);
		} break;

		case TBB_GET_CONFIG:
		case TBB_RCU_INFO:
		case TBB_TRIG_SETTINGS: {
			sendInfo(event, port);
		} break;

		default: {

			// look if there is already a command running
			if (busy) {
				if (addTbbCommandToQueue(event, port)) {
					LOG_DEBUG_STR("idle_state: received TBB cmd, and put on queue");
				} else {
					status = GCFEvent::NOT_HANDLED;
				}
			} else {
				// look if the event is a Tbb event
				if (SetTbbCommand(event.signal)) {
					busy = true;
					status = itsCmdHandler->doEvent(event,port);
					TRAN(TBBDriver::busy_state);
				} else {
					// if not a Tbb event, return not-handled
					status = GCFEvent::NOT_HANDLED;
				}
			}
		} break;
	}

	return(status);
}

//-----------------------------------------------------------------------------
// enabled(event, port)
//
GCFEvent::TResult TBBDriver::busy_state(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("busy_state:" << eventName(event) << "@" << port.getName());

	switch(event.signal) {
		case F_INIT: {
		} break;

		case F_EXIT: {
			if (itsCmd) delete itsCmd;
		} break;

		case F_ENTRY: {
			LOG_DEBUG_STR("Entering busy_state");
			if (itsCmdHandler->tpCmdDone()) {
				TRAN(TBBDriver::idle_state);
			}
		} break;

		case F_ACCEPT_REQ: {
			GCFTCPPort* client = new GCFTCPPort();
			client->init(*this, "client", GCFPortInterface::SPP, TBB_PROTOCOL);
			itsAcceptor.accept(*client);
			itsClientList.push_back(client);

			LOG_DEBUG_STR("NEW CLIENT CONNECTED: " << itsClientList.size() << " clients connected");
		} break;

		case F_CONNECTED: {
			LOG_DEBUG_STR("CONNECTED: port '" << port.getName() << "'");
		} break;

		case F_DISCONNECTED: {
			LOG_DEBUG_STR("DISCONNECTED: port '" << port.getName() << "'");
			port.close();

			if (&port == &itsAcceptor) {
				LOG_FATAL_STR("Failed to start listening for client connections.");
				exit(EXIT_FAILURE);
			} else {
				itsClientList.remove(&port);
				itsMsgHandler->removeTriggerClient(port);
				itsMsgHandler->removeHardwareClient(port);
			}
		} break;

		case F_TIMER: {
			if (&port == itsSetupTimer) {
			}
			else if (&port == itsAliveTimer) {
				CheckAlive(event, port);
			}
			else {
				if (itsCmdHandler->tpCmdDone()) {
					TRAN(TBBDriver::idle_state);
				}
				else {
					status = itsCmdHandler->doEvent(event,port); // dispatch timer event
				}
			}
		} break;

		case F_DATAIN: {
			status = RawEvent::dispatch(*this, port);
		} break;

		case TP_ALIVE_ACK: {
			LOG_DEBUG_STR("TP_ALIVE_ACK received");
			if (itsAliveCheck) {
				 CheckAlive(event, port);
			}
		} break;

		case TBB_SUBSCRIBE: {
			TBBSubscribeEvent subscribe(event);
			if (subscribe.triggers) { itsMsgHandler->addTriggerClient(port); }
			if (subscribe.hardware) { itsMsgHandler->addHardwareClient(port); }

			TBBSubscribeAckEvent subscribeack;
			port.send(subscribeack);
		} break;

		case TBB_UNSUBSCRIBE: {
			itsMsgHandler->removeTriggerClient(port);
			itsMsgHandler->removeHardwareClient(port);
		} break;

		case TP_TRIGGER: {
			for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
				if (&port == &TS->boardPort(boardnr)) {
					itsMsgHandler->sendTrigger(event,boardnr);
					break;
				}
			}
		} break;

		case TP_ERROR: {
			itsMsgHandler->sendError(event);
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
		case TP_ERASEF_SPEC_ACK:
		case TP_READF_ACK:
		case TP_WRITEF_ACK:
		case TP_WRITEF_SPEC_ACK:
		case TP_PROTECT_ACK:
		case TP_UNPROTECT_ACK:
		case TP_READW_ACK:
		case TP_WRITEW_ACK:
		case TP_READR_ACK:
		case TP_WRITER_ACK:
		case TP_READX_ACK:
		case TP_ARP_ACK:
		case TP_ARP_MODE_ACK:
		case TP_STOP_CEP_ACK:
		case TP_CEP_STATUS_ACK:
		case TP_CEP_DELAY_ACK:
		case TP_WATCHDOG_ACK:
		case TP_TEMP_LIMIT_ACK:
		{
			status = itsCmdHandler->doEvent(event,port); // dispatch ack from boards
		} break;

		default: {
			if (addTbbCommandToQueue(event, port)) {
				LOG_DEBUG_STR("busy_state: received TBB cmd, and put on queue");
			} else {
				status = GCFEvent::NOT_HANDLED;
			}
		} break;
	}
	return(status);
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
bool TBBDriver::boardsConnected()
{
	bool connected = true;
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++)
	{
		if (!itsBoard[boardnr].isConnected())
		{
			connected = false;
			break;
		}
	}
	return(connected);
}

//-----------------------------------------------------------------------------
bool TBBDriver::CheckAlive(GCFEvent& event, GCFPortInterface& port)
{
	//bool done = false;
	int32 boardnr;
	static uint32 activeboards;
	static uint32 sendmask;
	static bool boardreset;
	static uint32 retries; 
	
	TPAliveEvent tp_event;
	tp_event.opcode = oc_ALIVE;
	tp_event.status = 0;
	
	if (!itsAliveCheck) {
		itsAliveCheck = true;

		boardnr      = 0;
		sendmask     = 0;
		activeboards = 0;
		boardreset   = false;
		retries      = 0;

		// mask all boards to check
		for(int nr = 0; nr < TS->maxBoards(); nr++) {
			// if board is busy, don't check alive
			if ((itsCmd != 0) && (itsCmd->getBoardNr() == nr)) {
				sendmask |= (1 << nr);
				activeboards |= (1 << nr);
			}
			// check only boards with the listed states
			else if ((TS->getBoardState(nr) == noBoard) ||
					 (TS->getBoardState(nr) == boardReady) ||
					 (TS->getBoardState(nr) == boardError) ||
					 (TS->getBoardState(nr) == boardCleared)) {
				itsBoard[nr].send(tp_event);
				sendmask |= (1 << nr);
			}
			// not busy or active state, must be in service
			else {
				sendmask |= (1 << nr);
				activeboards |= (1 << nr);
			}
		}
		
		// if all boards busy or in service, end check and shedule new one
		if (activeboards == sendmask) {
			itsAliveTimer->setTimer(ALIVECHECKTIME);
			itsAliveCheck = false;
			return(!itsAliveCheck);
		}
		
		// one time-out timer for al events
		itsAliveTimer->setTimer(5.0);
	}
	else {
		if (event.signal == TP_ALIVE_ACK) {
			boardnr = TS->port2Board(&port);
			if (boardnr != -1) {
				activeboards |= (1 << boardnr);

				TPAliveAckEvent ack(event);
				// board is reset
				if ((ack.resetflag == 0) || ((TS->activeBoardsMask() & (1 << boardnr)) == 0)) {
					if (itsFirstAliveCheck == false) {
						TS->clearRcuSettings(boardnr);
						// if a new image is loaded by config, do not reload it to image-1
						if (TS->getFreeToReset(boardnr)) {
							TS->setImageNr(boardnr, 0);
							TS->setBoardState(boardnr,setImage1);
						} else {
							// new image loaded, auto reconfigure is now possible again
							TS->setFreeToReset(boardnr, true);
							TS->setBoardState(boardnr,enableWatchdog);
						}
						boardreset = true;
					}
				}

				if (ack.resetflag == 0) {   // board reset
					itsResetCount[boardnr]++;
					LOG_INFO_STR("TB board " << boardnr << " has been reset " << itsResetCount[boardnr] << " times");
				}
				if ((TS->activeBoardsMask() & (1 << boardnr)) == 0) {   // new board
					LOG_INFO_STR("TB board " << boardnr << " is new");
				}
				//TS->setImageNr(boardnr, ack.imagenr);
			}
		}

		if ((event.signal == F_TIMER) || (activeboards == sendmask )) {
			if (activeboards == sendmask) {
				itsAliveTimer->cancelAllTimers();
				retries = 0;
			}
			else { 
				++retries;
			}
			
			for (int board = 0; board < TS->maxBoards(); board++) {
				if ((activeboards & (1 << board)) == 0) { // look for not active boards
					if (retries < 3) {
						itsBoard[board].send(tp_event);
						LOG_INFO_STR("retry(" << retries << ") AliveCmd for board " << board);
					}
					else {
						TS->setBoardState(board, noBoard);
					}
				}
			}
			if (retries == 3) { retries = 0; }
			
			if (retries > 0) {
				itsAliveTimer->setTimer(5.0);
				return(!itsAliveCheck);
			}
			
			if ((itsFirstAliveCheck == false) && (activeboards != TS->activeBoardsMask())) {
				LOG_DEBUG_STR("sendmask[" << sendmask 
							<< "] activeboards[" << activeboards 
							<< "] activeBoardsMask()[" << TS->activeBoardsMask() << "]");
				TS->setActiveBoardsMask(activeboards);

				char boardstr[40];
				char instr[5];
				strcpy(boardstr,"");
				for (int i = 0; i < TS->maxBoards(); i++) {
					if (activeboards & (1 << i)) {
						sprintf(instr," %d",i);
						strcat(boardstr,instr);
					} else {
						strcat(boardstr," .");
					}
				}
				itsMsgHandler->sendBoardChange(TS->activeBoardsMask());
				LOG_INFO_STR("Available TBB boards changed:" << boardstr);
			}
			if (boardreset) {
				itsSetupTimer->cancelAllTimers();
				itsSetupTimer->setTimer(1.0, 1.0);
			}
			itsFirstAliveCheck = false;
			
			LOG_DEBUG_STR("Active TBB boards check");
			itsAliveTimer->setTimer(ALIVECHECKTIME);
			itsAliveCheck = false;
		}
	}
	return(!itsAliveCheck);
}

//-----------------------------------------------------------------------------
bool TBBDriver::sendInfo(GCFEvent& event, GCFPortInterface& port)
{
	bool valid = true;

	switch (event.signal) {

		case TBB_GET_CONFIG: {
			TBBGetConfigAckEvent ack;
			ack.max_boards = TS->maxBoards();
			ack.active_boards_mask = TS->activeBoardsMask();
			port.send(ack);
		} break;

		case TBB_RCU_INFO: {
			if (TS->activeBoardsMask() != 0) {
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
					//  ,ch,TS->getChRcuNr(ch), TS->getChStatus(ch), TS->getChState(ch),TS->getChStartAddr(ch),TS->getChPageSize(ch)));
				}
				port.send(ack);
			} else {
				TBBDriverBusyAckEvent ack;
				port.send(ack);
			}
		} break;

		case TBB_TRIG_SETTINGS: {
			if (TS->activeBoardsMask() != 0) {
				TBBTrigSettingsAckEvent ack;
				ack.status_mask = TS->activeBoardsMask();
				int rcu;
				for (int32 ch = 0; ch < TS->maxChannels(); ch++) {
					rcu = TS->getChRcuNr(ch);

					ack.setup[rcu].level = TS->getChTriggerLevel(ch);
					ack.setup[rcu].start_mode = TS->getChTriggerStartMode(ch);
					ack.setup[rcu].stop_mode = TS->getChTriggerStopMode(ch);
					ack.setup[rcu].filter_select = TS->getChFilterSelect(ch);
					ack.setup[rcu].window = TS->getChDetectWindow(ch);
					ack.setup[rcu].trigger_mode = TS->getChTriggerMode(ch);
					ack.setup[rcu].operating_mode = TS->getChOperatingMode(ch);
					ack.coefficients[rcu].c0 = TS->getChFilterCoefficient(ch,0);
					ack.coefficients[rcu].c1 = TS->getChFilterCoefficient(ch,1);
					ack.coefficients[rcu].c2 = TS->getChFilterCoefficient(ch,2);
					ack.coefficients[rcu].c3 = TS->getChFilterCoefficient(ch,3);
				}
				port.send(ack);
			} else {
				TBBDriverBusyAckEvent ack;
				port.send(ack);
			}
		} break;

		default: {
			valid = false;
		} break;
	}
	return (valid);
}

//-----------------------------------------------------------------------------
bool TBBDriver::addTbbCommandToQueue(GCFEvent& event, GCFPortInterface& port)
{
	bool event_saved;

	switch(event.signal)
	{
		case TBB_GET_CONFIG:
		case TBB_RCU_INFO:
		case TBB_ALLOC:
		case TBB_FREE:
		case TBB_RECORD:
		case TBB_STOP:
		case TBB_TRIG_RELEASE:
		case TBB_TRIG_GENERATE:
		case TBB_TRIG_SETUP:
		case TBB_TRIG_SETUP_SAME:
		case TBB_TRIG_COEF:
		case TBB_TRIG_COEF_SAME:
		case TBB_TRIG_SETTINGS:
		case TBB_TRIG_INFO:
		case TBB_READ:
		case TBB_MODE:
		case TBB_PAGEPERIOD:
		case TBB_VERSION:
		case TBB_SIZE:
		case TBB_STATUS:
		case TBB_CLEAR:
		case TBB_RESET:
		case TBB_CONFIG:
		case TBB_ERASE_IMAGE:
		case TBB_READ_IMAGE:
		case TBB_WRITE_IMAGE:
		case TBB_IMAGE_INFO:
		case TBB_READX:
		case TBB_READW:
		case TBB_WRITEW:
		case TBB_READR:
		case TBB_WRITER:
		case TBB_ARP:
		case TBB_ARP_MODE:
		case TBB_STOP_CEP:
		case TBB_CEP_STATUS:
		case TBB_CEP_DELAY:
		case TBB_WATCHDOG:
		case TBB_TEMP_LIMIT: {
			// put event on the queue
			LOG_DEBUG_STR("Put event on queue");
#if 0
			uint8* bufptr = new uint8[sizeof(GCFPortInterface*)
					+ sizeof(uint32)
					+ sizeof(GCFEvent)
					+ event.length];
			TbbEvent* tbbevent = new (bufptr) TbbEvent;

			memcpy(&(tbbevent->event), &event, (sizeof(GCFEvent) + event.length));
			tbbevent->length = sizeof(GCFEvent) + event.length;
			tbbevent->port = &port;
			itsTbbQueue->push_back(tbbevent);
#else
		   //e.pack();
		   TbbEvent* tbbevent = new TbbEvent;
		   tbbevent->event = new GCFEvent(event.signal);
		   ASSERTSTR(tbbevent->event, "can't allocate a new event");
		   tbbevent->event->length  = event.length;   // copy settings of the packedbuffer
		   tbbevent->event->_buffer = event._buffer;
		   event.length  = 0; // remove them from the original
		   event._buffer = 0;
		   tbbevent->port = &port;
		   if (event.signal == TBB_STOP) {
		       itsTbbQueue->push_front(tbbevent);
		   }
		   else {
		       itsTbbQueue->push_back(tbbevent);
		   }
#endif
			event_saved = true;
		} break;

		default: {
			event_saved = false;
		} break;
	}
	return(event_saved);
}

//-----------------------------------------------------------------------------
bool TBBDriver::SetTbbCommand(unsigned short signal)
{
	if (itsCmd) {
		//itsCmdHandler->setTpCmd(0);
		delete itsCmd;
	}
	switch(signal)
	{
		case TBB_ALLOC: {
			AllocCmd *itsCmd;
			itsCmd = new AllocCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_FREE: {
			FreeCmd *itsCmd;
			itsCmd = new FreeCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_RECORD:  {
			RecordCmd *itsCmd;
			itsCmd = new RecordCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_STOP:  {
			StopCmd *itsCmd;
			itsCmd = new StopCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_TRIG_RELEASE:  {
			TrigReleaseCmd *itsCmd;
			itsCmd = new TrigReleaseCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_TRIG_GENERATE: {
			TrigGenCmd *itsCmd;
			itsCmd = new TrigGenCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_TRIG_SETUP:  {
			TrigSetupCmd *itsCmd;
			itsCmd = new TrigSetupCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;
		
		case TBB_TRIG_SETUP_SAME:  {
			TrigSetupSameCmd *itsCmd;
			itsCmd = new TrigSetupSameCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_TRIG_COEF: {
			TrigCoefCmd *itsCmd;
			itsCmd = new TrigCoefCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_TRIG_COEF_SAME: {
			TrigCoefSameCmd *itsCmd;
			itsCmd = new TrigCoefSameCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_TRIG_INFO: {
			TrigInfoCmd *itsCmd;
			itsCmd = new TrigInfoCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_READ:  {
			ReadCmd *itsCmd;
			itsCmd = new ReadCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_MODE: {
			UdpCmd *itsCmd;
			itsCmd = new UdpCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_PAGEPERIOD: {
			PageperiodCmd *itsCmd;
			itsCmd = new PageperiodCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_VERSION: {
			VersionCmd *itsCmd;
			itsCmd = new VersionCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_SIZE:  {
			SizeCmd *itsCmd;
			itsCmd = new SizeCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_STATUS:  {
			StatusCmd *itsCmd;
			itsCmd = new StatusCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_CLEAR: {
			ClearCmd *itsCmd;
			itsCmd = new ClearCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_RESET: {
			ResetCmd *itsCmd;
			itsCmd = new ResetCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_CONFIG:  {
			ConfigCmd *itsCmd;
			itsCmd = new ConfigCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_ERASE_IMAGE: {
			ErasefCmd *itsCmd;
			itsCmd = new ErasefCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_READ_IMAGE:  {
			ReadfCmd *itsCmd;
			itsCmd = new ReadfCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_WRITE_IMAGE:
		{
			WritefCmd *itsCmd;
			itsCmd = new WritefCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_IMAGE_INFO:
		{
			ImageInfoCmd *itsCmd;
			itsCmd = new ImageInfoCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_READW: {
			ReadwCmd *itsCmd;
			itsCmd = new ReadwCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_WRITEW:  {
			WritewCmd *itsCmd;
			itsCmd = new WritewCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_READR: {
			ReadrCmd *itsCmd;
			itsCmd = new ReadrCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_WRITER:  {
			WriterCmd *itsCmd;
			itsCmd = new WriterCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_READX: {
			ReadxCmd *itsCmd;
			itsCmd = new ReadxCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_ARP: {
			ArpCmd *itsCmd;
			itsCmd = new ArpCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_ARP_MODE:  {
			ArpModeCmd *itsCmd;
			itsCmd = new ArpModeCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_STOP_CEP:  {
			StopCepCmd *itsCmd;
			itsCmd = new StopCepCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_CEP_STATUS:  {
			CepStatusCmd *itsCmd;
			itsCmd = new CepStatusCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_CEP_DELAY:  {
			CepDelayCmd *itsCmd;
			itsCmd = new CepDelayCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_WATCHDOG:  {
			WatchDogCmd *itsCmd;
			itsCmd = new WatchDogCmd();
			itsCmdHandler->setTpCmd(itsCmd);
		} break;

		case TBB_TEMP_LIMIT:  {
			TempLimitCmd *itsCmd;
			itsCmd = new TempLimitCmd();
			itsCmdHandler->setTpCmd(itsCmd);
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
	LOFAR::GCF::TM::GCFScheduler::instance()->init(argc, argv, "TBBDriver");    // initializes log system

	// Inform Logprocessor who we are
	LOG_INFO("MACProcessScope: LOFAR_PermSW_TBBDriver");

	LOG_INFO_STR("Starting up " << argv[0]);

	// adopt commandline switches
	LOG_DEBUG_STR("Parsing options");
	parseOptions (argc, argv);

	// daemonize if required
	if (itsDaemonize) {
		LOG_DEBUG_STR("background this process");
		if (daemonize(false) == 0) {
			//cerr << "Failed to background this process: " << strerror(errno) << endl;
			exit(EXIT_FAILURE);
		}
	}

	LOG_DEBUG_STR("Reading configuration files");
	try {
		LOFAR::ConfigLocator cl;
		LOFAR::globalParameterSet()->adoptFile(cl.locate("TBBDriver.conf"));
		LOFAR::globalParameterSet()->adoptFile(cl.locate("RemoteStation.conf"));
	}
	catch (LOFAR::Exception& e) {
		LOG_ERROR_STR("Failed to load configuration files: " << e.text());
		exit(EXIT_FAILURE);
	}

	LOFAR::TBB::TBBDriver tbb("TBBDriver");

	tbb.start(); // make initialsition

	try {
		LOFAR::GCF::TM::GCFScheduler::instance()->run();
	}
	catch (LOFAR::Exception& e) {
		LOG_ERROR_STR("Exception: " << e.text());
		exit(EXIT_FAILURE);
	}

	LOG_INFO("Normal termination of program");

	return(0);
}

