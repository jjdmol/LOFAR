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
	itsStage(0), itsChannels(4)
{
	TS = TbbSettings::instance();
	itsChannelStopMask.reset();
	itsChannelStartMask.reset();
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
		
	for (int i = 0; i < TS->maxChannels(); i++) {
		itsChannelStopMask[i] = tbb_event.rcu_stop_mask[i];
		itsChannelStartMask[i] = tbb_event.rcu_start_mask[i];
	}
	
	setChannels(itsChannelStopMask);
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void TrigReleaseCmd::sendTpEvent()
{
	// send cmd if no errors
	TPTrigReleaseEvent tp_event;
	tp_event.opcode = oc_TRIG_RELEASE;
	tp_event.status = 0;
	
	switch (itsStage) {
		case 0: {
			// look if all channels are selected
			if (getBoardChannels(getBoardNr()) == 0xFFFF) {
				tp_event.mp = 0xFFFFFFFF; // set mp = -1, to select all mp's
				tp_event.channel_mask = 0;
				itsChannels = 16;
			} else {
				tp_event.mp = TS->getChMpNr(getChannelNr());
				tp_event.channel_mask = ~(getMpChannels(getBoardNr(), TS->getChMpNr(getChannelNr()))) & 0xF;
				itsChannels = 4;
			}
		} break;
		
		case 1: {
			// look if all channels are selected
			if (getBoardChannels(getBoardNr()) == 0xFFFF) {
				tp_event.mp = 0xFFFFFFFF; // set mp = -1, to select all mp's
				tp_event.channel_mask = 0xF;
			} else {
				tp_event.mp = TS->getChMpNr(getChannelNr());
				tp_event.channel_mask = getMpChannels(getBoardNr(), TS->getChMpNr(getChannelNr()));
				for (int i = 0; i < 4; ++i) {
					if (TS->isChTriggerReleased(getChannelNr()+i)) {
						tp_event.channel_mask |= (1 << i);
					}
				}
			}
		} break;

		default: { 
		} break;
	}
	LOG_DEBUG_STR(formatString("Sending TrigReleaseCmd to board %d, mp %d, ",
										getBoardNr(), TS->getChMpNr(getChannelNr())));
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void TrigReleaseCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	} else {
		TPTrigReleaseAckEvent tp_ack(event);
		
		LOG_DEBUG_STR(formatString("Received TrigReleaseAck from boardnr[%d]", getBoardNr()));
		switch (itsStage) {
			case 0: {
				if (tp_ack.status == 0) {
					int32 first_channel = TS->getFirstChannelNr(getBoardNr(), TS->getChMpNr(getChannelNr()));
					for (int32 i = 0; i < 4; i++) {
						if (itsChannelStopMask.test(first_channel + i)) {
							TS->setChTriggerReleased((first_channel + i), false);
							TS->setChTriggered((first_channel + i), false);
						} 
					}
				} else {
					setStatus(getBoardNr(), (tp_ack.status << 24));
				}
			} break;
			
			case 1: {
				if (tp_ack.status == 0) {
					int32 first_channel = TS->getFirstChannelNr(getBoardNr(), TS->getChMpNr(getChannelNr()));
					for (int32 i = 0; i < 4; i++) {
						if (itsChannelStartMask.test(first_channel + i)) {
							TS->setChTriggerReleased((first_channel + i), true);
						} 
					}
				} else {
					setStatus(getBoardNr(), (tp_ack.status << 24));
				}
			} break;
			
			default: {
			} break;
		}
	}
	
	if (itsChannels == 16) {
		nextBoardNr(); // 16 channels done, go to next board
	} else {
		// one mp done, go to next mp
		setChannelNr(TS->getFirstChannelNr(getBoardNr(), TS->getChMpNr(getChannelNr())) + 3);
		nextChannelNr();
	}
	
	// if all selected rcus are stopt, set start mask and run again
	if ((itsStage == 0) && (getChannelNr() == -1)) {
		itsStage = 1;
		setDone(false);
		setChannels(itsChannelStartMask);
		nextChannelNr();
	}
}

// ----------------------------------------------------------------------------
void TrigReleaseCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBTrigReleaseAckEvent tbb_ack;
	
	for (int32 i = 0; i < MAX_N_TBBOARDS; i++) { 
		tbb_ack.status_mask[i] = getStatus(i);
	}
		
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
