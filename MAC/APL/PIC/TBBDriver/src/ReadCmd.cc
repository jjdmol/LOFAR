//#  ReadCmd.cc: implementation of the ReadCmd class
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
#include <Common/StringUtil.h>

#include "ReadCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;


//--Constructors for a ReadCmd object.----------------------------------------
ReadCmd::ReadCmd():
	itsStatus(0), itsSecondstime(0), itsSampletime(0), itsPrepages(0), itsPostpages(0)
{
	TS = TbbSettings::instance();
	setWaitAck(true);
}
	  
//--Destructor for ReadCmd.---------------------------------------------------
ReadCmd::~ReadCmd() { }

// ----------------------------------------------------------------------------
bool ReadCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_READ)
		||(event.signal == TP_READ_ACK)
		||(event.signal == TP_WATCHDOG_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ReadCmd::saveTbbEvent(GCFEvent& event)
{
	TBBReadEvent tbb_event(event);
	
	int32 boardnr;
	int32 channelnr;
	TS->convertRcu2Ch(tbb_event.rcu,&boardnr,&channelnr);
		
	setChannelNr(channelnr);
	setBoardNr(boardnr);
	
	itsSecondstime = tbb_event.secondstime;
	itsSampletime  = tbb_event.sampletime;
	itsPrepages    = tbb_event.prepages;
	itsPostpages   = tbb_event.postpages;
	
	itsStatus = 0;
	if (!TS->isBoardActive(getBoardNr())) {	
		itsStatus |= TBB_NO_BOARD ;
		setDone(true);
	}
}

// ----------------------------------------------------------------------------
void ReadCmd::sendTpEvent()
{
	TPReadEvent tp_event;

	tp_event.opcode      = TPREAD;
	tp_event.status      = 0;
	tp_event.channel     = getChannelNr();
	tp_event.secondstime = itsSecondstime;
	tp_event.sampletime  = itsSampletime;
	tp_event.prepages    = itsPrepages;
	tp_event.postpages   = itsPostpages;
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void ReadCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus |= TBB_COMM_ERROR;
	}	else {
		TPReadAckEvent tp_ack(event);
		// check if busy
		if (tp_ack.status == 0xfd) {
			LOG_DEBUG_STR(formatString("TBB busy, %d pages left, trying until free", tp_ack.pages_left));
			setSleepTime(0.1);		
		} else {
			LOG_DEBUG_STR(formatString("Received ReadAck from boardnr[%d]", getBoardNr()));
			setDone(true);
		}
	}
}

// ----------------------------------------------------------------------------
void ReadCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBReadAckEvent tbb_ack;
	
	if (itsStatus == 0) {
		tbb_ack.status_mask = TBB_SUCCESS;
	} else {
		tbb_ack.status_mask = itsStatus;
	}
	
	if (clientport->isConnected()) {clientport->send(tbb_ack); }
}
