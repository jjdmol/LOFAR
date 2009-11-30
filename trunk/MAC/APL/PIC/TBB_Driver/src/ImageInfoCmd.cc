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
#include <Common/StringUtil.h>

#include "ImageInfoCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using	namespace TBB;

//--Constructors for a ImageInfoCmd object.----------------------------------------
ImageInfoCmd::ImageInfoCmd():
		itsBoard(0), itsImage(0), itsBlock(0)
{
	TS = TbbSettings::instance();
	for (int i = 0; i < TS->flashMaxImages(); i++) {
		itsImageVersion[i] = 0;	  
		itsWriteDate[i] = 0;	
		memset(itsTpFileName[i],'\0',16);
		memset(itsMpFileName[i],'\0',16);
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

	setBoard(tbb_event.board);
	itsBoard = tbb_event.board;
	itsImage = 0;
	
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void ImageInfoCmd::sendTpEvent()
{
	TPReadfEvent tp_event;
	tp_event.opcode = oc_READF;
	tp_event.status = 0;
	
	itsBlock = (itsImage * TS->flashBlocksInImage()) + (TS->flashBlocksInImage() - 1);
	tp_event.addr = static_cast<uint32>(itsBlock * TS->flashBlockSize());
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void ImageInfoCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(0, TBB_TIME_OUT);
		setDone(true);	
	} else {
		TPReadfAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received ReadfAck from boardnr[%d]", getBoardNr()));
		
		if (tp_ack.status != 0) {
			setStatus(0, (tp_ack.status << 24));
			setDone(true);
		} else {
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
			if (itsImage == TS->flashMaxImages()) {
				setDone(true);
			}
		}
	}
}

// ----------------------------------------------------------------------------
void ImageInfoCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBImageInfoAckEvent tbb_ack;
	tbb_ack.board = itsBoard;
	tbb_ack.active_image = TS->getImageNr(getBoardNr());
	
	for (int image = 0; image < MAX_N_IMAGES; image++) {
		tbb_ack.image_version[image] = itsImageVersion[image];	  
		tbb_ack.write_date[image] = itsWriteDate[image];	
		memcpy(tbb_ack.tp_file_name[image], itsTpFileName[image], 16);
		memcpy(tbb_ack.mp_file_name[image], itsMpFileName[image], 16);
	}
	tbb_ack.status_mask = getStatus(0);
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
