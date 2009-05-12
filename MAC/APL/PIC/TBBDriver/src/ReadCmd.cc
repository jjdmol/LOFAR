//#  ReadCmd.cc: implementation of the ReadCmd class
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

#include "ReadCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;


//--Constructors for a ReadCmd object.----------------------------------------
ReadCmd::ReadCmd():
	itsStage(0), itsSecondstime(0), itsSampletime(0), itsPrepages(0), itsPostpages(0)
{
	TS = TbbSettings::instance();
	setWaitAck(true);
}
	  
//--Destructor for ReadCmd.---------------------------------------------------
ReadCmd::~ReadCmd() { }

// ----------------------------------------------------------------------------
bool ReadCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_READ)
		||(event.signal == TP_READ_ACK)
		||(event.signal == TP_WATCHDOG_ACK)
		||(event.signal == TP_CEP_STATUS_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ReadCmd::saveTbbEvent(GCFEvent& event)
{
	TBBReadEvent tbb_event(event);
	
	setChannel(tbb_event.rcu);
	
	itsSecondstime = tbb_event.secondstime;
	itsSampletime  = tbb_event.sampletime;
	itsPrepages    = tbb_event.prepages;
	itsPostpages   = tbb_event.postpages;
	
	nextChannelNr();
	// check if channel is stopped
	if (TS->getChState(getChannelNr()) != 'S') {
		setStatus(0, TBB_CH_NOT_STOPPED);
		setDone(true);
	}
}

// ----------------------------------------------------------------------------
void ReadCmd::sendTpEvent()
{
	switch (itsStage) {
		case 0: {
			TPReadEvent tp_event;
			
			tp_event.opcode      = oc_READ;
			tp_event.status      = 0;
			tp_event.channel     = TS->getChInputNr(getChannelNr());
			tp_event.secondstime = itsSecondstime;
			tp_event.sampletime  = itsSampletime;
			tp_event.prepages    = itsPrepages;
			tp_event.postpages   = itsPostpages;
			TS->boardPort(getBoardNr()).send(tp_event);
		} break;
		
		case 1: {
			TPCepStatusEvent tp_event;
			
			tp_event.opcode = oc_CEP_STATUS;
			tp_event.status = 0;
			TS->boardPort(getBoardNr()).send(tp_event);
		} break;
		
		default: {
		} break;
	}
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void ReadCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	}
	else if (event.signal == TP_READ_ACK) {
		TPReadAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received ReadAck from boardnr[%d]", getBoardNr()));
		LOG_DEBUG_STR(formatString("ReadAck.status=%d", tp_ack.status));
				
		if (tp_ack.status == 0xfd) {
			LOG_DEBUG_STR(formatString("TBB busy, %d pages left, trying until free", tp_ack.pages_left));
			setSleepTime(0.1);		
		} else {
			if (tp_ack.status != 0) {
				setStatus(getBoardNr(), (tp_ack.status << 24));
				setDone(true);
			}
			itsStage = 1;
		}
	}
	else if (event.signal == TP_CEP_STATUS_ACK) {
		TPCepStatusAckEvent tp_ack(event);
		
		if (tp_ack.status != 0) {
			setStatus(getBoardNr(), (tp_ack.status << 24));
			setDone(true);
		}
		else if (tp_ack.pages_left == 0) {
			setDone(true);
		} else {
			setSleepTime(0.1);
		}
	}
}

// ----------------------------------------------------------------------------
void ReadCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBReadAckEvent tbb_ack;
	
	tbb_ack.status_mask = getStatus(0);
		
	if (clientport->isConnected()) {clientport->send(tbb_ack); }
}
