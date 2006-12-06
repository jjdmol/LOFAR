//#  ErasefCmd.cc: implementation of the ErasefCmd class
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

#include "ErasefCmd.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a ErasefCmd object.----------------------------------------
ErasefCmd::ErasefCmd():
		itsBoardMask(0),itsBoardsMask(0),itsBoardStatus(0)
{
	itsTPE 			= new TPErasefEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBErasefackEvent();
}
	  
//--Destructor for ErasefCmd.---------------------------------------------------
ErasefCmd::~ErasefCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool ErasefCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_ERASEF)||(event.signal == TP_ERASEFACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ErasefCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE 			= new TBBErasefEvent(event);
		
	itsBoardMask = (1 << itsTBBE->board);
	
	// mask for the installed boards
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();
	
	// Send only commands to boards installed
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	itsTBBackE->status = 0;
	
	// initialize TP send frame
	itsTPE->opcode	= TPERASEF;
	itsTPE->status	=	0;
	itsTPE->addr		= itsTBBE->addr;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
bool ErasefCmd::sendTpEvent(int32 boardnr, int32)
{
	bool sending = false;
	DriverSettings*		ds = DriverSettings::instance();
	
	if (ds->boardPort(boardnr).isConnected()) {
		ds->boardPort(boardnr).send(*itsTPE);
		ds->boardPort(boardnr).setTimer(ds->timeout());
		sending = true;
	}
	else 
		itsTBBackE->status |= CMD_ERROR;
	
	return(sending);
}

// ----------------------------------------------------------------------------
void ErasefCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status |= COMM_ERROR;
	}
	else {
		itsTPackE = new TPErasefackEvent(event);
		
		itsBoardStatus	= itsTPackE->status;
		
		LOG_DEBUG_STR(formatString("Received ErasefAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void ErasefCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if (itsTBBackE->status == 0)
			itsTBBackE->status = SUCCESS;

	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
CmdTypes ErasefCmd::getCmdType()
{
	return(BoardCmd);
}

// ----------------------------------------------------------------------------
uint32 ErasefCmd::getBoardMask()
{
	return(itsBoardMask);
}

// ----------------------------------------------------------------------------
bool ErasefCmd::waitAck()
{
	return(true);
}

	} // end TBB namespace
} // end LOFAR namespace
