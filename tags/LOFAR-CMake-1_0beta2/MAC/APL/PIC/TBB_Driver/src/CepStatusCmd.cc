//#  CepStatusCmd.cc: implementation of the CepStatusCmd class
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

#include "CepStatusCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;	
using namespace TBB;

//--Constructors for a VersionCmd object.--------------------------------------
CepStatusCmd::CepStatusCmd()
{
	TS = TbbSettings::instance();
	
	for(int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		itsPagesLeft[boardnr] = 0;
	}
	setWaitAck(true);
}
  
//--Destructor for GetVersions.------------------------------------------------
CepStatusCmd::~CepStatusCmd() { }

// ----------------------------------------------------------------------------
bool CepStatusCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_CEP_STATUS)||(event.signal == TP_CEP_STATUS_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void CepStatusCmd::saveTbbEvent(GCFEvent& event)
{
	TBBCepStatusEvent tbb_event(event);
	
	setBoards(tbb_event.boardmask);
	
	nextBoardNr(); // select first board
}

// ----------------------------------------------------------------------------
void CepStatusCmd::sendTpEvent()
{
	TPCepStatusEvent tp_event;
	tp_event.opcode = oc_CEP_STATUS;
	tp_event.status = 0;
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void CepStatusCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	}	else {
		TPCepStatusAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received CepStatusAck from boardnr[%d]", getBoardNr()));
		
		if (tp_ack.status != 0) {
			setStatus(getBoardNr(), (tp_ack.status << 24));
		} else {
			itsPagesLeft[getBoardNr()] = tp_ack.pages_left;
		}
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void CepStatusCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBCepStatusAckEvent tbb_ack;
	
	for (int32 i = 0; i < MAX_N_TBBOARDS; i++) { 
		tbb_ack.status_mask[i] = getStatus(i);
		tbb_ack.pages_left[i] = itsPagesLeft[i];
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
