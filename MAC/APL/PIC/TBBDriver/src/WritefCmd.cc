//#  WritefCmd.cc: implementation of the WritefCmd class
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

#include "WritefCmd.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a WritefCmd object.----------------------------------------
WritefCmd::WritefCmd():
		itsBoardMask(0),itsErrorMask(0),itsBoardsMask(0),itsBoardStatus(0)
{
	itsTPE 			= new TPWritefEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBWritefackEvent();
}
	  
//--Destructor for WritefCmd.---------------------------------------------------
WritefCmd::~WritefCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool WritefCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_WRITEF)||(event.signal == TP_WRITEFACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void WritefCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE 			= new TBBWritefEvent(event);
		
	itsBoardMask = (1 << itsTBBE->board);
	
	// mask for the installed boards
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();
	
	// Send only commands to boards installed
	itsErrorMask = itsBoardMask & ~itsBoardsMask;
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	itsTBBackE->status = 0;
	
	// initialize TP send frame
	itsTPE->opcode			= TPWRITEF;
	itsTPE->status			=	0;
	itsTPE->addr	 			= itsTBBE->addr;
	
	for(int an = 0; an < 256;an++) {
		itsTPE->data[an] = itsTBBE->data[an];
	}

	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
bool WritefCmd::sendTpEvent(int32 boardnr, int32)
{
	bool sending = false;
	DriverSettings*		ds = DriverSettings::instance();
	
	if (ds->boardPort(boardnr).isConnected()) {
		ds->boardPort(boardnr).send(*itsTPE);
		ds->boardPort(boardnr).setTimer(ds->timeout());
		sending = true;
	}
	else
		itsErrorMask |= (1 << boardnr);
		
	return(sending);
}

// ----------------------------------------------------------------------------
void WritefCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPWritefackEvent(event);
		
		itsBoardStatus	= itsTPackE->status;
		
		LOG_DEBUG_STR(formatString("Received WritefAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void WritefCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	itsTBBackE->status = 0;
	if (itsErrorMask != 0) {
		itsTBBackE->status |= COMM_ERROR;
		itsTBBackE->status |= (itsErrorMask << 16);
	}
	if (itsBoardMask == 0) itsTBBackE->status |= SELECT_ERROR; 
	if (itsTBBackE->status == 0) itsTBBackE->status = SUCCESS;
	
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
CmdTypes WritefCmd::getCmdType()
{
	return(BoardCmd);
}

// ----------------------------------------------------------------------------
uint32 WritefCmd::getBoardMask()
{
	return(itsBoardMask);
}

// ----------------------------------------------------------------------------
bool WritefCmd::waitAck()
{
	return(true);
}

	} // end TBB namespace
} // end LOFAR namespace
