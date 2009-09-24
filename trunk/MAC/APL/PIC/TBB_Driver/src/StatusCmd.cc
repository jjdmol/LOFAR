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
StatusCmd::~StatusCmd() { }

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
	
	setBoards(tbb_event.boardmask);
	
	nextBoardNr(); // select first boards
}

// ----------------------------------------------------------------------------
void StatusCmd::sendTpEvent()
{
	TPStatusEvent tp_event;
	tp_event.opcode = oc_STATUS;
	tp_event.status = 0;
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void StatusCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	}	else {
		TPStatusAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received StatusAck from boardnr[%d]", getBoardNr()));
		
		if (tp_ack.status != 0) {
			setStatus(getBoardNr(), (tp_ack.status << 24));
		} else {
			itsV12[getBoardNr()]  = tp_ack.V12;
			itsV25[getBoardNr()]  = tp_ack.V25;
			itsV33[getBoardNr()]  = tp_ack.V33;
			itsTpcb[getBoardNr()] = tp_ack.Tpcb;
			itsTtp[getBoardNr()]  = tp_ack.Ttp;
			itsTmp0[getBoardNr()] = tp_ack.Tmp0;
			itsTmp1[getBoardNr()] = tp_ack.Tmp1;
			itsTmp2[getBoardNr()] = tp_ack.Tmp2;
			itsTmp3[getBoardNr()] = tp_ack.Tmp3;
			itsImage[getBoardNr()] = TS->getImageNr(getBoardNr());
			itsWatchDogMode[getBoardNr()] = ((tp_ack.info[5] >> 16) & 0xf);
			itsPgood[getBoardNr()] = tp_ack.info[0];
				
			LOG_INFO_STR(formatString("Status info = 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x",
												tp_ack.info[0], tp_ack.info[5], tp_ack.info[6], 
												tp_ack.info[7], tp_ack.info[8], tp_ack.info[9]));
		}
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void StatusCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBStatusAckEvent tbb_ack;
	
	for (int32 i = 0; i < MAX_N_TBBOARDS; i++) { 
		tbb_ack.status_mask[i] = getStatus(i);
		tbb_ack.V12[i]  = itsV12[i];
		tbb_ack.V25[i]  = itsV25[i];
		tbb_ack.V33[i]  = itsV33[i];
		tbb_ack.Tpcb[i] = itsTpcb[i];
		tbb_ack.Ttp[i]  = itsTtp[i];
		tbb_ack.Tmp0[i] = itsTmp0[i];
		tbb_ack.Tmp1[i] = itsTmp1[i];
		tbb_ack.Tmp2[i] = itsTmp2[i];
		tbb_ack.Tmp3[i] = itsTmp3[i];
		tbb_ack.Image[i] = itsImage[i];
		tbb_ack.WatchDogMode[i] = itsWatchDogMode[i];
		tbb_ack.Pgood[i] = itsPgood[i];
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
