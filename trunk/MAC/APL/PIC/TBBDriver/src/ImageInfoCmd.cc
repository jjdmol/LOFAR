//#  ImageInfoCmd.cc: implementation of the ImageInfoCmd class
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

#include "ImageInfoCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

// information about the flash memory
static const int FL_SIZE            = 64 * 1024 *1024; // 64 MB in bytes
static const int FL_N_BLOCKS        = 65536; // 65336 blocks in flash
static const int FL_N_IMAGES        = 16; // 16 pages in flash

static const int FL_IMAGE_SIZE      = FL_SIZE / FL_N_IMAGES; // 2.097.152 bytes  
static const int FL_BLOCK_SIZE      = FL_SIZE / FL_N_BLOCKS; // 1.024 bytes

static const int FL_BLOCKS_IN_IMAGE = FL_IMAGE_SIZE / FL_BLOCK_SIZE; // 2048 blocks per page


//--Constructors for a ImageInfoCmd object.----------------------------------------
ImageInfoCmd::ImageInfoCmd():
		itsStatus(0), itsImage(0), itsBlock(0)
{
	TS = TbbSettings::instance();
	for (int image = 0; image < MAX_N_IMAGES; image++) {
		itsImageVersion[image] = 0;	  
		itsWriteDate[image] = 0;	
		memset(itsTpFileName[image],'\0',16);
		memset(itsMpFileName[image],'\0',16);
	}
	setWaitAck(true);
}

//--Destructor for ImageInfoCmd.---------------------------------------------------
ImageInfoCmd::~ImageInfoCmd() { }

// ----------------------------------------------------------------------------
bool ImageInfoCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_IMAGE_INFO)||(event.signal == TP_READF_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ImageInfoCmd::saveTbbEvent(GCFEvent& event)
{
	TBBImageInfoEvent tbb_event(event);

	if (TS->isBoardActive(tbb_event.board)) {
		setBoardNr(tbb_event.board);
	} else {
		itsStatus |= TBB_NO_BOARD ;
		setDone(true);
	}
	
	itsImage = 0;
}

// ----------------------------------------------------------------------------
void ImageInfoCmd::sendTpEvent()
{
	TPReadfEvent tp_event;
	tp_event.opcode = oc_READF;
	tp_event.status = 0;
	
	itsBlock = (itsImage * FL_BLOCKS_IN_IMAGE) + (FL_BLOCKS_IN_IMAGE - 1);
	tp_event.addr = static_cast<uint32>(itsBlock * FL_BLOCK_SIZE);
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void ImageInfoCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus |= TBB_COMM_ERROR;
		setDone(true);	
	} else {
		TPReadfAckEvent tp_ack(event);
		
		if (tp_ack.status == 0) {
			char info[256];
			memset(info,0,256);
			memcpy(info,&tp_ack.data[2],256);
			
			LOG_DEBUG_STR("ImageInfoCmd: " << info); 
			
			itsImageVersion[itsImage]= tp_ack.data[0];	  
			itsWriteDate[itsImage] = tp_ack.data[1];	
			char* startptr;
			char* stopptr;
			int namesize;
			
			startptr = &info[1];
			stopptr = strstr(startptr," ");
			if (stopptr != 0) {
				namesize = stopptr - startptr;
				memcpy(itsTpFileName[itsImage], startptr, namesize);
				
				startptr = stopptr + 1;
				stopptr = strstr(startptr + 1," ");
				if (stopptr != 0) {	
					namesize = stopptr - startptr;
					memcpy(itsMpFileName[itsImage], startptr, namesize);
				}
			}			
			LOG_DEBUG_STR("tp_file_name: " << itsTpFileName[itsImage]);
			LOG_DEBUG_STR("mp_file_name: " << itsMpFileName[itsImage]);
		
			itsImage++;
			if (itsImage == FL_N_IMAGES) {
				setDone(true);
			}
		}
	}
}

// ----------------------------------------------------------------------------
void ImageInfoCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBImageInfoAckEvent tbb_ack;
	tbb_ack.board = getBoardNr();
	tbb_ack.active_image = TS->getImageNr(getBoardNr());
	
	for (int image = 0; image < MAX_N_IMAGES; image++) {
		tbb_ack.image_version[image] = itsImageVersion[image];	  
		tbb_ack.write_date[image] = itsWriteDate[image];	
		memcpy(tbb_ack.tp_file_name[image], itsTpFileName[image], 16);
		memcpy(tbb_ack.mp_file_name[image], itsMpFileName[image], 16);
	}
	
	if (itsStatus == 0) {
		tbb_ack.status_mask = TBB_SUCCESS;
	} else {
		tbb_ack.status_mask = itsStatus;
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
