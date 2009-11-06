//#  FreeCmd.cc: implementation of the FreeCmd class
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

#include "FreeCmd.h"

using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a FreeCmd object.----------------------------------------
FreeCmd::FreeCmd():
		itsBoardNr(0),itsBoardFreeAll(0),itsRcuStatus(0)
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPFreeEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBFreeAckEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBOARDS;boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
		itsChannelMask[boardnr]	= 0;
	}
	setWaitAck(true);
}
	  
//--Destructor for FreeCmd.---------------------------------------------------
FreeCmd::~FreeCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool FreeCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_FREE)||(event.signal == TP_FREE_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void FreeCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE = new TBBFreeEvent(event);
	
	// convert rcu-bitmask to tbb-channelmask
	int32 board;				// board 0 .. 11
	int32 board_channel;// board_channel 0 .. 15	
	int32 channel;			// channel 0 .. 191 (= maxboard * max_channels_on_board)	
	for (int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		TS->convertRcu2Ch(rcunr,&board,&board_channel);
		channel = (board * TS->nrChannelsOnBoard()) + board_channel;
		if(itsTBBE->rcu_mask.test(rcunr)) {
			itsChannelMask[board] |= (1 << board_channel);
			TS->setChSelected(channel,true);
		}
	} 
	
	uint32 boardmask = 0;		
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsTBBackE->status_mask[boardnr] = 0;
		if (itsChannelMask[boardnr] != 0) boardmask |= (1 << boardnr); 
			
		if (!TS->isBoardActive(boardnr)) 
			itsTBBackE->status_mask[boardnr] |= TBB_NO_BOARD;
		
		if ((itsChannelMask[boardnr] & ~0xFFFF) != 0) 
			itsTBBackE->status_mask[boardnr] |= (TBB_SELECT_ERROR | TBB_CHANNEL_SEL_ERROR);
				
		if (!TS->isBoardActive(boardnr) &&  (itsChannelMask[boardnr] != 0))
			itsTBBackE->status_mask[boardnr] |= (TBB_SELECT_ERROR | TBB_BOARD_SEL_ERROR);
		LOG_DEBUG_STR(formatString("FreeCmd savetbb status board[%d] = 0x%08X",boardnr,itsTBBackE->status_mask[boardnr]));
	}
	
	setBoardMask(boardmask);
	
	// select firt channel to handle
	nextSelectedChannelNr();
	
	// initialize TP send frame
	itsTPE->opcode			= TPFREE;
	itsTPE->status			= 0;
	
	itsBoardFreeAll = false;	
	delete itsTBBE;
}

// ----------------------------------------------------------------------------
void FreeCmd::sendTpEvent()
{
	itsRcuStatus = 0;
	
	if (itsBoardNr != getBoardNr()) itsBoardFreeAll = false; // new board
	itsBoardNr = getBoardNr();
	
	if (!itsBoardFreeAll) {
		
		if ((itsChannelMask[getBoardNr()] == 0xFFFF) && ((getChannelNr()% 16) == 0)) {
			itsTPE->channel = 0xFFFFFFFF; // uint32 -> -1 to free all
			itsBoardFreeAll = true;
		} else {
			itsTPE->channel = TS->getChInputNr(getChannelNr()); 
		}
				
		if (itsTBBackE->status_mask[getBoardNr()] == 0) {
			TS->boardPort(getBoardNr()).send(*itsTPE);
			
			LOG_DEBUG_STR(formatString("Sending FreeCmd to boardnr[%d], channel[%08X]", getBoardNr(), itsTPE->channel));
		} 
	}
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void FreeCmd::saveTpAckEvent(GCFEvent& event)
{
	int32 channels = 1;
	
	if (itsBoardFreeAll) channels = 16;
	
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask[getBoardNr()] |= TBB_RCU_COMM_ERROR;
		itsRcuStatus |= TBB_TIMEOUT_ETH;
	}
	else {
		itsTPackE = new TPFreeAckEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsRcuStatus |= (1 << (16 + (itsTPackE->status & 0x0F)));
				
		if ((itsTPackE->status == 0) && (itsRcuStatus == 0)) {
			for (int ch = getChannelNr(); ch < (getChannelNr() + channels); ch++) {
				TS->setChSelected(ch, false); 	
				TS->setChState(ch, 'F'); 	
				TS->setChStartAddr(ch, 0);
				TS->setChPageSize(ch, 0); 
			}
		} 
			
		LOG_DEBUG_STR(formatString("Received FreeAck from boardnr[%d]", getBoardNr()));
		delete itsTPackE;
	}
	
	for (int ch = getChannelNr(); ch < (getChannelNr() + channels); ch++) {
		if (itsRcuStatus) TS->setChState(ch, 'E');
		TS->setChStatus(ch,(uint16)(itsRcuStatus >> 16));
	}
	
	if (itsRcuStatus || itsTBBackE->status_mask[getBoardNr()]) {
		int32 rcu;
		TS->convertCh2Rcu(getChannelNr(),&rcu);
		if (channels == 1)
			LOG_INFO_STR(formatString("ERROR FreeCmd Rcu[%d], DriverStatus[0x%x], RcuStatus[0x%x]",
				rcu, itsTBBackE->status_mask[getBoardNr()],itsRcuStatus));
		else
			LOG_INFO_STR(formatString("ERROR FreeCmd Rcu[%d .. %d], DriverStatus[0x%x], RcuStatus[0x%x]",
				(getBoardNr() * 16),((getBoardNr() + 1) * 16), itsTBBackE->status_mask[getBoardNr()],itsRcuStatus));
	}
	itsTBBackE->status_mask[getBoardNr()] |= itsRcuStatus;
	if (itsTPE->channel == 0xFFFFFFFF) {
		setChannelNr((getBoardNr() * 16) + 15);
	}
	nextSelectedChannelNr();
}

// ----------------------------------------------------------------------------
void FreeCmd::sendTbbAckEvent(GCFPortInterface* clientport)
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
 
	if(clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
