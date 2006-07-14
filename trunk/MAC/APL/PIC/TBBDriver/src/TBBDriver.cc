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

//# Includes
#include <Common/LofarLogger.h>
#include <TBBDriver.h>
// include all cmd and msg classes
#include <AllocCmd.h>
#include <FreeCmd.h>
#include <RecordCmd.h>
#include <Stop.h>
#include <TrigclrCmd.h>
#include <ReadCmd.h>
#include <UdpCmd.h>
#include <VersionCmd.h>
#include <SizeCmd.h>
#include <ClearCmd.h>
#include <ResetCmd.h>
#include <ConfigCmd.h>
#include <ErasefCmd.h>
#include <ReadfCmd.h>
#include <WritefCmd.h>
#include <ReadwCmd.h>
#include <WritewCmd.h>
#include <TriggerMsg.h>
#include <ErrorMsg.h>


#define ETHERTYPE_TP 0x10FA			// first 4 letters of LOFAr

namespace LOFAR;
namespace TBB;

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
		
TBBDriver::TBBDriver(string name)
  : GCFTask((State)&TBBDriver::initial, name), m_board(0)
{
  // first initialize the global settins
  LOG_DEBUG("Setting up station settings");
  StationSettings*      ssp = StationSettings::instance();
  ssp->setMaxTbbBoards  (GCF::ParamterSet::instance()->getInt("RS.N_TBBBOARDS"));
  ssp->setNrTbbBoards   (GCF::ParamterSet::instance()->getInt("RS.N_RSPBOARDS"));

  if (GET_CONFIG("TBBDriver.OPERATION_MODE", i) == MODE_SUBSTATION) {
	 ssp->setNrTbbBoards(1);
  };
  ASSERTSTR (g_instancenr <= StationSettings::instance()->maxTbbBoards(),
				 "instancenumber larger than MAX_TBBBOARDS");
  ASSERTSTR ((GET_CONFIG("TBBDriver.OPERATION_MODE" ,i) == MODE_SUBSTATION) == 
				 (g_instancenr != -1), 
				 "--instance option does not match OPERATION_MODE setting");
  LOG_DEBUG_STR (*ssp);

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
  m_acceptor.init(*this, "acceptor_v3"+acceptorID, GCFPortInterface::MSPP, TBB_PROTOCOL);

  // open port with TBB board
  LOG_DEBUG("Connecting to TBB boards");
  m_board = new GCFETHRawPort[StationSettings::instance()->nrTbbBoards()];
  ASSERT(m_board);

  //
  // Attempt access of TBBDriver.MAC_BASE, if it fails use the TBBDriver.ADDR0
  // parameters.
  //
  bool bUseMAC_BASE = true;
  try
	 {
	 
		(void)GCF::ParamterSet::instance()->getInt("TBBDriver.MAC_BASE"); 
	 }
  catch (...)
	 {
		bUseMAC_BASE = false;
	 }

  char boardname[64];
  char paramname[64];
  char macaddrstr[64];
  for (int boardid = 0; boardid < StationSettings::instance()->nrTbbBoards(); boardid++)
	 {
		snprintf(boardname, 64, "board%d", boardid);
		
		if (bUseMAC_BASE)
		  {
			 snprintf(macaddrstr, 64, "00:00:00:00:00:%02x", boardid + GCF::ParamterSet::instance()->getInt("TBBDriver.MAC_BASE"));
		  }
		else
		  {
			 snprintf(paramname, 64, "TBBDriver.MAC_ADDR_%d", boardid);
			 strncpy(macaddrstr, GET_CONFIG_STRING(paramname), 64);
		  }

		LOG_DEBUG_STR("initializing board " << boardname << ":" << macaddrstr);
		m_board[boardid].init(*this, boardname, GCFPortInterface::SAP, TP_PROTOCOL,true /*raw*/);
		m_board[boardid].setAddr(GET_CONFIG_STRING("TBBDriver.IF_NAME"), macaddrstr);
		
		// set ethertype to 0x10FA so Ethereal can decode EPA messages
		m_board[boardid].setEtherType(ETHERTYPE_TP);
	 }
	 BoardCmdHandler bch = new BoardCmdHandler();
	 bch->setBoardPorts(m_board);
}


TBBDriver::~TBBDriver()
{
	delete bch; // delete BoardCmdHandler
}

//
// idle(event, port)
//
GCFEvent::TResult TBBDriver::idle_state(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch(event.signal)
	{
		case F_INIT: {
		}	break;
        
		case F_ENTRY:	{
			if(!tp_queue_emty)
			{
				SetTpCommand(event);
				status = ClientAction.dispatch(event,port);
				
				TRAN(TBBDriver::busy);
			}
			
			if(!tbb_queue_emty)
			{
				SetTbbCommand(event);
				status = boardAction.dispatch(event,port);
				
				TRAN(TBBDriver::busy);
			}
		}	break;
        
		case F_CONNECTED:	{
			LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
		}	break;
		
		case F_DATAIN: {
			status = RawEvent::dispatch(*this, port);	
		}	break;
			
		default: {
			if(SetTbbCommand(event))
			{
				status = BoardCmdHandler.dispatch(event,port);
				TRAN(TBBDriver::busy_state);
			}
			else if(SetTpCommand(event))
			{
				status = ClientCmdHandler.dispatch(event,port);	
				TRAN(TBBDriver::busy_state);
			}
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
	
	switch(event.signal)
	{
		case F_ENTRY: {
		}	break;
		
		case F_ACCEPT_REQ: {
		}	break;
		
		case F_CONNECTED:	{
		}	break;
		
		case F_DATAIN: {
			status = RawEvent::dispatch(*this, port);
		}	break;
		
		case TBB_ALLOC:	
		case TBB_FREE:
		case TBB_RECORD: 
		case TBB_STOP:
		case TBB_TRIGCLR:
		case TBB_READ:
		case TBB_UDP:
		case TBB_VERSION:
		case TBB_SIZE:
		case TBB_CLEAR:
		case TBB_RESET:
		case TBB_CONFIG:
		case TBB_ERASEF:
		case TBB_READF:
		case TBB_WRITEF:
		case TBB_READW:
		case TBB_WRITEW:
		{
			// put event on tbb queue
		}	break;	
			
		case TP_TRIGGER:
		case TP_ERROR:
		{
			// put event on tp queue
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
			status = BoardAction.dispatch(event,port); // dispatch ack from boards
		}	break;	
		
		case TBB_TRIGGERACK:
		case TBB_ERRORACK:
		{
			status = ClientAction.dispatch(event,port); // dispatch ack from client
		}	break;		
			
		case F_TIMER:	{
		} break;
		
		case F_DISCONNECTED: {
		}	break;
		
		case F_EXIT: {
		} break;
		
		default: {
			status = GCFEvent::NOT_HANDLED;
		}	break;
	}
	return status;
}


bool TBBDriver::SetTbbCommand(GCFEvent& event)
{
	cmd = 0;
	switch(event.signal)
	{
		case TBB_ALLOC:	{
			AllocCmd *cmd = AllocCmd();
		} break;
		
		case TBB_FREE:	{
			FreeCmd *cmd = FreeCmd();
		} break;
		
		case TBB_RECORD: 	{
			RecordCmd *cmd = RecordCmd();
		} break;
		
		case TBB_STOP:	{
			Stopcmd *cmd = StopCmd();
		} break;
		
		case TBB_TRIGCLR:	{
			TrigclrCmd *cmd = TrigclrCmd();
		} break;
		
		case TBB_READ:	{
			ReadCmd *cmd = ReadCmd();
		} break;
		
		case TBB_UDP: {
			UdpCmd *cmd = UdpCmd();
		} break;
		
		case TBB_VERSION: {
			VersionCmd *cmd = VersionCmd();
		} break;
		
		case TBB_SIZE:	{
			SizeCmd *cmd = SizeCmd();
		} break;
		
		case TBB_CLEAR:	{
			ClearCmd *cmd = ClearCmd();
		} break;
		
		case TBB_RESET:	{
			ResetCmd *cmd = ResetCmd();
		} break;
		
		case TBB_CONFIG:	{
			ConfigCmd *cmd = ConfigCmd();
		} break;
		
		case TBB_ERASEF:	{
			ErasefCmd *cmd = ErasefCmd();
		} break;
		
		case TBB_READF:	{
			ReadfCmd *cmd = ReadfCmd();
		} break;
		
		case TBB_WRITEF:
		{
			WritefCmd *cmd = WritefCmd();
		} break;
		
		case TBB_READW: {
			ReadwCmd *cmd = ReadwCmd();
		} break;
		
		case TBB_WRITEW:	{
			WritewCmd *cmd = WritewCmd();
		} break; 
	}
	if(cmd)
	{
		bch.SetCmd(cmd);
		return true;
	}
	return false;
}


bool TBBDriver::SetTpCommand(GCFEvent& event)
{
	cmd = 0;
	switch(event.signal)
	{
		case TP_TRIGGER: {
			TriggerMsg *cmd = TriggerMsg();
		} break;
		
		case TP_ERROR:	{
			ErrorMsg *cmd = ErrorMsg();
		} break; 
	}
	if(cmd)
	{
		cch.SetCmd(Command cmd);
		return true;
	}
	return false;
}

        
//
// main (argc, argv)
//
int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);    // initializes log system
  
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
  
  LOG_DEBUG ("Reading configuration files");
  try
	 {
		GCF::ParameterSet::instance()->adoptFile("TBBDriverPorts.conf");
		GCF::ParameterSet::instance()->adoptFile("RemoteStation.conf");
	 }
  catch (Exception e)
	 {
		LOG_ERROR_STR("Failed to load configuration files: " << e.text());
		exit(EXIT_FAILURE);
	 }
  
  TBBDriver tbb("TBBDriver");
  
  tbb.start(); // make initial transition
  
  try
	 {
		GCFTask::run();
	 }
  catch (Exception e)
	 {
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

