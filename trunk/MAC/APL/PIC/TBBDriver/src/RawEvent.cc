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
  
	
  //LOG_DEBUG(formatString("in RawEvent::F_DATAIN: Opcode=0x%08x",buf.opcode));
 
  //
  // If no error, lookup buf.opcode number, else assign ACK_ERROR buf.event.signal number
  //
  switch(buf.opcode)
	 {
	 
	 case TPALLOC:
  		buf.event.signal = TP_ALLOCACK;
  		buf.event.length = 8;
  		break;
	 case TPFREE:
  		buf.event.signal = TP_FREEACK;
  		buf.event.length = 8;
  		break;
	 case TPRECORD:
  		buf.event.signal = TP_RECORDACK;
  		buf.event.length = 8;
  		break;
	 case TPSTOP:
  		buf.event.signal = TP_STOPACK;
  		buf.event.length = 8;
  		break;
	 case TPTRIGGER:
  		buf.event.signal = TP_TRIGGER;
  		buf.event.length = 20;
  		break;
	 case TPTRIGCLR:
  		buf.event.signal = TP_TRIGCLRACK;
  		buf.event.length = 8;
  		break;
	 case TPREAD:
  		buf.event.signal = TP_READACK;
  		buf.event.length = 8;
  		break;
	 case TPUDP:
  		buf.event.signal = TP_UDPACK;
  		buf.event.length = 8;
  		break;
	 case TPPAGEPERIOD:
  		buf.event.signal = TP_PAGEPERIODACK;
  		buf.event.length = 12;
  		break;
  		
	 case TPVERSION:
  		buf.event.signal = TP_VERSIONACK;
  		buf.event.length = 40;
  		break;
	 case TPSIZE:
  		buf.event.signal = TP_SIZEACK;
  		buf.event.length = 12;
  		break;
   case TPSTATUS:
  		buf.event.signal = TP_STATUSACK;
  		buf.event.length = 44;
  		break;
	 case TPERROR:
  		buf.event.signal = TP_ERROR;
  		buf.event.length = 12;
  		break;
	 case TPCLEAR:
  		buf.event.signal = TP_CLEARACK;
  		buf.event.length = 8;
  		break;
	 case TPRESET:
  		buf.event.signal = TP_RESETACK;
  		buf.event.length = 8;
  		break;
	 case TPCONFIG:
  		buf.event.signal = TP_CONFIGACK;
  		buf.event.length = 8;
  		break;
	 case TPERASEF:
  		buf.event.signal = TP_ERASEFACK;
  		buf.event.length = 8;
  		break;
	 case TPREADF:
  		buf.event.signal = TP_READFACK;
  		buf.event.length = 1032;
  		break;
	 case TPWRITEF:
  		buf.event.signal = TP_WRITEFACK;
  		buf.event.length = 8;
  		break;
	 case TPREADW:
  		buf.event.signal = TP_READWACK;
  		buf.event.length = 16;
  		break;
	 case TPWRITEW:
  		buf.event.signal = TP_WRITEWACK;
  		buf.event.length = 8;
  		break;
	 case TPREADR:
  		buf.event.signal = TP_READRACK;
  		buf.event.length = 2056;
  		break;
   case TPWRITER:
  		buf.event.signal = TP_WRITERACK;
  		buf.event.length = 8;
  		break;
   case TPREADX:
  		buf.event.signal = TP_READXACK;
  		buf.event.length = 1032;
  		break;	
   case TPALIVE:
  		buf.event.signal = TP_ALIVEACK;
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
  
  return(status);
}

	} // end namespace TBB
} // end namespace LOFAR
