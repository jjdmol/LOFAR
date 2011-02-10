//#  WritewCmd.cc: implementation of the WritewCmd class
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

#include "WritewCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a WritewCmd object.----------------------------------------
WritewCmd::WritewCmd():
		itsMp(0), itsAddr(0)
{
	TS = TbbSettings::instance();
	for (int i = 0; i < 8; i++) {
		itsData[i] = 0;
	}
	setWaitAck(true);
}

//--Destructor for WritewCmd.---------------------------------------------------
WritewCmd::~WritewCmd() { }

// ----------------------------------------------------------------------------
bool WritewCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_WRITEW)||(event.signal == TP_WRITEW_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void WritewCmd::saveTbbEvent(GCFEvent& event)
{
	TBBWritewEvent tbb_event(event);
	
	setBoard(tbb_event.board);
	itsMp = static_cast<uint32>(tbb_event.mp);
	itsAddr = tbb_event.addr;
	for (int i = 0; i < 8; i++) {
		itsData[i] = tbb_event.word[i];
	}
	
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void WritewCmd::sendTpEvent()
{
	TPWritewEvent tp_event;
	tp_event.opcode = oc_WRITEW;
	tp_event.status = 0;
	tp_event.mp = itsMp;
	tp_event.addr = itsAddr;
	for (int i = 0; i < 8; i++) {
		tp_event.word[i] = itsData[i];
	}
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void WritewCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(0, TBB_TIME_OUT);
	}
	else {
		TPWritewAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received WritewAck from boardnr[%d]", getBoardNr()));
		
		if (tp_ack.status != 0) {
			setStatus(0, (tp_ack.status << 24));
		}
	}
	setDone(true);
}

// ----------------------------------------------------------------------------
void WritewCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBWritewAckEvent tbb_ack;
	
	tbb_ack.status_mask = getStatus(0);

	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
