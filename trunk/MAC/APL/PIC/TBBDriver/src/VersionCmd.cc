//#  GetVersions.cc: implementation of the GetVersions class
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

#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include "TP_Protocol.ph"

#include "VersionCmd.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;	
	namespace TBB {


// Constructors for a GetVersions object.
VersionCmd::VersionCmd():
	itsSendMask(0),itsRecvMask(0),itsErrorMask(0)
{

}
	  
// Destructor for GetVersions.
VersionCmd::~VersionCmd()
{

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
void VersionCmd::saveTbbEvent(GCFEvent& event)
{
	TBBVersionEvent tbbe(event);
	
	itsSendMask = tbbe.tbbmask; // for some commands board-id is used ???
	itsRecvMask = 0;
	itsErrorMask = 0;
}

// ----------------------------------------------------------------------------
void VersionCmd::makeTpEvent()
{
	itsVersionEvent.opcode 				= 0x00000701;
	itsVersionEvent.boardid 			= 0;
	itsVersionEvent.swversion 		= 0;
	itsVersionEvent.boardversion 	= 0;
	itsVersionEvent.tpversion 		= 0;
	itsVersionEvent.mp1version		= 0;
	itsVersionEvent.mp2version		= 0;
	itsVersionEvent.mp3version		= 0;
	itsVersionEvent.mp4version		= 0;
}

// ----------------------------------------------------------------------------
void VersionCmd::sendTpEvent(GCFPortInterface& port)
{
	port.send(itsVersionEvent);
	port.setTimer(TIME_OUT);
}

// ----------------------------------------------------------------------------
void VersionCmd::saveTpAckEvent(GCFEvent& event, int boardnr)
{
	itsRecvMask |= (1 << boardnr);
	// in case of a time-out, save alle zero's
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
		itsBoardId[boardnr] 		= 0;
		itsSwVersion[boardnr] 	= 0;
		itsBoardVersion[boardnr]= 0;
		itsTpVersion[boardnr] 	= 0;
		itsMp1Version[boardnr] 	= 0;
		itsMp2Version[boardnr] 	= 0;
		itsMp3Version[boardnr] 	= 0;
		itsMp4Version[boardnr] 	= 0;	
	}
	else {
		TPVersionEvent tpe(event);
		itsBoardId[boardnr] 		= tpe.boardid;
		itsSwVersion[boardnr] 	= tpe.swversion;
		itsBoardVersion[boardnr]= tpe.boardversion;
		itsTpVersion[boardnr] 	= tpe.tpversion;
		itsMp1Version[boardnr] 	= tpe.mp1version;
		itsMp2Version[boardnr] 	= tpe.mp2version;
		itsMp3Version[boardnr] 	= tpe.mp3version;
		itsMp4Version[boardnr] 	= tpe.mp4version;
	}
}

// ----------------------------------------------------------------------------
void VersionCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBVersionackEvent tbbe;
	
	tbbe.status = 0;	
	if(itsErrorMask) {
		tbbe.status = 1;
		tbbe.status |= (itsErrorMask << 16);
	} 
	for(int boardnr = 0;boardnr < 12;boardnr++) {
		tbbe.boardinfo[boardnr].boardid 			= itsBoardId[boardnr];
		tbbe.boardinfo[boardnr].swversion 		= itsSwVersion[boardnr];
		tbbe.boardinfo[boardnr].boardversion  = itsBoardVersion[boardnr];
		tbbe.boardinfo[boardnr].tpversion 		= itsTpVersion[boardnr];
		tbbe.boardinfo[boardnr].mp1version  	= itsMp1Version[boardnr];
		tbbe.boardinfo[boardnr].mp2version  	= itsMp2Version[boardnr];
		tbbe.boardinfo[boardnr].mp3version  	= itsMp3Version[boardnr];
		tbbe.boardinfo[boardnr].mp4version  	= itsMp4Version[boardnr];		
	}
	clientport->send(tbbe);
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
uint32 VersionCmd::done()
{
	return (itsRecvMask == itsSendMask);
}
	} // end TBB namespace
} // end LOFAR namespace

