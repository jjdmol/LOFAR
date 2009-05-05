//#  ErasefCmd.cc: implementation of the ErasefCmd class
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

#include "ErasefCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;
// information about the flash memory
static const int FL_SIZE            = 64 * 1024 *1024; // 64 MB in bytes
static const int FL_N_SECTORS       = 512; // 512 sectors in flash
static const int FL_N_BLOCKS        = 65536; // 65336 blocks in flash
static const int FL_N_IMAGES        = 16; // 16 images in flash

static const int FL_IMAGE_SIZE      = FL_SIZE / FL_N_IMAGES; // 4194304 bytes  
static const int FL_SECTOR_SIZE     = FL_SIZE / FL_N_SECTORS; // 131.072 bytes
static const int FL_BLOCK_SIZE      = FL_SIZE / FL_N_BLOCKS; // 1.024 bytes

static const int FL_SECTORS_IN_IMAGE = FL_IMAGE_SIZE / FL_SECTOR_SIZE; // 32 sectors per image
static const int FL_BLOCKS_IN_SECTOR = FL_SECTOR_SIZE / FL_BLOCK_SIZE; // 128 blocks per sector
static const int FL_BLOCKS_IN_IMAGE  = FL_IMAGE_SIZE / FL_BLOCK_SIZE; // 4096 blocks per image


//--Constructors for a ErasefCmd object.----------------------------------------
ErasefCmd::ErasefCmd():
		itsStatus(0), itsImage(0),itsSector(0)
{
	TS = TbbSettings::instance();
	
	setWaitAck(true);
}
	  
//--Destructor for ErasefCmd.---------------------------------------------------
ErasefCmd::~ErasefCmd() { }

// ----------------------------------------------------------------------------
bool ErasefCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_ERASE_IMAGE)||(event.signal == TP_ERASEF_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ErasefCmd::saveTbbEvent(GCFEvent& event)
{
	TBBEraseImageEvent tbb_event(event);
	
	itsStatus = 0;	
	if (TS->isBoardActive(tbb_event.board)) {	
		setBoardNr(tbb_event.board);
	} else {
		itsStatus |= TBB_NO_BOARD ;
		setDone(true);
	}
	
	itsImage = tbb_event.image;
	itsSector = (itsImage * FL_SECTORS_IN_IMAGE);
}

// ----------------------------------------------------------------------------
void ErasefCmd::sendTpEvent()
{
	TPErasefEvent tp_event;
	tp_event.opcode = oc_ERASEF;
	tp_event.status = 0;
	
	tp_event.addr = static_cast<uint32>(itsSector * FL_SECTOR_SIZE);
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout()); // erase time of sector = 500 mSec
}

// ----------------------------------------------------------------------------
void ErasefCmd::saveTpAckEvent(GCFEvent& event)
{
	LOG_DEBUG_STR(formatString("Received ErasefAck from boardnr[%d]", getBoardNr()));
	
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus |= TBB_COMM_ERROR;
	} else {
		TPErasefAckEvent tp_ack(event);
		
		if (tp_ack.status == 0) {
			itsSector++; 
			
			if (itsSector == ((itsImage + 1) * FL_SECTORS_IN_IMAGE)) {
				setDone(true);
			}
		} else {
			LOG_DEBUG_STR("Received status > 0  (ErasefCmd)");
			setDone(true);
		}
	}
}

// ----------------------------------------------------------------------------
void ErasefCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBEraseImageAckEvent tbb_ack;
	
	if (itsStatus == 0) {
		tbb_ack.status_mask = TBB_SUCCESS;
	} else {
		tbb_ack.status_mask = itsStatus;
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
