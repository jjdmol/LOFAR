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
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

// information about the flash memory
static const int FL_SIZE 						= 64 * 1024 *1024; // 64 MB in bytes
static const int FL_N_PAGES 				= 32; // 32 pages in flash
static const int FL_N_SECTORS				= 512; // 512 sectors in flash
static const int FL_N_BLOCKS				= 65536; // 65336 blocks in flash

static const int FL_PAGE_SIZE 			= FL_SIZE / FL_N_PAGES; // 2.097.152 bytes  
static const int FL_SECTOR_SIZE			= FL_SIZE / FL_N_SECTORS; // 131.072 bytes
static const int FL_BLOCK_SIZE 			= FL_SIZE / FL_N_BLOCKS; // 1.024 bytes

static const int FL_SECTORS_IN_PAGE	= FL_PAGE_SIZE / FL_SECTOR_SIZE; // 16 sectors per page
static const int FL_BLOCKS_IN_SECTOR= FL_SECTOR_SIZE / FL_BLOCK_SIZE; // 128 blocks per sector
static const int FL_BLOCKS_IN_PAGE	= FL_PAGE_SIZE / FL_BLOCK_SIZE; // 2048 blocks per page

//--Constructors for a ErasefCmd object.----------------------------------------
ErasefCmd::ErasefCmd():
		itsImage(0),itsSector(0),itsBoardStatus(0)
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPErasefEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBEraseImageAckEvent();
	
	setWaitAck(true);
}
	  
//--Destructor for ErasefCmd.---------------------------------------------------
ErasefCmd::~ErasefCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

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
	itsTBBE = new TBBEraseImageEvent(event);
	
	itsTBBackE->status_mask = 0;	
	if (TS->isBoardActive(itsTBBE->board)) {	
		setBoardNr(itsTBBE->board);
	} else {
		itsTBBackE->status_mask |= TBB_NO_BOARD ;
		setDone(true);
	}
	
	itsImage = itsTBBE->image;
	itsSector = (itsImage * FL_SECTORS_IN_PAGE);
	
	// initialize TP send frame
	itsTPE->opcode	= TPERASEF;
	itsTPE->status	=	0;
	
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void ErasefCmd::sendTpEvent()
{
	itsTPE->addr = static_cast<uint32>(itsSector * FL_SECTOR_SIZE);
	TS->boardPort(getBoardNr()).send(*itsTPE);
	TS->boardPort(getBoardNr()).setTimer((long)1); // erase time of sector = 500 mSec
}

// ----------------------------------------------------------------------------
void ErasefCmd::saveTpAckEvent(GCFEvent& event)
{
	LOG_DEBUG_STR(formatString("Received ErasefAck from boardnr[%d]", getBoardNr()));
	
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask |= TBB_COMM_ERROR;
	} else {
		itsTPackE = new TPErasefAckEvent(event);
		
		itsBoardStatus	= itsTPackE->status;
		if (itsBoardStatus == 0) {
			itsSector++; 
			
			if (itsSector == ((itsImage + 1) * FL_SECTORS_IN_PAGE)) {
				setDone(true);
			}
		}
		delete itsTPackE;
	}
	
	if (itsBoardStatus != 0) {
		LOG_DEBUG_STR("Received status > 0  (ErasefCmd)");
		setDone(true);
	}
}

// ----------------------------------------------------------------------------
void ErasefCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if (itsTBBackE->status_mask == 0) {
		itsTBBackE->status_mask = TBB_SUCCESS;
	}
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
