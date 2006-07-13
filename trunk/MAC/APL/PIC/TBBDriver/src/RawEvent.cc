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
//#  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/TM/GCF_ETHRawPort.h>

#include <APL/RSP_Protocol/TP_Protocol.ph>

#include "RawEvent.h"

using namespace LOFAR;
using namespace TP_Protocol;

static const uint8 OPCODE_LEN = 4;

//
// GCFEvent must be 4byte aligned
//
typedef struct {
	GCFEvent  event;
	uint32		opcode;
	uint8     payload[ETH_DATA_LEN-4];
} RawFrame;
static RawFrame buf;


GCFEvent::TResult RawEvent::dispatch(GCFTask& task, GCFPortInterface& port)
{
  
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  
  // Receive a raw packet
  ssize_t size = port.recv(buf.payload, ETH_DATA_LEN);
  
	// at least 4 bytes
  if (size < OPCODE_LEN) return GCFEvent::NOT_HANDLED;
  
	
  LOG_DEBUG(formatString("F_DATAIN: Opcode=0x%04x",buf.opcode));
 
  //
  // If no error, lookup buf.opcode number, else assign ACK_ERROR buf.event.signal number
  //
  switch(buf.opcode)
	 {
	 case TPALLOC:
  		buf.event.signal = TP_ALLOC;
  		buf.event.length = 16;
  		break;
	 case TPFREE:
  		buf.event.signal = TP_FREE;
  		buf.event.length = 8;
  		break;
	 case TPRECORD:
  		buf.event.signal = TP_RECORD;
  		buf.event.length = 8;
  		break;
	 case TPSTOP:
  		buf.event.signal = TP_RECORD;
  		buf.event.length = 8;
  		break;
	 case TPTRIGGER:
  		buf.event.signal = TP_TRIGGER;
  		buf.event.length = 16;
  		break;
	 case TPTRIGCLR:
  		buf.event.signal = TP_TRIGCLR;
  		buf.event.length = 8;
  		break;
	 case TPREAD:
  		buf.event.signal = TP_READ;
  		buf.event.length = 16;
  		break;
	 case TPUDP:
  		buf.event.signal = TP_UDP;
  		buf.event.length = 40;
  		break;
	 case TPVERSION:
  		buf.event.signal = TP_VERSION;
  		buf.event.length = 36;
  		break;
	 case TPSIZE:
  		buf.event.signal = TP_SIZE;
  		buf.event.length = 8;
  		break;
	 case TPERROR:
  		buf.event.signal = TP_ERROR;
  		buf.event.length = 8;
  		break;
	 case TPCLEAR:
  		buf.event.signal = TP_CLEAR;
  		buf.event.length = 4;
  		break;
	 case TPRESET:
  		buf.event.signal = TP_RESET;
  		buf.event.length = 4;
  		break;
	 case TPCONFIG:
  		buf.event.signal = TP_CONFIG;
  		buf.event.length = 8;
  		break;
	 case TPERASEF:
  		buf.event.signal = TP_ERASEF;
  		buf.event.length = 8;
  		break;
	 case TPREADF:
  		buf.event.signal = TP_READF;
  		buf.event.length = 1028;
  		break;
	 case TPWRITEF:
  		buf.event.signal = TP_WRITEF;
  		buf.event.length = 1028;
  		break;
	 case TPREADW:
  		buf.event.signal = TP_READW;
  		buf.event.length = 20;
  		break;
	 case TPWRITEW:
  		buf.event.signal = TP_WRITEW;
  		buf.event.length = 20;
  		break;
	 default:
  		buf.event.signal = 0;
			buf.event.length = 0;
  		break;
	 }
  
  if (buf.event.signal) // buf.event.signal == 0 indicates unrecognised or invalid MEP message
	{
		
		 (void)new((void*)&buf.event) GCFEvent(buf.event.signal); // placement new does in place construction
				
		//
		// Print debugging info
		// 
		LOG_DEBUG(formatString("%s receives '%s' on port '%s'",
							task.getName().c_str(),
							task.evtstr(buf.event),
							port.getName().c_str()));
				
		//
		// dispatch the TP message as a GCFEvent (which it now is)
		//
		status = task.dispatch(buf.event, port);
	}
  else
	{
		LOG_WARN("F_DATAIN: Discarding unknown message.");
	}
  
  return status;
}


