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

#include "WritewCmd.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a WritewCmd object.----------------------------------------
WritewCmd::WritewCmd():
		itsBoardMask(0),itsErrorMask(0),itsBoardsMask(0),itsBoardStatus(0),
		itsMp(0),itsAddr(0),itsWordLo(0),itsWordHi(0)
{
	itsTPE 			= new TPWritewEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBWritewackEvent();
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
	if((event.signal == TBB_WRITEW)||(event.signal == TP_WRITEWACK)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void WritewCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE 			= new TBBWritewEvent(event);
		
	itsBoardMask = (1 << itsTBBE->board);
	
	// mask for the installed boards
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();
	
	// Send only commands to boards installed
	itsErrorMask = itsBoardMask & ~itsBoardsMask;
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	itsTBBackE->status = 0;
	
	// initialize TP send frame
	itsTPE->opcode	= TPWRITEW;
	itsTPE->status	= 0;
	itsTPE->mp			=	itsTBBE->mp;
	itsTPE->addr		=	itsTBBE->addr;
	itsTPE->wordlo	= itsTBBE->wordlo;
	itsTPE->wordhi	= itsTBBE->wordhi;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void WritewCmd::sendTpEvent(int32 boardnr, int32)
{
	DriverSettings*		ds = DriverSettings::instance();
	
	if(ds->boardPort(boardnr).isConnected()) {
		ds->boardPort(boardnr).send(*itsTPE);
		ds->boardPort(boardnr).setTimer(ds->timeout());
	}
	else
		itsErrorMask |= (1 << boardnr);
}

// ----------------------------------------------------------------------------
void WritewCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPWritewackEvent(event);
		
		itsBoardStatus	= itsTPackE->status;
				
		LOG_DEBUG_STR(formatString("Received WritewAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void WritewCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if(itsErrorMask != 0) {
		itsTBBackE->status |= COMM_ERROR;
		itsTBBackE->status |= (itsErrorMask << 16);
	}
	if(itsTBBackE->status == 0) itsTBBackE->status = SUCCESS;

	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
CmdTypes WritewCmd::getCmdType()
{
	return BoardCmd;
}

// ----------------------------------------------------------------------------
uint32 WritewCmd::getBoardMask()
{
	return itsBoardMask;
}

// ----------------------------------------------------------------------------
bool WritewCmd::waitAck()
{
	return true;
}

	} // end TBB namespace
} // end LOFAR namespace
