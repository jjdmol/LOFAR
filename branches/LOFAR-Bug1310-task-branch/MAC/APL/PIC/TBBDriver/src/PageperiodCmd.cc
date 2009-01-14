//#  PageperiodCmd.cc: implementation of the PageperiodCmd class
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
#include <Common/StringUtil.h>

#include "PageperiodCmd.h"

using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using namespace TBB;

//--Constructors for a PageperiodCmd object.----------------------------------------
PageperiodCmd::PageperiodCmd():
		itsBoardStatus(0)
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPPageperiodEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBPageperiodAckEvent();
	setWaitAck(true);
}
	  
//--Destructor for PageperiodCmd.---------------------------------------------------
PageperiodCmd::~PageperiodCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool PageperiodCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_PAGEPERIOD)||(event.signal == TP_PAGEPERIOD_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void PageperiodCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE = new TBBPageperiodEvent(event);
	
	int32 boardnr;
	int32 channelnr;
	TS->convertRcu2Ch(itsTBBE->rcu,&boardnr,&channelnr);
		
	setBoardNr(boardnr);
			
	itsTBBackE->status_mask = 0;
	
	// initialize TP send frame
	itsTPE->opcode			= TPPAGEPERIOD;
	itsTPE->status			=	0;
	itsTPE->channel 		= channelnr; 
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void PageperiodCmd::sendTpEvent()
{
	TS->boardPort(getBoardNr()).send(*itsTPE);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void PageperiodCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask |= TBB_COMM_ERROR;
	}
	else {
		itsTPackE = new TPPageperiodAckEvent(event);
		
		itsBoardStatus 					= itsTPackE->status;
		itsTBBackE->pageperiod	= itsTPackE->pageperiod;
		
		LOG_DEBUG_STR(formatString("Received PageperiodAck from boardnr[%d]", getBoardNr()));
		delete itsTPackE;
	}
	setDone(true);
}

// ----------------------------------------------------------------------------
void PageperiodCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if (itsTBBackE->status_mask == 0)
			itsTBBackE->status_mask = TBB_SUCCESS;
	 
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
