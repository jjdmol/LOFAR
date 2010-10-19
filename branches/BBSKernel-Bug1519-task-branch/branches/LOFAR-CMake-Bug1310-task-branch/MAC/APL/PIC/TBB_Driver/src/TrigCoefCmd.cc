//#  TrigCoefCmd.cc: implementation of the TrigCoefCmd class
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

#include "TrigCoefCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a TrigCoefCmd object.----------------------------------------
TrigCoefCmd::TrigCoefCmd()
{
	TS = TbbSettings::instance();
	setWaitAck(true);		
}
	  
//--Destructor for TrigCoefCmd.---------------------------------------------------
TrigCoefCmd::~TrigCoefCmd() { }

// ----------------------------------------------------------------------------
bool TrigCoefCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_TRIG_COEF)||(event.signal == TP_TRIG_COEF_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void TrigCoefCmd::saveTbbEvent(GCFEvent& event)
{
	TBBTrigCoefEvent tbb_event(event);
		
	int32 board;         // board 0 .. 11
	int32 board_channel; // board_channel 0 .. 15	
	int32 channel;       // channel 0 .. 191 (= maxboard * max_channels_on_board)
	
	for(int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		TS->convertRcu2Ch(rcunr,&board,&board_channel);	
		channel = (board * TS->nrChannelsOnBoard()) + board_channel;
		TS->setChFilterCoefficient(channel, 0, tbb_event.coefficients[rcunr].c0);
		TS->setChFilterCoefficient(channel, 1, tbb_event.coefficients[rcunr].c1);
		TS->setChFilterCoefficient(channel, 2, tbb_event.coefficients[rcunr].c2);
		TS->setChFilterCoefficient(channel, 3, tbb_event.coefficients[rcunr].c3);
	}
	
	std::bitset<MAX_N_RCUS> channels;
	channels.set();
	setChannels(channels);
	
	// select firt channel to handle
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void TrigCoefCmd::sendTpEvent()
{
	TPTrigCoefEvent tp_event;
	
	tp_event.opcode = oc_TRIG_COEF;
	tp_event.status = 0;
	
	tp_event.mp = TS->getChMpNr(getChannelNr());
	for (int i = 0; i < 4; i++) {
		tp_event.channel[i].c0 = TS->getChFilterCoefficient((getChannelNr() + i), 0);
		tp_event.channel[i].c1 = TS->getChFilterCoefficient((getChannelNr() + i), 1);
		tp_event.channel[i].c2 = TS->getChFilterCoefficient((getChannelNr() + i), 2);
		tp_event.channel[i].c3 = TS->getChFilterCoefficient((getChannelNr() + i), 3);
	}

	LOG_DEBUG_STR(formatString("Sending TrigCoef to boardnr[%d]",getBoardNr()));
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());	
}

// ----------------------------------------------------------------------------
void TrigCoefCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	}	else {
		TPTrigSetupAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received TrigCoefAck from boardnr[%d]", getBoardNr()));
		
		if (tp_ack.status != 0) {
			setStatus(getBoardNr(), (tp_ack.status << 24));
		}
	}
	// one mp done, go to next mp
	setChannelNr(TS->getFirstChannelNr(getBoardNr(), TS->getChMpNr(getChannelNr())) + 3);
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void TrigCoefCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBTrigCoefAckEvent tbb_ack;
	
	for (int32 i = 0; i < MAX_N_TBBOARDS; i++) { 
		tbb_ack.status_mask[i] = getStatus(i);
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
