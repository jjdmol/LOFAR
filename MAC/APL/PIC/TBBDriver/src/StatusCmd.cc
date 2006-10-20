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

#include "StatusCmd.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	using namespace TP_Protocol;
	namespace TBB {

//--Constructors for a StatusCmd object.----------------------------------------
StatusCmd::StatusCmd():
		itsBoardMask(0),itsErrorMask(0),itsBoardsMask(0)
{
	itsTPE 			= new TPStatusEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBStatusackEvent();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) { 
		itsBoardStatus[boardnr]	= 0;
		itsV12[boardnr] 				= 0;
		itsV25[boardnr] 				= 0;
		itsV33[boardnr] 				= 0;
		itsTpcb[boardnr] 				= 0;
		itsTtp[boardnr] 				= 0;
		itsTmp0[boardnr] 				= 0;
		itsTmp1[boardnr] 				= 0;
		itsTmp2[boardnr] 				= 0;
		itsTmp3[boardnr] 				= 0;
	}		
}
	  
//--Destructor for StatusCmd.---------------------------------------------------
StatusCmd::~StatusCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool StatusCmd::isValid(GCFEvent& event)
{
	if((event.signal == TBB_STATUS)||(event.signal == TP_STATUSACK)) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void StatusCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE 			= new TBBStatusEvent(event);
		
	itsBoardMask = itsTBBE->boardmask; // for some commands board-id is used ???
		
	itsBoardsMask = DriverSettings::instance()->activeBoardsMask();
	
	// Send only commands to boards installed
	itsErrorMask = itsBoardMask & ~itsBoardsMask;
	itsBoardMask = itsBoardMask & itsBoardsMask;
	
	itsTBBackE->status = 0;
	
	// initialize TP send frame
	itsTPE->opcode	= TPSTATUS;
	itsTPE->status	=	0;
			
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void StatusCmd::sendTpEvent(int32 boardnr, int32)
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
void StatusCmd::saveTpAckEvent(GCFEvent& event, int32 boardnr)
{
	// in case of a time-out, set error mask
	if(event.signal == F_TIMER) {
		itsErrorMask |= (1 << boardnr);
	}
	else {
		itsTPackE = new TPStatusackEvent(event);
		
		itsBoardStatus[boardnr]	= itsTPackE->status;
		itsV12[boardnr]					= itsTPackE->V12;
		itsV25[boardnr]					= itsTPackE->V25;
		itsV33[boardnr]					= itsTPackE->V33;
		itsTpcb[boardnr]				= itsTPackE->Tpcb;
		itsTtp[boardnr]					= itsTPackE->Ttp;
		itsTmp0[boardnr]				= itsTPackE->Tmp0;
		itsTmp1[boardnr]				= itsTPackE->Tmp1;
		itsTmp2[boardnr]				= itsTPackE->Tmp2;
		itsTmp3[boardnr]				= itsTPackE->Tmp3;
		
		LOG_DEBUG_STR(formatString("Received StatusAck from boardnr[%d]", boardnr));
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void StatusCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if(itsErrorMask != 0) {
		itsTBBackE->status |= COMM_ERROR;
		itsTBBackE->status |= (itsErrorMask << 16);
	}
	if(itsTBBackE->status == 0) itsTBBackE->status = SUCCESS;
	
	for(int boardnr = 0;boardnr < MAX_N_TBBBOARDS;boardnr++) {
		itsTBBackE->V12[boardnr]					= itsV12[boardnr];
		itsTBBackE->V25[boardnr]					= itsV25[boardnr];
		itsTBBackE->V33[boardnr]					= itsV33[boardnr];
		itsTBBackE->Tpcb[boardnr]					= itsTpcb[boardnr];
		itsTBBackE->Ttp[boardnr]					= itsTtp[boardnr];
		itsTBBackE->Tmp0[boardnr]					= itsTmp0[boardnr];
		itsTBBackE->Tmp1[boardnr]					= itsTmp1[boardnr];
		itsTBBackE->Tmp2[boardnr]					= itsTmp2[boardnr];
		itsTBBackE->Tmp3[boardnr]					= itsTmp3[boardnr];
	}
	clientport->send(*itsTBBackE);
}

// ----------------------------------------------------------------------------
CmdTypes StatusCmd::getCmdType()
{
	return BoardCmd;
}

// ----------------------------------------------------------------------------
uint32 StatusCmd::getBoardMask()
{
	return itsBoardMask;
}

// ----------------------------------------------------------------------------
bool StatusCmd::waitAck()
{
	return true;
}

	} // end TBB namespace
} // end LOFAR namespace
