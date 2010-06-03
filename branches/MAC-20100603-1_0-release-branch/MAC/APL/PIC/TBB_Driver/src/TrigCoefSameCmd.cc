//#  TrigCoefSameCmd.cc: implementation of the TrigCoefSameCmd class
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

#include "TrigCoefSameCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a TrigCoefSameCmd object.----------------------------------------
TrigCoefSameCmd::TrigCoefSameCmd()
{
	TS = TbbSettings::instance();
	setWaitAck(true);		
}
	  
//--Destructor for TrigCoefSameCmd.---------------------------------------------------
TrigCoefSameCmd::~TrigCoefSameCmd() { }

// ----------------------------------------------------------------------------
bool TrigCoefSameCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_TRIG_COEF_SAME)||(event.signal == TP_TRIG_COEF_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void TrigCoefSameCmd::saveTbbEvent(GCFEvent& event)
{
	TBBTrigCoefSameEvent tbb_event(event);
		
	int32 board;         // board 0 .. 11
	int32 board_channel; // board_channel 0 .. 15	
	int32 channel;       // channel 0 .. 191 (= maxboard * max_channels_on_board)
	
	for(int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		if (tbb_event.rcu_mask.test(rcunr) == true) {
			TS->convertRcu2Ch(rcunr,&board,&board_channel);	
			channel = (board * TS->nrChannelsOnBoard()) + board_channel;
                        for (int c = 0; c < 4; c++) {
                            TS->setChFilterCoefficient(channel, 0, c, tbb_event.coefficients.filter0[c]);
                            TS->setChFilterCoefficient(channel, 1, c, tbb_event.coefficients.filter1[c]);
                        }
		}
	}
	
	setChannels(tbb_event.rcu_mask);
	
	// select firt channel to handle
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void TrigCoefSameCmd::sendTpEvent()
{
	TPTrigCoefEvent tp_event;
	
	tp_event.opcode = oc_TRIG_COEF;
	tp_event.status = 0;
	
	tp_event.mp = TS->getChMpNr(getChannelNr());
	int start_channel = TS->getFirstChannelNr(getBoardNr(), TS->getChMpNr(getChannelNr()));
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
void TrigCoefSameCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	}	else {
		TPTrigSetupAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received TrigCoefAck from boardnr[%d]", getBoardNr()));
		
		if (tp_ack.status != 0) {
			setStatus(getBoardNr(), (tp_ack.status << 24));
			int start_channel = TS->getFirstChannelNr(getBoardNr(), TS->getChMpNr(getChannelNr()));
                        for (int ch = 0; ch < 4; ch++) {
                            for (int f = 0; f < 2; f++) {
                                for (int c = 0; c < 4; c++) {
                                        TS->setChFilterCoefficient((start_channel + ch), f, c, 0);
                                }
                            }
                        }
		}
	}
	// one mp done, go to next mp
	setChannelNr(TS->getFirstChannelNr(getBoardNr(), TS->getChMpNr(getChannelNr())) + 3);
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void TrigCoefSameCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBTrigCoefAckEvent tbb_ack;
	
	for (int32 i = 0; i < MAX_N_TBBOARDS; i++) { 
		tbb_ack.status_mask[i] = getStatus(i);
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
