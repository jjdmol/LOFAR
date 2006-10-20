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
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;	
	namespace TBB {

//--Constructors for a VersionCmd object.--------------------------------------
VersionCmd::VersionCmd():
		itsBoardMask(0),itsErrorMask(0),itsBoardsMask(0)
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
	if((event.signal == TBB_VERSION)||(event.signal == TP_VERSIONACK)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void VersionCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE 			= new TBBVersionEvent(event);
		
	itsBoardMask = itsTBBE->boardmask; // for some commands board-id is used ???
			
	// mask for the installed boards
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();

	// Send only commands to boards installed
	itsErrorMask = itsBoardMask & ~itsBoardsMask;
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	itsTBBackE->status = 0;
	
	// fill TP command, to send
	itsTPE->opcode 			  = TPVERSION;
	itsTPE->status				= 0;
			
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void VersionCmd::sendTpEvent(int32 boardnr, int32)
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
void VersionCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		//TPVersionEvent tpe(event);
		itsTPackE = new TPVersionackEvent(event);
		
		itsBoardStatus[boardnr]	= itsTPackE->status;
		itsBoardId[boardnr] 		= itsTPackE->boardid;
		itsSwVersion[boardnr] 	= itsTPackE->swversion;
		itsBoardVersion[boardnr]= itsTPackE->boardversion;
		itsTpVersion[boardnr] 	= itsTPackE->tpversion;
		itsMp0Version[boardnr] 	= itsTPackE->mp0version;
		itsMp1Version[boardnr] 	= itsTPackE->mp1version;
		itsMp2Version[boardnr] 	= itsTPackE->mp2version;
		itsMp3Version[boardnr] 	= itsTPackE->mp3version;
		
		LOG_DEBUG_STR(formatString("VersionCmd: board[%d] %u;%u;%u;%u;%u;%u;%u;%u;%u",
				boardnr,itsTPackE->status,itsTPackE->boardid,itsTPackE->swversion,itsTPackE->boardversion,
				itsTPackE->tpversion,itsTPackE->mp0version,itsTPackE->mp1version,itsTPackE->mp2version,itsTPackE->mp3version));
		
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void VersionCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if(itsErrorMask != 0) {
		itsTBBackE->status |= COMM_ERROR;
		itsTBBackE->status |= (itsErrorMask << 16);
	}
	if(itsTBBackE->status == 0) itsTBBackE->status = SUCCESS;
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) {
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
CmdTypes VersionCmd::getCmdType()
{
	return BoardCmd;
}

// ----------------------------------------------------------------------------
uint32 VersionCmd::getBoardMask()
{
	return itsBoardMask;
}

// ----------------------------------------------------------------------------
bool VersionCmd::waitAck()
{
	return true;
}

	} // end namespace TBB
} // end namespace LOFAR
