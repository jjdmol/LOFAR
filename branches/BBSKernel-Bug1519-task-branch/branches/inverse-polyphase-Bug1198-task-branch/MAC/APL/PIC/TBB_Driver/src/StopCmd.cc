//#  StopCmd.cc: implementation of the StopCmd class
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
#include <Common/hexdump.h>
#include "StopCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a StopCmd object.----------------------------------------
StopCmd::StopCmd():
		itsChannels(1)
{
	TS = TbbSettings::instance();
	setWaitAck(true);		
}
	  
//--Destructor for StopCmd.---------------------------------------------------
StopCmd::~StopCmd() { }

// ----------------------------------------------------------------------------
bool StopCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_STOP)||(event.signal == TP_STOP_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void StopCmd::saveTbbEvent(GCFEvent& event)
{
	TBBStopEvent tbb_event(event);
	
	//setChannels(tbb_event.rcu_mask);
	
	int32 board;
	int32 rcu;
	string hdstr;
	hexdump(hdstr, &tbb_event.rcu_mask, sizeof(tbb_event.rcu_mask));
	LOG_DEBUG_STR(hdstr);
	LOG_DEBUG_STR("StopCmd rcu_mask=" << tbb_event);
	for (int i = 0; i < TS->maxChannels(); i++) {
		TS->convertCh2Rcu(i,&rcu);
		if (tbb_event.rcu_mask.test(rcu) == true) {
			if (TS->getChState(i) == 'R') {
				setChannel(rcu);
			}
		}
	}
	LOG_DEBUG_STR("StopCmd channel_mask=" << getChannels());
	
	// select firt channel to handle
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void StopCmd::sendTpEvent()
{
	TPStopEvent tp_event;
	
	tp_event.opcode = oc_STOP;
	tp_event.status = 0;
	
	if (getBoardChannels(getBoardNr()) == 0xFFFF) {
		tp_event.channel = 0xFFFFFFFF; // uint32 -> -1 to stop all
		itsChannels = 16;
	} else {
		tp_event.channel = TS->getChInputNr(getChannelNr());
		itsChannels = 1;
	}
			
	if (getStatus(getBoardNr()) == TBB_SUCCESS) {
		TS->boardPort(getBoardNr()).send(tp_event);
		TS->boardPort(getBoardNr()).setTimer(TS->timeout());
		LOG_DEBUG_STR(formatString("Sending StopCmd to boardnr[%d], channel[%08X]", getBoardNr(), tp_event.channel));
	} else {
		TS->boardPort(getBoardNr()).setTimer(0.0);
	}
}

// ----------------------------------------------------------------------------
void StopCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	} else {
		TPStopAckEvent tp_ack(event);
		
		if (tp_ack.status == 0) {
			for (int ch = getChannelNr(); ch < (getChannelNr() + itsChannels); ch++) {
				TS->setChState(ch, 'S');
				TS->setChStatus(ch,0);
			}
		} else {
			for (int ch = getChannelNr(); ch < (getChannelNr() + itsChannels); ch++) {
				TS->setChState(ch, 'E');
				TS->setChStatus(ch,(uint16)tp_ack.status);
			}
			addStatus(getBoardNr(), (tp_ack.status << 24)); 
		}
		LOG_DEBUG_STR(formatString("Received StopAck from boardnr[%d]", getBoardNr()));
	}
	
	if (getStatus(getBoardNr()) != TBB_SUCCESS) {
		int32 rcu;
		TS->convertCh2Rcu(getChannelNr(),&rcu);
		if (itsChannels == 1) {
			LOG_INFO_STR(formatString("ERROR StopCmd Rcu[%d], Status[0x%08x]",
				rcu, getStatus(getBoardNr())));
		} else {
			LOG_INFO_STR(formatString("ERROR StopCmd Rcu[%d .. %d], Status[0x%08x]",
				rcu, (rcu + 16), getStatus(getBoardNr())));
		}
	}

	if (itsChannels == 16) {
		incChannelNr(16);
	} 
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void StopCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBStopAckEvent tbb_ack;
	
	tbb_ack.rcu_mask.reset();
	for (int32 channelnr = 0; channelnr < TS->maxChannels(); channelnr++) {
		if (TS->getChStatus(channelnr) == 0) {
			int32 rcunr;
			TS->convertCh2Rcu(channelnr,&rcunr);
			tbb_ack.rcu_mask.set(rcunr);
		}
	}
	
	for (int32 boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		tbb_ack.status_mask[boardnr] = getStatus(boardnr);
	}
		
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}

