//#  FreeCmd.cc: implementation of the FreeCmd class
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

#include "FreeCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a FreeCmd object.----------------------------------------
FreeCmd::FreeCmd():
		itsChannels(1)
{
	TS = TbbSettings::instance();
	
	setWaitAck(true);
}

//--Destructor for FreeCmd.---------------------------------------------------
FreeCmd::~FreeCmd() { }

// ----------------------------------------------------------------------------
bool FreeCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_FREE)||(event.signal == TP_FREE_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void FreeCmd::saveTbbEvent(GCFEvent& event)
{
	TBBFreeEvent tbb_event(event);
	
	setChannels(tbb_event.rcu_mask);
	
	nextChannelNr(); // select firt channel to handle
}

// ----------------------------------------------------------------------------
void FreeCmd::sendTpEvent()
{
	TPFreeEvent tp_event;
	tp_event.opcode = oc_FREE;
	tp_event.status = 0;
	
	if (getBoardChannels(getBoardNr()) == 0xFFFF) {
		tp_event.channel = 0xFFFFFFFF; // uint32 -> -1 to free all
		itsChannels = 16;
	} else {
		tp_event.channel = TS->getChInputNr(getChannelNr());
		itsChannels = 1;
	}
			
	if (getStatus(getBoardNr()) == TBB_SUCCESS) {
		TS->boardPort(getBoardNr()).send(tp_event);
		TS->boardPort(getBoardNr()).setTimer(TS->timeout());
		LOG_DEBUG_STR(formatString("Sending FreeCmd to boardnr[%d], channel[%08X]", getBoardNr(), tp_event.channel));
	} else {
		TS->boardPort(getBoardNr()).setTimer(0.0);
	}
}

// ----------------------------------------------------------------------------
void FreeCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	} else {
		TPFreeAckEvent tp_ack(event);
		
		if (tp_ack.status == 0) {
			for (int ch = getChannelNr(); ch < (getChannelNr() + itsChannels); ch++) {
				TS->setChState(ch, 'F'); 	
				TS->setChStartAddr(ch, 0);
				TS->setChPageSize(ch, 0);
				TS->setChStatus(ch,0); 
			}
		} else {
			for (int i = getChannelNr(); i < (getChannelNr() + itsChannels); i++) {
				TS->setChState(i, 'E');
				TS->setChStatus(i,(uint16)tp_ack.status);
			}
			addStatus(getBoardNr(), (tp_ack.status << 24)); 
		}
		
		LOG_DEBUG_STR(formatString("Received FreeAck from boardnr[%d]", getBoardNr()));
	}
	
	if (getStatus(getBoardNr()) != TBB_SUCCESS) {
		int32 rcu;
		TS->convertCh2Rcu(getChannelNr(),&rcu);
		if (itsChannels == 1) {
			LOG_INFO_STR(formatString("ERROR FreeCmd Rcu[%d], Status[0x%08x]",
				rcu, getStatus(getBoardNr())));
		} else {
			LOG_INFO_STR(formatString("ERROR FreeCmd Rcu[%d .. %d], Status[0x%08x]",
				rcu, (rcu + 16), getStatus(getBoardNr())));
		}
	}

	if (itsChannels == 16) {
		incChannelNr(16);
	} 
	nextChannelNr();
}

// ----------------------------------------------------------------------------
void FreeCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBFreeAckEvent tbb_ack;
	
	int32 rcunr;
	
	tbb_ack.rcu_mask.reset();
	for (int32 i = 0; i < TS->maxChannels(); i++) {
		if (TS->getChStatus(i) != 0) {
			TS->convertCh2Rcu(i,&rcunr);
			tbb_ack.rcu_mask.set(rcunr);
		}
	}
	
	for (int32 i = 0; i < MAX_N_TBBOARDS; i++) { 
		tbb_ack.status_mask[i] = getStatus(i);
	}
 
	if(clientport->isConnected()) { clientport->send(tbb_ack); }
}
