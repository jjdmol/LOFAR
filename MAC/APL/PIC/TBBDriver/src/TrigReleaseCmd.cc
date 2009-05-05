//#  TrigReleaseCmd.cc: implementation of the TrigReleaseCmd class
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

#include "TrigReleaseCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using namespace TBB;

//--Constructors for a TrigReleaseCmd object.----------------------------------------
TrigReleaseCmd::TrigReleaseCmd():
	itsStage(0), itsMp(0), itsMpMask(0)
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		itsStatus[boardnr] = TBB_NO_BOARD;
		itsChannelStopMask[boardnr] = 0;
		itsChannelStartMask[boardnr] = 0;
	}
	setWaitAck(true);
}
	  
//--Destructor for TrigReleaseCmd.---------------------------------------------------
TrigReleaseCmd::~TrigReleaseCmd() { }

// ----------------------------------------------------------------------------
bool TrigReleaseCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_TRIG_RELEASE) || (event.signal == TP_TRIG_RELEASE_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void TrigReleaseCmd::saveTbbEvent(GCFEvent& event)
{
	TBBTrigReleaseEvent tbb_event(event);
		
	// convert rcu-bitmask to tbb-channelmask
	int32 board;
	int32 channel;
	for (int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		if(tbb_event.rcu_stop_mask.test(rcunr)) {
			TS->convertRcu2Ch(rcunr,&board,&channel);	
			itsChannelStopMask[board] |= (1 << channel);
			TS->setChSelected((channel + (board * TS->nrChannelsOnBoard())),true);
		}
		if(tbb_event.rcu_start_mask.test(rcunr)) {
			TS->convertRcu2Ch(rcunr,&board,&channel);	
			itsChannelStartMask[board] |= (1 << channel);
			TS->setChSelected((channel + (board * TS->nrChannelsOnBoard())),true);
		}
	} 
	
	uint32 boardmask = 0;	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsStatus[boardnr] = 0;
		if (TS->isBoardActive(boardnr)) {
			if ((itsChannelStopMask[boardnr] != 0) || (itsChannelStartMask[boardnr] != 0)) {
				boardmask |= (1 << boardnr);
			}
		} else {
			itsStatus[boardnr] |= TBB_NO_BOARD;
		}
		
		if (itsStatus[boardnr] != 0) {
			LOG_DEBUG_STR(formatString("TrigReleaseCmd savetbb bnr[%d], status[0x%08X]",boardnr, itsStatus[boardnr]));
		}
	}	
	
	setBoardMask(boardmask);
	
	nextSelectedChannelNr();
	
	if (itsChannelStopMask[getBoardNr()] != 0) itsStage = 0;
}

// ----------------------------------------------------------------------------
void TrigReleaseCmd::sendTpEvent()
{
	// send cmd if no errors
	if (itsStatus[getBoardNr()] == 0) {
		TPTrigReleaseEvent tp_event;
		tp_event.opcode = oc_TRIG_RELEASE;
		tp_event.status = 0;
		
		switch (itsStage) {
			case 0: {
				// look if all channels are selected
				if (itsChannelStopMask[getBoardNr()] == 0xFFFF) {
					// all channels are selected
					// set mp = -1, now all 16 channels will be set to 0 (reset)
					tp_event.mp = 0xFFFFFFFF; 
					tp_event.channel_mask = 0;
				} else {
					uint32 mpnr = TS->getChMpNr(getChannelNr());
					tp_event.mp = mpnr;
					
					uint32	reset_mask = 0; // default, reset all 4 channels
					int32		first_channel = (getBoardNr() * 16) + (mpnr * 4);
					uint32	mp_mask = (itsChannelStopMask[getBoardNr()] >> (mpnr * 4)) & 0xF; // only 4 bits
					LOG_DEBUG_STR(formatString("mp_mask[0x%08X]",mp_mask));
					// mask channels that NOT require a reset
					for (int ch = 0; ch < 4; ch++) {
						if (	TS->isChTriggerReleased(first_channel + ch)
									&& ((mp_mask & (1 << ch)) == 0)) {
							reset_mask |= (1 << ch);
						}			
					}
					LOG_DEBUG_STR(formatString("reset_mask[0x%08X]",reset_mask));
					tp_event.channel_mask = reset_mask;
					
					itsMp = tp_event.mp;
					itsMpMask = tp_event.channel_mask;	
				}
			} break;
			
			case 1: {
				// look if all channels are selected
				if (itsChannelStartMask[getBoardNr()] == 0xFFFF) {
					// all channels are selected
					// set mp = -1, all channels on all mp's will be set to 1 (released)
					tp_event.mp = 0xFFFFFFFF; 
					tp_event.channel_mask = 0xF;
				} else {
					uint32 mpnr = TS->getChMpNr(getChannelNr());
					tp_event.mp = mpnr;
					
					uint32	release_mask = 0x0; // default, don't select channels 
					int32		first_channel = (getBoardNr() * 16) + (mpnr * 4);
					uint32	mp_mask = (itsChannelStartMask[getBoardNr()] >> (mpnr * 4)) & 0xF; // only 4 bits
					
					for (int ch = 0; ch < 4; ch++) {
						if (	TS->isChTriggerReleased(first_channel + ch) 
									|| (mp_mask & (1 << ch))) {
							release_mask |= (1 << ch);
						}			
					}
					tp_event.channel_mask = release_mask;
				
					itsMp = tp_event.mp;
					itsMpMask = tp_event.channel_mask;	
				}
			} break;

			default: { 
			} break;
		}
		TS->boardPort(getBoardNr()).send(tp_event);
	}
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void TrigReleaseCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus[getBoardNr()] |= TBB_COMM_ERROR;
	} else {
		TPTrigReleaseAckEvent tp_ack(event);
		
		switch (itsStage) {
			case 0: {
				if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) {
					itsStatus[getBoardNr()] |= (1 << (16 + (tp_ack.status & 0x0F)));
				}
				uint32 mpnr = TS->getChMpNr(getChannelNr());
				int32		first_channel = (getBoardNr() * 16) + (mpnr * 4);
				for (int32 ch = 0; ch < 4; ch++) {
					if ((itsMpMask & (1 << ch)) == 0) {
						TS->setChTriggerReleased((first_channel + ch), false);
						TS->setChTriggered((first_channel + ch), false);
					} 
				}
				LOG_DEBUG_STR(formatString("Received TrigReleaseAck from boardnr[%d]", getBoardNr()));
				if (itsChannelStartMask[getBoardNr()] != 0) {
					itsStage = 1;
				} else {
					if (itsMp == 0xFFFFFFFF) {
					// all channels done, go to next board
						setChannelNr((getBoardNr() * 16) + 15);
					} else {
					// one mp done, go to next mp
						setChannelNr((getBoardNr() * 16) + (TS->getChMpNr(getChannelNr()) * 4) + 3);
					}
					nextSelectedChannelNr();
				}
			} break;
			
			case 1: {
				if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) {
					itsStatus[getBoardNr()] |= (1 << (16 + (tp_ack.status & 0x0F)));
				} else {
					uint32 mpnr = TS->getChMpNr(getChannelNr());
					int32		first_channel = (getBoardNr() * 16) + (mpnr * 4);
					for (int32 ch = 0; ch < 4; ch++) {
						if (itsMpMask & (1 << ch)) {
							TS->setChTriggerReleased((first_channel + ch), true);
						} 
					}
				}
				LOG_DEBUG_STR(formatString("Received TrigReleaseAck from boardnr[%d]", getBoardNr()));
				
				if (itsMp == 0xFFFFFFFF) {
					// all channels done, go to next board
					setChannelNr((getBoardNr() * 16) + 15);
				} else {
					// one mp done, go to next mp
					setChannelNr((getBoardNr() * 16) + (TS->getChMpNr(getChannelNr()) * 4) + 3);
				}
				if (itsChannelStopMask[getBoardNr()] != 0) { itsStage = 0; }
				nextSelectedChannelNr();
			} break;
			default: break;
		}
	}
}

// ----------------------------------------------------------------------------
void TrigReleaseCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBTrigReleaseAckEvent tbb_ack;
	
	for (int32 boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		if (itsStatus[boardnr] == 0) {
			tbb_ack.status_mask[boardnr] = TBB_SUCCESS;
		} else {
			tbb_ack.status_mask[boardnr] = itsStatus[boardnr];
		}
	}
		
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
