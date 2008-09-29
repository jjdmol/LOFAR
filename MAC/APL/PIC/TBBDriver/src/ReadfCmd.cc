//#  ReadfCmd.cc: implementation of the ReadfCmd class
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

#include "ReadfCmd.h"

using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

// information about the flash memory
static const int FL_SIZE 						= 64 * 1024 *1024; // 64 MB in bytes
static const int FL_N_IMAGES 				= 16; // 32 pages in flash
//static const int FL_N_SECTORS				= 512; // 512 sectors in flash
static const int FL_N_BLOCKS				= 65536; // 65536 blocks in flash

static const int FL_PAGE_SIZE 			= FL_SIZE / FL_N_IMAGES; // 2.097.152 bytes  
//static const int FL_SECTOR_SIZE			= FL_SIZE / FL_N_SECTORS; // 131.072 bytes
static const int FL_BLOCK_SIZE 			= FL_SIZE / FL_N_BLOCKS; // 1.024 bytes

//static const int FL_SECTORS_IN_PAGE	= FL_PAGE_SIZE / FL_SECTOR_SIZE; // 16 sectors per page
//static const int FL_BLOCKS_IN_SECTOR= FL_SECTOR_SIZE / FL_BLOCK_SIZE; // 128 blocks per sector
static const int FL_BLOCKS_IN_PAGE	= FL_PAGE_SIZE / FL_BLOCK_SIZE; // 2048 blocks per page

//static const int IMAGE_SIZE					= 977489; // 977489 bytes in 1 image 
//static const int IMAGE_BLOCKS				= IMAGE_SIZE / FL_BLOCK_SIZE; // 977489 bytes in 1 image 


//--Constructors for a ReadfCmd object.----------------------------------------
ReadfCmd::ReadfCmd():
		itsFile(0),itsImage(0),itsBlock(0),itsBoardStatus(0)
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPReadfEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBReadImageAckEvent();
	setWaitAck(true);
}
	  
//--Destructor for ReadfCmd.---------------------------------------------------
ReadfCmd::~ReadfCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

// ----------------------------------------------------------------------------
bool ReadfCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_READ_IMAGE)||(event.signal == TP_READF_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ReadfCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE = new TBBReadImageEvent(event);
	
	itsTBBackE->status_mask = 0;
	if (TS->isBoardActive(itsTBBE->board)) {	
		setBoardNr(itsTBBE->board);
	} else {
		itsTBBackE->status_mask |= TBB_NO_BOARD ;
		setDone(true);
	}
	
	itsImage = itsTBBE->image;
	itsBlock = (itsImage * FL_BLOCKS_IN_PAGE);
	
	// initialize TP send frame
	itsTPE->opcode	= TPREADF;
	itsTPE->status	=	0;
	
	itsFile = fopen("image.hex","wb");
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void ReadfCmd::sendTpEvent()
{
	itsTPE->addr		= static_cast<uint32>(itsBlock * FL_BLOCK_SIZE);
	TS->boardPort(getBoardNr()).send(*itsTPE);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void ReadfCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask |= TBB_COMM_ERROR;
		setDone(true);	
	} else {
		itsTPackE = new TPReadfAckEvent(event);
		
		itsBoardStatus	= itsTPackE->status;
		  
		int byte_val;
		int nible_val;
		if (itsBoardStatus == 0) { 
			for (int dp = 0; dp < 256; dp++) {	// there are 256 words in 1 message
				for (int bn = 0; bn < 4; bn++) {
					byte_val = (itsTPackE->data[dp] >> (bn * 8)) & 0xFF; // take 1 byte
				
					nible_val = ((byte_val & 0xF0) >> 4);
					if (nible_val < 10) {
						nible_val += 48;
					}	else {
						nible_val += 87;	
					}
					putc(nible_val,itsFile);
					
					nible_val = (byte_val & 0x0F);
					if (nible_val < 10) {
						nible_val += 48;
					}	else {
						nible_val += 87;	
					}
					putc(nible_val,itsFile);
				}
			}
			
			itsBlock++;
			if (itsBlock == ((itsImage + 1) * FL_BLOCKS_IN_PAGE)) {
				setDone(true);
			}
		} else {
			LOG_DEBUG_STR(formatString("Block %d received status = 0x%08X (ReadfCmd)", itsBlock, itsBoardStatus));
			setDone(true);
		}
		
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void ReadfCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	fclose(itsFile);
	if (itsTBBackE->status_mask == 0)
			itsTBBackE->status_mask = TBB_SUCCESS;
	
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
