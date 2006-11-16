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

#include "ConfigCmd.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a ConfigCmd object.----------------------------------------
ConfigCmd::ConfigCmd():
		itsBoardMask(0),itsErrorMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPConfigEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBConfigackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsTBBackE->status[boardnr]	= 0;
	}		
}
	  
//--Destructor for ConfigCmd.---------------------------------------------------
ConfigCmd::~ConfigCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool ConfigCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_CONFIG)||(event.signal == TP_CONFIGACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ConfigCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE 			= new TBBConfigEvent(event);
		
	// mask for the installed boards
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();
	itsBoardMask = itsTBBE->boardmask; // for some commands board-id is used ???
	
	for (int boardnr = 0; boardnr < DriverSettings::instance()->maxBoards(); boardnr++) {
		
		if (!(itsBoardsMask & (1 << boardnr))) 
			itsTBBackE->status[boardnr] |= NO_BOARD;
		
		if (!(itsBoardsMask & (1 << boardnr)) &&  (itsBoardMask & (1 << boardnr)))
			itsTBBackE->status[boardnr] |= (SELECT_ERROR & BOARD_SEL_ERROR);
	}
	
	// Send only commands to boards installed
	itsErrorMask = itsBoardMask & ~itsBoardsMask;
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	// initialize TP send frame
	itsTPE->opcode	= TPCONFIG;
	itsTPE->status	=	0;
	itsTPE->imagenr	 	= itsTBBE->imagenr;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
bool ConfigCmd::sendTpEvent(int32 boardnr, int32)
{
	bool sending = false;
	DriverSettings*		ds = DriverSettings::instance();
	
	if (ds->boardPort(boardnr).isConnected() && (itsTBBackE->status[boardnr] == 0)) {
		ds->boardPort(boardnr).send(*itsTPE);
		ds->boardPort(boardnr).setTimer(ds->timeout());
		sending = true;
	}
	else
		itsErrorMask |= (1 << boardnr);
		
	return(sending);
}

// ----------------------------------------------------------------------------
void ConfigCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPConfigackEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status[boardnr] |= (1 << (16 + (itsTPackE->status & 0x0F)));	
		
		LOG_DEBUG_STR(formatString("Received ConfigAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void ConfigCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < DriverSettings::instance()->maxBoards(); boardnr++) { 
		if (itsTBBackE->status[boardnr] == 0)
			itsTBBackE->status[boardnr] = SUCCESS;
	}
	 
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
CmdTypes ConfigCmd::getCmdType()
{
	return(BoardCmd);
}

// ----------------------------------------------------------------------------
uint32 ConfigCmd::getBoardMask()
{
	return(itsBoardMask);
}

// ----------------------------------------------------------------------------
bool ConfigCmd::waitAck()
{
	return(false);
}

	} // end TBB namespace
} // end LOFAR namespace
