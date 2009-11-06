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

#include "StatusCmd.h"

using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a StatusCmd object.----------------------------------------
StatusCmd::StatusCmd()
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPStatusEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBStatusAckEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBOARDS;boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
		itsTBBackE->V12[boardnr]	= 0;
		itsTBBackE->V25[boardnr]	= 0;
		itsTBBackE->V33[boardnr]	= 0;
		itsTBBackE->Tpcb[boardnr]	= 0;
		itsTBBackE->Ttp[boardnr]	= 0;
		itsTBBackE->Tmp0[boardnr]	= 0;
		itsTBBackE->Tmp1[boardnr]	= 0;
		itsTBBackE->Tmp2[boardnr]	= 0;
		itsTBBackE->Tmp3[boardnr]	= 0;
	}
	setWaitAck(true);		
}
	  
//--Destructor for StatusCmd.---------------------------------------------------
StatusCmd::~StatusCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool StatusCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_STATUS)||(event.signal == TP_STATUS_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void StatusCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE	= new TBBStatusEvent(event);
	
	setBoardMask(itsTBBE->boardmask);
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsTBBackE->status_mask[boardnr] = 0;
		if (!TS->isBoardActive(boardnr)) 
			itsTBBackE->status_mask[boardnr] |= TBB_NO_BOARD;
	}	
		
	// select first boards
	nextBoardNr();
	
	// initialize TP send frame
	itsTPE->opcode	= TPSTATUS;
	itsTPE->status	=	0;
			
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void StatusCmd::sendTpEvent()
{
		TS->boardPort(getBoardNr()).send(*itsTPE);
		TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void StatusCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask[getBoardNr()] |= TBB_COMM_ERROR;
	}	else {
		itsTPackE = new TPStatusAckEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status_mask[getBoardNr()] |= (1 << (16 + (itsTPackE->status & 0x0F)));	
		
		itsTBBackE->V12[getBoardNr()]	= itsTPackE->V12;
		itsTBBackE->V25[getBoardNr()]	= itsTPackE->V25;
		itsTBBackE->V33[getBoardNr()]	= itsTPackE->V33;
		itsTBBackE->Tpcb[getBoardNr()]	= itsTPackE->Tpcb;
		itsTBBackE->Ttp[getBoardNr()]	= itsTPackE->Ttp;
		itsTBBackE->Tmp0[getBoardNr()]	= itsTPackE->Tmp0;
		itsTBBackE->Tmp1[getBoardNr()]	= itsTPackE->Tmp1;
		itsTBBackE->Tmp2[getBoardNr()]	= itsTPackE->Tmp2;
		itsTBBackE->Tmp3[getBoardNr()]	= itsTPackE->Tmp3;
		
		LOG_DEBUG_STR(formatString("Received StatusAck from boardnr[%d]", getBoardNr()));
		delete itsTPackE;
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void StatusCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < TS->maxBoards(); boardnr++) { 
		if (itsTBBackE->status_mask[boardnr] == 0)
			itsTBBackE->status_mask[boardnr] = TBB_SUCCESS;
	}
	
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
