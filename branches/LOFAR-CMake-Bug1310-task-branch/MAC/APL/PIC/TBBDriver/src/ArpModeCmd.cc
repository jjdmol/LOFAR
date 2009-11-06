//#  ArpModeCmd.cc: implementation of the ArpModeCmd class
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

#include "ArpModeCmd.h"


using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;	
using namespace TBB;

//--Constructors for a VersionCmd object.--------------------------------------
ArpModeCmd::ArpModeCmd()
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPArpModeEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBArpModeAckEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBOARDS;boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
	}
	
	setWaitAck(true);
}
  
//--Destructor for GetVersions.------------------------------------------------
ArpModeCmd::~ArpModeCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool ArpModeCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_ARP_MODE)||(event.signal == TP_ARP_MODE_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ArpModeCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE	= new TBBArpModeEvent(event);
	
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
	itsTPE->opcode  = TPARPMODE;
	itsTPE->status  = 0;
	itsTPE->mode    = itsTBBE->mode;
			
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void ArpModeCmd::sendTpEvent()
{
		TS->boardPort(getBoardNr()).send(*itsTPE);
		TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void ArpModeCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask[getBoardNr()] |= TBB_COMM_ERROR;
	}	else {
		itsTPackE = new TPArpModeAckEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status_mask[getBoardNr()] |= (1 << (16 + (itsTPackE->status & 0x0F)));	
		
		// fill in extra code 
		LOG_DEBUG_STR(formatString("ArpModeCmd: board[%d] %08X",
				getBoardNr(),itsTBBackE->status_mask[getBoardNr()]));
		
		delete itsTPackE;
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void ArpModeCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < TS->maxBoards(); boardnr++) { 
		if (itsTBBackE->status_mask[boardnr] == 0)
			itsTBBackE->status_mask[boardnr] = TBB_SUCCESS;
	}

	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
