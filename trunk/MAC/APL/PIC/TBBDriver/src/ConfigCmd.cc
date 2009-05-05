//#  ConfigCmd.cc: implementation of the ConfigCmd class
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
#include <unistd.h>

#include "ConfigCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a ConfigCmd object.----------------------------------------
ConfigCmd::ConfigCmd():
	itsImage(0)
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		itsStatus[boardnr] = TBB_NO_BOARD;
	}
	setWaitAck(true);
}

//--Destructor for ConfigCmd.---------------------------------------------------
ConfigCmd::~ConfigCmd() { }

// ----------------------------------------------------------------------------
bool ConfigCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_CONFIG)||(event.signal == TP_CONFIG_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ConfigCmd::saveTbbEvent(GCFEvent& event)
{
	TBBConfigEvent tbb_event(event);

	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsStatus[boardnr] = 0;
		if (TS->isBoardActive(boardnr) == false) { 
			itsStatus[boardnr] |= TBB_NO_BOARD;
		}
	}
	
	setBoardMask(tbb_event.boardmask);
	itsImage = tbb_event.imagenr;
	
	// select first boards
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void ConfigCmd::sendTpEvent()
{
	TPConfigEvent tp_event;
	tp_event.opcode = oc_CONFIG;
	tp_event.status = 0;
	tp_event.imagenr = itsImage;
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(5.0);
}

// ----------------------------------------------------------------------------
void ConfigCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus[getBoardNr()] |= TBB_COMM_ERROR;
	}	else {
		TPConfigAckEvent tp_ack(event);
		TS->setImageNr(getBoardNr(), itsImage);
		TS->setFreeToReset(getBoardNr(), false);
		if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) {
			itsStatus[getBoardNr()] |= (1 << (16 + (tp_ack.status & 0x0F)));
		}
		
		LOG_DEBUG_STR(formatString("Received ConfigAck from boardnr[%d]", getBoardNr()));
	}
	nextBoardNr();
	if (isDone()) { 
		setSleepTime(10.0);
	}
}

// ----------------------------------------------------------------------------
void ConfigCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBConfigAckEvent tbb_ack;
	
	for (int32 boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		if (itsStatus[boardnr] == 0) {
			tbb_ack.status_mask[boardnr] = TBB_SUCCESS;
		} else {
			tbb_ack.status_mask[boardnr] = itsStatus[boardnr];
		}
	}
	LOG_DEBUG_STR("Sending ConfigAck to client");
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
