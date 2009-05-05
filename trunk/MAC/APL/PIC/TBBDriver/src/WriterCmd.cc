//#  WriterCmd.cc: implementation of the WriterCmd class
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

#include "WriterCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a WriterCmd object.----------------------------------------
WriterCmd::WriterCmd():
		itsStatus(0), itsMp(0), itsPid(0), itsRegId(0)
{
	TS = TbbSettings::instance();
	setWaitAck(true);
	for(int an = 0; an < 3;an++) {
		itsData[an] = 0;
	}
}
	  
//--Destructor for WriterCmd.---------------------------------------------------
WriterCmd::~WriterCmd() { }

// ----------------------------------------------------------------------------
bool WriterCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_WRITER)||(event.signal == TP_WRITER_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void WriterCmd::saveTbbEvent(GCFEvent& event)
{
	TBBWriterEvent tbb_event(event);
		
	if (TS->isBoardActive(tbb_event.board)) {	
		setBoardNr(tbb_event.board);
		itsMp = static_cast<uint32>(tbb_event.mp);
		itsPid = static_cast<uint32>(tbb_event.pid);
		itsRegId = static_cast<uint32>(tbb_event.regid);
		for(int an = 0; an < 3;an++) {
			itsData[an] = tbb_event.data[an];
		}
	} else {
		itsStatus |= TBB_NO_BOARD ;
		setDone(true);
	}
}

// ----------------------------------------------------------------------------
void WriterCmd::sendTpEvent()
{
	TPWriterEvent tp_event;
	tp_event.opcode = oc_WRITER;
	tp_event.status = 0;
	tp_event.mp = itsMp;
	tp_event.pid = itsPid;
	tp_event.regid = itsRegId;
	for (int an = 0; an < 3; an++) {
		tp_event.data[an] = itsData[an];
	}

	//LOG_DEBUG_STR(formatString("send Writer tpevent %d %d %d %u %u %u",
	//							tbb_event.mp,tbb_event.pid,tbb_event.regid,tbb_event.data[0],tbb_event.data[1],tbb_event.data[2]));	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void WriterCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus |= TBB_COMM_ERROR;
	}	else {
		TPWriterAckEvent tp_ack(event);
	
		itsStatus = tp_ack.status;
		
		//LOG_DEBUG_STR(formatString("Received WriterAck from board %d [0x%08X]", getBoardNr(), tp_ack.status));
	}
	setDone(true);
}

// ----------------------------------------------------------------------------
void WriterCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBWriterAckEvent tbb_ack;
	
	if (itsStatus == 0) {
		tbb_ack.status_mask = TBB_SUCCESS;
	} else {
		tbb_ack.status_mask = itsStatus;
	}
	
	//LOG_DEBUG_STR(formatString("send Writer tbbackevent board %d [0x%08X]", getBoardNr(), itsStatus)); 
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
