//#  PageperiodCmd.cc: implementation of the PageperiodCmd class
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

#include "PageperiodCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using namespace TBB;

//--Constructors for a PageperiodCmd object.----------------------------------------
PageperiodCmd::PageperiodCmd():
		itsStatus(0), itsChannel(0), itsPagePeriod(0)
{
	TS = TbbSettings::instance();
	setWaitAck(true);
}
	  
//--Destructor for PageperiodCmd.---------------------------------------------------
PageperiodCmd::~PageperiodCmd() { }

// ----------------------------------------------------------------------------
bool PageperiodCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_PAGEPERIOD)||(event.signal == TP_PAGEPERIOD_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void PageperiodCmd::saveTbbEvent(GCFEvent& event)
{
	TBBPageperiodEvent tbb_event(event);
	
	int32 boardnr;
	TS->convertRcu2Ch(tbb_event.rcu,&boardnr,&itsChannel);
		
	setBoardNr(boardnr);
			
	itsStatus = 0;
}

// ----------------------------------------------------------------------------
void PageperiodCmd::sendTpEvent()
{
	TPPageperiodEvent tp_event;
	tp_event.opcode = TPPAGEPERIOD;
	tp_event.status = 0;
	tp_event.channel = static_cast<uint32>(itsChannel); 
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void PageperiodCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus |= TBB_COMM_ERROR;
	}
	else {
		TPPageperiodAckEvent tp_ack(event);
		
		if (tp_ack.status == 0) {
			itsPagePeriod = tp_ack.pageperiod;
		}
		LOG_DEBUG_STR(formatString("Received PageperiodAck from boardnr[%d]", getBoardNr()));
	}
	setDone(true);
}

// ----------------------------------------------------------------------------
void PageperiodCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBPageperiodAckEvent tbb_ack;
	
	tbb_ack.pageperiod = itsPagePeriod;
	if (itsStatus == 0) {
		tbb_ack.status_mask = TBB_SUCCESS;
	} else {
		tbb_ack.status_mask = itsStatus;
	}
	 
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
