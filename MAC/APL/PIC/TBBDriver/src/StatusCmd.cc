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
#include <Common/StringUtil.h>

#include "StatusCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a StatusCmd object.----------------------------------------
StatusCmd::StatusCmd()
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0;boardnr < MAX_N_TBBOARDS;boardnr++) { 
		itsStatus[boardnr] = TBB_NO_BOARD;
		itsV12[boardnr] = 0;
		itsV25[boardnr] = 0;
		itsV33[boardnr] = 0;
		itsTpcb[boardnr] = 0;
		itsTtp[boardnr] = 0;
		itsTmp0[boardnr] = 0;
		itsTmp1[boardnr] = 0;
		itsTmp2[boardnr] = 0;
		itsTmp3[boardnr] = 0;
		itsImage[boardnr] = 0;
		itsWatchDogMode[boardnr] = 0;
		itsPgood[boardnr] = 0;
	}
	setWaitAck(true);		
}
	  
//--Destructor for StatusCmd.---------------------------------------------------
StatusCmd::~StatusCmd()
{

}

// ----------------------------------------------------------------------------
bool StatusCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_STATUS)||(event.signal == TP_STATUS_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void StatusCmd::saveTbbEvent(GCFEvent& event)
{
	TBBStatusEvent tbb_event(event);
	
	setBoardMask(tbb_event.boardmask);
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsStatus[boardnr] = 0;
		if (!TS->isBoardActive(boardnr)) 
			itsStatus[boardnr] |= TBB_NO_BOARD;
	}	
		
	// select first boards
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void StatusCmd::sendTpEvent()
{
	TPStatusEvent tp_event;
	tp_event.opcode = TPSTATUS;
	tp_event.status = 0;
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void StatusCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus[getBoardNr()] |= TBB_COMM_ERROR;
	}	else {
		TPStatusAckEvent tp_ack(event);
		
		if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) {
			itsStatus[getBoardNr()] |= (1 << (16 + (tp_ack.status & 0x0F)));
		}
		if (tp_ack.status == 0) {
			itsV12[getBoardNr()]  = tp_ack.V12;
			itsV25[getBoardNr()]  = tp_ack.V25;
			itsV33[getBoardNr()]  = tp_ack.V33;
			itsTpcb[getBoardNr()] = tp_ack.Tpcb;
			itsTtp[getBoardNr()]  = tp_ack.Ttp;
			itsTmp0[getBoardNr()] = tp_ack.Tmp0;
			itsTmp1[getBoardNr()] = tp_ack.Tmp1;
			itsTmp2[getBoardNr()] = tp_ack.Tmp2;
			itsTmp3[getBoardNr()] = tp_ack.Tmp3;
			itsImage[getBoardNr()] = (tp_ack.info[5] & 0xf);
			itsWatchDogMode[getBoardNr()] = ((tp_ack.info[5] >> 16) & 0xf);
			itsPgood[getBoardNr()] = tp_ack.info[0];
		}
		LOG_DEBUG_STR(formatString("Received StatusAck from boardnr[%d]", getBoardNr()));
		
		LOG_INFO_STR(formatString("Status info = 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x",
			tp_ack.info[0], tp_ack.info[5], tp_ack.info[6], tp_ack.info[7], tp_ack.info[8], tp_ack.info[9]));
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void StatusCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBStatusAckEvent tbb_ack;
	
	for (int32 boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		if (itsStatus[boardnr] == 0) {
			tbb_ack.status_mask[boardnr] = TBB_SUCCESS;
		} else {
			tbb_ack.status_mask[boardnr] = itsStatus[boardnr];
		}
		tbb_ack.V12[boardnr]  = itsV12[boardnr];
		tbb_ack.V25[boardnr]  = itsV25[boardnr];
		tbb_ack.V33[boardnr]  = itsV33[boardnr];
		tbb_ack.Tpcb[boardnr] = itsTpcb[boardnr];
		tbb_ack.Ttp[boardnr]  = itsTtp[boardnr];
		tbb_ack.Tmp0[boardnr] = itsTmp0[boardnr];
		tbb_ack.Tmp1[boardnr] = itsTmp1[boardnr];
		tbb_ack.Tmp2[boardnr] = itsTmp2[boardnr];
		tbb_ack.Tmp3[boardnr] = itsTmp3[boardnr];
		tbb_ack.Image[boardnr] = itsImage[boardnr];
		tbb_ack.WatchDogMode[boardnr] = itsWatchDogMode[boardnr];
		tbb_ack.Pgood[boardnr] = itsPgood[boardnr];
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
