//#  StopCmd.cc: implementation of the StopCmd class
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

#include "StopCmd.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a StopCmd object.----------------------------------------
StopCmd::StopCmd():
		itsBoardMask(0),itsErrorMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPStopEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBStopackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsBoardStatus[boardnr]	= 0;
		itsChannelMask[boardnr]	= 0;
	}		
}
	  
//--Destructor for StopCmd.---------------------------------------------------
StopCmd::~StopCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool StopCmd::isValid(GCFEvent& event)
{
	if((event.signal == TBB_STOP)||(event.signal == TP_STOPACK)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void StopCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE 			= new TBBStopEvent(event);
		
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
	itsTPE->opcode			= TPSTOP;
	itsTPE->status			=	0;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void StopCmd::sendTpEvent(int32 boardnr, int32 channelnr)
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
void StopCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPStopackEvent(event);
		
		itsBoardStatus[boardnr]	= itsTPackE->status;
		
		LOG_DEBUG_STR(formatString("Received StopAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void StopCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if(itsErrorMask != 0) {
		itsTBBackE->status |= COMM_ERROR;
		itsTBBackE->status |= (itsErrorMask << 16);
	}
	if(itsTBBackE->status == 0) itsTBBackE->status = SUCCESS;
		
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
uint32 StopCmd::getBoardMask()
{
	return itsBoardMask;
}

// ----------------------------------------------------------------------------
CmdTypes StopCmd::getCmdType()
{
	return ChannelCmd;
}

// ----------------------------------------------------------------------------
bool StopCmd::waitAck()
{
	return true;
}

	} // end TBB namespace
} // end LOFAR namespace
