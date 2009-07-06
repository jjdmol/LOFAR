//#  ResetCmd.cc: implementation of the ResetCmd class
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

#include "ResetCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a ResetCmd object.----------------------------------------
ResetCmd::ResetCmd():
	itsBoardMask(0), itsBoardNr(0)
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		itsStatus[boardnr] = TBB_NO_BOARD;
	}
	setWaitAck(true);	
	setRetry(false);	
}

//--Destructor for ResetCmd.---------------------------------------------------
ResetCmd::~ResetCmd() { }

// ----------------------------------------------------------------------------
bool ResetCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_RESET)||(event.signal == TP_RESET_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ResetCmd::saveTbbEvent(GCFEvent& event)
{
	TBBResetEvent tbb_event(event);
	
	itsBoardMask = tbb_event.boardmask;
	
	LOG_DEBUG_STR("boardMask= " << formatString("%08x",itsBoardMask));
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsStatus[boardnr] = TBB_NO_BOARD;
	}
	
	// look for first board in mask
	while ((itsBoardMask & (1 << itsBoardNr)) == 0) {
		itsBoardNr++;
		if (itsBoardNr >= TS->maxBoards()) { 
			break;	
		}
	}
	
	if (itsBoardNr < TS->maxBoards()) {
		setBoardNr(itsBoardNr);
	} else {
		setDone(true);
	}
}

// ----------------------------------------------------------------------------
void ResetCmd::sendTpEvent()
{
	
	TPResetEvent tp_event;
	tp_event.opcode = oc_RESET;
	tp_event.status = 0;

	if (TS->boardPort(getBoardNr()).isConnected()) {
		TS->boardPort(getBoardNr()).send(tp_event);
		TS->boardPort(getBoardNr()).setTimer(5.0);
	}
	LOG_DEBUG_STR("Reset is send to boardnr " << getBoardNr());
}

// ----------------------------------------------------------------------------
void ResetCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		TS->setBoardState(getBoardNr(),noBoard);
	}	else {
		TPResetAckEvent tp_ack(event);
		TS->setImageNr(getBoardNr(), 0);
		itsStatus[getBoardNr()] = tp_ack.status;
		if (tp_ack.status == 0) {
			TS->setBoardState(getBoardNr(),setImage1);
		} else {
			TS->setBoardState(getBoardNr(),boardError);
		}
	}
	
	itsBoardNr++;
	// look for next board in mask
	while ((itsBoardMask & (1 << itsBoardNr)) == 0) {
		itsBoardNr++;
		if (itsBoardNr >= TS->maxBoards()) { 
			break;
		}	
	}
	if (itsBoardNr < TS->maxBoards()) {
		setBoardNr(itsBoardNr);
	} else {
		setSleepTime(10.0);
		setDone(true);
	}
}

// ----------------------------------------------------------------------------
void ResetCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBResetAckEvent tbb_ack;
	
	for (int32 boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		if (itsStatus[boardnr] == 0) {
			tbb_ack.status_mask[boardnr] = TBB_SUCCESS;
		} else {
			tbb_ack.status_mask[boardnr] = itsStatus[boardnr];
		}
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
