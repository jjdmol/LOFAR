//#  ReadwCmd.cc: implementation of the ReadwCmd class
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

#include "ReadwCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a ReadwCmd object.----------------------------------------
ReadwCmd::ReadwCmd():
		itsStatus(0), itsMp(0), itsAddr(0)
{
	TS = TbbSettings::instance();
	for (int i = 0; i < 8; i++) {
		itsData[i] = 0;
	}
	setWaitAck(true);
}
	  
//--Destructor for ReadwCmd.---------------------------------------------------
ReadwCmd::~ReadwCmd() { }

// ----------------------------------------------------------------------------
bool ReadwCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_READW)||(event.signal == TP_READW_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ReadwCmd::saveTbbEvent(GCFEvent& event)
{
	TBBReadwEvent tbb_event(event);
	
	if (TS->isBoardActive(tbb_event.board)) {	
		setBoardNr(tbb_event.board);
	} else {
		itsStatus |= TBB_NO_BOARD ;
		setDone(true);
	}
	itsMp = static_cast<uint32>(tbb_event.mp);
	itsAddr = tbb_event.addr;
}

// ----------------------------------------------------------------------------
void ReadwCmd::sendTpEvent()
{
	TPReadwEvent tp_event;
	tp_event.opcode = TPREADW;
	tp_event.status = 0;
	tp_event.mp     = itsMp;
	tp_event.addr   = itsAddr;
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void ReadwCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus |= TBB_COMM_ERROR;
	} else {
		TPReadwAckEvent tp_ack(event);
		
		if (tp_ack.status == 0) {
			for (int i = 0; i < 8; i++) {
				itsData[i] = tp_ack.word[i];
			}
		}
		LOG_DEBUG_STR(formatString("Received ReadwAck from boardnr[%d]", getBoardNr()));
	}
	setDone(true);
}

// ----------------------------------------------------------------------------
void ReadwCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBReadwAckEvent tbb_ack;
	
	if (itsStatus == 0) {
		tbb_ack.status_mask = TBB_SUCCESS;
	} else {
		tbb_ack.status_mask = itsStatus;
	}	
	
	for (int i = 0; i < 8; i++) {
		tbb_ack.word[i]= itsData[i];
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
