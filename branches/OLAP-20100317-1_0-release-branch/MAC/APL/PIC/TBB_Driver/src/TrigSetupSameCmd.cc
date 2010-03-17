//#  TrigSetupSameCmd.cc: implementation of the TrigSetupSameCmd class
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

#include "TrigSetupSameCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a TrigSetupSameCmd object.----------------------------------------
TrigSetupSameCmd::TrigSetupSameCmd()
{
	TS = TbbSettings::instance();
	setWaitAck(true);		
}
	  
//--Destructor for TrigSetupSameCmd.---------------------------------------------------
TrigSetupSameCmd::~TrigSetupSameCmd() { }

// ----------------------------------------------------------------------------
bool TrigSetupSameCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_TRIG_SETUP_SAME)||(event.signal == TP_TRIG_SETUP_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void TrigSetupSameCmd::saveTbbEvent(GCFEvent& event)
{
	TBBTrigSetupSameEvent tbb_event(event);
	
	int32 board;				// board 0 .. 11
	int32 board_channel;// board_channel 0 .. 15	
	int32 channel;			// channel 0 .. 191 (= maxboard * max_channels_on_board)
	
	for(int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		if (tbb_event.rcu_mask.test(rcunr) == true) {
			TS->convertRcu2Ch(rcunr,&board,&board_channel);	
			channel = (board * TS->nrChannelsOnBoard()) + board_channel;
			
			TS->setChTriggerLevel(channel, tbb_event.setup.level);
			TS->setChTriggerStartMode(channel, (tbb_event.setup.start_mode));
			TS->setChTriggerStopMode(channel, (tbb_event.setup.stop_mode));
			TS->setChFilterSelect(channel, tbb_event.setup.filter_select);
			TS->setChDetectWindow(channel, tbb_event.setup.window);
			TS->setChTriggerMode(channel, tbb_event.setup.trigger_mode);
		}
	}
		
	setChannels(tbb_event.rcu_mask);
	
	// select firt channel to handle
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void TrigSetupSameCmd::sendTpEvent()
{
	TPTrigSetupEvent tp_event;
	tp_event.opcode = oc_TRIG_SETUP;
	tp_event.status = 0;

	tp_event.mp = TS->getChMpNr(getChannelNr());
	int start_channel = TS->getFirstChannelNr(getBoardNr(), TS->getChMpNr(getChannelNr()));
	for (int i = 0; i < 4; i++) {
		tp_event.channel[i].level         = static_cast<uint32>(TS->getChTriggerLevel(start_channel + i));
		tp_event.channel[i].td_mode       = static_cast<uint32>((TS->getChTriggerStartMode(start_channel + i) +
												(TS->getChTriggerStopMode(start_channel + i) << 4)));
		tp_event.channel[i].filter_select = static_cast<uint32>(TS->getChFilterSelect(start_channel + i));
		tp_event.channel[i].window        = static_cast<uint32>(TS->getChDetectWindow(start_channel + i));
		tp_event.channel[i].trigger_mode  = static_cast<uint32>(TS->getChTriggerMode(start_channel + i));
	}
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());	
}

// ----------------------------------------------------------------------------
void TrigSetupSameCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	}	else {
		TPTrigSetupAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received TrigSetupAck from boardnr[%d]", getBoardNr()));
		
		if (tp_ack.status != 0) {
			setStatus(getBoardNr(), (tp_ack.status << 24));
			int start_channel = TS->getFirstChannelNr(getBoardNr(), TS->getChMpNr(getChannelNr()));
			for (int i = 0; i < 4; i++) {
				TS->setChTriggerLevel((start_channel + i), 0);
				TS->setChTriggerStartMode((start_channel + i), 0);
				TS->setChTriggerStopMode((start_channel + i), 0);
				TS->setChFilterSelect((start_channel + i), 0);
				TS->setChDetectWindow((start_channel + i), 0);
				TS->setChTriggerMode((start_channel + i), 0);
			}
		}
	}
	// one mp done, go to next mp
	setChannelNr(TS->getFirstChannelNr(getBoardNr(), TS->getChMpNr(getChannelNr())) + 3);
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void TrigSetupSameCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBTrigSetupSameAckEvent tbb_ack;
	
	for (int32 i = 0; i < MAX_N_TBBOARDS; i++) { 
		tbb_ack.status_mask[i] = getStatus(i);
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
