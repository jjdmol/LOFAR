//#  VersionCmd.cc: implementation of the VersionsCmd class
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

#include "VersionCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;	
using namespace TBB;

//--Constructors for a VersionCmd object.--------------------------------------
VersionCmd::VersionCmd()
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBOARDS;boardnr++) { 
		itsStatus[boardnr] = TBB_NO_BOARD;
		itsBoardId[boardnr] = 0;
		itsTpSwVersion[boardnr] = 0;
		itsBoardVersion[boardnr] = 0;
		itsTpHwVersion[boardnr] = 0;
		itsMp0Version[boardnr] = 0;
		itsMp1Version[boardnr] = 0;
		itsMp2Version[boardnr] = 0;
		itsMp3Version[boardnr] = 0;
	}
	setWaitAck(true);
}
  
//--Destructor for GetVersions.------------------------------------------------
VersionCmd::~VersionCmd() { }

// ----------------------------------------------------------------------------
bool VersionCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_VERSION)||(event.signal == TP_VERSION_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void VersionCmd::saveTbbEvent(GCFEvent& event)
{
	TBBVersionEvent tbb_event(event);
	
	setBoardMask(tbb_event.boardmask);
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsStatus[boardnr] = 0;
		if (!TS->isBoardActive(boardnr)) {
			itsStatus[boardnr] |= TBB_NO_BOARD;
		}
	}
	
	
	// select firt board
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void VersionCmd::sendTpEvent()
{
	TPVersionEvent tp_event;
	tp_event.opcode = oc_VERSION;
	tp_event.status = 0;
		
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void VersionCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus[getBoardNr()] |= TBB_COMM_ERROR;
	}
	else {
		//TPVersionEvent tpe(event);
		TPVersionAckEvent tp_ack(event);
		
		if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) {
			itsStatus[getBoardNr()] |= (1 << (16 + (tp_ack.status & 0x0F)));
		}
		
		itsBoardId[getBoardNr()]     = tp_ack.boardid;
		itsTpSwVersion[getBoardNr()]   = tp_ack.tpswversion;
		itsBoardVersion[getBoardNr()]= tp_ack.boardversion;
		itsTpHwVersion[getBoardNr()]   = tp_ack.tphwversion;
		itsMp0Version[getBoardNr()]  = (tp_ack.mp0version >> 24);
		itsMp1Version[getBoardNr()]  = (tp_ack.mp1version >> 24);
		itsMp2Version[getBoardNr()]  = (tp_ack.mp2version >> 24);
		itsMp3Version[getBoardNr()]  = (tp_ack.mp3version >> 24);
		
		LOG_DEBUG_STR(formatString("VersionCmd: board[%d] %08X;%u;%u;%u;%u;%u;%u;%u;%u",
				getBoardNr(),itsStatus[getBoardNr()],tp_ack.boardid,tp_ack.tpswversion,tp_ack.boardversion,
				tp_ack.tphwversion,tp_ack.mp0version,tp_ack.mp1version,tp_ack.mp2version,tp_ack.mp3version));
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void VersionCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBVersionAckEvent tbb_ack;
	
	for (int32 boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		if (itsStatus[boardnr] == 0) {
			tbb_ack.status_mask[boardnr] = TBB_SUCCESS;
		} else {
			tbb_ack.status_mask[boardnr] = itsStatus[boardnr];
		}
		tbb_ack.boardid[boardnr]      = itsBoardId[boardnr];
		tbb_ack.tpswversion[boardnr]  = itsTpSwVersion[boardnr];
		tbb_ack.boardversion[boardnr] = itsBoardVersion[boardnr];
		tbb_ack.tphwversion[boardnr]  = itsTpHwVersion[boardnr];
		tbb_ack.mp0version[boardnr]   = itsMp0Version[boardnr];
		tbb_ack.mp1version[boardnr]   = itsMp1Version[boardnr];
		tbb_ack.mp2version[boardnr]   = itsMp2Version[boardnr];
		tbb_ack.mp3version[boardnr]   = itsMp3Version[boardnr];
	}

	tbb_ack.driverversion = TS->driverVersion(); // set cvs version of TBBDriver.c


	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
