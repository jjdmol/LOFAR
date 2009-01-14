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

#include "TrigSetupCmd.h"


using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a TrigSetupCmd object.----------------------------------------
TrigSetupCmd::TrigSetupCmd()
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPTrigSetupEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBTrigSetupAckEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBOARDS;boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
	}
	setWaitAck(true);		
}
	  
//--Destructor for TrigSetupCmd.---------------------------------------------------
TrigSetupCmd::~TrigSetupCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

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
	itsTBBE	= new TBBTrigSetupEvent(event);
	
	int32 board;				// board 0 .. 11
	int32 board_channel;// board_channel 0 .. 15	
	int32 channel;			// channel 0 .. 191 (= maxboard * max_channels_on_board)
	
	for(int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		TS->convertRcu2Ch(rcunr,&board,&board_channel);	
		channel = (board * TS->nrChannelsOnBoard()) + board_channel;
		
		TS->setChTriggerLevel(channel, itsTBBE->setup[rcunr].level);
		TS->setChTriggerStartMode(channel, (itsTBBE->setup[rcunr].start_mode));
		TS->setChTriggerStopMode(channel, (itsTBBE->setup[rcunr].stop_mode));
		TS->setChFilterSelect(channel, itsTBBE->setup[rcunr].filter_select);
		TS->setChDetectWindow(channel, itsTBBE->setup[rcunr].window);
		TS->setChOperatingMode(channel, itsTBBE->setup[rcunr].operating_mode);
	}
	TS->setTriggerMode(itsTBBE->trigger_mode);
	
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
	itsTPE->opcode			= TPTRIGSETUP;
	itsTPE->status			=	0;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void TrigSetupCmd::sendTpEvent()
{
	// send cmd if no errors
	if (itsTBBackE->status_mask[getBoardNr()] == 0) {
		itsTPE->mp = TS->getChMpNr(getChannelNr());
		for (int ch = 0; ch < 4; ch++) {
			itsTPE->channel[ch].level = static_cast<uint32>(TS->getChTriggerLevel(getChannelNr() + ch));
			itsTPE->channel[ch].td_mode = static_cast<uint32>((TS->getChTriggerStartMode(getChannelNr() + ch) +
																		(TS->getChTriggerStopMode(getChannelNr() + ch) << 4)));
			itsTPE->channel[ch].filter_select = static_cast<uint32>(TS->getChFilterSelect(getChannelNr() + ch));
			itsTPE->channel[ch].window = static_cast<uint32>(TS->getChDetectWindow(getChannelNr() + ch));
			itsTPE->channel[ch].dummy = static_cast<uint32>(TS->getTriggerMode());
		}
		
		TS->boardPort(getBoardNr()).send(*itsTPE);
		
	}
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());	
}

// ----------------------------------------------------------------------------
void TrigSetupCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask[getBoardNr()] |= TBB_COMM_ERROR;
	}	else {
		itsTPackE = new TPTrigSetupAckEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status_mask[getBoardNr()] |= (1 << (16 + (itsTPackE->status & 0x0F)));
		
		LOG_DEBUG_STR(formatString("Received TrigSetupAck from boardnr[%d]", getBoardNr()));
		delete itsTPackE;
	}
	// one mp done, go to next mp
	setChannelNr((getBoardNr() * TS->nrChannelsOnBoard()) + (TS->getChMpNr(getChannelNr()) * TS->nrMpsOnBoard()) + 3);
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void TrigSetupCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < TS->maxBoards(); boardnr++) { 
		if (itsTBBackE->status_mask[boardnr] == 0)
			itsTBBackE->status_mask[boardnr] = TBB_SUCCESS;
	}
	
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
