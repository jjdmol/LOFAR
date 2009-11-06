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
	
	setBoards(tbb_event.boardmask);
	
	nextBoardNr(); // select firt board
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
		setStatus(getBoardNr(), TBB_TIME_OUT);
	}
	else {
		//TPVersionEvent tpe(event);
		TPVersionAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received VersionAck from boardnr[%d]", getBoardNr()));
		if (tp_ack.status != 0) {
			setStatus(getBoardNr(), (tp_ack.status << 24));
		} else {
			itsBoardId[getBoardNr()]     = tp_ack.boardid;
			itsTpSwVersion[getBoardNr()] = tp_ack.tpswversion;
			itsBoardVersion[getBoardNr()]= tp_ack.boardversion;
			itsTpHwVersion[getBoardNr()] = tp_ack.tphwversion;
			itsMp0Version[getBoardNr()]  = (tp_ack.mp0version >> 24);
			itsMp1Version[getBoardNr()]  = (tp_ack.mp1version >> 24);
			itsMp2Version[getBoardNr()]  = (tp_ack.mp2version >> 24);
			itsMp3Version[getBoardNr()]  = (tp_ack.mp3version >> 24);
			
			LOG_DEBUG_STR(formatString("VersionCmd: board[%d] %08X;%u;%u;%u;%u;%u;%u;%u;%u",
							  getBoardNr(),getStatus(getBoardNr()),tp_ack.boardid,tp_ack.tpswversion,tp_ack.boardversion,
					tp_ack.tphwversion,tp_ack.mp0version,tp_ack.mp1version,tp_ack.mp2version,tp_ack.mp3version));
		}
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void VersionCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBVersionAckEvent tbb_ack;
	
	for (int32 i = 0; i < MAX_N_TBBOARDS; i++) { 
		tbb_ack.status_mask[i]  = getStatus(i);
		tbb_ack.boardid[i]      = itsBoardId[i];
		tbb_ack.tpswversion[i]  = itsTpSwVersion[i];
		tbb_ack.boardversion[i] = itsBoardVersion[i];
		tbb_ack.tphwversion[i]  = itsTpHwVersion[i];
		tbb_ack.mp0version[i]   = itsMp0Version[i];
		tbb_ack.mp1version[i]   = itsMp1Version[i];
		tbb_ack.mp2version[i]   = itsMp2Version[i];
		tbb_ack.mp3version[i]   = itsMp3Version[i];
	}
	tbb_ack.driverversion = TS->driverVersion(); 

	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}