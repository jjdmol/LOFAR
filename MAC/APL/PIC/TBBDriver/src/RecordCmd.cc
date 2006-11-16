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
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a RecordCmd object.----------------------------------------
RecordCmd::RecordCmd():
		itsBoardMask(0), itsErrorMask(0), itsBoardsMask(0),itsChannel(0)
{
	itsTPE 			= new TPRecordEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBRecordackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsTBBackE->status[boardnr]	= 0;
		itsChannelMask[boardnr] = 0;
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
	if ((event.signal == TBB_RECORD)||(event.signal == TP_RECORDACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void RecordCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE 			= new TBBRecordEvent(event);
		
	// mask for the installed boards
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();
		
	for (int boardnr = 0; boardnr < DriverSettings::instance()->maxBoards(); boardnr++) {
		
		if (!(itsBoardsMask & (1 << boardnr))) 
			itsTBBackE->status[boardnr] |= NO_BOARD;
		
		itsChannelMask[boardnr] = itsTBBE->channelmask[boardnr]; 		
		
		if (itsChannelMask[boardnr] != 0)
			itsBoardMask |= (1 << boardnr);
			
		if ((itsChannelMask[boardnr] & ~0xFFFF) != 0) 
			itsTBBackE->status[boardnr] |= (SELECT_ERROR & CHANNEL_SEL_ERROR);
				
		if (!(itsBoardsMask & (1 << boardnr)) &&  (itsChannelMask[boardnr] != 0))
			itsTBBackE->status[boardnr] |= (SELECT_ERROR & BOARD_SEL_ERROR);
	}
	
	// Send only commands to boards installed
	itsErrorMask = itsBoardMask & ~itsBoardsMask;
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	// initialize TP send frame
	itsTPE->opcode	= TPRECORD;
	itsTPE->status	=	0;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
bool RecordCmd::sendTpEvent(int32 boardnr, int32 channelnr)
{
	bool sending = false;
	DriverSettings*		ds = DriverSettings::instance();
	
	itsTPE->channel = DriverSettings::instance()->getChInputNr(channelnr);
	itsChannel = channelnr;
	
	if (ds->boardPort(boardnr).isConnected() && (itsTBBackE->status[boardnr] == 0)) {
		ds->boardPort(boardnr).send(*itsTPE);
		ds->boardPort(boardnr).setTimer(ds->timeout());
		sending = true;
	}
	else
		itsErrorMask |= (1 << boardnr);
	
	return(sending);
}

// ----------------------------------------------------------------------------
void RecordCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPRecordackEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status[boardnr] |= (1 << (16 + (itsTPackE->status & 0x0F)));
		
		if (itsTPackE->status == 0)
			DriverSettings::instance()->setChActive((itsChannel + (boardnr * 16)), true);	
		
		LOG_DEBUG_STR(formatString("Received RecordAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void RecordCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < DriverSettings::instance()->maxBoards(); boardnr++) { 
		if (itsTBBackE->status[boardnr] == 0)
			itsTBBackE->status[boardnr] = SUCCESS;
	}
	
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
uint32 RecordCmd::getBoardMask()
{
	return(itsBoardMask);
}

// ----------------------------------------------------------------------------
bool RecordCmd::waitAck()
{
	return(true);
}

// ----------------------------------------------------------------------------
CmdTypes RecordCmd::getCmdType()
{
	return(ChannelCmd);
}

	} // end TBB namespace
} // end LOFAR namespace
