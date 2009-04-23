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
#include <Common/StringUtil.h>

#include "RecordCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a RecordCmd object.----------------------------------------
RecordCmd::RecordCmd():
		itsRcuStatus(0), itsChannels(1)
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBOARDS;boardnr++) { 
		itsStatus[boardnr] = TBB_NO_BOARD;
		itsChannelMask[boardnr] = 0;
	}
	setWaitAck(true);		
}

//--Destructor for RecordCmd.---------------------------------------------------
RecordCmd::~RecordCmd() { }

// ----------------------------------------------------------------------------
bool RecordCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_RECORD)||(event.signal == TP_RECORD_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void RecordCmd::saveTbbEvent(GCFEvent& event)
{
	TBBRecordEvent tbb_event(event);
	
	// convert rcu-bitmask to tbb-channelmask
	int32 board;				// board 0 .. 11
	int32 board_channel;// board_channel 0 .. 15	
	int32 channel;			// channel 0 .. 191 (= maxboard * max_channels_on_board)	
	for (int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		TS->convertRcu2Ch(rcunr,&board,&board_channel);
		channel = (board * TS->nrChannelsOnBoard()) + board_channel;
		if (tbb_event.rcu_mask.test(rcunr)) { 
			if ((TS->getChState(channel) == 'A') || (TS->getChState(channel) == 'S')) {
				itsChannelMask[board] |= (1 << board_channel);
				TS->setChSelected(channel,true);
			}
			if (TS->getChState(channel) == 'F') {
				TS->setChStatus(channel,(uint16)(TBB_RCU_NOT_ALLOCATED >> 16));
			}
		}
	} 
	
	uint32 boardmask = 0;		
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsStatus[boardnr] = 0;
		if (itsChannelMask[boardnr] != 0) boardmask |= (1 << boardnr); 
			
		if (!TS->isBoardActive(boardnr)) {
			itsStatus[boardnr] |= TBB_NO_BOARD;
		}
		
		if ((itsChannelMask[boardnr] & ~0xFFFF) != 0) {
			itsStatus[boardnr] |= (TBB_SELECT_ERROR | TBB_CHANNEL_SEL_ERROR);
		}		
		
		if (!TS->isBoardActive(boardnr) &&  (itsChannelMask[boardnr] != 0)) {
			itsStatus[boardnr] |= (TBB_SELECT_ERROR | TBB_BOARD_SEL_ERROR);
		}
	}
	setBoardMask(boardmask);
	
	// select firt channel to handle
	nextSelectedChannelNr();
}

// ----------------------------------------------------------------------------
void RecordCmd::sendTpEvent()
{
	TPRecordEvent tp_event;
	tp_event.opcode = TPRECORD;
	tp_event.status = 0;
	if ((itsChannelMask[getBoardNr()] == 0xFFFF) && ((getChannelNr()% 16) == 0)) {
		tp_event.channel = ~0;
		itsChannels = 16;
	} else {
		tp_event.channel = TS->getChInputNr(getChannelNr());
		itsChannels = 1;
	}
	itsRcuStatus = 0;
	
	if ((itsStatus[getBoardNr()] == 0) && (itsRcuStatus == 0)) {
		TS->boardPort(getBoardNr()).send(tp_event);
		LOG_DEBUG_STR(formatString("Sending RecordCmd to boardnr[%d], channel[%08X]", getBoardNr(), tp_event.channel));
	}
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void RecordCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus[getBoardNr()] |= TBB_RCU_COMM_ERROR;
		itsRcuStatus |= TBB_TIMEOUT_ETH;
	}	else {
		TPRecordAckEvent tp_ack(event);
		
		if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) {
			itsRcuStatus |= (1 << (16 + (tp_ack.status & 0x0F)));
		}
		
		if ((tp_ack.status == 0) && (itsRcuStatus == 0)) {
			for (int ch = getChannelNr(); ch < (getChannelNr() + itsChannels); ch++) {
				TS->setChState(ch, 'R');
			}
		} 
		LOG_DEBUG_STR(formatString("Received RecordAck from boardnr[%d]", getBoardNr()));
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
		LOG_INFO_STR(formatString("ERROR RecordCmd Rcu[%d], DriverStatus[0x%08x], RcuStatus[0x%08x]",
			rcu, itsStatus[getBoardNr()],itsRcuStatus));
	}
	
	itsStatus[getBoardNr()] |= itsRcuStatus;
	
	if (itsChannels == 16) {
		setChannelNr((getBoardNr() * 16) + 15);
	}
	
	nextSelectedChannelNr();
}

// ----------------------------------------------------------------------------
void RecordCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBRecordAckEvent tbb_ack;
	
	tbb_ack.rcu_mask.reset();
	for (int32 channelnr = 0; channelnr < TS->maxChannels(); channelnr++) {
		if (TS->getChStatus(channelnr)) {
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

