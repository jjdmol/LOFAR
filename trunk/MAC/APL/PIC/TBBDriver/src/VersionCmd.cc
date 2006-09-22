//#  VersionCmd.cc: implementation of the VersionsCmd class
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

#include "VersionCmd.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;	
	namespace TBB {

//--Constructors for a VersionCmd object.--------------------------------------
VersionCmd::VersionCmd():
		itsSendMask(0),itsRecvMask(0),itsErrorMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPVersionEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBVersionackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsBoardStatus[boardnr]		= 0;
		itsBoardId[boardnr]				= 0;
		itsSwVersion[boardnr]			= 0;
		itsBoardVersion[boardnr]	= 0;
		itsTpVersion[boardnr] 		= 0;
		itsMp0Version[boardnr]		= 0;
		itsMp1Version[boardnr]		= 0;
		itsMp2Version[boardnr]		= 0;
		itsMp3Version[boardnr]		= 0;
	}
}
  
//--Destructor for GetVersions.------------------------------------------------
VersionCmd::~VersionCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool VersionCmd::isValid(GCFEvent& event)
{
	if((event.signal == TBB_VERSION)||(event.signal == TP_VERSION)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void VersionCmd::saveTbbEvent(GCFEvent& event, int32 boards)
{
	itsTBBE 			= new TBBVersionEvent(event);
		
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
	
	// fill TP command, to send
	itsTPE->opcode 			  = TPVERSION;
	itsTPE->status				= 0;
	itsTPE->boardid 			= 0;
	itsTPE->swversion 		= 0;
	itsTPE->boardversion  = 0;
	itsTPE->tpversion 		= 0;
	itsTPE->mp0version		= 0;
	itsTPE->mp1version		= 0;
	itsTPE->mp2version		= 0;
	itsTPE->mp3version		= 0;
		
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void VersionCmd::sendTpEvent(GCFPortInterface& port)
{
	port.send(*itsTPE);
}

// ----------------------------------------------------------------------------
void VersionCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	itsRecvMask |= (1 << boardnr);
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		//TPVersionEvent tpe(event);
		itsTPackE = new TPVersionEvent(event);
		
		itsBoardStatus[boardnr]	= itsTPackE->status;
		itsBoardId[boardnr] 		= itsTPackE->boardid;
		itsSwVersion[boardnr] 	= itsTPackE->swversion;
		itsBoardVersion[boardnr]= itsTPackE->boardversion;
		itsTpVersion[boardnr] 	= itsTPackE->tpversion;
		itsMp0Version[boardnr] 	= itsTPackE->mp0version;
		itsMp1Version[boardnr] 	= itsTPackE->mp1version;
		itsMp2Version[boardnr] 	= itsTPackE->mp2version;
		itsMp3Version[boardnr] 	= itsTPackE->mp3version;
		
		LOG_DEBUG_STR(formatString("Received VersionAck from boardnr[%d]", boardnr));
		/*
		LOG_DEBUG_STR(formatString("opcode       = 0X%08X", itsTPackE->opcode));
		LOG_DEBUG_STR(formatString("status       = 0X%08X", itsTPackE->status));
		LOG_DEBUG_STR(formatString("boardid      = 0X%08X", itsTPackE->boardid));
		LOG_DEBUG_STR(formatString("swversion    = 0X%08X", itsTPackE->swversion));
		LOG_DEBUG_STR(formatString("boardversion = 0X%08X", itsTPackE->boardversion));
		LOG_DEBUG_STR(formatString("tpversion    = 0X%08X", itsTPackE->tpversion));
		LOG_DEBUG_STR(formatString("mp0version   = 0X%08X", itsTPackE->mp0version));
		LOG_DEBUG_STR(formatString("mp1version   = 0X%08X", itsTPackE->mp1version));
		LOG_DEBUG_STR(formatString("mp2version   = 0X%08X", itsTPackE->mp2version));
		LOG_DEBUG_STR(formatString("mp3version   = 0X%08X", itsTPackE->mp3version));
		*/
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void VersionCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	itsTBBackE->commstatus = SUCCESS;	
	if(itsErrorMask) {
		itsTBBackE->commstatus = FAILURE;
		itsTBBackE->commstatus |= (itsErrorMask << 16);
	} 
	// fill all the fields of the TBBackEvent
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) {
		itsTBBackE->boardstatus[boardnr]	= itsBoardStatus[boardnr];
		itsTBBackE->boardid[boardnr] 			= itsBoardId[boardnr];
		itsTBBackE->swversion[boardnr] 		= itsSwVersion[boardnr];
		itsTBBackE->boardversion[boardnr] = itsBoardVersion[boardnr];
		itsTBBackE->tpversion[boardnr] 		= itsTpVersion[boardnr];
		itsTBBackE->mp0version[boardnr]  	= itsMp0Version[boardnr];
		itsTBBackE->mp1version[boardnr]  	= itsMp1Version[boardnr];
		itsTBBackE->mp2version[boardnr]  	= itsMp2Version[boardnr];
		itsTBBackE->mp3version[boardnr]  	= itsMp3Version[boardnr];		
	}
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
void VersionCmd::portError(int32 boardnr)
{
	itsRecvMask |= (1 << boardnr);
	itsErrorMask |= (1 << boardnr);
}

// ----------------------------------------------------------------------------
uint32 VersionCmd::getSendMask()
{
	return itsSendMask;
}

// ----------------------------------------------------------------------------
uint32 VersionCmd::getRecvMask()
{
	return itsRecvMask;
}

// ----------------------------------------------------------------------------
bool VersionCmd::done()
{
	if (itsRecvMask == itsSendMask) return true;
	return false;
}

// ----------------------------------------------------------------------------
bool VersionCmd::waitAck()
{
	return true;
}

	} // end namespace TBB
} // end namespace LOFAR
