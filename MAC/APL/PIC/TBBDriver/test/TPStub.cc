//#
//#  TPStub.cc: implementation of TPStub class
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/ParameterSet.h>

#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <math.h>

#include "TPStub.h"
#include "TP_Protocol.ph"
#include "StubRawEvent.h"


using namespace std;
using namespace LOFAR;
using namespace TP_Protocol;
using namespace TBB;
using namespace TBB_Test;

#define ETHERTYPE_TP 0x7BB0	// TBB

TPStub::TPStub(string name)
  : GCFTask((State)&TPStub::initial, name), Test(name)
{
  registerProtocol (TP_PROTOCOL,      TP_PROTOCOL_STRINGS);
	 
  char addrstr[64];
  snprintf(addrstr, 64, "TPStub.MAC_ADDR_TBBDRIVER");

  LOG_INFO("TPStub constructor");
  
  itsServer.init(*this, "tp_server", GCFPortInterface::SPP, TP_PROTOCOL, true /*raw*/);
  itsServer.setAddr(	globalParameterSet()->getString("TPStub.IF_NAME").c_str(),
  									globalParameterSet()->getString(addrstr).c_str());
  itsServer.setEtherType(ETHERTYPE_TP);
}


TPStub::~TPStub()
{
	// TODO
}

GCFEvent::TResult TPStub::initial(GCFEvent &event, GCFPortInterface &port)
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
      	itsServer.open();
    	}
    	break;

    case F_CONNECTED:
			{
				TRAN(TPStub::connected);
			}
			break;

    case F_DISCONNECTED:
			{
				port.setTimer((long)3);
				LOG_WARN(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
				port.close();
			}
			break;

    case F_TIMER:
			{
				// try again
				LOG_DEBUG(formatString("port '%s' retry of open...", port.getName().c_str()));
				itsServer.open();
			}
			break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult TPStub::connected(GCFEvent &event, GCFPortInterface &port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR("Connected and waiting.");	
  switch (event.signal)
  {
    case F_ENTRY:
			{
			} break;

    case F_DATAIN:
			{
				status = RawEvent::dispatch(*this, port); //*this
			} break;

    case TP_ALLOC:
			{
				TPAllocAckEvent allocack(event);
				allocack.opcode = TPALLOC;
				allocack.status = 0;
				port.send(allocack);
			} break;
			
		case TP_FREE:
			{
				TPFreeAckEvent freeack(event);
				freeack.opcode = TPFREE;
				freeack.status = 0;
				port.send(freeack);
			} break;
			
    case TP_RECORD:
			{
				TPRecordAckEvent recordack(event);
				recordack.opcode = TPRECORD;
				recordack.status = 0;
				port.send(recordack);
			} break;
			
    case TP_STOP:
			{
				TPStopAckEvent stopack(event);
				stopack.opcode = TPSTOP;
				stopack.status = 0;
				port.send(stopack);
			} break;
				
    case TP_TRIG_RELEASE:
			{
				TPTrigReleaseAckEvent trigreleaseack(event);
				trigreleaseack.opcode = TPTRIGRELEASE;
				trigreleaseack.status = 0;
				port.send(trigreleaseack);
			} break;
				
    case TP_READ:
			{
				TPReadAckEvent readack(event);
				readack.opcode = TPREAD;
				readack.status = 0;
				port.send(readack);
			} break;
			
    case TP_UDP:
			{
				TPUdpAckEvent udpack(event);
				udpack.opcode = TPUDP;
				udpack.status = 0;
				port.send(udpack);
			} break;
		
		case TP_VERSION:
			{	
				TPVersionAckEvent versionack;
				versionack.opcode = TPVERSION;
				versionack.status = 0x3;
				versionack.boardid = 0;
				versionack.swversion = 10;
				versionack.boardversion = 56;
      	versionack.tpversion = 4;
      	versionack.mp0version = 0;
      	versionack.mp1version = 1;
      	versionack.mp2version = 2;
      	versionack.mp3version = 3;
				
				port.send(versionack);
			} break;
		
		case TP_STATUS:
			{	
				TPStatusAckEvent statusack;
				statusack.opcode = TPSTATUS;
				statusack.status = 0;
				statusack.V12 = 12;
				statusack.V25 = 25;
				statusack.V33 = 33;
      	statusack.Tpcb = 40;
      	statusack.Ttp = 44;
      	statusack.Tmp0 = 1;
      	statusack.Tmp1 = 2;
      	statusack.Tmp2 = 3;
				statusack.Tmp3 = 3;
				
				port.send(statusack);
			} break;
			
		case TP_SIZE:
			{
				TPSizeAckEvent sizeack;
				sizeack.opcode = TPSIZE;
				sizeack.status = 0;
				sizeack.npages = 4000000;
				port.send(sizeack);
			} break;
				
    case TP_CLEAR:
			{
				TPClearAckEvent clearack(event);
				clearack.opcode = TPCLEAR;
				clearack.status = 0;
				port.send(clearack);
			} break;
				
    case TP_RESET:
			{
				TPResetAckEvent resetack(event);
				resetack.opcode = TPRESET;
				resetack.status = 0;
				port.send(resetack);
			} break;
			
    case TP_CONFIG:
			{
				TPConfigAckEvent configack(event);
				configack.opcode = TPCONFIG;
				configack.status = 0;
				port.send(configack);
			} break;
			
    case TP_ERASEF:
			{
				TPErasefAckEvent erasefack(event);
				erasefack.opcode = TPERASEF;
				erasefack.status = 0;
				port.send(erasefack);
			} break;
			
		case TP_READF:
			{
				TPReadfAckEvent readfack(event);
				readfack.opcode = TPREADF;
				readfack.status = 0;
				port.send(readfack);
			} break;
			
    case TP_WRITEF:
			{
				TPWritefAckEvent writefack(event);
				writefack.opcode = TPWRITEF;
				writefack.status = 0;
				port.send(writefack);
			} break; 
    
		case TP_READW:
			{
				TPReadwAckEvent readwack(event);
				readwack.opcode = TPREADW;
				readwack.status = 0;
				port.send(readwack);
			} break;
    
		case TP_WRITEW:
			{
				TPWritewAckEvent writewack(event);
				writewack.opcode = TPWRITEW;
				writewack.status = 0;
				port.send(writewack);
			} break;
		
		case F_DISCONNECTED:
			{
				port.close();
				TRAN(TPStub::initial);
			} break;

    case F_EXIT:
			{
			} break;

    default: {
      LOG_DEBUG_STR("default: unknown command");
      status = GCFEvent::NOT_HANDLED;
    } break;
	}
  return status;
}

GCFEvent::TResult TPStub::final(GCFEvent &event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(event.signal)
  {
    case F_ENTRY:
			{
				GCFTask::stop();
    	} break;

    case F_EXIT:
    	break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void TPStub::run()
{
  start(); // make initial transition
  GCFTask::run();
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);
	
	LOG_DEBUG_STR("Reading configuration files");
  try {
		ConfigLocator cl;
		globalParameterSet()->adoptFile(cl.locate("TPStub.conf"));
	}
	catch (LOFAR::Exception e) {
		LOG_ERROR_STR("Failed to load configuration files: " << e.text());
		//exit(EXIT_FAILURE);
	}
	
  //LOG_INFO(formatString("Program %s has started", argv[0]));

  TPStub stub("TPStub");
  stub.run();

  LOG_INFO("Normal termination of program");

  return 0;
}
