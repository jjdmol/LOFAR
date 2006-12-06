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

#include "VersionCmd.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;	
	namespace TBB {

//--Constructors for a VersionCmd object.--------------------------------------
VersionCmd::VersionCmd():
		itsBoardMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPVersionEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBVersionackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsTBBackE->status[boardnr]		= 0;
	}
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
	if ((event.signal == TBB_VERSION)||(event.signal == TP_VERSIONACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void VersionCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE 			= new TBBVersionEvent(event);
	
	// mask for the installed boards
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();	
	itsBoardMask = itsTBBE->boardmask;
	
	for (int boardnr = 0; boardnr < DriverSettings::instance()->maxBoards(); boardnr++) {
		if (!(itsBoardsMask & (1 << boardnr))) 
			itsTBBackE->status[boardnr] |= NO_BOARD;
		
		if (!(itsBoardsMask & (1 << boardnr)) &&  (itsBoardMask & (1 << boardnr)))
			itsTBBackE->status[boardnr] |= (SELECT_ERROR | BOARD_SEL_ERROR);
	}
	
	// Send only commands to boards installed
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	// fill TP command, to send
	itsTPE->opcode 			  = TPVERSION;
	itsTPE->status				= 0;
			
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
bool VersionCmd::sendTpEvent(int32 boardnr, int32)
{
	bool sending = false;
	DriverSettings*		ds = DriverSettings::instance();
	
	if (ds->boardPort(boardnr).isConnected() && (itsTBBackE->status[boardnr] == 0)) {
		ds->boardPort(boardnr).send(*itsTPE);
		ds->boardPort(boardnr).setTimer(ds->timeout());
		sending = true;
	}
	else
		itsTBBackE->status[boardnr] |= CMD_ERROR;
	
	return(sending);
}

// ----------------------------------------------------------------------------
void VersionCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status[boardnr] |= COMM_ERROR;
	}
	else {
		//TPVersionEvent tpe(event);
		itsTPackE = new TPVersionackEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status[boardnr] |= (1 << (16 + (itsTPackE->status & 0x0F)));	
		
		itsTBBackE->boardid[boardnr] 			= itsTPackE->boardid;
		itsTBBackE->swversion[boardnr]  	= itsTPackE->swversion;
		itsTBBackE->boardversion[boardnr]	= itsTPackE->boardversion;
		itsTBBackE->tpversion[boardnr]		= itsTPackE->tpversion;
		itsTBBackE->mp0version[boardnr] 	= itsTPackE->mp0version;
		itsTBBackE->mp1version[boardnr] 	= itsTPackE->mp1version;
		itsTBBackE->mp2version[boardnr] 	= itsTPackE->mp2version;
		itsTBBackE->mp3version[boardnr] 	= itsTPackE->mp3version;
		
		LOG_DEBUG_STR(formatString("VersionCmd: board[%d] %08X;%u;%u;%u;%u;%u;%u;%u;%u",
				boardnr,itsTBBackE->status[boardnr],itsTPackE->boardid,itsTPackE->swversion,itsTPackE->boardversion,
				itsTPackE->tpversion,itsTPackE->mp0version,itsTPackE->mp1version,itsTPackE->mp2version,itsTPackE->mp3version));
		
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void VersionCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < DriverSettings::instance()->maxBoards(); boardnr++) { 
		if (itsTBBackE->status[boardnr] == 0)
			itsTBBackE->status[boardnr] = SUCCESS;
	}

	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
CmdTypes VersionCmd::getCmdType()
{
	return(BoardCmd);
}

// ----------------------------------------------------------------------------
uint32 VersionCmd::getBoardMask()
{
	return(itsBoardMask);
}

// ----------------------------------------------------------------------------
bool VersionCmd::waitAck()
{
	return(true);
}

	} // end namespace TBB
} // end namespace LOFAR
