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
	GCFEvent event;
	uint32   opcode;
	uint8    payload[ETH_DATA_LEN - sizeof(uint32)];
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
	// for all commands opcode(4) + status(4)
	switch(buf.opcode) {
		case oc_ALLOC:
			buf.event.signal = TP_ALLOC_ACK;
			buf.event.length = 8;
			break;
		case oc_FREE:
			buf.event.signal = TP_FREE_ACK;
			buf.event.length = 8;
			break;
		case oc_RECORD:
			buf.event.signal = TP_RECORD_ACK;
			buf.event.length = 8;
			break;
		case oc_STOP:
			buf.event.signal = TP_STOP_ACK;
			buf.event.length = 8;
			break;
		case oc_TRIGGER:
			buf.event.signal = TP_TRIGGER;
			buf.event.length = 48;
			break;
		case oc_TRIG_RELEASE:
			buf.event.signal = TP_TRIG_RELEASE_ACK;
			buf.event.length = 8;
			break;
		case oc_TRIG_GENERATE:
			buf.event.signal = TP_TRIG_GENERATE_ACK;
			buf.event.length = 8;
			break;
		case oc_TRIG_SETUP:
			buf.event.signal = TP_TRIG_SETUP_ACK;
			buf.event.length = 8;
			break;
		case oc_TRIG_COEF:
			buf.event.signal = TP_TRIG_COEF_ACK;
			buf.event.length = 8;
			break;
		case oc_TRIG_INFO:
			buf.event.signal = TP_TRIG_INFO_ACK;
			buf.event.length = 44;
			break;	
		case oc_READ:
			buf.event.signal = TP_READ_ACK;
			buf.event.length = 28;
			break;
		case oc_UDP:
			buf.event.signal = TP_UDP_ACK;
			buf.event.length = 8;
			break;
		case oc_PAGE_PERIOD:
			buf.event.signal = TP_PAGEPERIOD_ACK;
			buf.event.length = 12;
			break;
			
		case oc_VERSION:
			buf.event.signal = TP_VERSION_ACK;
			buf.event.length = 40;
			break;
		case oc_SIZE:
			buf.event.signal = TP_SIZE_ACK;
			buf.event.length = 12;
			break;
		case oc_STATUS:
			buf.event.signal = TP_STATUS_ACK;
			buf.event.length = 84;
			break;
		case oc_ERROR:
			buf.event.signal = TP_ERROR;
			buf.event.length = 12;
			break;
		case oc_CLEAR:
			buf.event.signal = TP_CLEAR_ACK;
			buf.event.length = 8;
			break;
		case oc_RESET:
			buf.event.signal = TP_RESET_ACK;
			buf.event.length = 8;
			break;
		case oc_CONFIG:
			buf.event.signal = TP_CONFIG_ACK;
			buf.event.length = 8;
			break;
		case oc_ERASEF:
			buf.event.signal = TP_ERASEF_ACK;
			buf.event.length = 8;
			break;
		case oc_ERASEF_SPEC:
			buf.event.signal = TP_ERASEF_SPEC_ACK;
			buf.event.length = 8;
			break;
		case oc_READF:
			buf.event.signal = TP_READF_ACK;
			buf.event.length = 1032;
			break;
		case oc_WRITEF:
			buf.event.signal = TP_WRITEF_ACK;
			buf.event.length = 8;
			break;
		case oc_WRITEF_SPEC:
			buf.event.signal = TP_WRITEF_SPEC_ACK;
			buf.event.length = 8;
			break;
		case oc_PROTECT:
			buf.event.signal = TP_PROTECT_ACK;
			buf.event.length = 8;
			break;
		case oc_UNPROTECT:
			buf.event.signal = TP_UNPROTECT_ACK;
			buf.event.length = 8;
			break;
		case oc_READW:
			buf.event.signal = TP_READW_ACK;
			buf.event.length = 16;
			break;
		case oc_WRITEW:
			buf.event.signal = TP_WRITEW_ACK;
			buf.event.length = 8;
			break;
		case oc_READR:
			buf.event.signal = TP_READR_ACK;
			buf.event.length = 2056;
			break;
		case oc_WRITER:
			buf.event.signal = TP_WRITER_ACK;
			buf.event.length = 8;
			break;
		case oc_READX:
			buf.event.signal = TP_READX_ACK;
			buf.event.length = 1032;
			break;	
		case oc_ALIVE:
			buf.event.signal = TP_ALIVE_ACK;
			buf.event.length = 12;
			break;	
		case oc_ARP:
			buf.event.signal = TP_ARP_ACK;
			buf.event.length = 8;
			break;
		case oc_ARPMODE:
			buf.event.signal = TP_ARP_MODE_ACK;
			buf.event.length = 8;
			break;
		case oc_STOP_CEP:
			buf.event.signal = TP_STOP_CEP_ACK;
			buf.event.length = 8;
			break;
		case oc_CEP_STATUS:
			buf.event.signal = TP_CEP_STATUS_ACK;
			buf.event.length = 12;
			break;
		case oc_CEP_DELAY:
			buf.event.signal = TP_CEP_DELAY_ACK;
			buf.event.length = 8;
			break;
		case oc_WATCHDOG:
			buf.event.signal = TP_WATCHDOG_ACK;
			buf.event.length = 8;
			break;
		case oc_TEMP_LIMIT:
			buf.event.signal = TP_TEMP_LIMIT_ACK;
			buf.event.length = 8;
			break;					
		case oc_TESTMODE:
			buf.event.signal = TP_TESTMODE_ACK;
			buf.event.length = 8;
			break;
		case oc_UDP_EN_PRG:
			buf.event.signal = TP_UDP_EN_PRG_ACK;
			buf.event.length = 8;
			break;
		case oc_UDP_DIS_PRG:
			buf.event.signal = TP_UDP_DIS_PRG_ACK;
			buf.event.length = 8;
			break;
		case oc_UDP_CLR_ERR:
			buf.event.signal = TP_UDP_CLR_ERR_ACK;
			buf.event.length = 8;
			break;
		case oc_UDP_GET_ERR:
			buf.event.signal = TP_UDP_GET_ERR_ACK;
			buf.event.length = 32;
			break;
			 
 		default:
			buf.event.signal = 0;
			buf.event.length = 0;
			break;
	 }
	
	if (buf.event.signal) // buf.event.signal == 0 indicates unrecognised or invalid TP message
	{				
		//
		// Print debugging info
		// 
#if 0
		LOG_DEBUG(formatString("%s receives '%s' on port '%s'",
							task.getName().c_str(),
							task.evtstr(buf.event),
							port.getName().c_str()));
#endif
				
		//
		// dispatch the TP message as a GCFEvent (which it now is)
		//
		buf.event._buffer = (char*)(&buf.opcode) - GCFEvent::sizePackedGCFEvent;
		status = task.doEvent(buf.event, port);
		buf.event._buffer = 0;
	}
	else
	{
		LOG_WARN("F_DATAIN: Discarding unknown message.");
	}
	
	return(status);
}

	} // end namespace TBB
} // end namespace LOFAR
