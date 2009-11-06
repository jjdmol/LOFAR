//#  TrigclrCmd.cc: implementation of the TrigclrCmd class
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

#include "TrigclrCmd.h"


using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a TrigclrCmd object.----------------------------------------
TrigclrCmd::TrigclrCmd()
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPTrigclrEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBTrigclrackEvent();
	
	itsTBBackE->status_mask = 0;
	setWaitAck(true);
}
	  
//--Destructor for TrigclrCmd.---------------------------------------------------
TrigclrCmd::~TrigclrCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool TrigclrCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_TRIGCLR)||(event.signal == TP_TRIGCLRACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void TrigclrCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE	= new TBBTrigclrEvent(event);
		
	int32 boardnr;
	int32 channelnr;
	TS->convertRcu2Ch(itsTBBE->channel,&boardnr,&channelnr);
		
	setBoardNr(boardnr);
		
	// initialize TP send frame
	itsTPE->opcode	= TPTRIGCLR;
	itsTPE->status	=	0;
	itsTPE->channel = channelnr;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void TrigclrCmd::sendTpEvent()
{
		TS->boardPort(getBoardNr()).send(*itsTPE);
		TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void TrigclrCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask |= TBB_COMM_ERROR;
	}
	else {
		itsTPackE = new TPTrigclrackEvent(event);
		
		LOG_DEBUG_STR(formatString("Received TrigclrAck from boardnr[%d]", getBoardNr()));
		delete itsTPackE;
	}
	setDone(true);
}

// ----------------------------------------------------------------------------
void TrigclrCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if (itsTBBackE->status_mask == 0)
			itsTBBackE->status_mask = TBB_SUCCESS;
		
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
