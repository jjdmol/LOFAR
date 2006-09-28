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

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a FreeCmd object.----------------------------------------
FreeCmd::FreeCmd():
		itsSendMask(0),itsRecvMask(0),itsErrorMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPFreeEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBFreeackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsBoardStatus[boardnr]	= 0;
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
	if((event.signal == TBB_FREE)||(event.signal == TP_FREE)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void FreeCmd::saveTbbEvent(GCFEvent& event, uint32 activeboards)
{
	itsTBBE 			= new TBBFreeEvent(event);
		
	itsSendMask = itsTBBE->tbbmask; // for some commands board-id is used ???
	// if SendMask = 0, select all boards
	if(itsSendMask == 0) {
		for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) {
			itsSendMask |= (1 << boardnr);
		}
	} 
	
	// mask for the installed boards
		itsBoardsMask = activeboards;
	
	// Send only commands to boards installed
	itsErrorMask = itsSendMask & ~itsBoardsMask;
	itsSendMask = itsSendMask & itsBoardsMask;
	
	// initialize TP send frame
	itsTPE->opcode			= TPFREE;
	itsTPE->status			= 0;
	itsTPE->channel			= itsTBBE->channel;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void FreeCmd::sendTpEvent(GCFPortInterface& port)
{
	port.send(*itsTPE);
}

// ----------------------------------------------------------------------------
void FreeCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	itsRecvMask |= (1 << boardnr);
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPFreeEvent(event);
		
		itsBoardStatus[boardnr]			= itsTPackE->status;
		LOG_DEBUG_STR(formatString("Received FreeAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void FreeCmd::sendTbbAckEvent(GCFPortInterface* clientport)
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
void FreeCmd::portError(int32 boardnr)
{
	itsRecvMask |= (1 << boardnr);
	itsErrorMask |= (1 << boardnr);
}

// ----------------------------------------------------------------------------
uint32 FreeCmd::getSendMask()
{
	return itsSendMask;
}

// ----------------------------------------------------------------------------
uint32 FreeCmd::getRecvMask()
{
	return itsRecvMask;
}

// ----------------------------------------------------------------------------
bool FreeCmd::done()
{
	return (itsRecvMask == itsSendMask);
}

// ----------------------------------------------------------------------------
bool FreeCmd::waitAck()
{
	return true;
}

	} // end TBB namespace
} // end LOFAR namespace
