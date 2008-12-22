//#  StopCmd.cc: implementation of the StopCmd class
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

#include "StopCmd.h"

using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a StopCmd object.----------------------------------------
StopCmd::StopCmd():
		itsRcuStatus(0)
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPStopEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBStopAckEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBOARDS;boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
		itsChannelMask[boardnr]	= 0;
	}
	setWaitAck(true);		
}
	  
//--Destructor for StopCmd.---------------------------------------------------
StopCmd::~StopCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool StopCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_STOP)||(event.signal == TP_STOP_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void StopCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE	= new TBBStopEvent(event);
	
		
	// convert rcu-bitmask to tbb-channelmask
	int32 board;				// board 0 .. 11
	int32 board_channel;// board_channel 0 .. 15	
	int32 channel;			// channel 0 .. 191 (= maxboard * max_channels_on_board)	
	for (int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		TS->convertRcu2Ch(rcunr,&board,&board_channel);
		channel = (board * TS->nrChannelsOnBoard()) + board_channel;	
		if(itsTBBE->rcu_mask.test(rcunr)) { 
			if (TS->getChState(channel) == 'R') {
				itsChannelMask[board] |= (1 << board_channel);
				TS->setChSelected(channel,true);
			} else {
				TS->setChStatus(channel,(uint16)(TBB_RCU_NOT_RECORDING >> 16));	
			}
		}
	} 
	
	uint32 boardmask = 0;		
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsTBBackE->status_mask[boardnr] = 0;
		if (!TS->isBoardActive(boardnr)) { 
			if (itsChannelMask[boardnr] != 0) boardmask |= (1 << boardnr);
			itsTBBackE->status_mask[boardnr] |= TBB_NO_BOARD;
		}
		
		if ((itsChannelMask[boardnr] & ~0xFFFF) != 0) {
			itsTBBackE->status_mask[boardnr] |= (TBB_SELECT_ERROR | TBB_CHANNEL_SEL_ERROR);
		}
		
		if (!TS->isBoardActive(boardnr) &&  (itsChannelMask[boardnr] != 0)) {
			itsTBBackE->status_mask[boardnr] |= (TBB_SELECT_ERROR | TBB_BOARD_SEL_ERROR);
		}
		LOG_DEBUG_STR(formatString("StopCmd savetbb boardnr[%d], status[0x%x]",boardnr, itsTBBackE->status_mask[boardnr]));
	}
		
	setBoardMask(boardmask);
	
	// select firt channel to handle
	nextSelectedChannelNr();
	
	// initialize TP send frame
	itsTPE->opcode			= TPSTOP;
	itsTPE->status			=	0;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void StopCmd::sendTpEvent()
{
	itsTPE->channel = TS->getChInputNr(getChannelNr());
	itsRcuStatus = 0;
	
	if ((itsTBBackE->status_mask[getBoardNr()] == 0) && (itsRcuStatus == 0)) {
		TS->boardPort(getBoardNr()).send(*itsTPE);
		TS->boardPort(getBoardNr()).setTimer(TS->timeout());
		LOG_DEBUG_STR(formatString("Sending StopCmd to boardnr[%d], channel[%08X]", getBoardNr(), itsTPE->channel));
	}
}

// ----------------------------------------------------------------------------
void StopCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask[getBoardNr()] |= TBB_RCU_COMM_ERROR;
		itsRcuStatus |= TBB_TIMEOUT_ETH;
	}
	else {
		itsTPackE = new TPStopAckEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsRcuStatus |= (1 << (16 + (itsTPackE->status & 0x0F)));
		
		if ((itsTPackE->status == 0) && (itsRcuStatus == 0))
			TS->setChState(getChannelNr(), 'S'); 	
		
		LOG_DEBUG_STR(formatString("Received StopCmd savetp boardnr[%d], status[0x%x]",
			getBoardNr(), itsTBBackE->status_mask[getBoardNr()]));
		delete itsTPackE;
	}
	
	if (itsRcuStatus) TS->setChState(getChannelNr(), 'E');
	TS->setChStatus(getChannelNr(),(uint16)(itsRcuStatus >> 16));
	
	if (itsRcuStatus || itsTBBackE->status_mask[getBoardNr()]) {
		int32 rcu;
		TS->convertCh2Rcu(getChannelNr(),&rcu);
		LOG_INFO_STR(formatString("ERROR StopCmd Rcu[%d], DriverStatus[0x%x], RcuStatus[0x%x]",
			rcu, itsTBBackE->status_mask[getBoardNr()],itsRcuStatus));
	}
	itsTBBackE->status_mask[getBoardNr()] |= itsRcuStatus;
	nextSelectedChannelNr();
}

// ----------------------------------------------------------------------------
void StopCmd::sendTbbAckEvent(GCFPortInterface* clientport)
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
		LOG_DEBUG_STR(formatString("StopCmd sendtbb status[0x%x]", itsTBBackE->status_mask[boardnr]));		
	}
	
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}

