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
		case TPALLOC:
			buf.event.signal = TP_ALLOC_ACK;
			buf.event.length = 8;
			break;
		case TPFREE:
			buf.event.signal = TP_FREE_ACK;
			buf.event.length = 8;
			break;
		case TPRECORD:
			buf.event.signal = TP_RECORD_ACK;
			buf.event.length = 8;
			break;
		case TPSTOP:
			buf.event.signal = TP_STOP_ACK;
			buf.event.length = 8;
			break;
		case TPTRIGGER:
			buf.event.signal = TP_TRIGGER;
			buf.event.length = 44;
			break;
		case TPTRIGRELEASE:
			buf.event.signal = TP_TRIG_RELEASE_ACK;
			buf.event.length = 8;
			break;
		case TPTRIGGENERATE:
			buf.event.signal = TP_TRIG_GENERATE_ACK;
			buf.event.length = 8;
			break;
		case TPTRIGSETUP:
			buf.event.signal = TP_TRIG_SETUP_ACK;
			buf.event.length = 8;
			break;
		case TPTRIGCOEF:
			buf.event.signal = TP_TRIG_COEF_ACK;
			buf.event.length = 8;
			break;
		case TPTRIGINFO:
			buf.event.signal = TP_TRIG_INFO_ACK;
			buf.event.length = 40;
			break;	
		case TPREAD:
			buf.event.signal = TP_READ_ACK;
			buf.event.length = 28;
			break;
		case TPUDP:
			buf.event.signal = TP_UDP_ACK;
			buf.event.length = 8;
			break;
		case TPPAGEPERIOD:
			buf.event.signal = TP_PAGEPERIOD_ACK;
			buf.event.length = 12;
			break;
			
		case TPVERSION:
			buf.event.signal = TP_VERSION_ACK;
			buf.event.length = 40;
			break;
		case TPSIZE:
			buf.event.signal = TP_SIZE_ACK;
			buf.event.length = 12;
			break;
		case TPSTATUS:
			buf.event.signal = TP_STATUS_ACK;
			buf.event.length = 84;
			break;
		case TPERROR:
			buf.event.signal = TP_ERROR;
			buf.event.length = 12;
			break;
		case TPCLEAR:
			buf.event.signal = TP_CLEAR_ACK;
			buf.event.length = 8;
			break;
		case TPRESET:
			buf.event.signal = TP_RESET_ACK;
			buf.event.length = 8;
			break;
		case TPCONFIG:
			buf.event.signal = TP_CONFIG_ACK;
			buf.event.length = 8;
			break;
		case TPERASEF:
			buf.event.signal = TP_ERASEF_ACK;
			buf.event.length = 8;
			break;
		case TPERASEFSPEC:
			buf.event.signal = TP_ERASEF_SPEC_ACK;
			buf.event.length = 8;
			break;
		case TPREADF:
			buf.event.signal = TP_READF_ACK;
			buf.event.length = 1032;
			break;
		case TPWRITEF:
			buf.event.signal = TP_WRITEF_ACK;
			buf.event.length = 8;
			break;
		case TPWRITEFSPEC:
			buf.event.signal = TP_WRITEF_SPEC_ACK;
			buf.event.length = 8;
			break;
		case TPPROTECT:
			buf.event.signal = TP_PROTECT_ACK;
			buf.event.length = 8;
			break;
		case TPUNPROTECT:
			buf.event.signal = TP_UNPROTECT_ACK;
			buf.event.length = 8;
			break;
		case TPREADW:
			buf.event.signal = TP_READW_ACK;
			buf.event.length = 16;
			break;
		case TPWRITEW:
			buf.event.signal = TP_WRITEW_ACK;
			buf.event.length = 8;
			break;
		case TPREADR:
			buf.event.signal = TP_READR_ACK;
			buf.event.length = 2056;
			break;
		case TPWRITER:
			buf.event.signal = TP_WRITER_ACK;
			buf.event.length = 8;
			break;
		case TPREADX:
			buf.event.signal = TP_READX_ACK;
			buf.event.length = 1032;
			break;	
		case TPALIVE:
			buf.event.signal = TP_ALIVE_ACK;
			buf.event.length = 12;
			break;	
		case TPARP:
			buf.event.signal = TP_ARP_ACK;
			buf.event.length = 8;
			break;
		case TPARPMODE:
			buf.event.signal = TP_ARP_MODE_ACK;
			buf.event.length = 8;
			break;
		case TPSTOPCEP:
			buf.event.signal = TP_STOP_CEP_ACK;
			buf.event.length = 8;
			break;
		case TPCEPSTATUS:
			buf.event.signal = TP_CEP_STATUS_ACK;
			buf.event.length = 12;
			break;
		case TPCEPDELAY:
			buf.event.signal = TP_CEP_DELAY_ACK;
			buf.event.length = 8;
			break;
		case TPWATCHDOG:
			buf.event.signal = TP_WATCHDOG_ACK;
			buf.event.length = 8;
			break;
		case TPTEMPLIMIT:
			buf.event.signal = TP_TEMP_LIMIT_ACK;
			buf.event.length = 8;
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
