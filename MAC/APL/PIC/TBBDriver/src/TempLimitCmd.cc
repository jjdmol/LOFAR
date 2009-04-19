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
TempLimitCmd::TempLimitCmd()
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPTempLimitEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBTempLimitAckEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBOARDS;boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
	}
	
	setWaitAck(true);
}
  
//--Destructor for GetVersions.------------------------------------------------
TempLimitCmd::~TempLimitCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

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
	itsTBBE	= new TBBTempLimitEvent(event);
	
	setBoardMask(itsTBBE->boardmask);
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsTBBackE->status_mask[boardnr] = 0;
		if (!TS->isBoardActive(boardnr)) {
			itsTBBackE->status_mask[boardnr] |= TBB_NO_BOARD;
		}
	}
	
	// select first board
	nextBoardNr();
	
	// fill TP command, to send
	itsTPE->opcode 			  = TPTEMPLIMIT;
	itsTPE->status				= 0;
	itsTPE->high    = itsTBBE->high;
	itsTPE->low    = itsTBBE->low; 		
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void TempLimitCmd::sendTpEvent()
{
		TS->boardPort(getBoardNr()).send(*itsTPE);
		TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void TempLimitCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask[getBoardNr()] |= TBB_COMM_ERROR;
	}	else {
		itsTPackE = new TPTempLimitAckEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status_mask[getBoardNr()] |= (1 << (16 + (itsTPackE->status & 0x0F)));	
		
		// fill in extra code 
		LOG_DEBUG_STR(formatString("TempLimitCmd: board[%d] %08X",
				getBoardNr(),itsTBBackE->status_mask[getBoardNr()]));
		
		delete itsTPackE;
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void TempLimitCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < TS->maxBoards(); boardnr++) { 
		if (itsTBBackE->status_mask[boardnr] == 0)
			itsTBBackE->status_mask[boardnr] = TBB_SUCCESS;
	}

	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
