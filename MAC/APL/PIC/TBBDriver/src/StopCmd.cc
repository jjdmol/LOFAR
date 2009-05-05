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
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a StopCmd object.----------------------------------------
StopCmd::StopCmd():
		itsRcuStatus(0), itsChannels(1)
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBOARDS;boardnr++) { 
		itsStatus[boardnr] = TBB_NO_BOARD;
		itsChannelMask[boardnr] = 0;
	}
	setWaitAck(true);		
}
	  
//--Destructor for StopCmd.---------------------------------------------------
StopCmd::~StopCmd() { }

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
	TBBStopEvent tbb_event(event);
			
	// convert rcu-bitmask to tbb-channelmask
	int32 board;				// board 0 .. 11
	int32 board_channel;// board_channel 0 .. 15	
	int32 channel;			// channel 0 .. 191 (= maxboard * max_channels_on_board)	
	for (int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		TS->convertRcu2Ch(rcunr,&board,&board_channel);
		channel = (board * TS->nrChannelsOnBoard()) + board_channel;	
		if(tbb_event.rcu_mask.test(rcunr)) { 
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
		itsStatus[boardnr] = 0;
		if (!TS->isBoardActive(boardnr)) { 
			if (itsChannelMask[boardnr] != 0) boardmask |= (1 << boardnr);
			itsStatus[boardnr] |= TBB_NO_BOARD;
		}
		
		if ((itsChannelMask[boardnr] & ~0xFFFF) != 0) {
			itsStatus[boardnr] |= (TBB_SELECT_ERROR | TBB_CHANNEL_SEL_ERROR);
		}
		
		if (!TS->isBoardActive(boardnr) && (itsChannelMask[boardnr] != 0)) {
			itsStatus[boardnr] |= (TBB_SELECT_ERROR | TBB_BOARD_SEL_ERROR);
		}
		LOG_DEBUG_STR(formatString("StopCmd savetbb boardnr[%d], status[0x%x]",boardnr, itsStatus[boardnr]));
	}
		
	setBoardMask(boardmask);
	
	// select firt channel to handle
	nextSelectedChannelNr();
}

// ----------------------------------------------------------------------------
void StopCmd::sendTpEvent()
{
	TPStopEvent tp_event;
	
	tp_event.opcode = oc_STOP;
	tp_event.status = 0;
	if ((itsChannelMask[getBoardNr()] == 0xFFFF) && ((getChannelNr()% 16) == 0)) {
		tp_event.channel = ~0;
		itsChannels = 16;
	} else {
		tp_event.channel = TS->getChInputNr(getChannelNr());
		itsChannels = 1;
	}
	
	itsRcuStatus = 0;
	
	if (itsStatus[getBoardNr()] == 0) {
		TS->boardPort(getBoardNr()).send(tp_event);
		LOG_DEBUG_STR(formatString("Sending StopCmd to boardnr[%d], channel[%08X]", getBoardNr(), tp_event.channel));
	}
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void StopCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus[getBoardNr()] |= TBB_RCU_COMM_ERROR;
		itsRcuStatus |= TBB_TIMEOUT_ETH;
	} else {
		TPStopAckEvent tp_ack(event);
		
		if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) {
			itsRcuStatus |= (1 << (16 + (tp_ack.status & 0x0F)));
		}
		
		if ((tp_ack.status == 0) && (itsRcuStatus == 0)) {
			for (int ch = getChannelNr(); ch < (getChannelNr() + itsChannels); ch++) {
				TS->setChState(ch, 'S');
			}
		} 
		
		LOG_DEBUG_STR(formatString("Received StopCmd savetp boardnr[%d], status[0x%x]",
											getBoardNr(), itsStatus[getBoardNr()]));
	}
	
	for (int ch = getChannelNr(); ch < (getChannelNr() + itsChannels); ch++) {
		TS->setChStatus(ch,(uint16)(itsRcuStatus >> 16));
		if (itsRcuStatus) {
			TS->setChState(ch, 'E');
		}
	}
	
	if (itsRcuStatus || itsStatus[getBoardNr()]) {
		int32 rcu;
		TS->convertCh2Rcu(getChannelNr(),&rcu);
		LOG_INFO_STR(formatString("ERROR StopCmd Rcu[%d], DriverStatus[0x%x], RcuStatus[0x%x]",
											rcu, itsStatus[getBoardNr()],itsRcuStatus));
	}
	itsStatus[getBoardNr()] |= itsRcuStatus;
	
	if (itsChannels == 16) {
		setChannelNr((getBoardNr() * 16) + 15);
	}
	
	nextSelectedChannelNr();
}

// ----------------------------------------------------------------------------
void StopCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBStopAckEvent tbb_ack;
	
	tbb_ack.rcu_mask.reset();
	for (int32 channelnr = 0; channelnr < TS->maxChannels(); channelnr++) {
		if (TS->getChStatus(channelnr) == 0) {
			int32 rcunr;
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
		
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}

