//#  SizeCmd.cc: implementation of the SizeCmd class
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

#include "SizeCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a SizeCmd object.----------------------------------------
SizeCmd::SizeCmd()
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		itsStatus[boardnr] = TBB_NO_BOARD;
		itsNpages[boardnr] = 0;
	}
	setWaitAck(true);		
}
	  
//--Destructor for SizeCmd.---------------------------------------------------
SizeCmd::~SizeCmd() { }

// ----------------------------------------------------------------------------
bool SizeCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_SIZE)||(event.signal == TP_SIZE_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void SizeCmd::saveTbbEvent(GCFEvent& event)
{
	TBBSizeEvent tbb_event(event);
	
	setBoardMask(tbb_event.boardmask);
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsStatus[boardnr] = 0;
		if (!TS->isBoardActive(boardnr))
			itsStatus[boardnr] |= TBB_NO_BOARD;
	}
		
	// get first board
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void SizeCmd::sendTpEvent()
{
	TPSizeEvent tp_event;
	
	tp_event.opcode	= TPSIZE;
	tp_event.status	= 0;
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void SizeCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus[getBoardNr()] |= TBB_COMM_ERROR;
	} else {
		TPSizeAckEvent tp_event(event);
		
		if ((tp_event.status >= 0xF0) && (tp_event.status <= 0xF6)) 
			itsStatus[getBoardNr()] |= (1 << (16 + (tp_event.status & 0x0F)));	
		
		LOG_DEBUG_STR(formatString("SizeCmd: board[%d] status[0x%08X] pages[%u]", 
																getBoardNr(), tp_event.status, tp_event.npages));
																
		TS->setMemorySize(getBoardNr(),tp_event.npages);
		itsNpages[getBoardNr()]	= tp_event.npages;
		
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void SizeCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBSizeAckEvent tbb_ack;
	
	for (int32 boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		if (itsStatus[boardnr] == 0) {
			tbb_ack.status_mask[boardnr] = TBB_SUCCESS;
		} else {
			tbb_ack.status_mask[boardnr] = itsStatus[boardnr];
		}
		tbb_ack.npages[boardnr] = itsNpages[boardnr];
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
