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
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a TrigCoefCmd object.----------------------------------------
TrigCoefCmd::TrigCoefCmd()
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPTrigCoefEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBTrigCoefAckEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBOARDS;boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
	}
	setWaitAck(true);		
}
	  
//--Destructor for TrigCoefCmd.---------------------------------------------------
TrigCoefCmd::~TrigCoefCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

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
	itsTBBE	= new TBBTrigCoefEvent(event);
		
	int32 board;				// board 0 .. 11
	int32 board_channel;// board_channel 0 .. 15	
	int32 channel;			// channel 0 .. 191 (= maxboard * max_channels_on_board)
	
	for(int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		TS->convertRcu2Ch(rcunr,&board,&board_channel);	
		channel = (board * TS->nrChannelsOnBoard()) + board_channel;
		TS->setChFilterCoefficient(channel, 0, itsTBBE->coefficients[rcunr].c0);
		TS->setChFilterCoefficient(channel, 1, itsTBBE->coefficients[rcunr].c1);
		TS->setChFilterCoefficient(channel, 2, itsTBBE->coefficients[rcunr].c2);
		TS->setChFilterCoefficient(channel, 3, itsTBBE->coefficients[rcunr].c3);
	}
	
	// Send only commands to boards installed
	uint32 boardmask;
	boardmask = TS->activeBoardsMask();
	setBoardMask(boardmask);
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsTBBackE->status_mask[boardnr] = 0;
		if (TS->isBoardActive(boardnr) == false) {
			itsTBBackE->status_mask[boardnr] |= TBB_NO_BOARD;
		}
	}
	
	// select firt channel to handle
	nextChannelNr();
	
	// initialize TP send frame
	itsTPE->opcode			= TPTRIGCOEF;
	itsTPE->status			=	0;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void TrigCoefCmd::sendTpEvent()
{
	// send cmd if no errors
	if (itsTBBackE->status_mask[getBoardNr()] == 0) {
		uint32 mpnr = TS->getChMpNr(getChannelNr());
		itsTPE->mp = mpnr;
		
		for (int ch = 0; ch < 4; ch++) {
			itsTPE->channel[ch].c0 = TS->getChFilterCoefficient((getChannelNr() + ch), 0);
			itsTPE->channel[ch].c1 = TS->getChFilterCoefficient((getChannelNr() + ch), 1);
			itsTPE->channel[ch].c2 = TS->getChFilterCoefficient((getChannelNr() + ch), 2);
			itsTPE->channel[ch].c3 = TS->getChFilterCoefficient((getChannelNr() + ch), 3);
		}
		
		TS->boardPort(getBoardNr()).send(*itsTPE);
		LOG_DEBUG_STR(formatString("Sending TrigCoef to boardnr[%d], channel[%d]",getBoardNr(),getChannelNr()));
	}
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());	
}

// ----------------------------------------------------------------------------
void TrigCoefCmd::saveTpAckEvent(GCFEvent& event)
{
		// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask[getBoardNr()] |= TBB_COMM_ERROR;
	}
	else {
		itsTPackE = new TPTrigCoefAckEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status_mask[getBoardNr()] |= (1 << (16 + (itsTPackE->status & 0x0F)));
			
		LOG_DEBUG_STR(formatString("Received TrigCoefAck from boardnr[%d]", getBoardNr()));
		delete itsTPackE;
	}
	// one mp done, go to next mp
	setChannelNr((getBoardNr() * TS->nrChannelsOnBoard()) + (TS->getChMpNr(getChannelNr()) * TS->nrMpsOnBoard()) + 3);
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void TrigCoefCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < TS->maxBoards(); boardnr++) { 
		if (itsTBBackE->status_mask[boardnr] == 0)
			itsTBBackE->status_mask[boardnr] = TBB_SUCCESS;
	}
	
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
