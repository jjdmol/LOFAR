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
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a FreeCmd object.----------------------------------------
FreeCmd::FreeCmd():
		itsBoardNr(0), itsChannels(1), itsRcuStatus(0)
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		itsStatus[boardnr] = TBB_NO_BOARD;
		itsChannelMask[boardnr] = 0;
	}
	setWaitAck(true);
}

//--Destructor for FreeCmd.---------------------------------------------------
FreeCmd::~FreeCmd() { }

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
	TBBFreeEvent tbb_event(event);
	
	// convert rcu-bitmask to tbb-channelmask
	int32 board;         // board 0 .. 11
	int32 board_channel; // board_channel 0 .. 15	
	int32 channel;       // channel 0 .. 191 (= maxboard * max_channels_on_board)	
	for (int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		TS->convertRcu2Ch(rcunr,&board,&board_channel);
		channel = (board * TS->nrChannelsOnBoard()) + board_channel;
		if(tbb_event.rcu_mask.test(rcunr)) {
			itsChannelMask[board] |= (1 << board_channel);
			TS->setChSelected(channel,true);
		}
	} 
	
	uint32 boardmask = 0;		
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsStatus[boardnr] = 0;
		if (itsChannelMask[boardnr] != 0) { 
			boardmask |= (1 << boardnr);
		} 
			
		if (!TS->isBoardActive(boardnr)) {
			itsStatus[boardnr] |= TBB_NO_BOARD;
		}
		
		if ((itsChannelMask[boardnr] & ~0xFFFF) != 0) {
			itsStatus[boardnr] |= (TBB_SELECT_ERROR | TBB_CHANNEL_SEL_ERROR);
		}
		
		if (!TS->isBoardActive(boardnr) &&  (itsChannelMask[boardnr] != 0)) {
			itsStatus[boardnr] |= (TBB_SELECT_ERROR | TBB_BOARD_SEL_ERROR);
		}
		LOG_DEBUG_STR(formatString("FreeCmd savetbb status board[%d] = 0x%08X",boardnr,itsStatus[boardnr]));
	}
	
	setBoardMask(boardmask);
	
	// select firt channel to handle
	nextSelectedChannelNr();
}

// ----------------------------------------------------------------------------
void FreeCmd::sendTpEvent()
{
	TPFreeEvent tp_event;
	tp_event.opcode = oc_FREE;
	tp_event.status = 0;
	
	itsRcuStatus = 0;
	
	if (itsBoardNr != getBoardNr()) itsChannels = 1; // new board
	itsBoardNr = getBoardNr();
	
	if ((itsChannelMask[getBoardNr()] == 0xFFFF) && ((getChannelNr()% 16) == 0)) {
		tp_event.channel = 0xFFFFFFFF; // uint32 -> -1 to free all
		itsChannels = 16;
	} else {
		tp_event.channel = TS->getChInputNr(getChannelNr());
		itsChannels = 1;
	}
			
	if (itsStatus[getBoardNr()] == 0) {
		TS->boardPort(getBoardNr()).send(tp_event);
		
		LOG_DEBUG_STR(formatString("Sending FreeCmd to boardnr[%d], channel[%08X]", getBoardNr(), tp_event.channel));
	} 
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void FreeCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus[getBoardNr()] |= TBB_RCU_COMM_ERROR;
		itsRcuStatus |= TBB_TIMEOUT_ETH;
	} else {
		TPFreeAckEvent tp_ack(event);
		
		if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) {
			itsRcuStatus |= (1 << (16 + (tp_ack.status & 0x0F)));
		}
				
		if ((tp_ack.status == 0) && (itsRcuStatus == 0)) {
			for (int ch = getChannelNr(); ch < (getChannelNr() + itsChannels); ch++) {
				TS->setChSelected(ch, false); 	
				TS->setChState(ch, 'F'); 	
				TS->setChStartAddr(ch, 0);
				TS->setChPageSize(ch, 0); 
			}
		} 
			
		LOG_DEBUG_STR(formatString("Received FreeAck from boardnr[%d]", getBoardNr()));
	}
	
	for (int ch = getChannelNr(); ch < (getChannelNr() + itsChannels); ch++) {
		if (itsRcuStatus) TS->setChState(ch, 'E');
		TS->setChStatus(ch,(uint16)(itsRcuStatus >> 16));
	}
	
	if (itsRcuStatus || itsStatus[getBoardNr()]) {
		int32 rcu;
		TS->convertCh2Rcu(getChannelNr(),&rcu);
		if (itsChannels == 1) {
			LOG_INFO_STR(formatString("ERROR FreeCmd Rcu[%d], DriverStatus[0x%x], RcuStatus[0x%x]",
				rcu, itsStatus[getBoardNr()],itsRcuStatus));
		} else {
			LOG_INFO_STR(formatString("ERROR FreeCmd Rcu[%d .. %d], DriverStatus[0x%x], RcuStatus[0x%x]",
				(getBoardNr() * 16),((getBoardNr() + 1) * 16), itsStatus[getBoardNr()],itsRcuStatus));
		}
	}
	itsStatus[getBoardNr()] |= itsRcuStatus;
	if (itsChannels == 16) {
		setChannelNr((getBoardNr() * 16) + 15);
	}
	nextSelectedChannelNr();
}

// ----------------------------------------------------------------------------
void FreeCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBFreeAckEvent tbb_ack;
	
	int32 rcunr;
	
	tbb_ack.rcu_mask.reset();
	for (int32 channelnr = 0; channelnr < TS->maxChannels(); channelnr++) {
		if (TS->getChStatus(channelnr)) {
			TS->convertCh2Rcu(channelnr,&rcunr);
			tbb_ack.rcu_mask.set(rcunr);
		}
	}
	
	for (int32 boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		if (itsStatus[boardnr] == 0) {
			tbb_ack.status_mask[boardnr] = TBB_SUCCESS;
		} else {
			tbb_ack.status_mask[boardnr] = itsStatus[boardnr];
		}
	}
 
	if(clientport->isConnected()) { clientport->send(tbb_ack); }
}
