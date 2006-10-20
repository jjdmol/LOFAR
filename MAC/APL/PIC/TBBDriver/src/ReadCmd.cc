//#  ReadCmd.cc: implementation of the ReadCmd class
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

#include "ReadCmd.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a ReadCmd object.----------------------------------------
ReadCmd::ReadCmd():
		itsBoardMask(0),itsErrorMask(0),itsBoardsMask(0), itsChannel(0),itsBoardStatus(0)
{
	itsTPE 			= new TPReadEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBReadackEvent();
}
	  
//--Destructor for ReadCmd.---------------------------------------------------
ReadCmd::~ReadCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool ReadCmd::isValid(GCFEvent& event)
{
	if((event.signal == TBB_READ)||(event.signal == TP_READACK)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void ReadCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE 			= new TBBReadEvent(event);
	
	itsBoardMask = (1 << DriverSettings::instance()->getChBoardNr((int32)itsTBBE->channel));
				
	// mask for the installed boards
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();
	
	// Send only commands to boards installed
	itsErrorMask = itsBoardMask & ~itsBoardsMask;
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	itsTBBackE->status = 0;
	
	// initialize TP send frame
	itsTPE->opcode			= TPREAD;
	itsTPE->status			=	0;
	itsTPE->channel			= itsTBBE->channel;
	itsTPE->secondstime	= itsTBBE->secondstime;
	itsTPE->sampletime	= itsTBBE->sampletime;
	itsTPE->prepages		= itsTBBE->prepages;
	itsTPE->postpages		= itsTBBE->postpages;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void ReadCmd::sendTpEvent(int32 boardnr, int32)
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
void ReadCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPReadackEvent(event);
		
		itsBoardStatus = itsTPackE->status;
		
		LOG_DEBUG_STR(formatString("Received ReadAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void ReadCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if(itsErrorMask != 0) {
		itsTBBackE->status |= COMM_ERROR;
		itsTBBackE->status |= (itsErrorMask << 16);
	}
	if(itsTBBackE->status == 0) itsTBBackE->status = SUCCESS;
	 
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
uint32 ReadCmd::getBoardMask()
{
	return itsBoardMask;
}

// ----------------------------------------------------------------------------
CmdTypes ReadCmd::getCmdType()
{
	return ChannelCmd;
}

// ----------------------------------------------------------------------------
bool ReadCmd::waitAck()
{
	return true;
}

	} // end TBB namespace
} // end LOFAR namespace
