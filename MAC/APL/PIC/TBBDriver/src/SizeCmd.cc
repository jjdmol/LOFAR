//#  SizeCmd.cc: implementation of the SizeCmd class
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

#include "SizeCmd.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a SizeCmd object.----------------------------------------
SizeCmd::SizeCmd():
		itsSendMask(0),itsRecvMask(0),itsErrorMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPSizeEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBSizeackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsBoardStatus[boardnr]	= 0;
		itsSize[boardnr] 				= 0;
	}		
}
	  
//--Destructor for SizeCmd.---------------------------------------------------
SizeCmd::~SizeCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool SizeCmd::isValid(GCFEvent& event)
{
	if((event.signal == TBB_SIZE)||(event.signal == TP_SIZE)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void SizeCmd::saveTbbEvent(GCFEvent& event, uint32 activeboards)
{
	itsTBBE 			= new TBBSizeEvent(event);
		
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
	itsTPE->opcode	= TPSIZE;
	itsTPE->status	=	0;
	itsTPE->size		= 0;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void SizeCmd::sendTpEvent(GCFPortInterface& port)
{
	port.send(*itsTPE);
}

// ----------------------------------------------------------------------------
void SizeCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	itsRecvMask |= (1 << boardnr);
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPSizeEvent(event);
		
		itsBoardStatus[boardnr]	= itsTPackE->status;
		itsSize[boardnr] 				= itsTPackE->size;
		
		LOG_DEBUG_STR(formatString("Received SizeAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void SizeCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	itsTBBackE->commstatus = SUCCESS;	
	if(itsErrorMask) {
		itsTBBackE->commstatus = FAILURE;
		itsTBBackE->commstatus |= (itsErrorMask << 16);
	} 
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) {
		itsTBBackE->boardstatus[boardnr]	= itsBoardStatus[boardnr];
		itsTBBackE->size[boardnr]					= itsSize[boardnr];
	}
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
void SizeCmd::portError(int32 boardnr)
{
	itsRecvMask	|= (1 << boardnr);
	itsErrorMask |= (1 << boardnr);
}

// ----------------------------------------------------------------------------
uint32 SizeCmd::getSendMask()
{
	return itsSendMask;
}

// ----------------------------------------------------------------------------
uint32 SizeCmd::getRecvMask()
{
	return itsRecvMask;
}

// ----------------------------------------------------------------------------
bool SizeCmd::done()
{
	return (itsRecvMask == itsSendMask);
}

// ----------------------------------------------------------------------------
bool SizeCmd::waitAck()
{
	return true;
}

	} // end TBB namespace
} // end LOFAR namespace
