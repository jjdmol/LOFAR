//#  RecordCmd.cc: implementation of the RecordCmd class
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

#include "RecordCmd.h"

using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a RecordCmd object.----------------------------------------
RecordCmd::RecordCmd():
		itsRcuStatus(0)
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPRecordEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBRecordackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
		itsChannelMask[boardnr] = 0;
	}
	setWaitAck(true);		
}
	  
//--Destructor for RecordCmd.---------------------------------------------------
RecordCmd::~RecordCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool RecordCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_RECORD)||(event.signal == TP_RECORDACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void RecordCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE	= new TBBRecordEvent(event);
		
	// convert rcu-bitmask to tbb-channelmask
	int boardnr;
	int channelnr;
	for (int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		if(itsTBBE->rcu_mask.test(rcunr)) {
			TS->convertRcu2Ch(rcunr,&boardnr,&channelnr);	
			itsChannelMask[boardnr] |= (1 << channelnr);
		}
	} 
	
	uint32 boardmask = 0;		
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		if (itsChannelMask[boardnr] != 0) boardmask |= (1 << boardnr); 
			
		if (!TS->isBoardActive(boardnr)) {
			itsTBBackE->status_mask[boardnr] |= TBB_NO_BOARD;
		}
		
		if ((itsChannelMask[boardnr] & ~0xFFFF) != 0) {
			itsTBBackE->status_mask[boardnr] |= (TBB_SELECT_ERROR | TBB_CHANNEL_SEL_ERROR);
		}		
		
		if (!TS->isBoardActive(boardnr) &&  (itsChannelMask[boardnr] != 0)) {
			itsTBBackE->status_mask[boardnr] |= (TBB_SELECT_ERROR | TBB_BOARD_SEL_ERROR);
		}
	}
	setBoardMask(boardmask);
	
	// select firt channel to handle
	nextChannelNr();
	
	// initialize TP send frame
	itsTPE->opcode	= TPRECORD;
	itsTPE->status	=	0;
	
	delete itsTBBE;
}

// ----------------------------------------------------------------------------
void RecordCmd::sendTpEvent()
{
	itsRcuStatus = 0;
	
	itsTPE->channel = TS->getChInputNr(getChannelNr());
	
	if ((TS->getChState(getChannelNr()) != 'A') && (TS->getChState(getChannelNr()) != 'S'))
		itsRcuStatus |= TBB_RCU_NOT_ALLOCATED;
	
	if (itsTBBackE->status_mask[getBoardNr()] == 0) {
		if (itsRcuStatus == 0) {
			TS->boardPort(getBoardNr()).send(*itsTPE);
			LOG_DEBUG_STR(formatString("Sending RecordCmd to boardnr[%d], channel[%08X]", getBoardNr(), itsTPE->channel));
		}
	}
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void RecordCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask[getBoardNr()] |= TBB_RCU_COMM_ERROR;
		itsRcuStatus |= TBB_TIMEOUT_ETH;
	}
	else {
		itsTPackE = new TPRecordackEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsRcuStatus |= (1 << (16 + (itsTPackE->status & 0x0F)));
		
		if ((itsTPackE->status == 0) && (itsRcuStatus == 0))
			//TS->setChState((getChannelNr() + (getBoardNr() * 16)), 'R');	
			TS->setChState(getChannelNr(), 'R');	
		LOG_DEBUG_STR(formatString("Received RecordAck from boardnr[%d]", getBoardNr()));
		delete itsTPackE;
	}
	
	if (itsRcuStatus) TS->setChState(getChannelNr(), 'E');
	TS->setChStatus(getChannelNr(),(uint16)(itsRcuStatus >> 16));
	
	if (itsRcuStatus || itsTBBackE->status_mask[getBoardNr()]) {
		int32 rcu;
		TS->convertCh2Rcu(getChannelNr(),&rcu);
		LOG_INFO_STR(formatString("ERROR RecordCmd Rcu[%d], DriverStatus[0x%x], RcuStatus[0x%x]",
			rcu, itsTBBackE->status_mask[getBoardNr()],itsRcuStatus));
	}
	itsTBBackE->status_mask[getBoardNr()] |= itsRcuStatus;
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void RecordCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	int32 rcunr;
	
	itsTBBackE->rcu_mask.reset();
	for (int32 channelnr = 0; channelnr < TS->maxChannels(); channelnr++) {
		if (TS->getChStatus(channelnr)) {
			TS->convertCh2Rcu(channelnr,&rcunr);
			itsTBBackE->rcu_mask.set(rcunr);
		}
	}
	
	for (int32 boardnr = 0; boardnr < TS->maxBoards(); boardnr++) { 
		if (itsTBBackE->status_mask[boardnr] == 0)
			itsTBBackE->status_mask[boardnr] = TBB_SUCCESS;
	}
	
	clientport->send(*itsTBBackE);
}

