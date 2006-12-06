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

#include "StatusCmd.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a StatusCmd object.----------------------------------------
StatusCmd::StatusCmd():
		itsBoardMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPStatusEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBStatusackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsTBBackE->status[boardnr]	= 0;
	}		
}
	  
//--Destructor for StatusCmd.---------------------------------------------------
StatusCmd::~StatusCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool StatusCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_STATUS)||(event.signal == TP_STATUSACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void StatusCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE 			= new TBBStatusEvent(event);
	
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();	
	itsBoardMask = itsTBBE->boardmask; // for some commands board-id is used ???
	
	for (int boardnr = 0; boardnr < DriverSettings::instance()->maxBoards(); boardnr++) {
		
		if (!(itsBoardsMask & (1 << boardnr))) 
			itsTBBackE->status[boardnr] |= NO_BOARD;
		
		if (!(itsBoardsMask & (1 << boardnr)) &&  (itsBoardMask & (1 << boardnr)))
			itsTBBackE->status[boardnr] |= (SELECT_ERROR | BOARD_SEL_ERROR);
	}	
		
	// Send only commands to boards installed
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	// initialize TP send frame
	itsTPE->opcode	= TPSTATUS;
	itsTPE->status	=	0;
			
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
bool StatusCmd::sendTpEvent(int32 boardnr, int32)
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
void StatusCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status[boardnr] |= COMM_ERROR;
	}
	else {
		itsTPackE = new TPStatusackEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status[boardnr] |= (1 << (16 + (itsTPackE->status & 0x0F)));	
		
		itsTBBackE->V12[boardnr]	= itsTPackE->V12;
		itsTBBackE->V25[boardnr]	= itsTPackE->V25;
		itsTBBackE->V33[boardnr]	= itsTPackE->V33;
		itsTBBackE->Tpcb[boardnr]	= itsTPackE->Tpcb;
		itsTBBackE->Ttp[boardnr]	= itsTPackE->Ttp;
		itsTBBackE->Tmp0[boardnr]	= itsTPackE->Tmp0;
		itsTBBackE->Tmp1[boardnr]	= itsTPackE->Tmp1;
		itsTBBackE->Tmp2[boardnr]	= itsTPackE->Tmp2;
		itsTBBackE->Tmp3[boardnr]	= itsTPackE->Tmp3;
		
		LOG_DEBUG_STR(formatString("Received StatusAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void StatusCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < DriverSettings::instance()->maxBoards(); boardnr++) { 
		if (itsTBBackE->status[boardnr] == 0)
			itsTBBackE->status[boardnr] = SUCCESS;
	}
	
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
CmdTypes StatusCmd::getCmdType()
{
	return(BoardCmd);
}

// ----------------------------------------------------------------------------
uint32 StatusCmd::getBoardMask()
{
	return(itsBoardMask);
}

// ----------------------------------------------------------------------------
bool StatusCmd::waitAck()
{
	return(true);
}

	} // end TBB namespace
} // end LOFAR namespace
