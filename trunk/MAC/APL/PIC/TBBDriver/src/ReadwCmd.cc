//#  ReadwCmd.cc: implementation of the ReadwCmd class
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

#include "ReadwCmd.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a ReadwCmd object.----------------------------------------
ReadwCmd::ReadwCmd():
		itsSendMask(0),itsRecvMask(0),itsErrorMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPReadwEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBReadwackEvent();
	
	itsMp						= 0;
	itsBoardStatus	= 0;
	itsAddr					= 0;
	itsWordLo				= 0;
	itsWordHi				= 0;
}
	  
//--Destructor for ReadwCmd.---------------------------------------------------
ReadwCmd::~ReadwCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool ReadwCmd::isValid(GCFEvent& event)
{
	if((event.signal == TBB_READW)||(event.signal == TP_READW)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void ReadwCmd::saveTbbEvent(GCFEvent& event, uint32 activeboards)
{
	itsTBBE 			= new TBBReadwEvent(event);
		
	itsSendMask = itsTBBE->tbbmask; // for some commands board-id is used ???
	// if SendMask = 0, select all boards
	if(itsSendMask == 0) {
		for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) {
			itsSendMask |= (1 << boardnr);
		}
	} 
	
	// mask for the installed boards
	itsBoardsMask = activeboards;
	
	// Send only commands to boards installed
	itsErrorMask = itsSendMask & ~itsBoardsMask;
	itsSendMask = itsSendMask & itsBoardsMask;
	
	// initialize TP send frame
	itsTPE->opcode	= TPREADW;
	itsTPE->status	= 0;
	itsTPE->mp			=	itsTBBE->mp;
	itsTPE->addr		=	itsTBBE->addr;
	itsTPE->wordlo	= 0;
	itsTPE->wordhi	= 0;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void ReadwCmd::sendTpEvent(GCFPortInterface& port)
{
	port.send(*itsTPE);
}

// ----------------------------------------------------------------------------
void ReadwCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	itsRecvMask |= (1 << boardnr);
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPReadwEvent(event);
		
		itsBoardStatus	= itsTPackE->status;
		itsMp 		= itsTPackE->mp;
		itsAddr		= itsTPackE->addr;
		itsWordLo	= itsTPackE->wordlo;
		itsWordHi	= itsTPackE->wordhi;
		
		LOG_DEBUG_STR(formatString("Received ReadwAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void ReadwCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	itsTBBackE->commstatus = SUCCESS;	
	if(itsErrorMask) {
		itsTBBackE->commstatus = FAILURE;
		itsTBBackE->commstatus |= (itsErrorMask << 16);
	} 
	
	itsTBBackE->boardstatus	= itsBoardStatus;
	itsTBBackE->mp					= itsMp;
	itsTBBackE->addr 				= itsAddr;
	itsTBBackE->wordlo			=	itsWordLo;
	itsTBBackE->wordhi			= itsWordHi;
	
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
void ReadwCmd::portError(int32 boardnr)
{
	itsRecvMask	|= (1 << boardnr);
	itsErrorMask |= (1 << boardnr);
}

// ----------------------------------------------------------------------------
uint32 ReadwCmd::getSendMask()
{
	return itsSendMask;
}

// ----------------------------------------------------------------------------
uint32 ReadwCmd::getRecvMask()
{
	return itsRecvMask;
}

// ----------------------------------------------------------------------------
bool ReadwCmd::done()
{
	return (itsRecvMask == itsSendMask);
}

// ----------------------------------------------------------------------------
bool ReadwCmd::waitAck()
{
	return true;
}

	} // end TBB namespace
} // end LOFAR namespace
