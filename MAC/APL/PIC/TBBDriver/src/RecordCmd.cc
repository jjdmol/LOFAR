//#  RecordCmd.cc: implementation of the RecordCmd class
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

#include "RecordCmd.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a RecordCmd object.----------------------------------------
RecordCmd::RecordCmd():
		itsSendMask(0),itsRecvMask(0),itsErrorMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPRecordEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBRecordackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsBoardStatus[boardnr]	= 0;
	}		
}
	  
//--Destructor for RecordCmd.---------------------------------------------------
RecordCmd::~RecordCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool RecordCmd::isValid(GCFEvent& event)
{
	if((event.signal == TBB_RECORD)||(event.signal == TP_RECORD)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void RecordCmd::saveTbbEvent(GCFEvent& event, int32 boards)
{
	itsTBBE 			= new TBBRecordEvent(event);
		
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
	itsTPE->opcode	= TPRECORD;
	itsTPE->status	=	0;
	itsTPE->channel	= itsTBBE->channel;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void RecordCmd::sendTpEvent(GCFPortInterface& port)
{
	port.send(*itsTPE);
}

// ----------------------------------------------------------------------------
void RecordCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	itsRecvMask |= (1 << boardnr);
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPRecordEvent(event);
		
		itsBoardStatus[boardnr]	= itsTPackE->status;
		
		LOG_DEBUG_STR(formatString("Received RecordAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void RecordCmd::sendTbbAckEvent(GCFPortInterface* clientport)
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
void RecordCmd::portError(int32 boardnr)
{
	itsRecvMask	|= (1 << boardnr);
	itsErrorMask |= (1 << boardnr);
}

// ----------------------------------------------------------------------------
uint32 RecordCmd::getSendMask()
{
	return itsSendMask;
}

// ----------------------------------------------------------------------------
uint32 RecordCmd::getRecvMask()
{
	return itsRecvMask;
}

// ----------------------------------------------------------------------------
bool RecordCmd::done()
{
	return (itsRecvMask == itsSendMask);
}

// ----------------------------------------------------------------------------
bool RecordCmd::waitAck()
{
	return true;
}

	} // end TBB namespace
} // end LOFAR namespace
