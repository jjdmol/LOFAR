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

#include "ConfigCmd.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a ConfigCmd object.----------------------------------------
ConfigCmd::ConfigCmd():
		itsSendMask(0),itsRecvMask(0),itsErrorMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPConfigEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBConfigackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsBoardStatus[boardnr]	= 0;
	}		
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
	if((event.signal == TBB_CONFIG)||(event.signal == TP_CONFIG)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void ConfigCmd::saveTbbEvent(GCFEvent& event, int32 boards)
{
	itsTBBE 			= new TBBConfigEvent(event);
		
	itsSendMask = itsTBBE->tbbmask; // for some commands board-id is used ???
	// if SendMask = 0, select all boards
	if(itsSendMask == 0) {
		for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) {
			itsSendMask |= (1 << boardnr);
		}
	} 
	
	// mask for the installed boards
	if(boards > MAX_N_TBBBOARDS) boards = MAX_N_TBBBOARDS;
	for(int boardnr = 0;boardnr < boards;boardnr++) {
		itsBoardsMask |= (1 << boardnr);
	}
	
	// Send only commands to boards installed
	itsErrorMask = itsSendMask & ~itsBoardsMask;
	itsSendMask = itsSendMask & itsBoardsMask;
	
	// initialize TP send frame
	itsTPE->opcode	= TPCONFIG;
	itsTPE->status	=	0;
	itsTPE->image	 	= itsTBBE->image;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void ConfigCmd::sendTpEvent(GCFPortInterface& port)
{
	port.send(*itsTPE);
}

// ----------------------------------------------------------------------------
void ConfigCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	itsRecvMask |= (1 << boardnr);
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPConfigEvent(event);
		
		itsBoardStatus[boardnr]	= itsTPackE->status;
		
		LOG_DEBUG_STR(formatString("Received ConfigAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void ConfigCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	itsTBBackE->commstatus = SUCCESS;	
	if(itsErrorMask) {
		itsTBBackE->commstatus = FAILURE;
		itsTBBackE->commstatus |= (itsErrorMask << 16);
	} 
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) {
		itsTBBackE->boardstatus[boardnr]	= itsBoardStatus[boardnr];
	}
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
void ConfigCmd::portError(int32 boardnr)
{
	itsRecvMask	|= (1 << boardnr);
	itsErrorMask |= (1 << boardnr);
}

// ----------------------------------------------------------------------------
uint32 ConfigCmd::getSendMask()
{
	return itsSendMask;
}

// ----------------------------------------------------------------------------
uint32 ConfigCmd::getRecvMask()
{
	return itsRecvMask;
}

// ----------------------------------------------------------------------------
bool ConfigCmd::done()
{
	return (itsRecvMask == itsSendMask);
}

// ----------------------------------------------------------------------------
bool ConfigCmd::waitAck()
{
	return false;
}

	} // end TBB namespace
} // end LOFAR namespace
