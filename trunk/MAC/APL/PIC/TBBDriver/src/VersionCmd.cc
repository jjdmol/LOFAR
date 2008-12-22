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
using namespace TBB_Protocol;
using namespace TP_Protocol;	
using namespace TBB;

//--Constructors for a VersionCmd object.--------------------------------------
VersionCmd::VersionCmd()
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPVersionEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBVersionAckEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBOARDS;boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
		itsTBBackE->boardid[boardnr]			= 0;
		itsTBBackE->swversion[boardnr]  	= 0;
		itsTBBackE->boardversion[boardnr]	= 0;
		itsTBBackE->tpversion[boardnr]		= 0;
		itsTBBackE->mp0version[boardnr] 	= 0;
		itsTBBackE->mp1version[boardnr] 	= 0;
		itsTBBackE->mp2version[boardnr] 	= 0;
		itsTBBackE->mp3version[boardnr] 	= 0;
	}
	setWaitAck(true);
}
  
//--Destructor for GetVersions.------------------------------------------------
VersionCmd::~VersionCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

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
	itsTBBE	= new TBBVersionEvent(event);
	
	setBoardMask(itsTBBE->boardmask);
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsTBBackE->status_mask[boardnr] = 0;
		if (!TS->isBoardActive(boardnr)) {
			itsTBBackE->status_mask[boardnr] |= TBB_NO_BOARD;
		}
	}
	
	itsTBBackE->driverversion = TS->driverVersion(); // set cvs version of TBBDriver.c
	// select firt board
	nextBoardNr();
	
	// fill TP command, to send
	itsTPE->opcode 			  = TPVERSION;
	itsTPE->status				= 0;
			
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void VersionCmd::sendTpEvent()
{
		TS->boardPort(getBoardNr()).send(*itsTPE);
		TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void VersionCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask[getBoardNr()] |= TBB_COMM_ERROR;
	}
	else {
		//TPVersionEvent tpe(event);
		itsTPackE = new TPVersionAckEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status_mask[getBoardNr()] |= (1 << (16 + (itsTPackE->status & 0x0F)));	
		
		itsTBBackE->boardid[getBoardNr()] 		= itsTPackE->boardid;
		itsTBBackE->swversion[getBoardNr()]  	= itsTPackE->swversion;
		itsTBBackE->boardversion[getBoardNr()]= itsTPackE->boardversion;
		itsTBBackE->tpversion[getBoardNr()]		= itsTPackE->tpversion;
		itsTBBackE->mp0version[getBoardNr()] 	= (itsTPackE->mp0version >> 24);
		itsTBBackE->mp1version[getBoardNr()] 	= (itsTPackE->mp1version >> 24);
		itsTBBackE->mp2version[getBoardNr()] 	= (itsTPackE->mp2version >> 24);
		itsTBBackE->mp3version[getBoardNr()] 	= (itsTPackE->mp3version >> 24);
		
		LOG_DEBUG_STR(formatString("VersionCmd: board[%d] %08X;%u;%u;%u;%u;%u;%u;%u;%u",
				getBoardNr(),itsTBBackE->status_mask[getBoardNr()],itsTPackE->boardid,itsTPackE->swversion,itsTPackE->boardversion,
				itsTPackE->tpversion,itsTPackE->mp0version,itsTPackE->mp1version,itsTPackE->mp2version,itsTPackE->mp3version));
		
		delete itsTPackE;
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void VersionCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < TS->maxBoards(); boardnr++) { 
		if (itsTBBackE->status_mask[boardnr] == 0)
			itsTBBackE->status_mask[boardnr] = TBB_SUCCESS;
	}

	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
