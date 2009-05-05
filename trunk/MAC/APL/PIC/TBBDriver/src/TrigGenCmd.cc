//#  TrigGenCmd.cc: implementation of the TrigGenCmd class
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

#include "TrigGenCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a TrigGenCmd object.----------------------------------------
TrigGenCmd::TrigGenCmd():
	itsMp(0), itsStage(0)
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		itsStatus[boardnr] = TBB_NO_BOARD;
		itsChannelMask[boardnr] = 0;
	}
	setWaitAck(true);		
}

//--Destructor for TrigGenCmd.---------------------------------------------------
TrigGenCmd::~TrigGenCmd() { }

// ----------------------------------------------------------------------------
bool TrigGenCmd::isValid(GCFEvent& event)
{
	if (	(event.signal == TBB_TRIG_GENERATE)
				||(event.signal == TP_TRIG_GENERATE_ACK)
				||(event.signal == TP_TRIG_RELEASE_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void TrigGenCmd::saveTbbEvent(GCFEvent& event)
{
	
	TBBTrigGenerateEvent tbb_event(event);
	
	// convert rcu-bitmask to tbb-channelmask
	int32 board;				// board 0 .. 11
	int32 board_channel;// board_channel 0 .. 15	
	int32 channel;			// channel 0 .. 191 (= maxboard * max_channels_on_board)	
	for (int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		if(tbb_event.rcu_mask.test(rcunr)) {
			TS->convertRcu2Ch(rcunr,&board,&board_channel);
			channel = (board * TS->nrChannelsOnBoard()) + board_channel;	
			itsChannelMask[board] |= (1 << board_channel);
			TS->setChSelected(channel,true);
		}
		//LOG_DEBUG_STR(formatString("channelmask board[%d] = 0x%08X",board,itsChannelMask[board]));
	} 
	
	uint32 boardmask = 0;	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		if (itsChannelMask[boardnr] != 0)  boardmask |= (1 << boardnr);
	}
	setBoardMask(boardmask);
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsStatus[boardnr] = 0;
		if (TS->isBoardActive(boardnr) == false) {
			itsStatus[boardnr] |= TBB_NO_BOARD;
		}
	}
	
	LOG_DEBUG_STR(formatString("boardmask = 0x%08X",boardmask));
	
	// select firt channel to handle
	nextSelectedChannelNr();
}

// ----------------------------------------------------------------------------
void TrigGenCmd::sendTpEvent()
{
	// send cmd if no errors
	if (itsStatus[getBoardNr()] == 0) {
		switch (itsStage) {
			case 0: {
				// set all channels to zero
				TPTrigReleaseEvent tp_event;
				
				tp_event.opcode = oc_TRIG_RELEASE;
				tp_event.status = 0;
				tp_event.mp = 0xFFFFFFFF; 
				tp_event.channel_mask = 0;
				
				itsMp = tp_event.mp;
				TS->boardPort(getBoardNr()).send(tp_event);
			} break;
			
			case 1: {
				TPTrigGenerateEvent tp_event;
				tp_event.opcode = oc_TRIG_GENERATE;
				tp_event.status = 0;
				
				if (itsChannelMask[getBoardNr()] == 0xFFFF) {
					tp_event.mp = ~0; // set mp = -1, all channels on all mp's will be set
					tp_event.channel_mask = 0xF;
				} else {
				
					uint32 mpnr = TS->getChMpNr(getChannelNr());
					tp_event.mp = mpnr;
						
					uint32 chmask = (itsChannelMask[getBoardNr()] >> (mpnr * 4)) & 0xF; // only 4 bits
					tp_event.channel_mask = chmask;	
				}
				
				itsMp = tp_event.mp;
				TS->boardPort(getBoardNr()).send(tp_event);
				LOG_DEBUG_STR(formatString("Sending TrigGenerate to boardnr[%d], channel_mask[0x%08X]",getBoardNr(),itsChannelMask[getBoardNr()]));
			} break;
			
			default: break;
		}
	}
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void TrigGenCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus[getBoardNr()] |= TBB_COMM_ERROR;
	} else {
		switch (itsStage) {
			case 0: {
				TPTrigReleaseAckEvent tp_ack(event);
				if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) {
					itsStatus[getBoardNr()] |= (1 << (16 + (tp_ack.status & 0x0F)));
				}
				itsStage = 1;
			} break;
			
			case 1: {
				TPTrigGenerateAckEvent tp_ack(event);
		
				if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) {
					itsStatus[getBoardNr()] |= (1 << (16 + (tp_ack.status & 0x0F)));
				}
			
				LOG_DEBUG_STR(formatString("Received TrigGenerateAck from boardnr[%d]", getBoardNr()));
				
				if (itsMp == 0xFFFFFFFF) {
					// all channels done, go to next board
					setChannelNr((getBoardNr() * 16) + 15);
				} else {
					// one mp done, go to next mp
					setChannelNr((getBoardNr() * 16) + (TS->getChMpNr(getChannelNr()) * 4) + 3);
				}
				int board = getBoardNr();
				nextSelectedChannelNr();
				if (board != getBoardNr()) {
					itsStage = 0;
				}
			} break;
			
			default: break;
		}
	}
}

// ----------------------------------------------------------------------------
void TrigGenCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBTrigGenerateAckEvent tbb_ack;
	
	for (int32 boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		if (itsStatus[boardnr] == 0) {
			tbb_ack.status_mask[boardnr] = TBB_SUCCESS;
		} else {
			tbb_ack.status_mask[boardnr] = itsStatus[boardnr];
		}
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
