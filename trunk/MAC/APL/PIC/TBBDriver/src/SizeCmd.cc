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
	
	setBoards(tbb_event.boardmask);
	
	nextBoardNr(); // get first board
}

// ----------------------------------------------------------------------------
void SizeCmd::sendTpEvent()
{
	TPSizeEvent tp_event;
	tp_event.opcode = oc_SIZE;
	tp_event.status = 0;
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void SizeCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	} else {
		TPSizeAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received SizeAck from boardnr[%d]", getBoardNr()));
		
		if (tp_ack.status != 0) {
			setStatus(getBoardNr(), (tp_ack.status << 24));
		} else {
		
			LOG_DEBUG_STR(formatString("SizeCmd: board[%d] status[0x%08X] pages[%u]", 
																	getBoardNr(), tp_ack.status, tp_ack.npages));
																	
			TS->setMemorySize(getBoardNr(),tp_ack.npages);
			itsNpages[getBoardNr()]	= tp_ack.npages;
		}
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void SizeCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBSizeAckEvent tbb_ack;
	
	for (int32 i = 0; i < MAX_N_TBBOARDS; i++) { 
		tbb_ack.status_mask[i] = getStatus(i);
		tbb_ack.npages[i] = itsNpages[i];
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
