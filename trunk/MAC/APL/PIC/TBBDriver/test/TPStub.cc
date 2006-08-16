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

#include "TP_Protocol.ph"
#include <RawEvent.h>
#include <TPStub.h>
#include <APL/RTCCommon/PSAccess.h>
#include <GCF/ParameterSet.h>

#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <math.h>

using namespace std;
using namespace LOFAR;
using namespace TBB_Test;

#define ETHERTYPE_TP 0x10FA

TPStub::TPStub(string name)
  : GCFTask((State)&TPStub::initial, name), Test(name)
{
  registerProtocol(TP_PROTOCOL, TP_PROTOCOL_signalnames);

  char addrstr[64];
  snprintf(addrstr, 64, "TPStub.MAC_ADDR_TBBDRIVER");

  LOG_INFO("TPStub constructor");
  
  m_client.init(*this, "client", GCFPortInterface::SAP, TP_PROTOCOL, true /*raw*/);
  m_client.setAddr(GET_CONFIG_STRING("TPStub.IF_NAME"),
		   GET_CONFIG_STRING(addrstr));
  m_client.setEtherType(ETHERTYPE_TP);
}

TPStub::~TPStub()
{
	// TODO
}

GCFEvent::TResult TPStub::initial(GCFEvent& e, GCFPortInterface& port)
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
      	m_client.open();
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
				m_client.open();
			}
			break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}


GCFEvent::TResult TPStub::connected(GCFEvent& ack, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
		
  switch (ack.signal)
  {
    case F_ENTRY:
			{
			}
    break;

    case F_DATAIN:
			{
				status = RawEvent::dispatch(*this, port);
			}
    break;

    case TP_ALLOC:
			{
				TPAllocEvent allocack;
				allocack.opcode = TPALLOC;
				allocack.channel = ack.channel;
				allocack.pageaddr = ack.pageaddr;
				allocack.pagelength = ack.pagelength;
				port.send(allocack);
			}
			break;
			
		case TP_FREE:
			{
				TPFreeEvent freeack;
				freeack.opcode = TPFREE;
				freeack.channel = ack.channel;
				port.send(freeack);
			}
			break;
			
    case TP_RECORD:
			{
				TPRecordEvent recordack;
				recordack.opcode = TPRECORD;
				recordack.channel = ack.channel;
				port.send(recordack);
			}
			break;
			
    case TP_STOP:
			{
				TPStopEvent stopack;
				stopack.opcode = TPSTOP;
				stopack.channel = ack.channel;
				port.send(stopack);
			}
			break;
				
    case TP_TRIGCLR:
			{
				TPTrigclrEvent trigclrack;
				trigclrack.opcode = TPTRIGCLR;
				trigclrack.channel = ack.channel;
				port.send(trigclrack);
			}
			break;
				
    case TP_READ:
			{
				TPReadEvent readack;
				readack.opcode = TPREAD;
				readack.channel = ack.channel;
				readack.time = ack.time;
				readack.period = ack.period;
				port.send(readack);
			}
			break;
			
    case TP_UDP:
			{
				TPUdpEvent udpack;
				udpack.opcode = TPUDP;
				udpack.udp[0] = ack.udp[0];
				udpack.udp[1] = ack.udp[1];
				udpack.ip[0] = ack.ip[0];
				udpack.ip[1] = ack.ip[1];
				udpack.ip[2] = ack.ip[2];
				udpack.ip[3] = ack.ip[3];
				udpack.ip[4] = ack.ip[4];
				udpack.mac[0] = ack.mac[0];
				udpack.mac[1] = ack.mac[1];
				port.send(udpack);
			}
			break;
		
		case TP_VERSION:
			{
				TPVersionEvent versionack;
				versionack.opcode = TPVERSION;
				versionack.boardid = 1;
				versionack.swversion = 1;
				versionack.boardversion = 1;
				versionack.tp_version = 1;
				versionack.mp_version[0] = 2;
				versionack.mp_version[1] = 2;
				versionack.mp_version[2] = 2;
				versionack.mp_version[3] = 2;
				port.send(versionack);
			}
			break;
		
		case TP_SIZE:
			{
				TPSizeEvent sizeack;
				sizeack.opcode = TPSIZE;
				sizeack.size = 8;
				port.send(sizeack);
			}  
			break;
				
    case TP_CLEAR:
			{
				TPClearEvent clearack;
				clearack.opcode = TPCLEAR;
				port.send(clearack);
			}
			break;
				
    case TP_RESET:
			{
				TPResetEvent resetack;
				resetack.opcode = TPRESET;
				port.send(resetack);
			}
			break;
			
    case TP_CONFIG:
			{
				TPConfigEvent configack;
				configack.opcode = TPCONFIG;
				configack.image = ack.image;
			}
			break;
			
    case TP_ERASEF:
			{
				TPErasefEvent erasefack;
				erasefack.opcode = TPERASEF;
				erasefack.addr = ack.addr;
			}
			break;
			
		case TP_READF:
			{
				TPReadfEvent readfack;
				readfack.opcode = TPREADF;
				readfack.addr = ack.addr;
				readfack.data = ack.data;
				port.send(readfack);
			}
			break;
			
    case TP_WRITEF:
			{
				TPWritefEvent writefack;
				writefack.opcode = TPWRITEF;
				writefack.addr = ack.addr;
				writefack.data = ack.data;
				port.send(writefack);
			}
			break; 
    
		case TP_READW:
			{
				TPReadwEvent readwack;
				readwack.opcode = TPREADW;
				readwack.mp = ack.mp;
				readwack.addr = ack.addr;
				readwack.wordlo = ack.wordlo;
				readwack.wordhi = ack.wordhi;
				port.send(readwack);
			}
			break;
    
		case TP_WRITEW:
			{
				TPWritewEvent writewack;
				writewack.opcode = TPWRITEW;
				writewack.mp = ack.mp;
				writewack.addr = ack.addr;
				writewack.wordlo = ack.wordlo;
				writewack.wordhi = ack.wordhi;
				port.send(writewack);
			}
			break;
		
		case F_DISCONNECTED:
			{
				port.close();
				TRAN(TPStub::initial);
			}
			break;

    case F_EXIT:
			{
			}
			break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
	}
  return status;
}

GCFEvent::TResult TPStub::final(GCFEvent& ack, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(ack.signal)
  {
    case F_ENTRY:
			{
				GCFTask::stop();
    	}
			break;

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

  LOG_INFO(formatString("Program %s has started", argv[0]));

  TPStub stub("TPStub");
  stub.run();

  LOG_INFO("Normal termination of program");

  return 0;
}
