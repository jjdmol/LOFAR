//#  ReadrCmd.cc: implementation of the ReadrCmd class
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

#include "ReadrCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a ReadrCmd object.----------------------------------------
ReadrCmd::ReadrCmd():
		itsStatus(0), itsMp(0), itsPid(0), itsRegId(0)
{
	TS = TbbSettings::instance();
	for (int32 an = 0; an < 256;an++) {
		itsData[an] = 0;
	}
	setWaitAck(true);
}

//--Destructor for ReadrCmd.---------------------------------------------------
ReadrCmd::~ReadrCmd() { }

// ----------------------------------------------------------------------------
bool ReadrCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_READR)||(event.signal == TP_READR_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ReadrCmd::saveTbbEvent(GCFEvent& event)
{
	TBBReadrEvent tbb_event(event);
	
	itsMp    = static_cast<uint32>(tbb_event.mp);
	itsPid   = static_cast<uint32>(tbb_event.pid);
	itsRegId = static_cast<uint32>(tbb_event.regid);
	
	if (TS->isBoardActive(tbb_event.board)) {	
		setBoardNr(tbb_event.board);
	} else {
		itsStatus |= TBB_NO_BOARD ;
		setDone(true);
	}
}

// ----------------------------------------------------------------------------
void ReadrCmd::sendTpEvent()
{
	TPReadrEvent tp_event;
	
	tp_event.opcode = TPREADR;
	tp_event.status = 0;
	tp_event.mp     = itsMp;
	tp_event.pid    = itsPid;
	tp_event.regid  = itsRegId;
		
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void ReadrCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus |= TBB_COMM_ERROR;
	}	else {
		TPReadrAckEvent tp_ack(event);
		
		if (tp_ack.status == 0) {
			for (int32 an = 0; an < 256;an++) {
				itsData[an] = tp_ack.data[an];
			}
		}
		LOG_DEBUG_STR(formatString("Received ReadrAck from boardnr[%d]", getBoardNr()));
	}
	setDone(true);
}

// ----------------------------------------------------------------------------
void ReadrCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBReadrAckEvent tbb_ack;
	
	if (itsStatus == 0) {
		tbb_ack.status_mask = TBB_SUCCESS;
	} else {
		tbb_ack.status_mask = itsStatus;
	}
	
	for (int32 an = 0; an < 256;an++) {
		tbb_ack.data[an] = itsData[an];
	}
	
	if (clientport->isConnected()) {clientport->send(tbb_ack);}
}
