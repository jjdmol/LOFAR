//#  UdpCmd.cc: implementation of the UdpCmd class
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

#include "UdpCmd.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a UdpCmd object.----------------------------------------
UdpCmd::UdpCmd():
		itsBoardMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPUdpEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBUdpackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsTBBackE->status[boardnr]	= 0;
	}		
}
	  
//--Destructor for UdpCmd.---------------------------------------------------
UdpCmd::~UdpCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool UdpCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_UDP)||(event.signal == TP_UDPACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void UdpCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE 			= new TBBUdpEvent(event);
	
	// mask for the installed boards
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();
	itsBoardMask = itsTBBE->boardmask; 
		
	for (int boardnr = 0; boardnr < DriverSettings::instance()->maxBoards(); boardnr++) {
		
		if (!(itsBoardsMask & (1 << boardnr))) 
			itsTBBackE->status[boardnr] |= NO_BOARD;
				
		if (!(itsBoardsMask & (1 << boardnr)) &&  (itsBoardMask & (1 << boardnr)))
			itsTBBackE->status[boardnr] |= (SELECT_ERROR | BOARD_SEL_ERROR);
	}

	// Send only commands to boards installed
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	// initialize TP send frame
	itsTPE->opcode	= TPUDP;
	itsTPE->status	=	0;
	itsTPE->udp[0]	= itsTBBE->udp[0];
	itsTPE->udp[1]	= itsTBBE->udp[1];
	itsTPE->ip[0]		= itsTBBE->ip[0];
	itsTPE->ip[1]		= itsTBBE->ip[1];
	itsTPE->ip[2]	 	= itsTBBE->ip[2];
	itsTPE->ip[3]	 	= itsTBBE->ip[3];
	itsTPE->ip[4]	 	= itsTBBE->ip[4];
	itsTPE->mac[0]	= itsTBBE->mac[0];
	itsTPE->mac[1]	= itsTBBE->mac[1];
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
bool UdpCmd::sendTpEvent(int32 boardnr, int32)
{
	bool sending = false;
	DriverSettings*		ds = DriverSettings::instance();
	
	if (ds->boardPort(boardnr).isConnected() && (itsTBBackE->status[boardnr] == 0)) {
		ds->boardPort(boardnr).send(*itsTPE);
		ds->boardPort(boardnr).setTimer(ds->timeout());
		sending = true;
	}
	else
		itsTBBackE->status[boardnr] |= CMD_ERROR;
	
	return(sending);
}

// ----------------------------------------------------------------------------
void UdpCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status[boardnr] |= COMM_ERROR;
	}
	else {
		itsTPackE = new TPUdpackEvent(event);
		
		if ((itsTPackE->status >= 0xF0) && (itsTPackE->status <= 0xF6)) 
			itsTBBackE->status[boardnr] |= (1 << (16 + (itsTPackE->status & 0x0F)));
		
		LOG_DEBUG_STR(formatString("Received UdpAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void UdpCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	for (int32 boardnr = 0; boardnr < DriverSettings::instance()->maxBoards(); boardnr++) { 
		if (itsTBBackE->status[boardnr] == 0)
			itsTBBackE->status[boardnr] = SUCCESS;
	}

	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
CmdTypes UdpCmd::getCmdType()
{
	return(BoardCmd);
}

// ----------------------------------------------------------------------------
uint32 UdpCmd::getBoardMask()
{
	return(itsBoardMask);
}

// ----------------------------------------------------------------------------
bool UdpCmd::waitAck()
{
	return(true);
}

	} // end TBB namespace
} // end LOFAR namespace
