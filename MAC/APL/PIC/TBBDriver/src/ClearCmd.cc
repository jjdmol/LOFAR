//#  ClearCmd.cc: implementation of the ClearCmd class
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

#include "ClearCmd.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a ClearCmd object.----------------------------------------
ClearCmd::ClearCmd():
		itsSendMask(0),itsRecvMask(0),itsErrorMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPClearEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBClearackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsBoardStatus[boardnr]	= 0;
	}		
}
	  
//--Destructor for ClearCmd.---------------------------------------------------
ClearCmd::~ClearCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool ClearCmd::isValid(GCFEvent& event)
{
	if((event.signal == TBB_CLEAR)||(event.signal == TP_CLEAR)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void ClearCmd::saveTbbEvent(GCFEvent& event, int32 boards)
{
	itsTBBE 			= new TBBClearEvent(event);
		
	itsSendMask = itsTBBE->tbbmask; // for some commands board-id is used ???
	// if SendMask = 0, select all boards
	if(itsSendMask == 0) {
		for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) {
			itsSendMask |= (1 << boardnr);
		}
	} 
	
	// mask for the installed boards
	if(boards > MAX_N_TBBBOARDS) boards = MAX_N_TBBBOARDS;
	for(int boardnr = 0;boardnr < boards;boardnr++) {
		itsBoardsMask |= (1 << boardnr);
	}
	
	// Send only commands to boards installed
	itsErrorMask = itsSendMask & ~itsBoardsMask;
	itsSendMask = itsSendMask & itsBoardsMask;
	
	// initialize TP send frame
	itsTPE->opcode			= TPCLEAR;
	itsTPE->status			=	0;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void ClearCmd::sendTpEvent(GCFPortInterface& port)
{
	port.send(*itsTPE);
}

// ----------------------------------------------------------------------------
void ClearCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	itsRecvMask |= (1 << boardnr);
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPClearEvent(event);
		
		itsBoardStatus[boardnr]	= itsTPackE->status;
		
		LOG_DEBUG_STR(formatString("Received ClearAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void ClearCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	itsTBBackE->commstatus = SUCCESS;	
	if(itsErrorMask) {
		itsTBBackE->commstatus = FAILURE;
		itsTBBackE->commstatus |= (itsErrorMask << 16);
	} 
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) {
		itsTBBackE->boardstatus[boardnr]	= itsBoardStatus[boardnr];
	}
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
void ClearCmd::portError(int32 boardnr)
{
	itsRecvMask	|= (1 << boardnr);
	itsErrorMask |= (1 << boardnr);
}

// ----------------------------------------------------------------------------
uint32 ClearCmd::getSendMask()
{
	return itsSendMask;
}

// ----------------------------------------------------------------------------
uint32 ClearCmd::getRecvMask()
{
	return itsRecvMask;
}

// ----------------------------------------------------------------------------
bool ClearCmd::done()
{
	return (itsRecvMask == itsSendMask);
}

// ----------------------------------------------------------------------------
bool ClearCmd::waitAck()
{
	return false;
}

	} // end TBB namespace
} // end LOFAR namespace
