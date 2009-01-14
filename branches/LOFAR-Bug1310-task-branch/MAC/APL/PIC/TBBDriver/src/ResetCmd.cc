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
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a ResetCmd object.----------------------------------------
ResetCmd::ResetCmd():
	itsBoardMask(0), itsBoardNr(0)
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPResetEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBResetAckEvent();
	
	for(int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
	}
	setWaitAck(true);	
	setRetry(false);	
}
	  
//--Destructor for ResetCmd.---------------------------------------------------
ResetCmd::~ResetCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

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
	itsTBBE	= new TBBResetEvent(event);
	
	itsBoardMask = itsTBBE->boardmask;
	
	itsTPE->opcode			= TPRESET;
	itsTPE->status			=	0;
	LOG_DEBUG_STR("boardMask= " << formatString("%08x",itsBoardMask));
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsTBBackE->status_mask[boardnr] = 0;
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
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void ResetCmd::sendTpEvent()
{
	if (TS->boardPort(getBoardNr()).isConnected()) {
			TS->boardPort(getBoardNr()).send(*itsTPE);
			TS->boardPort(getBoardNr()).setTimer(1.0);
	}
}

// ----------------------------------------------------------------------------
void ResetCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		TS->setBoardState(getBoardNr(),noBoard);
	}	else {
		itsTPackE = new TPResetAckEvent(event);
		if (itsTPackE->status == 0) {
			TS->setBoardState(getBoardNr(),setImage1);
		} else {
			TS->setBoardState(getBoardNr(),boardError);
		}
		delete itsTPackE;
	}
	
	itsBoardNr++;
	// look for next board in mask
	while ((itsBoardMask & (1 << itsBoardNr)) == 0) {
		itsBoardNr++;
		if (itsBoardNr >= TS->maxBoards()) { 
			break;
		}	
	}
	LOG_DEBUG_STR("boardnr=" << itsBoardNr);
	if (itsBoardNr < TS->maxBoards()) {
		setBoardNr(itsBoardNr);
	} else {
		setSleepTime(20.0);
		setDone(true);
	}
}

// ----------------------------------------------------------------------------
void ResetCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < TS->maxBoards(); boardnr++) { 
		if (itsTBBackE->status_mask[boardnr] == 0)
			itsTBBackE->status_mask[boardnr] = TBB_SUCCESS;
	}
	
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
