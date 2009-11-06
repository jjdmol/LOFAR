//#  ConfigCmd.cc: implementation of the ConfigCmd class
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

#include "ConfigCmd.h"


using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a ConfigCmd object.----------------------------------------
ConfigCmd::ConfigCmd()
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPConfigEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBConfigAckEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBOARDS;boardnr++) { 
		itsTBBackE->status_mask[boardnr]	= 0;
	}
	setWaitAck(true);		
}
	  
//--Destructor for ConfigCmd.---------------------------------------------------
ConfigCmd::~ConfigCmd()
{
	delete itsTPE;
	delete itsTBBackE;	
}

// ----------------------------------------------------------------------------
bool ConfigCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_CONFIG)||(event.signal == TP_CONFIG_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ConfigCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE = new TBBConfigEvent(event);
	//TBBConfigEvent itsTBBE(event);
		
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsTBBackE->status_mask[boardnr] = 0;
		if (TS->isBoardActive(boardnr) == false) { 
			itsTBBackE->status_mask[boardnr] |= TBB_NO_BOARD;
		}
	}
	
	setBoardMask(itsTBBE->boardmask);
	
	// select first boards
	nextBoardNr();
	
	// initialize TP send frame
	itsTPE->opcode	= TPCONFIG;
	itsTPE->status	=	0;
	itsTPE->imagenr	= static_cast<uint32>(itsTBBE->imagenr);
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void ConfigCmd::sendTpEvent()
{
	TS->boardPort(getBoardNr()).send(*itsTPE);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void ConfigCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask[getBoardNr()] |= TBB_COMM_ERROR;
	}	else {
		itsTPackE = new TPConfigAckEvent(event);
		TS->setImageNr(getBoardNr(), itsTPE->imagenr);
		TS->setFreeToReset(getBoardNr(), false);
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status_mask[getBoardNr()] |= (1 << (16 + (itsTPackE->status & 0x0F)));	
		
		LOG_DEBUG_STR(formatString("Received ConfigAck from boardnr[%d]", getBoardNr()));
		delete itsTPackE;
	}
	nextBoardNr();
	if (isDone()) { 
	   setSleepTime(15.0);
	}
}

// ----------------------------------------------------------------------------
void ConfigCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < TS->maxBoards(); boardnr++) { 
		if (itsTBBackE->status_mask[boardnr] == 0)
			itsTBBackE->status_mask[boardnr] = TBB_SUCCESS;
	}
	 
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
