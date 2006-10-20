//#  AllocCmd.cc: implementation of the AllocCmd class
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

#include "AllocCmd.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a AllocCmd object.----------------------------------------
AllocCmd::AllocCmd():
		itsBoardMask(0),itsErrorMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPAllocEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE	= new TBBAllocackEvent();
	
	for(int boardnr = 0;boardnr < DriverSettings::instance()->maxBoards();boardnr++) { 
		itsBoardStatus[boardnr]	= 0;
		itsChannelMask[boardnr]	= 0;
	}
}
	  
//--Destructor for AllocCmd.---------------------------------------------------
AllocCmd::~AllocCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool AllocCmd::isValid(GCFEvent& event)
{
	if((event.signal == TBB_ALLOC)||(event.signal == TP_ALLOCACK)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void AllocCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE				= new TBBAllocEvent(event);
		
	for(int boardnr = 0;boardnr < DriverSettings::instance()->maxBoards();boardnr++) {
		itsChannelMask[boardnr] = itsTBBE->channelmask[boardnr]; // for some commands board-id is used ???
		if(itsChannelMask[boardnr] != 0)  itsBoardMask |= (1 << boardnr);
	}
	
	// mask for the installed boards
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();
	
	// Send only commands to boards installed
	itsErrorMask = itsBoardMask & ~itsBoardsMask;
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	itsTBBackE->status = 0;
	
	// initialize TP send frame
	itsTPE->opcode			= TPALLOC;
	itsTPE->status			=	0;
		
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
//void AllocCmd::sendTpEvent(GCFPortInterface& port, int32 channelnr)
void AllocCmd::sendTpEvent(int32 boardnr, int32 channelnr)
{
	DriverSettings*		ds = DriverSettings::instance();
	itsTPE->channel = 0;
	itsTPE->pageaddr = 0;
	itsTPE->pagelength =	0;	
	if(ds->boardPort(boardnr).isConnected()) {
		ds->boardPort(boardnr).send(*itsTPE);
		ds->boardPort(boardnr).setTimer(ds->timeout());
	}
	else
		itsErrorMask |= (1 << boardnr);
			
}

// ----------------------------------------------------------------------------
void AllocCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPAllocackEvent(event);
		
		itsBoardStatus[boardnr]	= itsTPackE->status;
		LOG_DEBUG_STR(formatString("Received AllocAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void AllocCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if(itsErrorMask != 0) {
		itsTBBackE->status |= COMM_ERROR;
		itsTBBackE->status |= (itsErrorMask << 16);
	}
	if(itsTBBackE->status == 0) itsTBBackE->status = SUCCESS;
		 
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
uint32 AllocCmd::getBoardMask()
{
	return itsBoardMask;
}

// ----------------------------------------------------------------------------
uint32 AllocCmd::getChannelMask(int32 boardnr)
{
	return itsChannelMask[boardnr];
}

// ----------------------------------------------------------------------------
bool AllocCmd::waitAck()
{
	return true;
}

// ----------------------------------------------------------------------------
CmdTypes AllocCmd::getCmdType()
{
	return ChannelCmd;
}

// ----------------------------------------------------------------------------
int32 getNrOfBits(uint32 mask)
{
	int bits = 0;
	for (int nr = 0; nr < 32; nr++) {
		if (mask & (1 << nr)) bits++;
	}
	return bits;
}

// ----------------------------------------------------------------------------
void AllocCmd::devideChannels()
{
	for (int boardnr = 0; boardnr < 12; boardnr++) {
		int32 channels;
		int32 memorysize;
		int32 channelsize;
		
		memorysize = DriverSettings::instance()->getMemorySize(boardnr);
		channels = getNrOfBits(itsChannelMask[boardnr]);
		
		switch (channels) {
			case 1: {
				channelsize = (memorysize / channels);			
				
			} break;
		}
	}
}


	} // end TBB namespace
} // end LOFAR namespace
