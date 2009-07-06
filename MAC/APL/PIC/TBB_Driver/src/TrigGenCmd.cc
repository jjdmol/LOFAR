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
	itsStage(0), itsChannels(0)
{
	TS = TbbSettings::instance();
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
	
	setChannels(tbb_event.rcu_mask);
	
	// select firt channel to handle
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void TrigGenCmd::sendTpEvent()
{
	switch (itsStage) {
		case 0: {
			TPTrigReleaseEvent tp_event;
			tp_event.opcode = oc_TRIG_RELEASE;
			tp_event.status = 0;
			
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
			LOG_DEBUG_STR(formatString("Sending TrigRelease(clear) to boardnr[%d]",getBoardNr()));
			TS->boardPort(getBoardNr()).send(tp_event);
		} break;
		
		case 1: {
			TPTrigGenerateEvent tp_event;
			tp_event.opcode = oc_TRIG_GENERATE;
			tp_event.status = 0;
			
			// look if all channels are selected
			if (getBoardChannels(getBoardNr()) == 0xFFFF) {
				tp_event.mp = 0xFFFFFFFF; // set mp = -1, to select all mp's
				tp_event.channel_mask = 0xF;
				itsChannels = 16;
			} else {
				tp_event.mp = TS->getChMpNr(getChannelNr());
				tp_event.channel_mask = getMpChannels(getBoardNr(), TS->getChMpNr(getChannelNr())) & 0xF;
				itsChannels = 4;
			}
			LOG_DEBUG_STR(formatString("Sending TrigGenerate to boardnr[%d]",getBoardNr()));
			TS->boardPort(getBoardNr()).send(tp_event);
		} break;
		
		default: {
		} break;
	}
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void TrigGenCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	} else {
		switch (itsStage) {
			case 0: {
				TPTrigReleaseAckEvent tp_ack(event);
				LOG_DEBUG_STR(formatString("Received TrigReleaseAck from boardnr[%d]", getBoardNr()));
				
				if (tp_ack.status != 0) {
					setStatus(getBoardNr(), (tp_ack.status << 24));
				}
			} break;
			
			case 1: {
				TPTrigGenerateAckEvent tp_ack(event);
				LOG_DEBUG_STR(formatString("Received TrigGenerateAck from boardnr[%d]", getBoardNr()));
				
				if (tp_ack.status != 0) {
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
		nextChannelNr();
	}
}

// ----------------------------------------------------------------------------
void TrigGenCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBTrigGenerateAckEvent tbb_ack;
	
	for (int32 i = 0; i < MAX_N_TBBOARDS; i++) { 
		tbb_ack.status_mask[i] = getStatus(i);
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
