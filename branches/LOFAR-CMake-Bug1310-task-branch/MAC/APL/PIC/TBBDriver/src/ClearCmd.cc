//#  ClearCmd.cc: implementation of the ClearCmd class
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

#include "ClearCmd.h"

using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a ClearCmd object.----------------------------------------
ClearCmd::ClearCmd()
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPClearEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBClearAckEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBOARDS;boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
	}
	setWaitAck(true);		
}
	  
//--Destructor for ClearCmd.---------------------------------------------------
ClearCmd::~ClearCmd()
{
	delete itsTPE;
	delete itsTBBackE;	
}

// ----------------------------------------------------------------------------
bool ClearCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_CLEAR)||(event.signal == TP_CLEAR_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ClearCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE	= new TBBClearEvent(event);
		
	setBoardMask(itsTBBE->boardmask);
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsTBBackE->status_mask[boardnr] = 0;
		if (TS->isBoardActive(boardnr) == false) {
			itsTBBackE->status_mask[boardnr] |= TBB_NO_BOARD;
		}
	}
	
	// get first board
	nextBoardNr();
	
	// initialize TP send frame
	itsTPE->opcode			= TPCLEAR;
	itsTPE->status			=	0;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void ClearCmd::sendTpEvent()
{
	TS->boardPort(getBoardNr()).send(*itsTPE);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void ClearCmd::saveTpAckEvent(GCFEvent& event)
{
	itsTPackE = new TPClearAckEvent(event);
	
	if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
		itsTBBackE->status_mask[getBoardNr()] |= (1 << (16 + (itsTPackE->status & 0x0F)));	
	
	// reset channel-information for selected board	
	if (itsTPackE->status == 0) {
		TS->clearRcuSettings(getBoardNr());
		TS->setBoardState(getBoardNr(),boardCleared);	
	}
	LOG_DEBUG_STR(formatString("Received ClearAck from boardnr[%d]", getBoardNr()));
	delete itsTPackE;
	
	nextBoardNr();
	if (isDone()) {
		setSleepTime(3.0); // clearing the registers will last 3 seconds
	}
}

// ----------------------------------------------------------------------------
void ClearCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < TS->maxBoards(); boardnr++) { 
		if (itsTBBackE->status_mask[boardnr] == 0)
			itsTBBackE->status_mask[boardnr] = TBB_SUCCESS;
	}
	
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
