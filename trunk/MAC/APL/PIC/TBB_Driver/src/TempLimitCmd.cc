//#  TempLimitCmd.cc: implementation of the TempLimitCmd class
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

#include "TempLimitCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;	
using namespace TBB;

//--Constructors for a VersionCmd object.--------------------------------------
TempLimitCmd::TempLimitCmd():
	itsHigh(0), itsLow(0)
{
	TS = TbbSettings::instance();
	
	setWaitAck(true);
}
  
//--Destructor for GetVersions.------------------------------------------------
TempLimitCmd::~TempLimitCmd() { }

// ----------------------------------------------------------------------------
bool TempLimitCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_TEMP_LIMIT)||(event.signal == TP_TEMP_LIMIT_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void TempLimitCmd::saveTbbEvent(GCFEvent& event)
{
	TBBTempLimitEvent tbb_event(event);
	
	setBoards(tbb_event.boardmask);
	itsHigh = tbb_event.high;
	itsLow = tbb_event.low;
	
	nextBoardNr(); // select first board
}

// ----------------------------------------------------------------------------
void TempLimitCmd::sendTpEvent()
{
	TPTempLimitEvent tp_event;
	tp_event.opcode = oc_TEMP_LIMIT;
	tp_event.status = 0;
	tp_event.high   = itsHigh;
	tp_event.low    = itsLow;
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void TempLimitCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	}	else {
		TPTempLimitAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received TempLimitAck from boardnr[%d]", getBoardNr()));
		
		if (tp_ack.status != 0) {
			setStatus(getBoardNr(), (tp_ack.status << 24));
		}
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void TempLimitCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBTempLimitAckEvent tbb_ack;
	
	for (int32 i = 0; i < MAX_N_TBBOARDS; i++) { 
		tbb_ack.status_mask[i] = getStatus(i);
	}

	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
