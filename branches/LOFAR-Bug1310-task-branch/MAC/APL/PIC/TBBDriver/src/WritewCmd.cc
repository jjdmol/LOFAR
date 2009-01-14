//#  WritewCmd.cc: implementation of the WritewCmd class
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

#include "WritewCmd.h"

using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a WritewCmd object.----------------------------------------
WritewCmd::WritewCmd():
		itsBoardStatus(0)		
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPWritewEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBWritewAckEvent();
	
	itsTBBackE->status_mask = 0;
	setWaitAck(true);
}
	  
//--Destructor for WritewCmd.---------------------------------------------------
WritewCmd::~WritewCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool WritewCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_WRITEW)||(event.signal == TP_WRITEW_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void WritewCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE	= new TBBWritewEvent(event);
	
	itsTBBackE->status_mask = 0;
	if (TS->isBoardActive(itsTBBE->board)) {	
		setBoardNr(itsTBBE->board);
	} else {
		itsTBBackE->status_mask |= TBB_NO_BOARD ;
		setDone(true);
	}
		
	// initialize TP send frame
	itsTPE->opcode	= TPWRITEW;
	itsTPE->status	= 0;
	itsTPE->mp			=	static_cast<uint32>(itsTBBE->mp);
	itsTPE->addr		=	itsTBBE->addr;
	itsTPE->wordlo	= itsTBBE->wordlo;
	itsTPE->wordhi	= itsTBBE->wordhi;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void WritewCmd::sendTpEvent()
{
	TS->boardPort(getBoardNr()).send(*itsTPE);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void WritewCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask |= TBB_COMM_ERROR;
	}
	else {
		itsTPackE = new TPWritewAckEvent(event);
		
		itsBoardStatus	= itsTPackE->status;
				
		LOG_DEBUG_STR(formatString("Received WritewAck from boardnr[%d]", getBoardNr()));
		delete itsTPackE;
	}
	setDone(true);
}

// ----------------------------------------------------------------------------
void WritewCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if (itsTBBackE->status_mask == 0)
			itsTBBackE->status_mask = TBB_SUCCESS;

	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
