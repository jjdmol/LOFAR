//#  WatchDogCmd.cc: implementation of the WatchDogCmd class
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
#include <unistd.h>

#include "WatchDogCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a WatchDogCmd object.----------------------------------------
WatchDogCmd::WatchDogCmd():
	itsMode(0)
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		itsStatus[boardnr] = TBB_NO_BOARD;
	}
	setWaitAck(true);	
}

//--Destructor for WatchDogCmd.---------------------------------------------------
WatchDogCmd::~WatchDogCmd() { }

// ----------------------------------------------------------------------------
bool WatchDogCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_WATCHDOG)||(event.signal == TP_WATCHDOG_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void WatchDogCmd::saveTbbEvent(GCFEvent& event)
{
	TBBWatchdogEvent tbb_event(event);
	
	setBoardMask(tbb_event.boardmask);
	itsMode = tbb_event.mode;
	
	LOG_DEBUG_STR("boardMask= " << formatString("%08x",tbb_event.boardmask));
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsStatus[boardnr] = 0;
		if (!TS->isBoardActive(boardnr)) {
			itsStatus[boardnr] |= TBB_NO_BOARD;
		}
	}
	
	// select first board
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void WatchDogCmd::sendTpEvent()
{
	TPWatchdogEvent tp_event;
	tp_event.opcode = oc_WATCHDOG;
	tp_event.status = 0;
	tp_event.mode = itsMode;
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void WatchDogCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		TS->setBoardState(getBoardNr(),noBoard);
	}	else {
		TPWatchdogAckEvent tp_ack(event);
/*
		TS->setImageNr(getBoardNr(), 0);
		
		if (tp_ack.status == 0) {
			TS->setBoardState(getBoardNr(),setImage1);
		} else {
			TS->setBoardState(getBoardNr(),boardError);
		}
*/
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void WatchDogCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBWatchdogAckEvent tbb_ack;
	
	for (int32 boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		if (itsStatus[boardnr] == 0) {
			tbb_ack.status_mask[boardnr] = TBB_SUCCESS;
		} else {
			tbb_ack.status_mask[boardnr] = itsStatus[boardnr];
		}
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
