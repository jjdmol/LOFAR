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
	itsBoardMask(0), itsBoardNr(0)
{
	TS				= TbbSettings::instance();
	itsTPE		= new TPWatchdogEvent();
	itsTPackE	= 0;
	itsTBBE		= 0;
	itsTBBackE	= new TBBWatchdogAckEvent();
	
	for(int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
	}
	setWaitAck(true);	
}
	  
//--Destructor for WatchDogCmd.---------------------------------------------------
WatchDogCmd::~WatchDogCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

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
	itsTBBE	= new TBBWatchdogEvent(event);
	
	itsBoardMask = itsTBBE->boardmask;
	
	itsTPE->opcode	= TPWATCHDOG;
	itsTPE->status	= 0;
	itsTPE->mode	= itsTBBE->mode;
	
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
void WatchDogCmd::sendTpEvent()
{
	if (TS->boardPort(getBoardNr()).isConnected()) {
		TS->boardPort(getBoardNr()).send(*itsTPE);
		TS->boardPort(getBoardNr()).setTimer(TS->timeout());
	}
}

// ----------------------------------------------------------------------------
void WatchDogCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		TS->setBoardState(getBoardNr(),noBoard);
	}	else {
		itsTPackE = new TPWatchdogAckEvent(event);
		TS->setImageNr(getBoardNr(), 0);
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
		setDone(true);
	}
}

// ----------------------------------------------------------------------------
void WatchDogCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < TS->maxBoards(); boardnr++) { 
		if (itsTBBackE->status_mask[boardnr] == 0)
			itsTBBackE->status_mask[boardnr] = TBB_SUCCESS;
	}
	
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
