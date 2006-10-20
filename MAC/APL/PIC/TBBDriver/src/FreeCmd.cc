//#  FreeCmd.cc: implementation of the FreeCmd class
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

#include "FreeCmd.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a FreeCmd object.----------------------------------------
FreeCmd::FreeCmd():
		itsBoardMask(0),itsErrorMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPFreeEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBFreeackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsBoardStatus[boardnr]	= 0;
		itsChannelMask[boardnr]	= 0;
	}
}
	  
//--Destructor for FreeCmd.---------------------------------------------------
FreeCmd::~FreeCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool FreeCmd::isValid(GCFEvent& event)
{
	if((event.signal == TBB_FREE)||(event.signal == TP_FREEACK)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void FreeCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE 			= new TBBFreeEvent(event);

	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) {
		itsChannelMask[boardnr] = itsTBBE->channelmask[boardnr]; // for some commands board-id is used ???
		if(itsChannelMask[boardnr] != 0)  itsBoardMask |= (1 << boardnr);
	}
	
	// mask for the installed boards
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();
	
	// Send only commands to boards installed
	itsErrorMask = itsBoardMask & ~itsBoardsMask;
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	itsTBBackE->status = 0;
	
	// initialize TP send frame
	itsTPE->opcode			= TPFREE;
	itsTPE->status			= 0;
		
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void FreeCmd::sendTpEvent(int32 boardnr, int32 channelnr)
{
	DriverSettings*		ds = DriverSettings::instance();
	itsTPE->channel = DriverSettings::instance()->getChBoardChannelNr(channelnr); 
	
	if(ds->boardPort(boardnr).isConnected()) {
		ds->boardPort(boardnr).send(*itsTPE);
		ds->boardPort(boardnr).setTimer(ds->timeout());
	}
	else
		itsErrorMask |= (1 << boardnr);
}

// ----------------------------------------------------------------------------
void FreeCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPFreeackEvent(event);
		
		itsBoardStatus[boardnr]			= itsTPackE->status;
		
		LOG_DEBUG_STR(formatString("Received FreeAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void FreeCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if(itsErrorMask != 0) {
		itsTBBackE->status |= COMM_ERROR;
		itsTBBackE->status |= (itsErrorMask << 16);
	}
	if(itsTBBackE->status == 0) itsTBBackE->status = SUCCESS;
 
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
CmdTypes FreeCmd::getCmdType()
{
	return BoardCmd;
}

// ----------------------------------------------------------------------------
uint32 FreeCmd::getBoardMask()
{
	return itsBoardMask;
}

// ----------------------------------------------------------------------------
bool FreeCmd::waitAck()
{
	return true;
}

	} // end TBB namespace
} // end LOFAR namespace
