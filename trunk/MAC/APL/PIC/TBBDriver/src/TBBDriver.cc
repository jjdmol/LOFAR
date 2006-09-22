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

#include "TBBDriver.h"
#include "RawEvent.h" 

// include all cmd and msg classes
#include "AllocCmd.h"
#include "FreeCmd.h"
#include <RecordCmd.h>
#include <StopCmd.h>
#include <TrigclrCmd.h>
#include <ReadCmd.h>
#include <UdpCmd.h>
#include "VersionCmd.h"
#include <SizeCmd.h>
#include <ClearCmd.h>
#include <ResetCmd.h>
#include <ConfigCmd.h>
#include <ErasefCmd.h>
#include <ReadfCmd.h>
#include <WritefCmd.h>
// #include <ReadwCmd.h>
// #include <WritewCmd.h>
// #include <TriggerMsg.h>
// #include <ErrorMsg.h>


#define ETHERTYPE_TP 0x7BB0			// letters of TBB

using namespace LOFAR;
using namespace GCFCommon;
using namespace ACC::APS;
using namespace TBB;

static int32    g_instancenr = -1;


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
TBBDriver::TBBDriver(string name)
  : GCFTask((State)&TBBDriver::init_state, name)
{
	cmd = 0;

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
	itsBoard = new GCFETHRawPort[globalParameterSet()->getInt32("RS.N_TBBBOARDS")];
  //itsBoard = new GCFETHRawPort[tbb_ss->nrTbbBoards()];
  ASSERT(itsBoard);
	
	itsMsgPort.init(*this, "MsgPort", GCFPortInterface::SAP, TBB_PROTOCOL);
	// TODO
	//itsMsgPort.setAddr(globalParameterSet()->getString("TBBDriver.IF_NAME").c_str(), macaddrstr);
	
  //
  // Attempt access of TBBDriver.MAC_BASE, if it fails use the TBBDriver.ADDR0
  // parameters.
  //
  bool bUseMAC_BASE = true;
  try {
		(void)globalParameterSet()->getInt32("TBBDriver.MAC_BASE"); 
	}
  catch (...) {
		bUseMAC_BASE = false;
	}

  char boardname[64];
  char paramname[64];
  char macaddrstr[64];
	for (int boardid = 0; boardid < globalParameterSet()->getInt32("RS.N_TBBBOARDS"); boardid++) {
		snprintf(boardname, 64, "board%d", boardid);
		
		if (bUseMAC_BASE) {
			snprintf(macaddrstr, 64, "10:FA:00:00:%02x:02", boardid + globalParameterSet()->getInt32("TBBDriver.MAC_BASE"));
		}
		else {
			snprintf(paramname, 64, "TBBDriver.MAC_ADDR_%d", boardid);
			strcpy(macaddrstr, globalParameterSet()->getString(paramname).c_str());
		}

		LOG_DEBUG_STR("initializing board " << boardname << ":" << macaddrstr);
		itsBoard[boardid].init(*this, boardname, GCFPortInterface::SAP, TP_PROTOCOL,true /*raw*/);
		itsBoard[boardid].setAddr(globalParameterSet()->getString("TBBDriver.IF_NAME").c_str(), macaddrstr);
		
		// set ethertype to 0x7BB0 so Ethereal can decode TBB messages
		itsBoard[boardid].setEtherType(ETHERTYPE_TP);
	}
	 
	 // create cmd & msg handler
	LOG_DEBUG_STR("initializing handlers");
	 cmdhandler = new BoardCmdHandler();
	 msghandler = new ClientMsgHandler(itsMsgPort);
	 
	 cmdhandler->setBoardPorts(itsBoard);
	 cmdhandler->setMaxTbbBoards(MAX_N_TBBBOARDS);
	 cmdhandler->setNrOfTbbBoards(globalParameterSet()->getInt32("RS.N_TBBBOARDS"));
	 cmdhandler->setTpRetries(globalParameterSet()->getInt32("TBBDriver.TP_RETRIES"));
	 cmdhandler->setTpTimeOut(globalParameterSet()->getDouble("TBBDriver.TP_TIMEOUT"));
	 	 	 
	 // set Tbb queue
	 LOG_DEBUG_STR("initializing TbbQueue");
	 itsTbbQueue = new deque<TbbEvent>(100);
	 itsTbbQueue->clear();
}


TBBDriver::~TBBDriver()
{
	delete cmdhandler; 
	delete msghandler;
}


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

//
// openBoards()
//
void TBBDriver::openBoards()
{
	LOG_DEBUG_STR("opening boards");
	for (int boardid = 0; boardid < globalParameterSet()->getInt32("RS.N_TBBBOARDS"); boardid++)
  {
    if (!itsBoard[boardid].isConnected()) itsBoard[boardid].open();
  }
}

//
// isEnabled()
//
bool TBBDriver::isEnabled()
{
  bool enabled = true;
	for (int boardid = 0; boardid < globalParameterSet()->getInt32("RS.N_TBBBOARDS"); boardid++)
  {
    if (!itsBoard[boardid].isConnected())
    {
      enabled = false;
      break;
    }
  }
  return enabled;
}


GCFEvent::TResult TBBDriver::init_state(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	
	switch(event.signal) {
		case F_INIT: {
		} break;
        
		case F_ENTRY:	{
			openBoards();
		}	break;
		
		case F_CONNECTED: {
      LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
      
      if (isEnabled()) {
        // All board ports are open, start waiting for clients
      	if (!itsAcceptor.isConnected())
      		itsAcceptor.open();
      }
      if (itsAcceptor.isConnected()) 
      	TRAN(TBBDriver::idle_state);
    } break;
		
		default: {
			status = GCFEvent::NOT_HANDLED;
		}	break;
	}
	return status;
}
	

//
// idle(event, port)
//
GCFEvent::TResult TBBDriver::idle_state(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
  undertaker();
  
	switch(event.signal) {
		case F_INIT: {
		} break;
        
		case F_ENTRY:	{
			// look if there is an Tbb command in queue
			
			if(!itsTbbQueue->empty()) {
				GCFEvent e;
				
				e.signal = itsTbbQueue->front().signal; //tbbevent->signal;
				
				SetTbbCommand(itsTbbQueue->front().signal);
				
				status = cmdhandler->dispatch(e,*itsTbbQueue->front().port);
				
				itsTbbQueue->pop_front();
				TRAN(TBBDriver::busy_state);
			}
		}	break;
        
		case F_CONNECTED:	{
			LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
		}	break;
		
		case F_DISCONNECTED: {
			
			LOG_INFO(formatString("DISCONNECTED: port '%s'", port.getName().c_str()));
      port.close();
      		
			if (&port == &itsAcceptor) {
        LOG_FATAL("Failed to start listening for client connections.");
        exit(EXIT_FAILURE);
      }
      else {
				itsClientList.remove(&port);
        itsDeadClients.push_back(&port);
      }
		} break;
		
		case F_ACCEPT_REQ: {
			GCFTCPPort* client = new GCFTCPPort();
			client->init(*this, "client", GCFPortInterface::SPP, TBB_PROTOCOL);
      itsAcceptor.accept(*client);
      itsClientList.push_back(client);

      LOG_INFO(formatString("NEW CLIENT CONNECTED: %d clients connected", itsClientList.size()));
		} break;
		
		case F_DATAIN: {
			status = RawEvent::dispatch(*this, port);	
		}	break;
			
		default: {
			// look if the event is a Tbb event
			if(SetTbbCommand(event.signal)) {
				status = cmdhandler->dispatch(event,port);
				TRAN(TBBDriver::busy_state);
			}
			// if it is not a Tbb event, look for Tp event
			else if(SetTpCommand(event.signal)) {
				status = msghandler->dispatch(event,port);	
				TRAN(TBBDriver::busy_state);
			}
			// if not a Tbb or Tp event, return not-handled 
			else
				status = GCFEvent::NOT_HANDLED;
		}	break;
	}
	return status;
}

//
// enabled(event, port)
//
GCFEvent::TResult TBBDriver::busy_state(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;    
	
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
			status = cmdhandler->dispatch(event,port); // dispatch time-out event	
		} break;
		
		case F_DISCONNECTED: {
			TRAN(TBBDriver::idle_state);	
		}	break;
		
		case F_EXIT: {
		} break;
		
		case F_DATAIN: {
			status = RawEvent::dispatch(*this, port);
		}	break;
					
		case TP_TRIGGER:
		case TP_ERROR: {
			// look for Tp event
			if(SetTpCommand(event.signal)) {
				status = msghandler->dispatch(event,port);	
			}
		}	break;		
		
		case TP_ALLOC:	
		case TP_FREE:
		case TP_RECORD: 
		case TP_STOP:
		case TP_TRIGCLR:
		case TP_READ:
		case TP_UDP:
		case TP_VERSION:
		case TP_SIZE:
		case TP_CLEAR:
		case TP_RESET:
		case TP_CONFIG:
		case TP_ERASEF:
		case TP_READF:
		case TP_WRITEF:
		case TP_READW:
		case TP_WRITEW:
		{
			status = cmdhandler->dispatch(event,port); // dispatch ack from boards
			if(cmdhandler->tpCmdDone()){
				TRAN(TBBDriver::idle_state);
			}
		}	break;	
		
		case TBB_TRIGGERACK:
		case TBB_ERRORACK:
		{
			status = cmdhandler->dispatch(event,port); // dispatch ack from client
		}	break;		
						
		default: {
			if(cmdhandler->tpCmdDone()){
				TRAN(TBBDriver::idle_state);
			}
			// put event on the queue
			TbbEvent tbbevent;
			tbbevent.signal = event.signal;
			tbbevent.port = &port;
			itsTbbQueue->push_back(tbbevent);
			status = GCFEvent::NOT_HANDLED;
		}	break;
	}
	return status;
}


bool TBBDriver::SetTbbCommand(unsigned short signal)
{
	if(cmd) delete cmd;
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
		/*
		case TBB_RECORD: 	{
			RecordCmd *cmd;
			cmd = new RecordCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_STOP:	{
			Stopcmd *cmd;
			cmd = new StopCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_TRIGCLR:	{
			TrigclrCmd *cmd;
			cmd = new TrigclrCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_READ:	{
			ReadCmd *cmd;
			cmd = new ReadCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_UDP: {
			UdpCmd *cmd;
			cmd = new UdpCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		*/
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
		
		case TBB_ERASEF:	{
			ErasefCmd *cmd;
			cmd = new ErasefCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_READF:	{
			ReadfCmd *cmd;
			cmd = new ReadfCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		
		case TBB_WRITEF:
		{
			WritefCmd *cmd;
			cmd = new WritefCmd();
			cmdhandler->setTpCmd(cmd);
		} break;
		/*
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
		*/ 
		default: {
			return false;
		}
	}
	return true;
}


bool TBBDriver::SetTpCommand(unsigned short signal)
{
	switch(signal) {
		/*
		case TP_TRIGGER: {
			TriggerMsg *msg = TriggerMsg();
			msghandler->SetTBBMsg(msg);
		} break;
		
		case TP_ERROR:	{
			ErrorMsg *msg = ErrorMsg();
			msghandler->SetTBBMsg(msg);
		} break;
		*/ 
		default: {
			return false;
		}
	}
	return true;
}

	//} // end namespace TBB
//} // end namespace LOFAR

        
//
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
		LOFAR::ACC::APS::globalParameterSet()->adoptFile(cl.locate("RemoteStation.conf"));
		LOFAR::ACC::APS::globalParameterSet()->adoptFile(cl.locate("TBBDriver.conf"));
	}
	catch (LOFAR::Exception e) {
		LOG_ERROR_STR("Failed to load configuration files: " << e.text());
		//exit(EXIT_FAILURE);
	}
  
	LOFAR::TBB::TBBDriver tbb("TBBDriver");
  
	tbb.start(); // make initial transition
  
  try {
		LOFAR::GCF::TM::GCFTask::run();
	}
	catch (LOFAR::Exception e) {
		LOG_ERROR_STR("Exception: " << e.text());
		exit(EXIT_FAILURE);
	}
  
  LOG_INFO("Normal termination of program");
  
  return 0;
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

