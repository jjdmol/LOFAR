//#  TrigSetupCmd.cc: implementation of the TrigSetupCmd class
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
#include <Common/lofar_bitset.h>

#include "TrigSetupCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a TrigSetupCmd object.----------------------------------------
TrigSetupCmd::TrigSetupCmd()
{
	TS = TbbSettings::instance();
	setWaitAck(true);		
}
	  
//--Destructor for TrigSetupCmd.---------------------------------------------------
TrigSetupCmd::~TrigSetupCmd() { }

// ----------------------------------------------------------------------------
bool TrigSetupCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_TRIG_SETUP)||(event.signal == TP_TRIG_SETUP_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void TrigSetupCmd::saveTbbEvent(GCFEvent& event)
{
	TBBTrigSetupEvent tbb_event(event);
	
	int32 board;				// board 0 .. 11
	int32 board_channel;// board_channel 0 .. 15	
	int32 channel;			// channel 0 .. 191 (= maxboard * max_channels_on_board)
	
	for(int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		TS->convertRcu2Ch(rcunr,&board,&board_channel);	
		channel = (board * TS->nrChannelsOnBoard()) + board_channel;
		
		TS->setChTriggerLevel(channel, tbb_event.rcu[rcunr].level);
		TS->setChTriggerStartMode(channel, (tbb_event.rcu[rcunr].start_mode));
		TS->setChTriggerStopMode(channel, (tbb_event.rcu[rcunr].stop_mode));
		TS->setChFilterSelect(channel, tbb_event.rcu[rcunr].filter_select);
		TS->setChDetectWindow(channel, tbb_event.rcu[rcunr].window);
		TS->setChTriggerMode(channel, tbb_event.rcu[rcunr].trigger_mode);
		TS->setChOperatingMode(channel, tbb_event.rcu[rcunr].operating_mode);
	}
	
	bitset<MAX_N_RCUS> channels;
	channels.set();
	setChannels(channels);
	
	// select firt channel to handle
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void TrigSetupCmd::sendTpEvent()
{
	TPTrigSetupEvent tp_event;
	tp_event.opcode = oc_TRIG_SETUP;
	tp_event.status = 0;

	tp_event.mp = TS->getChMpNr(getChannelNr());
	for (int i = 0; i < 4; i++) {
		tp_event.channel[i].level         = static_cast<uint32>(TS->getChTriggerLevel(getChannelNr() + i));
		tp_event.channel[i].td_mode       = static_cast<uint32>((TS->getChTriggerStartMode(getChannelNr() + i) +
														  (TS->getChTriggerStopMode(getChannelNr() + i) << 4)));
		tp_event.channel[i].filter_select = static_cast<uint32>(TS->getChFilterSelect(getChannelNr() + i));
		tp_event.channel[i].window        = static_cast<uint32>(TS->getChDetectWindow(getChannelNr() + i));
		tp_event.channel[i].trigger_mode  = static_cast<uint32>(TS->getChTriggerMode(getChannelNr() + i));
	}
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());	
}

// ----------------------------------------------------------------------------
void TrigSetupCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	}	else {
		TPTrigSetupAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received TrigSetupAck from boardnr[%d]", getBoardNr()));
		
		if (tp_ack.status != 0) {
			setStatus(getBoardNr(), (tp_ack.status << 24));
		}
	}
	// one mp done, go to next mp
	setChannelNr(TS->getFirstChannelNr(getBoardNr(), TS->getChMpNr(getChannelNr())) + 3);
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void TrigSetupCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBTrigSetupAckEvent tbb_ack;
	
	for (int32 i = 0; i < MAX_N_TBBOARDS; i++) { 
		tbb_ack.status_mask[i] = getStatus(i);
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
