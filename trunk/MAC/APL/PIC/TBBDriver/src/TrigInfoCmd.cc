//#  TrigInfoCmd.cc: implementation of the TrigInfoCmd class
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

#include "TrigInfoCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a TrigInfoCmd object.----------------------------------------
TrigInfoCmd::TrigInfoCmd():
	itsStatus(0), itsRcu(0), itsSequenceNr(0), itsTime(0), itsSampleNr(0), itsTriggerSum(0),
	itsTriggerSamples(0), itsPeakValue(0), itsPowerBefore(0), itsPowerAfter(0)
{
	TS = TbbSettings::instance();
	setWaitAck(true);
}

//--Destructor for TrigInfoCmd.---------------------------------------------------
TrigInfoCmd::~TrigInfoCmd() { }

// ----------------------------------------------------------------------------
bool TrigInfoCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_TRIG_INFO)||(event.signal == TP_TRIG_INFO_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void TrigInfoCmd::saveTbbEvent(GCFEvent& event)
{
	TBBTrigInfoEvent tbb_event(event);
	
	int32 board;			// board 0 .. 11
	int32 board_channel;// board_channel 0 .. 15	
	int32 channel;			// channel 0 .. 191 (= maxboard * max_channels_on_board)	
	
	itsRcu = tbb_event.rcu;
	TS->convertRcu2Ch(itsRcu,&board,&board_channel);
	channel = (board * TS->nrChannelsOnBoard()) + board_channel;
	TS->setChSelected(channel,true);
	
	// Send only commands to boards installed
	uint32 boardmask;
	boardmask = TS->activeBoardsMask();
	setBoardMask(boardmask);
	
	nextSelectedChannelNr();
}

// ----------------------------------------------------------------------------
void TrigInfoCmd::sendTpEvent()
{
	// send cmd if no errors
	if (itsStatus == 0) {
		TPTrigInfoEvent tp_event;
		
		tp_event.opcode = TPTRIGINFO;
		tp_event.status = 0;
		tp_event.channel = static_cast<uint32>(TS->getChInputNr(getChannelNr())); 
		
		TS->boardPort(getBoardNr()).send(tp_event);
		LOG_DEBUG_STR(formatString("Sending TrigInfo to boardnr[%d], channel[%d]",getBoardNr(),getChannelNr()));
	}
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());	
}

// ----------------------------------------------------------------------------
void TrigInfoCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus |= TBB_COMM_ERROR;
	} else {
		TPTrigInfoAckEvent tp_ack(event);
		
		if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) 
			itsStatus |= (1 << (16 + (tp_ack.status & 0x0F)));
		
		int32 channel = (getBoardNr() * TS->nrChannelsOnBoard()) + tp_ack.trigger.channel;
		TS->convertCh2Rcu(channel,&itsRcu);
		itsSequenceNr     = tp_ack.trigger.sequence_nr;
		itsTime           = tp_ack.trigger.time;
		itsSampleNr       = tp_ack.trigger.sample_nr;
		itsTriggerSum     = tp_ack.trigger.sum;
		itsTriggerSamples = tp_ack.trigger.samples;
		itsPeakValue      = tp_ack.trigger.peak;
		itsPowerBefore    = tp_ack.trigger.pwr_bt_at & 0x0000FFFF;
		itsPowerAfter     = (tp_ack.trigger.pwr_bt_at & 0xFFFF0000) >> 16;
		
		LOG_DEBUG_STR(formatString("Received TrigInfoAck from boardnr[%d]", getBoardNr()));
	}
	nextSelectedChannelNr();
}

// ----------------------------------------------------------------------------
void TrigInfoCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBTrigInfoAckEvent tbb_ack;
	
	if (itsStatus == 0) {
		tbb_ack.status_mask = TBB_SUCCESS;
	} else {
		tbb_ack.status_mask = itsStatus;
	}
	tbb_ack.rcu             = itsRcu;
	tbb_ack.sequence_nr     = itsSequenceNr;
	tbb_ack.time            = itsTime;
	tbb_ack.sample_nr       = itsSampleNr;
	tbb_ack.trigger_sum     = itsTriggerSum;
	tbb_ack.trigger_samples = itsTriggerSamples;
	tbb_ack.peak_value      = itsPeakValue;
	tbb_ack.power_before    = itsPowerBefore;
	tbb_ack.power_after     = itsPowerAfter;
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
