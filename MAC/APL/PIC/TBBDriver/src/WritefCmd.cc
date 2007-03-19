//#  WritefCmd.cc: implementation of the WritefCmd class
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
#include <time.h>

#include "WritefCmd.h"


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



//--Constructors for a WritefCmd object.----------------------------------------
WritefCmd::WritefCmd():
		itsStage(idle),itsImage(0),itsSector(0),itsBlock(0),itsImageSize(0),itsBoardStatus(0)
{
	TS					= TbbSettings::instance();
	itsTPE 			= new TPWritefEvent();
	itsTPackE 	= 0;
	itsTBBE 		= 0;
	itsTBBackE 	= new TBBWriteImageAckEvent();
	itsImageData= 0;
	setWaitAck(true);
}
	  
//--Destructor for WritefCmd.---------------------------------------------------
WritefCmd::~WritefCmd()
{
	delete itsTPE;
	delete itsTBBackE;
	if (itsTBBE) delete itsTBBE;
	if (itsImageData) delete itsImageData;
}

// ----------------------------------------------------------------------------
bool WritefCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_WRITE_IMAGE) 
		|| (event.signal == TP_ERASEFACK)
		|| (event.signal == TP_WRITEFACK)
		|| (event.signal == TP_READFACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void WritefCmd::saveTbbEvent(GCFEvent& event)
{
	itsTBBE	= new TBBWriteImageEvent(event);
	
	setBoardNr(itsTBBE->board);	
	
	memcpy(itsFileNameTp,itsTBBE->filename_tp,sizeof(char) * 64);
	memcpy(itsFileNameMp,itsTBBE->filename_mp,sizeof(char) * 64);
	
	LOG_DEBUG_STR(formatString("TP file: %s",itsFileNameTp));
	LOG_DEBUG_STR(formatString("MP file: %s",itsFileNameMp));
	
	itsImageData = new uint8[1966080];
			
	readFiles();
	
	itsTBBackE->status_mask = 0;
	
	itsImage 	= itsTBBE->image;
	itsSector	= (itsImage * FL_SECTORS_IN_PAGE);
	itsBlock	= (itsImage * FL_BLOCKS_IN_PAGE); 
	
	// initialize TP send frame
	itsTPE->opcode			= TPWRITEF;
	itsTPE->status			=	0;
	
	itsStage = erase_flash;
}

// ----------------------------------------------------------------------------
void WritefCmd::sendTpEvent()
{
		switch (itsStage) {
			
			// stage 1, erase flash
			case erase_flash: {
				TPErasefEvent *erasefEvent = new TPErasefEvent();
				erasefEvent->opcode	= TPERASEF;
				erasefEvent->status	=	0;
				erasefEvent->addr = static_cast<uint32>(itsSector * FL_SECTOR_SIZE);
				TS->boardPort(getBoardNr()).send(*erasefEvent);
				TS->boardPort(getBoardNr()).setTimer((long)1); // erase time sector is 500 mSec
				delete erasefEvent;
			} break;
			
			// stage 2, write flash
			case write_flash: {
				// fill event with data and send
				itsTPE->addr = static_cast<uint32>(itsBlock * FL_BLOCK_SIZE);
				
				int ptr = itsBlock - (itsImage * FL_BLOCKS_IN_PAGE);
				for (int tp_an=0; tp_an < 256; tp_an++) {
					itsTPE->data[tp_an]  = itsImageData[ptr]; ptr++;
					itsTPE->data[tp_an] |= (itsImageData[ptr] << 8); ptr++; 
					itsTPE->data[tp_an] |= (itsImageData[ptr] << 16); ptr++; 
					itsTPE->data[tp_an] |= (itsImageData[ptr] << 24); ptr++; 		
				}
				
				TS->boardPort(getBoardNr()).send(*itsTPE);
				TS->boardPort(getBoardNr()).setTimer(0.2);
			} break;
			
			// stage 3, verify flash
			case verify_flash: {
				TPReadfEvent *readfEvent = new TPReadfEvent();
				readfEvent->opcode	= TPREADF;
				readfEvent->status	=	0;
				readfEvent->addr = static_cast<uint32>(itsBlock * FL_BLOCK_SIZE);
				TS->boardPort(getBoardNr()).send(*readfEvent);
				TS->boardPort(getBoardNr()).setTimer(0.2);
				delete readfEvent;
			} break;
			
			case write_info: {
				// save Image info in last block
				itsBlock = (itsImage * FL_BLOCKS_IN_PAGE) + (FL_BLOCKS_IN_PAGE - 1);
				itsTPE->addr = static_cast<uint32>(itsBlock * FL_BLOCK_SIZE);
				for (int i = 0; i < 256; i++) {
					itsTPE->data[i]  = 0;
				}
				time_t write_time;
				time(&write_time);
				itsTPE->data[0] = static_cast<uint32>(itsTBBE->version);
				itsTPE->data[1] = static_cast<uint32>(write_time);
				itsTPE->data[2] = 300;
				itsTPE->data[3] = 400;
				itsTPE->data[4] = 500;
				itsTPE->data[5] = 600;
				itsTPE->data[6] = 700;
				itsTPE->data[7] = 800;
				itsTPE->data[8] = 900;
				itsTPE->data[9] = 1000;
				TS->boardPort(getBoardNr()).send(*itsTPE);
				TS->boardPort(getBoardNr()).setTimer(0.2);
				LOG_DEBUG_STR("Writing image info");
				LOG_DEBUG_STR(formatString("%u %u",itsTPE->data[0],itsTPE->data[1]));
			} break;
			
			case verify_info: {
				TPReadfEvent *readfEvent = new TPReadfEvent();
				readfEvent->opcode	= TPREADF;
				readfEvent->status	=	0;
				itsBlock = (itsImage * FL_BLOCKS_IN_PAGE) + (FL_BLOCKS_IN_PAGE - 1);
				readfEvent->addr = static_cast<uint32>(itsBlock * FL_BLOCK_SIZE);
				TS->boardPort(getBoardNr()).send(*readfEvent);
				TS->boardPort(getBoardNr()).setTimer(0.2);
				LOG_DEBUG_STR("Verifying image info");
				delete readfEvent;
			} break;
			
			default : {
			} break;
		}
}

// ----------------------------------------------------------------------------
void WritefCmd::saveTpAckEvent(GCFEvent& event)
{
	if (event.signal == F_TIMER) {
				itsTBBackE->status_mask |= TBB_COMM_ERROR;
				setDone(true);
	}	else {
		
		switch (itsStage) {
			
			case erase_flash: {
				TPErasefackEvent *erasefAckEvent = new TPErasefackEvent(event);
				
				if (erasefAckEvent->status == 0) {
					itsSector++;
					if (itsSector == ((itsImage + 1) * FL_SECTORS_IN_PAGE)) {
						itsStage = write_flash;
					}		
				} else {
					LOG_DEBUG_STR("Received status > 0 (WritefCmd(erase_flash stage))");
					setDone(true);
				}
				delete erasefAckEvent;
			} break;
			
			case write_flash: {
				itsTPackE = new TPWritefackEvent(event);
					
					if (itsTPackE->status == 0) {
						itsStage = verify_flash;		
					} else {
						LOG_DEBUG_STR("Received status > 0 (WritefCmd(write_flash stage))");
						setDone(true);
					}
					delete itsTPackE;
			} break;
			
			case verify_flash: {
				// check if write-data is read-data
				bool same = true;
				
				TPReadfackEvent *readfAckEvent = new TPReadfackEvent(event);
				
				if (readfAckEvent->status == 0) {
					for (int i = 0; i < (FL_BLOCK_SIZE / 4); i++) {
						if (readfAckEvent->data[i] != itsTPE->data[i]) {
							LOG_DEBUG_STR(formatString("data (%d) %d not same 0x%08X 0x%08X (WritefCmd(verify_flash stage))",itsBlock,i,readfAckEvent->data[i],itsTPE->data[i]));
							same = false;	
						}
					}
					if (same) {
						itsBlock++;
						itsStage = write_flash;
					} else {
						setDone(true);
					}
					
					int nextByte = ((itsBlock - (itsImage * FL_BLOCKS_IN_PAGE)) * FL_BLOCK_SIZE); 
					if (nextByte > itsImageSize) {
						//setDone(true);
						itsStage = write_info;
					}
				} else {
					LOG_DEBUG_STR("Received status > 0 (WritefCmd(verify_flash stage))");
					setDone(true);
				}				
				delete readfAckEvent;
			} break;
			
			case write_info: {
				itsTPackE = new TPWritefackEvent(event);
					
				if (itsTPackE->status == 0) {
					itsStage = verify_info;		
				} else {
					LOG_DEBUG_STR(formatString("Received status > 0 (0x%08X) (WritefCmd(write_info stage))",itsTPackE->status));
					setDone(true);
				}
				delete itsTPackE;
			} break;
			
			case verify_info: {
			// check if write-data is read-data
				bool same = true;
				
				TPReadfackEvent *readfAckEvent = new TPReadfackEvent(event);
				
				if (readfAckEvent->status == 0) {
					for (int i = 0; i < 10; i++) {
						if (readfAckEvent->data[i] != itsTPE->data[i]) {
							LOG_DEBUG_STR(formatString("image info %d not same 0x%08X 0x%08X (WritefCmd(verify_info stage))",i,readfAckEvent->data[i],itsTPE->data[i]));
							same = false;	
						}
					}
					if (!same) {
						itsTBBackE->status_mask |= TBB_FLASH_ERROR; 
					}
				} else {
					LOG_DEBUG_STR(formatString("Received status > 0 (0x%08X) (WritefCmd(verify_info stage))",readfAckEvent->status));
				}				
				delete readfAckEvent;
				setDone(true);
			} break;
			
			default : {
			} break;
		}
	}
}

// ----------------------------------------------------------------------------
void WritefCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if (itsTBBackE->status_mask == 0)
			itsTBBackE->status_mask = TBB_SUCCESS;
	
	clientport->send(*itsTBBackE);
}

void WritefCmd::readFiles()
{
	FILE 	*itsFile;
	int dataPtr = 0;
	int ch_h, ch_l;
	
	// load Tp hex file
	itsFile = fopen(itsFileNameTp,"r");
	
	ch_h = getc(itsFile);
	ch_l = getc(itsFile);
	while (ch_l != EOF) {
		itsImageData[dataPtr] = (charToHex(ch_h) << 4) + charToHex(ch_l);
		dataPtr++;
		ch_h = getc(itsFile);
		ch_l = getc(itsFile);
	}
	fclose(itsFile);
	
	// load Mp hex file
	itsFile = fopen(itsFileNameMp,"r");
	
	ch_h = getc(itsFile);
	ch_l = getc(itsFile);
	while (ch_l != EOF) {
		itsImageData[dataPtr] = (charToHex(ch_h) << 4) + charToHex(ch_l);
		dataPtr++;
		ch_h = getc(itsFile);
		ch_l = getc(itsFile);
	}
	fclose(itsFile);
	
	itsImageSize = dataPtr;
}


uint8 WritefCmd::charToHex(int ch)
{
	if ((ch >= '0') && (ch <= '9')) {
		return (ch & 0x0F);
	} 
	
	if (((ch >= 'A') && (ch <= 'F')) || ((ch >= 'a') && (ch <= 'f')))	{
		return ((ch & 0x0F) + 9);
	}
	return (0);
}

