//#  TempLimitCmd.cc: implementation of the TempLimitCmd class
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

#include "TempLimitCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;	
using namespace TBB;

//--Constructors for a VersionCmd object.--------------------------------------
TempLimitCmd::TempLimitCmd():
	itsHigh(0), itsLow(0)
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		itsStatus[boardnr] = TBB_NO_BOARD;
	}
	
	setWaitAck(true);
}
  
//--Destructor for GetVersions.------------------------------------------------
TempLimitCmd::~TempLimitCmd() { }

// ----------------------------------------------------------------------------
bool TempLimitCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_TEMP_LIMIT)||(event.signal == TP_TEMP_LIMIT_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void TempLimitCmd::saveTbbEvent(GCFEvent& event)
{
	TBBTempLimitEvent tbb_event(event);
	
	setBoardMask(tbb_event.boardmask);
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsStatus[boardnr] = 0;
		if (!TS->isBoardActive(boardnr)) {
			itsStatus[boardnr] |= TBB_NO_BOARD;
		}
	}
	itsHigh = tbb_event.high;
	itsLow = tbb_event.low;
	
	// select first board
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void TempLimitCmd::sendTpEvent()
{
	TPTempLimitEvent tp_event;
	tp_event.opcode = TPTEMPLIMIT;
	tp_event.status = 0;
	tp_event.high   = itsHigh;
	tp_event.low    = itsLow;
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void TempLimitCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus[getBoardNr()] |= TBB_COMM_ERROR;
	}	else {
		TPTempLimitAckEvent tp_ack(event);
		
		if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) {
			itsStatus[getBoardNr()] |= (1 << (16 + (tp_ack.status & 0x0F)));
		}
		
		// fill in extra code 
		LOG_DEBUG_STR(formatString("TempLimitCmd: board[%d] %08X",
				getBoardNr(),itsStatus[getBoardNr()]));
		
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void TempLimitCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBTempLimitAckEvent tbb_ack;
	
	for (int32 boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		if (itsStatus[boardnr] == 0) {
			tbb_ack.status_mask[boardnr] = TBB_SUCCESS;
		} else {
			tbb_ack.status_mask[boardnr] = itsStatus[boardnr];
		}
	}

	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
