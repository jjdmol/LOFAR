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
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

// information about the flash memory
static const int FL_SIZE 						= 64 * 1024 *1024; // 64 MB in bytes
//static const int FL_N_SECTORS				= 512; // 512 sectors in flash
static const int FL_N_BLOCKS				= 65536; // 65336 blocks in flash
static const int FL_N_IMAGES 				= 16; // 16 pages in flash

static const int FL_IMAGE_SIZE 			= FL_SIZE / FL_N_IMAGES; // 2.097.152 bytes  
//static const int FL_SECTOR_SIZE			= FL_SIZE / FL_N_SECTORS; // 131.072 bytes
static const int FL_BLOCK_SIZE 			= FL_SIZE / FL_N_BLOCKS; // 1.024 bytes

//static const int FL_SECTORS_IN_PAGE	= FL_IMAGE_SIZE / FL_SECTOR_SIZE; // 16 sectors per page
//static const int FL_BLOCKS_IN_SECTOR= FL_SECTOR_SIZE / FL_BLOCK_SIZE; // 128 blocks per sector
static const int FL_BLOCKS_IN_IMAGE	= FL_IMAGE_SIZE / FL_BLOCK_SIZE; // 2048 blocks per page

//static const int IMAGE_SIZE					= 977489; // 977489 bytes in 1 image 
//static const int IMAGE_BLOCKS				= IMAGE_SIZE / FL_BLOCK_SIZE; // 977489 bytes in 1 image 


//--Constructors for a ImageInfoCmd object.----------------------------------------
ImageInfoCmd::ImageInfoCmd():
		itsImage(0),itsBlock(0),itsBoardStatus(0)
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPReadfEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBImageInfoAckEvent();
	setWaitAck(true);
}
	  
//--Destructor for ImageInfoCmd.---------------------------------------------------
ImageInfoCmd::~ImageInfoCmd()
{
	delete itsTPE;
	delete itsTBBackE;
}

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
	itsTBBE = new TBBImageInfoEvent(event);
	
	setBoardNr(itsTBBE->board);	
	itsTBBackE->board = getBoardNr();	
	
	itsTBBackE->status_mask = 0;
	itsTBBackE->active_image = TS->getImageNr(getBoardNr());
	itsImage = 0;
		
	// initialize TP send frame
	itsTPE->opcode	= TPREADF;
	itsTPE->status	=	0;
	delete itsTBBE;	
}

// ----------------------------------------------------------------------------
void ImageInfoCmd::sendTpEvent()
{
	itsBlock = (itsImage * FL_BLOCKS_IN_IMAGE) + (FL_BLOCKS_IN_IMAGE - 1);
	itsTPE->addr		= static_cast<uint32>(itsBlock * FL_BLOCK_SIZE);
	TS->boardPort(getBoardNr()).send(*itsTPE);
	TS->boardPort(getBoardNr()).setTimer(0.5);
}

// ----------------------------------------------------------------------------
void ImageInfoCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsTBBackE->status_mask |= TBB_COMM_ERROR;
		setDone(true);	
	} else {
		itsTPackE = new TPReadfAckEvent(event);
		
		if (itsTPackE->status == 0) {
			char info[256];
			memset(info,0,256);
			memcpy(info,&itsTPackE->data[2],256);
			
			LOG_DEBUG_STR("ImageInfoCmd: " << info); 
			
			itsTBBackE->image_version[itsImage]= itsTPackE->data[0];	  
			itsTBBackE->write_date[itsImage] = itsTPackE->data[1];	
			
			memset(itsTBBackE->tp_file_name[itsImage],'\0',16);
			memset(itsTBBackE->mp_file_name[itsImage],'\0',16);
			//sscanf(info,"%s %s ",
			//			 itsTBBackE->tp_file_name[itsImage],
			//			 itsTBBackE->mp_file_name[itsImage]);
			char* startptr;
			char* stopptr;
			int namesize;
			
			startptr = &info[1];
			stopptr = strstr(startptr," ");
			if (stopptr != 0) {
				namesize = stopptr - startptr;
				memcpy(itsTBBackE->tp_file_name[itsImage],startptr,namesize);
				
				startptr = stopptr + 1;
				stopptr = strstr(startptr + 1," ");
				if (stopptr != 0) {	
					namesize = stopptr - startptr;
					memcpy(itsTBBackE->mp_file_name[itsImage],startptr,namesize);
				}
			}			
			LOG_DEBUG_STR("tp_file_name: " << itsTBBackE->tp_file_name[itsImage]);
			LOG_DEBUG_STR("mp_file_name: " << itsTBBackE->mp_file_name[itsImage]);
			//itsTBBackE->image_version[itsImage]= itsTPackE->data[0];	  
			//itsTBBackE->write_date[itsImage] = itsTPackE->data[1];		  		
		
			itsImage++;
			if (itsImage == FL_N_IMAGES) {
				setDone(true);
			}
		}
		delete itsTPackE;
	}
}

// ----------------------------------------------------------------------------
void ImageInfoCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if (itsTBBackE->status_mask == 0)
			itsTBBackE->status_mask = TBB_SUCCESS;
	
	if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}
