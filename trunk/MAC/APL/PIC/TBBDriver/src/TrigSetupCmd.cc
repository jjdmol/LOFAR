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
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a TrigSetupCmd object.----------------------------------------
TrigSetupCmd::TrigSetupCmd()
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		itsStatus[boardnr] = TBB_NO_BOARD;
	}
	setWaitAck(true);		
}
	  
//--Destructor for TrigSetupCmd.---------------------------------------------------
TrigSetupCmd::~TrigSetupCmd()
{

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
	TBBTrigSetupEvent tbb_event(event);
	
	int32 board;				// board 0 .. 11
	int32 board_channel;// board_channel 0 .. 15	
	int32 channel;			// channel 0 .. 191 (= maxboard * max_channels_on_board)
	
	for(int rcunr = 0; rcunr < TS->maxChannels(); rcunr++) {
		TS->convertRcu2Ch(rcunr,&board,&board_channel);	
		channel = (board * TS->nrChannelsOnBoard()) + board_channel;
		
		TS->setChTriggerLevel(channel, tbb_event.setup[rcunr].level);
		TS->setChTriggerStartMode(channel, (tbb_event.setup[rcunr].start_mode));
		TS->setChTriggerStopMode(channel, (tbb_event.setup[rcunr].stop_mode));
		TS->setChFilterSelect(channel, tbb_event.setup[rcunr].filter_select);
		TS->setChDetectWindow(channel, tbb_event.setup[rcunr].window);
		TS->setChOperatingMode(channel, tbb_event.setup[rcunr].operating_mode);
	}
	TS->setTriggerMode(tbb_event.trigger_mode);
	
	// Send only commands to boards installed
	uint32 boardmask;
	boardmask = TS->activeBoardsMask();
	setBoardMask(boardmask);
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsStatus[boardnr] = 0;
		if (TS->isBoardActive(boardnr) == false) {
			itsStatus[boardnr] |= TBB_NO_BOARD;
		}
	}
	
	// select firt channel to handle
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void TrigSetupCmd::sendTpEvent()
{
	// send cmd if no errors
	if (itsStatus[getBoardNr()] == 0) {
		TPTrigSetupEvent tp_event;
		tp_event.opcode = oc_TRIG_SETUP;
		tp_event.status = 0;
	
		tp_event.mp = TS->getChMpNr(getChannelNr());
		for (int ch = 0; ch < 4; ch++) {
			tp_event.channel[ch].level = static_cast<uint32>(TS->getChTriggerLevel(getChannelNr() + ch));
			tp_event.channel[ch].td_mode = static_cast<uint32>((TS->getChTriggerStartMode(getChannelNr() + ch) +
																		(TS->getChTriggerStopMode(getChannelNr() + ch) << 4)));
			tp_event.channel[ch].filter_select = static_cast<uint32>(TS->getChFilterSelect(getChannelNr() + ch));
			tp_event.channel[ch].window = static_cast<uint32>(TS->getChDetectWindow(getChannelNr() + ch));
			tp_event.channel[ch].dummy = static_cast<uint32>(TS->getTriggerMode());
		}
		
		TS->boardPort(getBoardNr()).send(tp_event);
	}
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());	
}

// ----------------------------------------------------------------------------
void TrigSetupCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus[getBoardNr()] |= TBB_COMM_ERROR;
	}	else {
		TPTrigSetupAckEvent tp_ack(event);
		
		if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) {
			itsStatus[getBoardNr()] |= (1 << (16 + (tp_ack.status & 0x0F)));
		}
		
		LOG_DEBUG_STR(formatString("Received TrigSetupAck from boardnr[%d]", getBoardNr()));
	}
	// one mp done, go to next mp
	setChannelNr((getBoardNr() * TS->nrChannelsOnBoard()) + (TS->getChMpNr(getChannelNr()) * TS->nrMpsOnBoard()) + 3);
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void TrigSetupCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBTrigSetupAckEvent tbb_ack;
	
	for (int32 boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		if (itsStatus[boardnr] == 0) {
			tbb_ack.status_mask[boardnr] = TBB_SUCCESS;
		} else {
			tbb_ack.status_mask[boardnr] = itsStatus[boardnr];
		}
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
