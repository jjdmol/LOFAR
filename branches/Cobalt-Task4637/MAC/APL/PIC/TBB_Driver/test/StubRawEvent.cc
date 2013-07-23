//#
//#  RawEvent.cc: implementation of the RawEvent class.
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
//#  $Id: RawEvent.cc,v 1.5 2006/10/20 12:27:50 donker Exp $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <GCF/TM/GCF_ETHRawPort.h>

#include "TP_Protocol.ph" 

#include "RawEvent.h"

namespace LOFAR {
	using namespace TP_Protocol;	
	namespace TBB {


static const uint8 OPCODE_LEN = 4;

//
// GCFEvent must be 4byte aligned
//
typedef struct {
	GCFEvent  event;
	uint32		opcode;
	uint8     payload[ETH_DATA_LEN - sizeof(uint32)];
} RawFrame;
static RawFrame buf;


GCFEvent::TResult RawEvent::dispatch(GCFTask& task, GCFPortInterface& port)
{
  
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  
  // Receive a raw packet
  //ssize_t size = port.recv(buf.payload, ETH_DATA_LEN);
  ssize_t size = port.recv(&buf.opcode, ETH_DATA_LEN);
  
	// at least 4 bytes
  if (size < OPCODE_LEN) return(GCFEvent::NOT_HANDLED);
  
	
  LOG_DEBUG(formatString("in RawEvent::F_DATAIN: Opcode=0x%08x",buf.opcode));
 
  //
  // If no error, lookup buf.opcode number, else assign _ERROR buf.event.signal number
  //
  switch(buf.opcode)
	 {
	 case oc_ALLOC:
  		buf.event.signal = TP_ALLOC;
  		buf.event.length = 8;
  		break;
	 case oc_FREE:
  		buf.event.signal = TP_FREE;
  		buf.event.length = 8;
  		break;
	 case oc_RECORD:
  		buf.event.signal = TP_RECORD;
  		buf.event.length = 8;
  		break;
	 case oc_STOP:
  		buf.event.signal = TP_RECORD;
  		buf.event.length = 8;
  		break;
	 case oc_TRIGGER:
  		buf.event.signal = TP_TRIGGER;
  		buf.event.length = 20;
  		break;
	 case oc_TRIG_RELEASE:
  		buf.event.signal = TP_TRIG_RELEASE;
  		buf.event.length = 8;
  		break;
	 case oc_READ:
  		buf.event.signal = TP_READ;
  		buf.event.length = 8;
  		break;
	 case oc_UDP:
  		buf.event.signal = TP_UDP;
  		buf.event.length = 8;
  		break;
	 case oc_PAGE_PERIOD:
  		buf.event.signal = TP_PAGEPERIOD;
  		buf.event.length = 12;
  		break;
  		
	 case oc_VERSION:
  		buf.event.signal = TP_VERSION;
  		buf.event.length = 40;
  		break;
	 case oc_SIZE:
  		buf.event.signal = TP_SIZE;
  		buf.event.length = 8;
  		break;
   case oc_STATUS:
  		buf.event.signal = TP_STATUS;
  		buf.event.length = 44;
  		break;
	 case oc_ERROR:
  		buf.event.signal = TP_ERROR;
  		buf.event.length = 12;
  		break;
	 case oc_CLEAR:
  		buf.event.signal = TP_CLEAR;
  		buf.event.length = 8;
  		break;
	 case oc_RESET:
  		buf.event.signal = TP_RESET;
  		buf.event.length = 8;
  		break;
	 case oc_CONFIG:
  		buf.event.signal = TP_CONFIG;
  		buf.event.length = 8;
  		break;
	 case oc_ERASEF:
  		buf.event.signal = TP_ERASEF;
  		buf.event.length = 8;
  		break;
	 case oc_READF:
  		buf.event.signal = TP_READF;
  		buf.event.length = 1032;
  		break;
	 case oc_WRITEF:
  		buf.event.signal = TP_WRITEF;
  		buf.event.length = 8;
  		break;
	 case oc_READW:
  		buf.event.signal = TP_READW;
  		buf.event.length = 16;
  		break;
	 case oc_WRITEW:
  		buf.event.signal = TP_WRITEW;
  		buf.event.length = 8;
  		break;
	 case oc_READR:
  		buf.event.signal = TP_READR;
  		buf.event.length = 2056;
  		break;
   case oc_WRITER:
  		buf.event.signal = TP_WRITER;
  		buf.event.length = 8;
  		break;
   case oc_READX:
  		buf.event.signal = TP_READX;
  		buf.event.length = 1032;
  		break;	
   case oc_ALIVE:
  		buf.event.signal = TP_ALIVE;
  		buf.event.length = 12;
  		break;	
  		
	 default:
  		buf.event.signal = 0;
			buf.event.length = 0;
  		break;
	 }
  
  if (buf.event.signal) // buf.event.signal == 0 indicates unrecognised or invalid MEP message
	{				
		//
		// Print debugging info
		// 
		LOG_DEBUG(formatString("%s receives '%s' on port '%s'",
							task.getName().c_str(),
							eventName(buf.event).c_str(),
							port.getName().c_str()));
				
		//
		// dispatch the TP message as a GCFEvent (which it now is)
		//
		status = task.doEvent(buf.event, port);
	}
  else
	{
		LOG_WARN("F_DATAIN: Discarding unknown message.");
	}
  
  return(status);
}

	} // end namespace TBB
} // end namespace LOFAR
