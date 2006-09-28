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

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a WritefCmd object.----------------------------------------
WritefCmd::WritefCmd():
		itsSendMask(0),itsRecvMask(0),itsErrorMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPWritefEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBWritefackEvent();
	
	itsBoardStatus	= 0;
	itsAddr					= 0;
		
	for(int an = 0; an < 256;an++) {
		itsData[an] = 0;
	}
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
	if((event.signal == TBB_WRITEF)||(event.signal == TP_WRITEF)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void WritefCmd::saveTbbEvent(GCFEvent& event, uint32 activeboards)
{
	itsTBBE 			= new TBBWritefEvent(event);
		
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
	itsTPE->opcode			= TPWRITEF;
	itsTPE->status			=	0;
	itsTPE->addr	 			= itsTBBE->addr;
	
	for(int an = 0; an < 256;an++) {
		itsTPE->data[an] = itsTBBE->data[an];
	}

	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void WritefCmd::sendTpEvent(GCFPortInterface& port)
{
	port.send(*itsTPE);
}

// ----------------------------------------------------------------------------
void WritefCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	itsRecvMask |= (1 << boardnr);
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPWritefEvent(event);
		
		itsBoardStatus	= itsTPackE->status;
		itsAddr					= itsTPackE->addr;
		
		LOG_DEBUG_STR(formatString("Received WritefAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void WritefCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	itsTBBackE->commstatus = SUCCESS;	
	if(itsErrorMask) {
		itsTBBackE->commstatus = FAILURE;
		itsTBBackE->commstatus |= (itsErrorMask << 16);
	} 
	
	itsTBBackE->boardstatus	= itsBoardStatus;
	itsTBBackE->addr				=	itsAddr;
	
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
void WritefCmd::portError(int32 boardnr)
{
	itsRecvMask	|= (1 << boardnr);
	itsErrorMask |= (1 << boardnr);
}

// ----------------------------------------------------------------------------
uint32 WritefCmd::getSendMask()
{
	return itsSendMask;
}

// ----------------------------------------------------------------------------
uint32 WritefCmd::getRecvMask()
{
	return itsRecvMask;
}

// ----------------------------------------------------------------------------
bool WritefCmd::done()
{
	return (itsRecvMask == itsSendMask);
}

// ----------------------------------------------------------------------------
bool WritefCmd::waitAck()
{
	return true;
}

	} // end TBB namespace
} // end LOFAR namespace
