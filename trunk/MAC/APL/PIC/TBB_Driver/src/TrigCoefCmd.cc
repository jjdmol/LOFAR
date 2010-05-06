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
#include <Common/lofar_bitset.h>

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
	
	// if filter coefficients for each RCU are send change next loops
	// now rcu-0 and rcu-8 have the same settings
	//     rcu-1 and rcu-9 have the same settings
	//     etc
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
	    int startRCU = boardnr * TS->nrChannelsOnBoard();
	    for(int rcunr = startRCU; rcunr < startRCU + (TS->nrChannelsOnBoard() / 2); rcunr++) {
    		TS->convertRcu2Ch(rcunr,&board,&board_channel);	
    		channel = (board * TS->nrChannelsOnBoard()) + board_channel;
            for (int c = 0; c < 4; c++) {
                TS->setChFilterCoefficient(channel, 0, c, tbb_event.rcu[rcunr].filter0[c]);
                TS->setChFilterCoefficient(channel, 1, c, tbb_event.rcu[rcunr].filter1[c]);
            }
            // set settings for upper 8 channels of each board
            TS->convertRcu2Ch(rcunr+8,&board,&board_channel);	
            channel = (board * TS->nrChannelsOnBoard()) + board_channel;
            for (int c = 0; c < 4; c++) {
                TS->setChFilterCoefficient(channel, 0, c, tbb_event.rcu[rcunr].filter0[c]);
                TS->setChFilterCoefficient(channel, 1, c, tbb_event.rcu[rcunr].filter1[c]);
            }
            }
	}
	
	bitset<MAX_N_RCUS> channels;
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
            tp_event.coeffients_even.filter_0[i] = TS->getChFilterCoefficient(getChannelNr(), 0, i);
            tp_event.coeffients_even.filter_1[i] = TS->getChFilterCoefficient(getChannelNr(), 1, i);
            tp_event.coeffients_odd.filter_0[i]  = TS->getChFilterCoefficient((getChannelNr() + 2), 0, i);
            tp_event.coeffients_odd.filter_1[i]  = TS->getChFilterCoefficient((getChannelNr() + 2), 1, i);
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
