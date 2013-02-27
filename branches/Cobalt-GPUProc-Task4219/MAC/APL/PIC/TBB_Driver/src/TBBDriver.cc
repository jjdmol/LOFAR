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
#include <Common/Exception.h>

#include <getopt.h>
#include <string>



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
//#include "UdpCmd.h"
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
using namespace std;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

static bool   itsDaemonize  = false;
static int32  itsInstancenr = -1;
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
    sleep(5);
    // use TS->getXXX() tot get settings of the driver
    TS  = TbbSettings::instance();

    // get settings adopted from config file
    TS->getTbbSettings();

    itsCmd = 0;
    //itsNewBoards = 0;
    itsAliveCheck = false;
    itsResetCount = 0;
    itsClockSubscription = 0;

    // tell broker we are here
    LOG_DEBUG_STR("Registering protocols");

    registerProtocol (TBB_PROTOCOL,   TBB_PROTOCOL_STRINGS);
    registerProtocol (TP_PROTOCOL,    TP_PROTOCOL_STRINGS);
    registerProtocol (RSP_PROTOCOL,   RSP_PROTOCOL_STRINGS);

    // open client port
    LOG_DEBUG_STR("Opening listener for clients");

    string  acceptorID;
    if (itsInstancenr >= 0) {
     acceptorID = formatString("(%d)", itsInstancenr);
    }

    itsAcceptor.init(*this, MAC_SVCMASK_TBBDRIVER + acceptorID, GCFPortInterface::MSPP, TBB_PROTOCOL);

    // open port with TB board
    LOG_DEBUG_STR("Connecting to TB boards");
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

    // prepare TCP port to RSPDriver.
	itsRSPDriver = new GCFTCPPort(*this, MAC_SVCMASK_RSPDRIVER, GCFPortInterface::SAP, RSP_PROTOCOL);
	ASSERTSTR(itsRSPDriver, "Cannot allocate TCPport to RSPDriver");

    itsAliveTimer = new GCFTimerPort(*this, "AliveTimer");
    itsSetupTimer = new GCFTimerPort(*this, "SetupTimer");
    itsCmdTimer = new GCFTimerPort(*this, "CmdTimer");
    itsQueueTimer = new GCFTimerPort(*this, "QueueTimer");
    itsTriggerTimer = new GCFTimerPort(*this, "TriggerTimer");

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
    LOG_INFO_STR("class init done");
}

//-----------------------------------------------------------------------------
TBBDriver::~TBBDriver()
{   
    cancelClockSubscription();
    delete [] itsBoard;
    delete itsRSPDriver;
    delete itsAliveTimer;
    delete itsSetupTimer;
    delete itsCmdTimer;
    delete itsQueueTimer;
    delete itsTriggerTimer;
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
            if (itsAcceptor.isConnected()) {
                itsAcceptor.close();
            }
            if (itsRSPDriver->isConnected()) {
                itsRSPDriver->close();
            }
        } break;

        case F_EXIT: {
        } break;

        case F_ENTRY: {
            itsAcceptor.open();
        } break;

        case F_CONNECTED: {
            LOG_DEBUG_STR("CONNECTED: port " << port.getName());

            if (&port == &itsAcceptor) {
                if (itsAcceptor.isConnected()) {
                    itsRSPDriver->open();
                }
            }
            else if (&port == itsRSPDriver) {
                if (itsRSPDriver->isConnected()) {
                    openBoards();
                }
            }
            else {
                if (boardsConnected()) {
                    // take subscription on clock changes
                    itsAliveTimer->setTimer(0.0);
                }
            }

        } break;

        case F_DATAIN: {
            status = RawEvent::dispatch(*this, port);
        } break;

        case F_TIMER: {
            if (&port == itsAliveTimer) {
                requestClockSubscription();
            }
        } break;

        case RSP_SUBCLOCKACK: {
    		RSPSubclockackEvent	ack(event);
    		if (ack.status != RSP_SUCCESS) {
    			LOG_WARN ("Could not get subscription on clock, retry in 2 seconds.");
    			//itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("subscribe failed"));
    			itsAliveTimer->setTimer(2.0);
    			break;
    		}
    		itsClockSubscription = ack.handle;
    		LOG_INFO("Subscription on the clock successful. going to operational mode");
    		//itsOwnPropertySet->setValue(PN_CLC_ACTUAL_CLOCK,GCFPVInteger(itsClock));
    		
    		// start trigger reset timer
            itsTriggerTimer->setTimer(90.0, 0.1);  // first start over 90 seconds, then every 0.1 Sec
    		itsAliveTimer->setTimer(45.0);
    		TRAN(TBBDriver::idle_state);
    	} break;
        
        case RSP_UPDCLOCK: {
            setClockState(event);
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
            return(status);
        } break;

        case F_ENTRY: {
            LOG_DEBUG_STR("entering setup_state");
            for (int board = 0; board < TS->maxBoards(); board++) {

                if (( TS->isSetupCmdDone(board) == false ) ||
                     ( TS->getSetupWaitTime(board) > 0 )) {
                    continue;
                } // board still busy, next board

                if ((TS->getBoardState(board) == noBoard) ||
                     (TS->getBoardState(board) == boardReady) ||
                     (TS->getBoardState(board) == boardError)) {
                    TS->setSetupCmdDone(board, true);
                    continue;
                }

                if (TS->getBoardState(board) == setImage1) {
                    TPConfigEvent config;
                    config.opcode = oc_CONFIG;
                    config.status = 0;
                    config.imagenr = 1;
                    //config.imagenr = 0;
                    itsBoard[board].send(config);
                    
                    TS->setBoardState(board, image1Set);
                    //TS->setImageNr(board, 1);
                    TS->setSetupWaitTime(board, 12);
                    
                    //itsBoard[board].setTimer(TS->timeout());
                    LOG_INFO_STR("CONFIG = 1  is send to port '" << itsBoard[board].getName() << "'");
                    TS->setSetupCmdDone(board, true);
                    continue;
                }

                if ((TS->getBoardState(board) == image1Set) ||
                    (TS->getBoardState(board) == checkAlive)) {
                    TPAliveEvent alive;
                    alive.opcode = oc_ALIVE;
                    alive.status = 0;
                    itsBoard[board].send(alive);
                    itsBoard[board].setTimer(TS->timeout());
                    LOG_INFO_STR("ALIVE is send to port '" << itsBoard[board].getName() << "'");
                    TS->setSetupCmdDone(board, false);
                    continue;
                }

                if ((TS->getBoardState(board) == checkStatus) ||
                     (TS->getBoardState(board) == boardAlive)) {
                    TPStatusEvent status;
                    status.opcode = oc_STATUS;
                    status.status = 0;
                    itsBoard[board].send(status);
                    itsBoard[board].setTimer(TS->timeout());
                    LOG_INFO_STR("STATUS is send to port '" << itsBoard[board].getName() << "'");
                    TS->setSetupCmdDone(board, false);
                    continue;
                }

                if ((TS->getBoardState(board) == freeBoard) ||
                    (TS->getBoardState(board) == statusChecked)) {

                    TPFreeEvent free;
                    free.opcode = oc_FREE;
                    free.status = 0;
                    free.channel = 0xFFFFFFFF;  // send channel = -1 to free all inputs
                    itsBoard[board].send(free);
                    itsBoard[board].setTimer(TS->timeout());
                    LOG_INFO_STR("FREE ALL is send to port '" << itsBoard[board].getName() << "'");
                    TS->setSetupCmdDone(board, false);
                    continue;
                }
            }
        } break;

        case F_CONNECTED: {
        } break;

        case F_DISCONNECTED: {
            LOG_DEBUG_STR("DISCONNECTED: port ''" << port.getName() << "'");
            port.close();

            if (&port == &itsAcceptor) {
                LOG_FATAL_STR("Client port closed.");
                exit(EXIT_FAILURE);
            }
            else if (&port == itsRSPDriver) {
                port.close();
                LOG_FATAL_STR("RSPDriver port closed.");
                exit(EXIT_FAILURE);
            }
            else {
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
            if (&port == itsTriggerTimer) {
                TS->resetTriggersLeft();
            }
            else if (&port == itsSetupTimer) {
                // blank
            }
            else if (&port == itsAliveTimer) {
                itsAliveTimer->setTimer(ALIVECHECKTIME);
                //CheckAlive(event, port);
            }
            else if (&port == itsQueueTimer) {
                // do nothing
            }
            else {
                // must be a board time-out
                int board = TS->port2Board(&port); // get board nr
                TS->setSetupWaitTime(board, 0);
                TS->incSetupRetries(board);
                LOG_INFO_STR(formatString("setup board %d, retries %d of %d", board, TS->getSetupRetries(board), TS->maxRetries()));
                if (TS->getSetupRetries(board) >= TS->maxRetries()) {
                    TS->resetActiveBoard(board);
                    TS->setBoardState(board,boardError);
                }
                // this command done(timed out), try again next setup interval
                TS->setSetupCmdDone(board, true);
            }
        } break;

        case F_DATAIN: {
            status = RawEvent::dispatch(*this, port);
            if (TS->isNewTrigger()) {
                itsMsgHandler->sendSavedTrigger();
            }
        } break;

        case TP_CONFIG_ACK: {
            int board = TS->port2Board(&port); // get board nr
            itsBoard[board].cancelAllTimers();
            TPConfigAckEvent ack(event);
            LOG_INFO_STR(formatString("Received ConfigAck from boardnr[%d], status=%d", board, ack.status));
            if (ack.status == TBB_SUCCESS) {
                TS->setBoardState(board, image1Set);
                //TS->setImageNr(board, 1);
                TS->setSetupWaitTime(board, 12);
            }
            TS->setSetupCmdDone(board, true);
        } break;

    
        case TP_FREE_ACK: {
            int board = TS->port2Board(&port); // get board nr
            itsBoard[board].cancelAllTimers();
            TPFreeAckEvent ack(event);
            LOG_INFO_STR(formatString("Received FreeAck from boardnr[%d], status=%d", board, ack.status));
            if (ack.status == TBB_SUCCESS) {
                TS->setBoardState(board,boardFreed);
            }
            TS->setSetupCmdDone(board, true);
        } break;

          case TP_STATUS_ACK: {
            int board = TS->port2Board(&port); // get board nr
            itsBoard[board].cancelAllTimers();
            TPStatusAckEvent ack(event);
            if (ack.status == TBB_SUCCESS) {
                // bit 0: // 0=image-0, 1=user-image
                // bit 1..2: 00=by-power, 01=by-user, 10=by watchdog, 11=by-reset-button
                TS->setFlashState(board, ((ack.info[5] >> 11) & 0x0007));
                if (TS->getConfigState(board) != 1) {  // config not done by user
                    TS->setImageNr(board, 0);
                    LOG_WARN_STR("Image 1 NOT loaded");
                    TS->setBoardState(board, setImage1);
                }
                else {
                    TS->setImageNr(board, 1);
                    TS->setBoardState(board, statusChecked);
                }

                LOG_INFO_STR(formatString("Received StatusAck from boardnr[%d], status=%d, imagenr=%d, configState=%d",
                                          board, ack.status, TS->getImageNr(board), TS->getConfigState(board)));
            }
            TS->setSetupCmdDone(board, true);
        } break;

        case TP_ALIVE_ACK: {
            int board = TS->port2Board(&port); // get board nr
            itsBoard[board].cancelAllTimers();
            TPAliveAckEvent ack(event);
            LOG_INFO_STR(formatString("Received AliveAck from boardnr[%d], status=%d, resetflag=%d, imagenr=%d",
                                      board, ack.status, ack.resetflag, ack.imagenr));
            if (ack.status == TBB_SUCCESS) {
                TS->setBoardState(board,boardAlive);
            }
            TS->setSetupCmdDone(board, true);
        } break;


        case TP_TRIGGER: {
            // tp_trigger message is handled in raw event
        } break;

        case RSP_UPDCLOCK: {
            setClockState(event);
        } break;
        
        default: {
            if (addTbbCommandToQueue(event, port)) {
                LOG_DEBUG_STR("setup_state: received TBB cmd, and put on queue");
            } else {
                status = GCFEvent::NOT_HANDLED;
            }
        } break;
    }

    if (TS->isSetupCmdDone(-1)) {
        bool allDone = true;
        for (int board = 0; board < TS->maxBoards(); board++) {
            if (TS->getBoardState(board) == boardFreed) {
                TS->setBoardState(board, boardReady);
                LOG_INFO_STR("'" << itsBoard[board].getName() << "' is Ready");
            }

            if (   (TS->getBoardState(board) > noBoard)
                && (TS->getBoardState(board) < boardReady) ) {
                allDone = false;
            }
            else {
                TS->resetSetupRetries(board);
            }
        }
        if (allDone) {
            LOG_INFO_STR("setting up all boards done");
            itsSetupTimer->cancelAllTimers();
        }
        else {
            LOG_DEBUG_STR("setting up boards not ready, wait one second till next setup command");
        }
        LOG_DEBUG_STR("leaving setup_state");
        TRAN(TBBDriver::idle_state);
    }
    return(status);
}

//-----------------------------------------------------------------------------
// idle(event, port)
//
GCFEvent::TResult TBBDriver::idle_state(GCFEvent& event, GCFPortInterface& port)
{
    GCFEvent::TResult status = GCFEvent::HANDLED;
    LOG_TRACE_VAR_STR("idle_state:" << eventName(event) << "@" << port.getName());

    switch(event.signal) {
        case F_INIT: {
        } break;

        case F_EXIT: {
        } break;

        case F_ENTRY: {
            // Setup needed can be set by reset, clear cmd or clock change
            if (TS->isSetupNeeded()) {
                TS->setSetupNeeded(false);
                itsAliveCheck = false;
                itsAliveTimer->cancelAllTimers();
                itsAliveTimer->setTimer(1.0);
            }

            LOG_DEBUG_STR("Entering idle_state");
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
                itsQueueTimer->setTimer(0.0);
            }
        } break;

        case F_CONNECTED: {
            LOG_DEBUG_STR("CONNECTED: port '" << port.getName() << "'");
        } break;

        case F_DISCONNECTED: {
            LOG_DEBUG_STR("DISCONNECTED: port '" << port.getName() << "'");
            port.close();

            if (&port == &itsAcceptor) {
                LOG_FATAL_STR("Client port closed.");
                exit(EXIT_FAILURE);
            }
            else if (&port == itsRSPDriver) {
                port.close();
                LOG_FATAL_STR("RSPDriver port closed.");
                exit(EXIT_FAILURE);
            }
            else {
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
            if (TS->isNewTrigger()) {
                itsMsgHandler->sendSavedTrigger();
            }
        } break;

        case F_TIMER: {
            if (&port == itsQueueTimer) {
                if (handleTbbCommandFromQueue()) {
                    TRAN(TBBDriver::busy_state);
                    return(status);
                }
                if (!itsTbbQueue->empty()) {
                    itsQueueTimer->setTimer(0.0);
                }
            }
            else if (&port == itsTriggerTimer) {
                  TS->resetTriggersLeft();
             }
            else if (&port == itsSetupTimer) {
                LOG_DEBUG_STR("need boards setup");
                TRAN(TBBDriver::setup_state);
                return(status);
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

        case RSP_UPDCLOCK: {
            setClockState(event);
        } break;
        

        case TP_TRIGGER: {
            // tp_trigger message is handled in raw event
        } break;

        case TP_ERROR: {
            itsMsgHandler->sendError(event);
        } break;

        default: {
            // look if there is already a command in the queue
            if (itsTbbQueue->empty()) {
                // look if the event is a Tbb event
                if (SetTbbCommand(event.signal)) {
                    status = itsCmdHandler->doEvent(event,port);
                    TRAN(TBBDriver::busy_state);
                    return(status);
                }
                else if (sendInfo(event, port)) {
                    // oke event is handled, no further action needed
                }
                else {
                    // if not a Tbb event, return not-handled
                    status = GCFEvent::NOT_HANDLED;
                }
            }
            else {
                if (addTbbCommandToQueue(event, port)) {
                    LOG_DEBUG_STR("idle_state: received TBB cmd, and put on queue");
                }
                else {
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
    LOG_TRACE_VAR_STR ("busy_state:" << eventName(event) << "@" << port.getName());

    switch(event.signal) {
        case F_INIT: {
        } break;

        case F_EXIT: {
            if (itsCmd) delete itsCmd;
            itsCmd = 0;
        } break;

        case F_ENTRY: {
            LOG_DEBUG_STR("Entering busy_state");
            //if (itsCmdHandler->tpCmdDone()) {
            //  TRAN(TBBDriver::idle_state);
            //}
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
            if (&port == &itsAcceptor) {
                port.close();
                LOG_FATAL_STR("Client port closed.");
                exit(EXIT_FAILURE);
            }
            else if (&port == itsRSPDriver) {
                port.close();
                LOG_FATAL_STR("RSPDriver port closed.");
                exit(EXIT_FAILURE);
            }
            else {
                port.close();
                itsClientList.remove(&port);
                itsMsgHandler->removeTriggerClient(port);
                itsMsgHandler->removeHardwareClient(port);
                if (&port == itsCmdHandler->getClientPort()) {
                    TRAN(TBBDriver::idle_state);
                    return(status);
                }
            }
        } break;

        case F_TIMER: {
            if (&port == itsCmdTimer) {
                 if (itsCmdHandler->tpCmdDone()) {
                    TRAN(TBBDriver::idle_state);
                    return(status);
                }
                status = itsCmdHandler->doEvent(event,port); // dispatch timer event
            }
            else if (&port == itsTriggerTimer) {
                  TS->resetTriggersLeft();
             }
            else if (&port == itsSetupTimer) {
            }
            else if (&port == itsAliveTimer) {
                CheckAlive(event, port);
            }
            else {
                if (itsCmdHandler->tpCmdDone()) {
                    TRAN(TBBDriver::idle_state);
                    return(status);
                }
                else {
                    status = itsCmdHandler->doEvent(event,port); // dispatch timer event
                }
            }
        } break;

        case F_DATAIN: {
            status = RawEvent::dispatch(*this, port);
            if (TS->isNewTrigger()) {
                itsMsgHandler->sendSavedTrigger();
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
        } break;

          case TBB_GET_CONFIG: {
                sendInfo(event, port);
          } break;

          case TBB_RESET: {
            SetTbbCommand(event.signal);
            status = itsCmdHandler->doEvent(event,port);
        } break;
        
        case TP_TRIGGER: {
            // tp_trigger message is handled in raw event
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
        
        case RSP_UPDCLOCK: {
            setClockState(event);
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

    TPAliveEvent tp_event;
    tp_event.opcode = oc_ALIVE;
    tp_event.status = 0;

     // If No Alivecheck running, startup one
    if (!itsAliveCheck) {
        itsAliveCheck = true;
        boardnr      = 0;
        sendmask     = 0;
        activeboards = 0;
        boardreset   = false;

        // mask all boards to check
        for(int nr = 0; nr < TS->maxBoards(); nr++) {
            // if board is busy, don't check alive
            //if ((itsCmd != 0) && (itsCmd->getBoardNr() == nr)) {

            // if board was busy last 30 seconds, don't check alive
            if (TS->isBoardUsed(nr)) {
                 LOG_DEBUG_STR(formatString("board %d is used last 30 seconds, no alive check needed", nr));
                sendmask |= (1 << nr);
                activeboards |= (1 << nr);
            }
            // check only boards with the listed states
            else if ((TS->getBoardState(nr) == noBoard) ||
                     (TS->getBoardState(nr) == boardReady) ||
                     (TS->getBoardState(nr) == boardError) ||
                     (TS->getBoardState(nr) == boardCleared)) {
                 LOG_DEBUG_STR(formatString("board %d alive check needed", nr));
                itsBoard[nr].send(tp_event);
                sendmask |= (1 << nr);
            }
            // not busy or active state, must be in service
            else {
                 //LOG_INFO_STR(formatString("board %d active now", nr));
                sendmask |= (1 << nr);
                activeboards |= (1 << nr);
            }
        }

        // if all boards busy or in service, end check and shedule new one
        if (activeboards == sendmask) {
             TS->resetBoardUsed();
            itsAliveTimer->setTimer(ALIVECHECKTIME);
            itsAliveCheck = false;
            return(!itsAliveCheck);
        }

        // one time-out timer for al events
        itsAliveTimer->setTimer(5.0);
    }
    // Alive check is running
    else {
        if (event.signal == TP_ALIVE_ACK) {
            boardnr = TS->port2Board(&port);
            if (boardnr != -1) {
                activeboards |= (1 << boardnr);

                TPAliveAckEvent ack(event);
                LOG_DEBUG_STR("Received AliveAck from board " << boardnr << ", status=" << ack.status << ", resetflag=" << ack.resetflag << ", imagenr=" << ack.imagenr);
                // board is reset
                if ((ack.resetflag == 0) || ((TS->activeBoardsMask() & (1 << boardnr)) == 0)) {
                    TS->clearRcuSettings(boardnr);
                    // if a new image is loaded by config, do not reload it to image-1
                    if (TS->getFreeToReset(boardnr)) {
                        TS->setImageNr(boardnr, 0);
                        TS->setBoardState(boardnr,setImage1);
                        TS->resetSetupRetries(boardnr);
                    } else {
                        // new image loaded, auto reconfigure is now possible again
                        TS->setFreeToReset(boardnr, true);
                        if (TS->getImageNr(boardnr) != 0) {
                            TS->setBoardState(boardnr, freeBoard);
                        }
                    }
                    boardreset = true;
                }
                
                if ((TS->activeBoardsMask() & (1 << boardnr)) == 0) {   // new board
                    LOG_INFO_STR("TB board " << boardnr << " is new");
                }
                else {
                    if (ack.resetflag == 0) {   // board reset
                        itsResetCount[boardnr]++;
                        LOG_INFO_STR("TB board " << boardnr << " has been reset " << itsResetCount[boardnr] << " times");
                    }
                }
            }
        }
        
        // if timeout or all boards done
        if (((event.signal == F_TIMER) && (&port == itsAliveTimer)) || (activeboards == sendmask )) {
            
            if (activeboards == sendmask) {
                itsAliveTimer->cancelAllTimers();
            }
            else {
                // look if boards need a resent
                int nResends = 0;  // number of boards with resen
                for (int board = 0; board < TS->maxBoards(); board++) {
                    if ((activeboards & (1 << board)) == 0) { // look for not active boards
                        if (TS->getSetupRetries(board) < TS->maxRetries()) {
                            nResends++;
                            TS->incSetupRetries(board);
                            itsBoard[board].send(tp_event);
                            LOG_INFO_STR("retry(" << TS->getSetupRetries(board) << ") AliveCmd for board " << board);
                        }
                        else {
                            if (TS->getBoardState(board) != boardError) {
                                TS->setBoardState(board, noBoard);
                            }
                        }
                    }
                }
    
                // is there a resend active
                if (nResends > 0) {
                    itsAliveTimer->setTimer(5.0);
                    return(!itsAliveCheck);
                }
            }

            if (activeboards != TS->activeBoardsMask()) {
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
                LOG_INFO_STR("Available TB boards changed:" << boardstr);
            }
            
            if (boardreset) {
                itsSetupTimer->cancelAllTimers();
                itsSetupTimer->setTimer(1.0, 1.0);
            }

            LOG_DEBUG_STR("Active TB boards check");
            TS->resetBoardUsed();
            itsAliveTimer->setTimer(ALIVECHECKTIME);
            itsAliveCheck = false;
        }
    }
    return(!itsAliveCheck);
}

//-----------------------------------------------------------------------------
void TBBDriver::setClockState(GCFEvent& event)
{
    /*
    After a clock change the TB Boards are in a unknown state, a clear is needed to solve this.
    First clock change event is received, after that the clock is changed by the RSPDriver.
    
    changing clock will take about 25 seconds, so wait first before clearing the boards
    */
    
    RSPUpdclockEvent	e(event);
	if (e.status != RSP_SUCCESS) {
		LOG_WARN ("Received and INVALID clock update, WHAT IS THE CLOCK?");
		return;
	}

	if (TS->getClockFreq() == 0) { // my clock still uninitialized?
		LOG_INFO_STR("My clock is still not initialized. StationClock is " << e.clock << " adopting this value");
		TS->setClockFreq((int32)e.clock);
	}

	else if ((int32)e.clock != TS->getClockFreq()) {
		LOG_WARN_STR ("CLOCK WAS CHANGED TO " << e.clock << "MHz, RESET NEEDED (wait 80 secs for done)");
		for (int bnr = 0; bnr < TS->maxBoards(); bnr++) {
		    TS->setBoardState(bnr, setImage1);
	    }
		TS->setClockFreq((int32)e.clock);
		
		itsSetupTimer->cancelAllTimers();
        itsSetupTimer->setTimer(60.0, 1.0);
		
		itsAliveCheck = false;
		itsAliveTimer->cancelAllTimers();
		itsAliveTimer->setTimer(90.0);
	}
}

//-----------------------------------------------------------------------------
bool TBBDriver::sendInfo(GCFEvent& event, GCFPortInterface& port)
{
    switch (event.signal) {

        case TBB_GET_CONFIG: {
            TBBGetConfigAckEvent ack;
            ack.status_mask = 0;
            ack.max_boards = TS->maxBoards();
            ack.active_boards_mask = TS->activeBoardsMask();
            port.send(ack);
        } break;

        case TBB_RCU_INFO: {
            if (TS->activeBoardsMask() != 0) {
                TBBRcuInfoAckEvent ack;
                ack.status_mask = 0;
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

                    ack.rcu[rcu].setup.level = TS->getChTriggerLevel(ch);
                    ack.rcu[rcu].setup.start_mode = TS->getChTriggerStartMode(ch);
                    ack.rcu[rcu].setup.stop_mode = TS->getChTriggerStopMode(ch);
                    ack.rcu[rcu].setup.filter_select = TS->getChFilterSelect(ch);
                    ack.rcu[rcu].setup.window = TS->getChDetectWindow(ch);
                    ack.rcu[rcu].setup.trigger_mode = TS->getChTriggerMode(ch);
                    ack.rcu[rcu].setup.operating_mode = TS->getChOperatingMode(ch);
                    for (int c = 0; c < 4; c++) {
                        ack.rcu[rcu].coef.filter0[c] = TS->getChFilterCoefficient(ch, 0, c);
                        ack.rcu[rcu].coef.filter1[c] = TS->getChFilterCoefficient(ch, 1, c);
                    }
                }
                port.send(ack);
            } else {
                TBBDriverBusyAckEvent ack;
                port.send(ack);
            }
        } break;

        case TBB_MODE: {
            if (TS->activeBoardsMask() != 0) {
                TBBModeEvent tbb_event(event);
                for (int32 rcu = 0; rcu < TS->maxChannels(); rcu++) {
                    if (tbb_event.rcu_mask.test(rcu)) {
                        TS->setChOperatingMode(TS->convertRcuToChan(rcu), tbb_event.rec_mode[rcu]);
                    }
                }

                TBBModeAckEvent ack;
                for (int32 i = 0; i < TS->maxBoards(); i++) {
                    ack.status_mask[i] = TBB_SUCCESS;
                }
                port.send(ack);
            } else {
                TBBDriverBusyAckEvent ack;
                port.send(ack);
            }
        } break;

        case TBB_CEP_STORAGE: {
            if (TS->activeBoardsMask() != 0) {
                TBBCepStorageEvent tbb_event(event);
                for (int32 rcu = 0; rcu < TS->maxChannels(); rcu++) {
                    if (tbb_event.rcu_mask.test(rcu)) {
                        TS->setDestination(TS->convertRcuToChan(rcu), tbb_event.destination);
                    }
                }
                TBBCepStorageAckEvent tbb_ack;
                for (int32 i = 0; i < TS->maxBoards(); i++) {
                    tbb_ack.status_mask[i] = TBB_SUCCESS;
                }
                port.send(tbb_ack);
            } else {
                TBBDriverBusyAckEvent ack;
                port.send(ack);
            }
        } break;

        default: {
            return(false);
        } break;
    }
    return (true);
}

//-----------------------------------------------------------------------------
bool TBBDriver::addTbbCommandToQueue(GCFEvent& event, GCFPortInterface& port)
{
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
        case TBB_TEMP_LIMIT:
        case TBB_CEP_STORAGE: {
            // put event on the queue
            LOG_DEBUG_STR("Put event on queue");

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
        } break;

        default: {
            return(false);
        } break;
    }
    return(true);
}

// return true if TP-cmd and Driver must go to busy_state
bool TBBDriver::handleTbbCommandFromQueue()
{
    bool tp_cmd;
    GCFEvent::TResult status = GCFEvent::HANDLED;
    GCFEvent* e = itsTbbQueue->front()->event;

    LOG_DEBUG_STR("queue: " << eventName(*e) << "@" << (*itsTbbQueue->front()->port).getName());

    if (SetTbbCommand(e->signal)) {
        // communication with board needed
        status = itsCmdHandler->doEvent(*e,*itsTbbQueue->front()->port);
        tp_cmd = true;
    } else {
        // no communication needed with board
        sendInfo(*e, *itsTbbQueue->front()->port);
        tp_cmd = false;
    }
    // command handled, now delete it from queue
    TbbEvent* tmp = itsTbbQueue->front();
    itsTbbQueue->pop_front();
    (void)status;
    delete e;
    delete tmp;
    return(tp_cmd);
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

//
// requestClockSubscription()
//
void TBBDriver::requestClockSubscription()
{
	LOG_INFO ("Taking subscription on clock settings");

	RSPSubclockEvent		msg;
//	msg.timestamp = 0;
	msg.period = 1;				// let RSPdriver check every second
	itsRSPDriver->send(msg);
}

//
// cancelClockSubscription()
//
void TBBDriver::cancelClockSubscription()
{
	LOG_INFO ("Canceling subscription on clock settings");

	RSPUnsubclockEvent		msg;
	msg.handle = itsClockSubscription;
	itsClockSubscription = 0;
	itsRSPDriver->send(msg);
}

    //} // end namespace TBB
//} // end namespace LOFAR


//-----------------------------------------------------------------------------
// main (argc, argv)
//
int main(int argc, char** argv)
{
    LOFAR::GCF::TM::GCFScheduler::instance()->init(argc, argv, "TBBDriver");    // initializes log system
    GCFScheduler::instance()->disableQueue();                  // run as fast as possible

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

