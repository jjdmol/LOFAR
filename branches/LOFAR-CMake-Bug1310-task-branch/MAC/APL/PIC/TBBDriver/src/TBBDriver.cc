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
#include "ArpCmd.h"
#include "ArpModeCmd.h"
#include "StopCepCmd.h"
#include "TempLimitCmd.h"


#define ETHERTYPE_TP 0x7BB0     // letters of TBB

using namespace LOFAR;
using namespace TBB;

static bool   itsDaemonize  = false;
static int32  itsInstancenr = -1;

static const double ALIVECHECKTIME = 60.0;


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
  
  cmd = 0;
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
  itsSetupTimer = new GCFTimerPort(*this, "SetupTimer");
  itsCmdTimer = new GCFTimerPort(*this, "CmdTimer");
  itsSaveTimer = new GCFTimerPort(*this, "SaveTimer");
  
      // create cmd & msg handler
  LOG_DEBUG_STR("initializing handlers");
  cmdhandler = new BoardCmdHandler(itsCmdTimer);
  msghandler = new MsgHandler();
  
  itsResetCount = new int32[TS->maxBoards()];
  memset(itsResetCount,0,sizeof(int32)*TS->maxBoards());
       
  // set Tbb queue
  LOG_DEBUG_STR("initializing TbbQueue");
  itsTbbQueue = new deque<TbbEvent*>(100);
  itsTbbQueue->clear();
   
  itsAlive          = new TPAliveEvent();
  itsAlive->opcode  = TPALIVE;
  itsAlive->status  = 0;
}

//-----------------------------------------------------------------------------
TBBDriver::~TBBDriver()
{
  delete [] itsBoard;
  delete itsAliveTimer;
  delete itsSetupTimer;
  delete cmdhandler;
  delete msghandler;
  delete [] itsResetCount;
  delete itsTbbQueue;
  delete itsAlive;
  if (cmd) delete cmd;
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
      
      if (isEnabled() && !itsAcceptor.isConnected()) {
        itsAcceptor.open();
      }             
      if (itsAcceptor.isConnected()) {
        TRAN(TBBDriver::idle_state);
      }
    } break;
    
    case F_TIMER: {
    } break;
    
    case F_DATAIN: {
    } break;
         
    default: {
      status = GCFEvent::NOT_HANDLED;
    } break;
  }
  return(status);
}


//-----------------------------------------------------------------------------
GCFEvent::TResult TBBDriver::setup_state(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  LOG_DEBUG_STR ("setup_state:" << eventName(event) << "@" << port.getName());
  
  bool setupDone;
  static int* retries;
  static int* waitTimer;
  
  switch(event.signal) {
    case F_INIT: {
    } break;
    
    case F_EXIT: {
      if (retries)   delete [] retries;
      if (waitTimer) delete [] waitTimer;
    } break;
        
    case F_ENTRY: { 
      itsSaveTimer->cancelAllTimers();
      itsAliveTimer->cancelAllTimers();
      retries = new int[TS->maxBoards()];
      waitTimer = new int[TS->maxBoards()];

      for (int board = 0; board < TS->maxBoards(); board++) {
        waitTimer[board] = 0;
        retries[board] = 0;
      }
      LOG_INFO_STR("setting up boards");  
      itsSetupTimer->setTimer(0.0);
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
        msghandler->removeTriggerClient(port);
        msghandler->removeHardwareClient(port);
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
        setupDone = true;
        for (int board = 0; board < TS->maxBoards(); board++) {
      
          if (waitTimer[board]) { waitTimer[board]--; }
          if (waitTimer[board]) { 
            setupDone = false;
            continue; 
          } // board still busy, next board
          
          if ((TS->getBoardState(board) == noBoard) 
              || (TS->getBoardState(board) == boardReady) 
              || (TS->getBoardState(board) == boardError)) {
            continue;
          }
          
          if (TS->getBoardState(board) == setImage1) {
            TPConfigEvent config;
            config.opcode = TPCONFIG;
            config.status = 0;
            config.imagenr = 1;
            //config.imagenr = 0;
            itsBoard[board].send(config);
            itsBoard[board].setTimer(5.0);  
            LOG_INFO_STR("CONFIG 1  is send to port '" << itsBoard[board].getName() << "'");
            waitTimer[board] = 10;
            setupDone = false;
            continue;
          }
          
          if (TS->getBoardState(board) == clearBoard) {
            TPClearEvent clear;
            clear.opcode = TPCLEAR;
            clear.status = 0;
            itsBoard[board].send(clear);
            itsBoard[board].setTimer(5.0);  
            LOG_INFO_STR("CLEAR is send to port '" << itsBoard[board].getName() << "'");
            waitTimer[board] = 5;
            setupDone = false;
            continue;
          }

          if ((TS->getBoardState(board) == freeBoard) 
            || (TS->getBoardState(board) == boardCleared)
            || (TS->getBoardState(board) == image1Set)) {
            TPFreeEvent free;
            free.opcode = TPFREE;
            free.status = 0;
            free.channel = 0xFFFFFFFF;  // send channel = -1 to free all inputs
            itsBoard[board].send(free);
            itsBoard[board].setTimer(5.0);  
            LOG_INFO_STR("FREE -1 is send to port '" << itsBoard[board].getName() << "'");
            waitTimer[board] = 5;
            setupDone = false;
            continue;
          }
          
          if (TS->getBoardState(board) == boardFreed) {
            TPAliveEvent alive;
            alive.opcode = TPALIVE;
            alive.status = 0;
            itsBoard[board].send(alive);
            itsBoard[board].setTimer(1.0);  
            LOG_INFO_STR("ALIVE is send to port '" << itsBoard[board].getName() << "'");
            waitTimer[board] = 1;
            setupDone = false;
            continue;
          }
        }
      
        if (!setupDone) {
          itsSetupTimer->setTimer(1.0); // run this every second
        }
        
      } else { // no setup-timer event, must be a board time-out
        int board = TS->port2Board(&port); // get board nr
        if (TS->getBoardState(board) == setImage1) {
          TS->resetActiveBoard(board);
          //TS->setBoardState(board,image1Set); 
        } else {
          waitTimer[board] = 0;
          retries[board]++;
          if (retries[board] >= TS->maxRetries()) { 
            TS->resetActiveBoard(board);           
            TS->setBoardState(board,boardError);
          }
        }
      }
      
      if (setupDone) {
        LOG_INFO_STR("setting up boards done"); 
        TS->clearBoardSetup();
        if (TS->saveTriggersToFile()) { itsSaveTimer->setTimer(2.0, 2.0); }
        itsAliveTimer->setTimer(ALIVECHECKTIME);  
        TRAN(TBBDriver::idle_state);
      } 
    } break;
    
    case F_DATAIN: {
      status = RawEvent::dispatch(*this, port); 
    } break;
    
    case TP_CONFIG_ACK: {
      int board = TS->port2Board(&port); // get board nr
      itsBoard[board].cancelAllTimers();
      TS->setBoardState(board,image1Set);
      TS->setImageNr(board, 1);
      waitTimer[board] = 15;
    } break;
    
    case TP_CLEAR_ACK: {
      int board = TS->port2Board(&port); // get board nr
      itsBoard[board].cancelAllTimers();
      TS->setBoardState(board,boardCleared);
      waitTimer[board] = 2;
    } break;
    
    case TP_FREE_ACK: {
      int board = TS->port2Board(&port); // get board nr
      itsBoard[board].cancelAllTimers();
        TS->setBoardState(board,boardFreed);
        waitTimer[board] = 2;
    } break;
    
    case TP_ALIVE_ACK: {
      int board = TS->port2Board(&port); // get board nr
      itsBoard[board].cancelAllTimers();
        TS->setBoardState(board,boardReady);
        LOG_INFO_STR("'" << itsBoard[board].getName() << "' is Ready");
        waitTimer[board] = 0;
    } break;
  
    case TP_TRIGGER: {
      for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
        if (&port == &TS->boardPort(boardnr)) {
          msghandler->sendTrigger(event,boardnr);
          break;
        }
      }
    } break;              
                
    case TBB_GET_CONFIG:             
    case TBB_RCU_INFO:               
    case TBB_SUBSCRIBE:         
    case TBB_UNSUBSCRIBE:            
    case TBB_ALLOC:                  
    case TBB_FREE:                   
    case TBB_RECORD:                
    case TBB_STOP:                   
    case TBB_TRIG_RELEASE:         
    case TBB_TRIG_GENERATE:          
    case TBB_TRIG_SETUP:             
    case TBB_TRIG_COEF:             
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
    case TBB_TEMP_LIMIT: {               
      TBBDriverBusyAckEvent ack;    
      port.send(ack);               
    } break;                        
                                    
    default: {                      
      status = GCFEvent::NOT_HANDLED;
    } break;                        
  }                                 
                                    
  return(status);                   
}                                   
                                    
//-----------------------------------------------------------------------------
// idle(event, port)                
//                                  
GCFEvent::TResult TBBDriver::idle_state(GCFEvent& event, GCFPortInterface& port)
{                                   
  GCFEvent::TResult status = GCFEvent::HANDLED;
  LOG_DEBUG_STR("idle:" << eventName(event) << "@" << port.getName());
                                    
  switch(event.signal) {            
    case F_INIT: {                  
    } break;                        
                                    
    case F_EXIT: {                  
    } break;                        
                                    
    case F_ENTRY: {                 
      LOG_DEBUG_STR("Entering Idle State");
      // if no tbb boards start check-alive
      if (TS->activeBoardsMask() == 0) {
        itsAliveTimer->setTimer(0.0);
        break;                      
      }                             
                                    
      // if board setup needed
      if (TS->boardSetupNeeded()) {      
        LOG_DEBUG_STR("need boards setup");
        TRAN(TBBDriver::setup_state);
        break;
      }
  
      if (itsClientList.empty()) {
        itsTbbQueue->clear();
        LOG_DEBUG_STR("Client list is empty, the queue is cleared");
      }
      
      // look if there is an Tbb command in queue 
      if (!itsTbbQueue->empty()) {
        LOG_DEBUG_STR("The queue is NOT empty");
      
        uint8* bufptr = new uint8[sizeof(GCFEvent) + itsTbbQueue->front()->length];
        
        GCFEvent* e = new (bufptr) GCFEvent;
        memcpy(e, &(itsTbbQueue->front()->event), itsTbbQueue->front()->length);
        
        
        LOG_DEBUG_STR("queue:" << eventName(*e));
        
        if (SetTbbCommand(e->signal)) {
          status = cmdhandler->dispatch(*e,*itsTbbQueue->front()->port);
          // delete e;

          TbbEvent* tmp = itsTbbQueue->front();
          itsTbbQueue->pop_front();
          delete tmp;

          TRAN(TBBDriver::busy_state);
        } else {
          sendInfo(*e, *itsTbbQueue->front()->port);
          
          TbbEvent* tmp = itsTbbQueue->front();
          itsTbbQueue->pop_front();
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
        msghandler->removeTriggerClient(port);
        msghandler->removeHardwareClient(port);
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
      if (&port == itsSaveTimer) {
        msghandler->saveTriggerMessage();
        break;
      }
      
      if (&port == itsAliveTimer) {
        CheckAlive(event, port);
      }
      // if new boards detected set them up
      if (itsAliveCheck == false && TS->boardSetupNeeded()) {
        LOG_DEBUG_STR("need boards setup");
        TRAN(TBBDriver::setup_state);
      }
    } break;
    
    case TP_ALIVE_ACK: {
      LOG_DEBUG_STR("TP_ALIVE_ACK received");
      if (itsAliveCheck) { CheckAlive(event, port); }
    } break;
        
    case TBB_SUBSCRIBE: {
      TBBSubscribeEvent subscribe(event);
      if (subscribe.triggers) { msghandler->addTriggerClient(port); }
      if (subscribe.hardware) { msghandler->addHardwareClient(port); }
      
      TBBSubscribeAckEvent subscribeack;
      port.send(subscribeack);
    } break;
    
    case TBB_UNSUBSCRIBE: {
      msghandler->removeTriggerClient(port);
      msghandler->removeHardwareClient(port);
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
    
    case TBB_GET_CONFIG:
    case TBB_RCU_INFO:
    case TBB_TRIG_SETTINGS: {
      sendInfo(event, port);
    } break;
    
    default: {
      // look if the event is a Tbb event
      if (SetTbbCommand(event.signal)) {
        if (TS->activeBoardsMask() != 0) {    
          itsAliveTimer->cancelAllTimers();
          itsAliveTimer->setTimer(ALIVECHECKTIME);
          itsAliveCheck = false;
          status = cmdhandler->dispatch(event,port);
          TRAN(TBBDriver::busy_state);
        } else {
          TBBDriverBusyAckEvent ack;    
          port.send(ack); 
        }
      } else {
        // if not a Tbb event, return not-handled 
        status = GCFEvent::NOT_HANDLED;
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
      if (cmd) delete cmd;
      itsAliveTimer->setTimer(1.0);
      itsAliveCheck = false;
    } break;
    
    case F_ENTRY: {
      if (cmdhandler->tpCmdDone()) {
        TRAN(TBBDriver::idle_state);
      }
    } break;
        
    case F_ACCEPT_REQ: {
      GCFTCPPort* client = new GCFTCPPort();
      client->init(*this, "client", GCFPortInterface::SPP, TBB_PROTOCOL);
        itsAcceptor.accept(*client);
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
        msghandler->removeTriggerClient(port);
        msghandler->removeHardwareClient(port);
        }
      
      TRAN(TBBDriver::idle_state);  
    } break;
    
    case F_TIMER: {
      if (&port == itsSaveTimer) {
        msghandler->saveTriggerMessage();
        break;
      }
      
      if (&port == itsAliveTimer) { 
        if (itsAliveCheck) {
          CheckAlive(event, port);// if true, then all boards are checked
        } else {
          itsAliveTimer->setTimer(ALIVECHECKTIME);
        }
      } else {
        status = cmdhandler->dispatch(event,port); // dispatch time-out event 
      }
    } break;
    
    case F_DATAIN: {
      status = RawEvent::dispatch(*this, port);
    } break;
    
    case TP_ALIVE_ACK: {
      LOG_DEBUG_STR("TP_ALIVE_ACK received");
      if (itsAliveCheck) { CheckAlive(event, port); }
    } break;
        
    case TBB_SUBSCRIBE: {
      TBBSubscribeEvent subscribe(event);
      if (subscribe.triggers) { msghandler->addTriggerClient(port); }
      if (subscribe.hardware) { msghandler->addHardwareClient(port); }
      
      TBBSubscribeAckEvent subscribeack;
      port.send(subscribeack);
    } break;
    
    case TBB_UNSUBSCRIBE: {
      msghandler->removeTriggerClient(port);
      msghandler->removeHardwareClient(port);
      if (cmdhandler->tpCmdDone()) {
        TRAN(TBBDriver::idle_state);
      }
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
    case TP_ARP_ACK:
    case TP_ARP_MODE_ACK:
    case TP_STOP_CEP_ACK:
    case TP_TEMP_LIMIT_ACK: 
    {
      status = cmdhandler->dispatch(event,port); // dispatch ack from boards  
      if (cmdhandler->tpCmdDone()) {
        TRAN(TBBDriver::idle_state);
      }
    } break;  
    
    case TBB_GET_CONFIG:             
    case TBB_RCU_INFO:               
    case TBB_ALLOC:                  
    case TBB_FREE:                   
    case TBB_RECORD:                
    case TBB_STOP:                   
    case TBB_TRIG_RELEASE:         
    case TBB_TRIG_GENERATE:          
    case TBB_TRIG_SETUP:             
    case TBB_TRIG_COEF:             
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
    case TBB_TEMP_LIMIT: {
      // put event on the queue
      LOG_DEBUG_STR("Put event on queue");
          
      uint8* bufptr = new uint8[sizeof(GCFPortInterface*) 
                              + sizeof(uint32)
                              + sizeof(GCFEvent) 
                              + event.length];
      TbbEvent* tbbevent = new (bufptr) TbbEvent;
      
      memcpy(&(tbbevent->event), &event, (sizeof(GCFEvent) + event.length));      
      tbbevent->length = sizeof(GCFEvent) + event.length;
      tbbevent->port = &port;
      
      itsTbbQueue->push_back(tbbevent);
      if (cmdhandler->tpCmdDone()) {
        TRAN(TBBDriver::idle_state);
      } 
    } break;                
    
    default: {
      LOG_DEBUG_STR("DEFAULT");
      
      status = GCFEvent::NOT_HANDLED;
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
  int32 boardnr;
  static uint32 activeboards;
  static uint32 sendmask;
  
  
  //itsAliveTimer->cancelAllTimers();
  if (!itsAliveCheck) {
    itsAliveCheck = true;
    //itsNewBoards  = 0;
    boardnr     = 0;
    sendmask      = 0;
    activeboards  = 0;
    
    itsAliveTimer->cancelAllTimers();
    // mask all boards to check 
    for(int nr = 0; nr < TS->maxBoards(); nr++) {
      itsBoard[nr].send(*itsAlive);
      sendmask |= (1 << nr);
    }
    // 1 timer for al events
    itsAliveTimer->setTimer(2.0);
  } else {
    
    if (event.signal == TP_ALIVE_ACK) {
      boardnr = TS->port2Board(&port);
      if (boardnr != -1) {
        activeboards |= (1 << boardnr);
          LOG_INFO_STR("TP_ALIVE_ACK " << activeboards << " <== activeboards mask");
        
        TPAliveAckEvent ack(event);
        // board is reset
        if (ack.resetflag == 0) {
          TS->clearRcuSettings(boardnr);
          if (TS->getFreeToReset(boardnr)) {
            TS->setImageNr(boardnr, 0);
            TS->setBoardState(boardnr,setImage1);
          } else {
            // new image loaded
            TS->setFreeToReset(boardnr, true);
            TS->setBoardState(boardnr,clearBoard);
          }
               itsResetCount[boardnr]++;
               LOG_INFO_STR("=BOARD-RESET=, TBB board " << boardnr << " has been reset " << itsResetCount[boardnr] << " times");
        } else if ((TS->activeBoardsMask() & (1 << boardnr)) == 0) { // new board
          TS->clearRcuSettings(boardnr);
          if (TS->getFreeToReset(boardnr)) {
            TS->setImageNr(boardnr, 0);
            TS->setBoardState(boardnr,setImage1);
          } else {
            // new image loaded
            TS->setFreeToReset(boardnr, true);
            TS->setBoardState(boardnr,clearBoard);
          }
              LOG_INFO_STR("=NEW_BOARD=, TBB board " << boardnr << " is new");
            }
      }
    }
    
    if ((event.signal == F_TIMER) || (activeboards == sendmask )) {
      if (activeboards == sendmask) {
        itsAliveTimer->cancelAllTimers();
      }
      
      for (int board = 0; board < TS->maxBoards(); board++) {
        if (~activeboards & (1 << board)) { // look for not active boards
          TS->setBoardState(board, noBoard);      
        }
      } 
      
      if (activeboards != TS->activeBoardsMask()) {
        TS->setActiveBoardsMask(activeboards);
        
        //itsActiveBoardsChange = true;
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
        msghandler->sendBoardChange(TS->activeBoardsMask());
        LOG_INFO_STR("Available TBB boards changed:" << boardstr);  
      }
  
      LOG_DEBUG_STR("Active TBB boards check");
      if (TS->activeBoardsMask() == 0) {
        itsAliveTimer->setTimer(5.0);
      } else {
        itsAliveTimer->setTimer(ALIVECHECKTIME);
      }
      itsAliveCheck = false;
      //done = true;
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
      if (TS->activeBoardsMask() != 0) {
        TBBGetConfigAckEvent ack;
        ack.max_boards = TS->maxBoards();
        ack.active_boards_mask = TS->activeBoardsMask();
            port.send(ack);
      } else {
        TBBDriverBusyAckEvent ack;    
        port.send(ack); 
      } 
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
        int rcu;
        for (int32 ch = 0; ch < TS->maxChannels(); ch++) {
          rcu = TS->getChRcuNr(ch);
          
          ack.setup[rcu].level = TS->getChTriggerLevel(ch);
          ack.setup[rcu].start_mode = TS->getChTriggerStartMode(ch);
          ack.setup[rcu].stop_mode = TS->getChTriggerStopMode(ch);
          ack.setup[rcu].filter_select = TS->getChFilterSelect(ch);
          ack.setup[rcu].window = TS->getChDetectWindow(ch);
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
bool TBBDriver::SetTbbCommand(unsigned short signal)
{
  if (cmd) {
    //cmdhandler->setTpCmd(0);
    delete cmd;
  }
  switch(signal)
  {
    case TBB_ALLOC: {
      AllocCmd *cmd;
      cmd = new AllocCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_FREE: {
      FreeCmd *cmd;
      cmd = new FreeCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_RECORD:  {
      RecordCmd *cmd;
      cmd = new RecordCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_STOP:  {
      StopCmd *cmd;
      cmd = new StopCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_TRIG_RELEASE:  {
      TrigReleaseCmd *cmd;
      cmd = new TrigReleaseCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_TRIG_GENERATE: {
      TrigGenCmd *cmd;
      cmd = new TrigGenCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_TRIG_SETUP:  {
      TrigSetupCmd *cmd;
      cmd = new TrigSetupCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_TRIG_COEF: {
      TrigCoefCmd *cmd;
      cmd = new TrigCoefCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_TRIG_INFO: {
      TrigInfoCmd *cmd;
      cmd = new TrigInfoCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_READ:  {
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
    
    case TBB_SIZE:  {
      SizeCmd *cmd;
      cmd = new SizeCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_STATUS:  {
      StatusCmd *cmd;
      cmd = new StatusCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_CLEAR: {
      ClearCmd *cmd;
      cmd = new ClearCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_RESET: {
      ResetCmd *cmd;
      cmd = new ResetCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_CONFIG:  {
      ConfigCmd *cmd;
      cmd = new ConfigCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_ERASE_IMAGE: {
      ErasefCmd *cmd;
      cmd = new ErasefCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_READ_IMAGE:  {
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
    
    case TBB_WRITEW:  {
      WritewCmd *cmd;
      cmd = new WritewCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_READR: {
      ReadrCmd *cmd;
      cmd = new ReadrCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_WRITER:  {
      WriterCmd *cmd;
      cmd = new WriterCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_READX: {
      ReadxCmd *cmd;
      cmd = new ReadxCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_ARP: {
      ArpCmd *cmd;
      cmd = new ArpCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_ARP_MODE:  {
      ArpModeCmd *cmd;
      cmd = new ArpModeCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_STOP_CEP:  {
      StopCepCmd *cmd;
      cmd = new StopCepCmd();
      cmdhandler->setTpCmd(cmd);
    } break;
    
    case TBB_TEMP_LIMIT:  {
      TempLimitCmd *cmd;
      cmd = new TempLimitCmd();
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
	LOFAR::GCF::TM::GCFTask::init(argc, argv, "TBBDriver");    // initializes log system
  
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
    LOFAR::GCF::TM::GCFTask::run();
  }
  catch (LOFAR::Exception& e) {
    LOG_ERROR_STR("Exception: " << e.text());
    exit(EXIT_FAILURE);
  }

	LOG_INFO("Normal termination of program");

	return(0);
}

