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
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a TrigInfoCmd object.----------------------------------------
TrigInfoCmd::TrigInfoCmd()
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPTrigInfoEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBTrigInfoAckEvent();
	
	itsTBBackE->status_mask	= 0;
	setWaitAck(true);		
}
	  
//--Destructor for TrigInfoCmd.---------------------------------------------------
TrigInfoCmd::~TrigInfoCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

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
	itsTBBE	= new TBBTrigInfoEvent(event);
	
	int32 board;				// board 0 .. 11
	int32 board_channel;// board_channel 0 .. 15	
	int32 channel;			// channel 0 .. 191 (= maxboard * max_channels_on_board)	
		
	TS->convertRcu2Ch(itsTBBE->rcu,&board,&board_channel);
	channel = (board * TS->nrChannelsOnBoard()) + board_channel;
	TS->setChSelected(channel,true);
	
	// Send only commands to boards installed
	uint32 boardmask;
	boardmask = TS->activeBoardsMask();
	setBoardMask(boardmask);
	
	itsTBBackE->status_mask = 0;
	
	nextSelectedChannelNr();
	
	// initialize TP send frame
	itsTPE->opcode			= TPTRIGINFO;
	itsTPE->status			=	0;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void TrigInfoCmd::sendTpEvent()
{
	// send cmd if no errors
	if (itsTBBackE->status_mask == 0) {
		
		itsTPE->channel = static_cast<uint32>(getChannelNr()); 
		TS->boardPort(getBoardNr()).send(*itsTPE);
		LOG_DEBUG_STR(formatString("Sending TrigInfo to boardnr[%d], channel[%d]",getBoardNr(),getChannelNr()));
	}
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());	
}

// ----------------------------------------------------------------------------
void TrigInfoCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask |= TBB_COMM_ERROR;
	} else {
		itsTPackE = new TPTrigInfoAckEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status_mask |= (1 << (16 + (itsTPackE->status & 0x0F)));
			
		TS->convertCh2Rcu(itsTPackE->trigger.channel,&itsTBBackE->rcu);
		itsTBBackE->sequence_nr			= itsTPackE->trigger.sequence_nr;
		itsTBBackE->time						= itsTPackE->trigger.time;
		itsTBBackE->sample_nr				=	itsTPackE->trigger.sample_nr;
		itsTBBackE->trigger_sum			= itsTPackE->trigger.sum;
		itsTBBackE->trigger_samples	= itsTPackE->trigger.samples;
		itsTBBackE->peak_value			= itsTPackE->trigger.peak;
		itsTBBackE->power_before		= itsTPackE->trigger.pwr_bt_at & 0x0000FFFF;
		itsTBBackE->power_after		= (itsTPackE->trigger.pwr_bt_at & 0xFFFF0000) >> 16;
		
		LOG_DEBUG_STR(formatString("Received TrigInfoAck from boardnr[%d]", getBoardNr()));
		delete itsTPackE;
	}
	nextSelectedChannelNr();
}

// ----------------------------------------------------------------------------
void TrigInfoCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if (itsTBBackE->status_mask == 0) {
		itsTBBackE->status_mask = TBB_SUCCESS;
	}
	
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
