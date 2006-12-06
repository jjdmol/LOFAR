//#  ResetCmd.cc: implementation of the ResetCmd class
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

#include "ResetCmd.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a ResetCmd object.----------------------------------------
ResetCmd::ResetCmd():
		itsBoardMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPResetEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBResetackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsTBBackE->status[boardnr]	= 0;
	}		
}
	  
//--Destructor for ResetCmd.---------------------------------------------------
ResetCmd::~ResetCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool ResetCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_RESET)||(event.signal == TP_RESETACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ResetCmd::saveTbbEvent(GCFEvent& event)
{
	DriverSettings*		ds = DriverSettings::instance();
	itsTBBE 			= new TBBResetEvent(event);
	itsTPE->opcode			= TPRESET;
	itsTPE->status			=	0;
		
	itsBoardMask = itsTBBE->boardmask;
	
	for (int boardnr = 0; boardnr < DriverSettings::instance()->maxBoards(); boardnr++) {
		if (itsBoardMask & (1 << boardnr)) {
			if (ds->boardPort(boardnr).isConnected())
				ds->boardPort(boardnr).send(*itsTPE);
			
			// reset channel information for selected board	
			for (int channelnr = (boardnr * 16); channelnr < ((boardnr * 16) + 16); channelnr++) {
				DriverSettings::instance()->setChSelected(channelnr, false);
				DriverSettings::instance()->setChStatus(channelnr, 'F');
				DriverSettings::instance()->setChStartAddr(channelnr, 0);
				DriverSettings::instance()->setChPageSize(channelnr, 0);				
			}
		} 
	}
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
bool ResetCmd::sendTpEvent(int32 boardnr, int32)
{
	bool sending = false;
	// sending reset is done in saveTbbEvent()
	// because sendTpEvent() is only posible for active boards
	return(sending);
}

// ----------------------------------------------------------------------------
void ResetCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		;
	}
	else {
		itsTPackE = new TPResetackEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status[boardnr] |= (1 << (16 + (itsTPackE->status & 0x0F)));	
		
		LOG_DEBUG_STR(formatString("Received ResetAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void ResetCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < DriverSettings::instance()->maxBoards(); boardnr++) { 
		if (itsTBBackE->status[boardnr] == 0)
			itsTBBackE->status[boardnr] = SUCCESS;
	}
	
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
CmdTypes ResetCmd::getCmdType()
{
	return(BoardCmd);
}

// ----------------------------------------------------------------------------
uint32 ResetCmd::getBoardMask()
{
	return(itsBoardMask);
}

// ----------------------------------------------------------------------------
bool ResetCmd::waitAck()
{
	return(false);
}

	} // end TBB namespace
} // end LOFAR namespace
