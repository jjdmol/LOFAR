//#  ReadfCmd.cc: implementation of the ReadfCmd class
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

#include "ReadfCmd.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a ReadfCmd object.----------------------------------------
ReadfCmd::ReadfCmd():
		itsBoardMask(0),itsBoardsMask(0),itsBoardStatus(0)
{
	itsTPE 			= new TPReadfEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBReadfackEvent();
}
	  
//--Destructor for ReadfCmd.---------------------------------------------------
ReadfCmd::~ReadfCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool ReadfCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_READF)||(event.signal == TP_READFACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ReadfCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE 			= new TBBReadfEvent(event);
		
	itsBoardMask = (1 << itsTBBE->board);
	
	// mask for the installed boards
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();

	// Send only commands to boards installed
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	itsTBBackE->status = 0;
	
	// initialize TP send frame
	itsTPE->opcode	= TPREADF;
	itsTPE->status	=	0;
	itsTPE->addr		= itsTBBE->addr;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
bool ReadfCmd::sendTpEvent(int32 boardnr, int32)
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
void ReadfCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status |= COMM_ERROR;
	}
	else {
		itsTPackE = new TPReadfackEvent(event);
		
		itsBoardStatus	= itsTPackE->status;
		for(int an=0; an < 256; an++) {
			itsTBBackE->data[an]	= itsTPackE->data[an];
		}
		
		LOG_DEBUG_STR(formatString("Received ReadfAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void ReadfCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if (itsTBBackE->status == 0)
			itsTBBackE->status = SUCCESS;
	
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
CmdTypes ReadfCmd::getCmdType()
{
	return(BoardCmd);
}

// ----------------------------------------------------------------------------
uint32 ReadfCmd::getBoardMask()
{
	return(itsBoardMask);
}

// ----------------------------------------------------------------------------
bool ReadfCmd::waitAck()
{
	return(true);
}

	} // end TBB namespace
} // end LOFAR namespace
