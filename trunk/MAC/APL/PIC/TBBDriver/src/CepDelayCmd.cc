//#  CepDelayCmd.cc: implementation of the CepDelayCmd class
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

#include "CepDelayCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;	
using namespace TBB;

//--Constructors for a VersionCmd object.--------------------------------------
CepDelayCmd::CepDelayCmd():
	itsDelay(0)
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		// fill in extra code
		itsStatus[boardnr] = TBB_NO_BOARD;
	}
	setWaitAck(true);
}
  
//--Destructor for GetVersions.------------------------------------------------
CepDelayCmd::~CepDelayCmd() { }

// ----------------------------------------------------------------------------
bool CepDelayCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_CEP_DELAY)||(event.signal == TP_CEP_DELAY_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void CepDelayCmd::saveTbbEvent(GCFEvent& event)
{
	TBBCepDelayEvent tbb_event(event);
	
	setBoardMask(tbb_event.boardmask);
	
	itsDelay = tbb_event.delay;
	 
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
void CepDelayCmd::sendTpEvent()
{
	TPCepDelayEvent tp_event;
	tp_event.opcode = TPCEPDELAY;
	tp_event.status = 0;
	tp_event.delay  = itsDelay;
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void CepDelayCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus[getBoardNr()] |= TBB_COMM_ERROR;
	}	else {
		TPCepDelayAckEvent tp_ack(event);
		
		if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) {
			itsStatus[getBoardNr()] |= (1 << (16 + (tp_ack.status & 0x0F)));	
		}

		// fill in extra code 
		LOG_DEBUG_STR(formatString("XxxCmd: board[%d] %08X",
				getBoardNr(),itsStatus[getBoardNr()]));
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void CepDelayCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBCepDelayAckEvent tbb_ack;
	
	for (int32 boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		if (itsStatus[boardnr] == 0) {
			tbb_ack.status_mask[boardnr] = TBB_SUCCESS;
		} else {
			tbb_ack.status_mask[boardnr] = itsStatus[boardnr];
		}
	}

	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
