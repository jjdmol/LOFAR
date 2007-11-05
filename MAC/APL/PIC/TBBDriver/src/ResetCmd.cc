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
#include <unistd.h>

#include "ResetCmd.h"

using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a ResetCmd object.----------------------------------------
ResetCmd::ResetCmd()
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPResetEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBResetAckEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
	}
	setWaitAck(false);		
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
	if ((event.signal == TBB_RESET)||(event.signal == TP_RESET_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ResetCmd::saveTbbEvent(GCFEvent& event)
{
	uint32	boardmask = 0;
	
	itsTBBE	= new TBBResetEvent(event);
	itsTPE->opcode			= TPRESET;
	itsTPE->status			=	0;
		
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		if (itsTBBE->boardmask & (1 << boardnr)) {
			boardmask |= (1 << boardnr);
			if (TS->boardPort(boardnr).isConnected()) {
				TS->boardPort(boardnr).send(*itsTPE);
			}
		} 
	}
	TS->setActiveBoardsMask(boardmask);
	setDone(true);
	setSleepTime(3);  // full reset will take 3 seconds
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void ResetCmd::sendTpEvent()
{
	// empty
}

// ----------------------------------------------------------------------------
void ResetCmd::saveTpAckEvent(GCFEvent& event)
{
	//itsTPackE = new TPResetAckEvent(event);
	//delete itsTPackE;
	setDone(true);
}

// ----------------------------------------------------------------------------
void ResetCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < TS->maxBoards(); boardnr++) { 
		if (itsTBBackE->status_mask[boardnr] == 0)
			itsTBBackE->status_mask[boardnr] = TBB_SUCCESS;
	}
	clientport->send(*itsTBBackE);
}
