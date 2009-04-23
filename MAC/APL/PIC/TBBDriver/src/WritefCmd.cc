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
#include <Common/StringUtil.h>
#include <time.h>

#include "WritefCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using namespace TBB;

// information about the flash memory
static const int FL_SIZE            = 64 * 1024 *1024; // 64 MB in bytes
static const int FL_N_SECTORS       = 512; // 512 sectors in flash
static const int FL_N_BLOCKS        = 65536; // 65336 blocks in flash
static const int FL_N_IMAGES        = 16; // 16 images in flash

static const int FL_IMAGE_SIZE      = FL_SIZE / FL_N_IMAGES; // 4194304 bytes  
static const int FL_SECTOR_SIZE     = FL_SIZE / FL_N_SECTORS; // 131.072 bytes
static const int FL_BLOCK_SIZE      = FL_SIZE / FL_N_BLOCKS; // 1.024 bytes

static const int FL_SECTORS_IN_IMAGE = FL_IMAGE_SIZE / FL_SECTOR_SIZE; // 32 sectors per image
//static const int FL_BLOCKS_IN_SECTOR = FL_SECTOR_SIZE / FL_BLOCK_SIZE; // 128 blocks per sector
static const int FL_BLOCKS_IN_IMAGE  = FL_IMAGE_SIZE / FL_BLOCK_SIZE; // 4096 blocks per image



//--Constructors for a WritefCmd object.----------------------------------------
WritefCmd::WritefCmd():
		itsStage(idle),itsImage(0),itsSector(0),itsBlock(0),itsImageSize(0),
		itsDataPtr(0),itsPassword(0),itsStatus(0)
{
	TS = TbbSettings::instance();
	itsImageData = new uint8[FL_IMAGE_SIZE];
	memset(itsImageData,0xFF,FL_IMAGE_SIZE);
	
	setWaitAck(true);
}
		
//--Destructor for WritefCmd.---------------------------------------------------
WritefCmd::~WritefCmd()
{
	delete [] itsImageData; 
}

// ----------------------------------------------------------------------------
bool WritefCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_WRITE_IMAGE) 
		|| (event.signal == TP_ERASEF_ACK)
		|| (event.signal == TP_ERASEF_SPEC_ACK)
		|| (event.signal == TP_WRITEF_ACK)
		|| (event.signal == TP_WRITEF_SPEC_ACK)
		|| (event.signal == TP_READF_ACK)
		|| (event.signal == TP_PROTECT_ACK)
		|| (event.signal == TP_UNPROTECT_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void WritefCmd::saveTbbEvent(GCFEvent& event)
{
	TBBWriteImageEvent tbb_event(event);
	
	if (TS->isBoardActive(tbb_event.board)) {  
		setBoardNr(tbb_event.board);
	} else {
		itsStatus |= TBB_NO_BOARD ;
		setDone(true);
	}
	
	// copy filename
	memcpy(itsFileNameTp,tbb_event.filename_tp,sizeof(char) * 64);
	memcpy(itsFileNameMp,tbb_event.filename_mp,sizeof(char) * 64);
	
	LOG_DEBUG_STR(formatString("TP file: %s",itsFileNameTp));
	LOG_DEBUG_STR(formatString("MP file: %s",itsFileNameMp));
	
	if (readFiles()) {
		LOG_DEBUG_STR("Image files are read");
	} else {
		itsStatus |= TBB_FLASH_ERROR;
		setDone(true);
	}
	
	// save Image info in last block
	time_t write_time;
	time(&write_time);
				
	// print write date and used TP and MP filename
	char info[256];
	memset(info,0,256);
				
	char *tp_name = strrchr(itsFileNameTp,'/');
	if (tp_name == 0) {
		tp_name = itsFileNameTp;
	} else {
		tp_name += 1;
	} 
				
	char *mp_name = strrchr(itsFileNameMp,'/');
	if (mp_name == 0) {
		mp_name = itsFileNameMp;
	} else {
		mp_name += 1;
	}
				
	sprintf(info," %s %s ",tp_name,mp_name);
	LOG_DEBUG_STR(formatString("ImageInfo: %s",info));
	
	int addr = (FL_BLOCKS_IN_IMAGE - 1) * FL_BLOCK_SIZE;
	memset(&itsImageData[addr],0x00,FL_BLOCK_SIZE);
	
	memcpy(&itsImageData[addr],&tbb_event.version,sizeof(uint32));
	addr += sizeof(uint32); 
	memcpy(&itsImageData[addr],&write_time,sizeof(uint32));
	addr += sizeof(uint32); 
	memcpy(&itsImageData[addr],info,sizeof(info)); 
			 

	// set start of image, 1 image-set = 2 pages
	itsImage  = tbb_event.image;
	itsSector = (itsImage * FL_SECTORS_IN_IMAGE);
	itsBlock  = (itsImage * FL_BLOCKS_IN_IMAGE); 
	
	itsPassword = tbb_event.password;
	
	if ((itsImage == 0) && (itsPassword != 0xfac)) {
		itsStage = unprotect;
	} else {
		itsStage = erase_flash;
	}
}

// ----------------------------------------------------------------------------
void WritefCmd::sendTpEvent()
{
		switch (itsStage) {

			case unprotect: {
				TPUnprotectEvent tp_event;
				tp_event.opcode = TPUNPROTECT;
				tp_event.status = 0;
				tp_event.password = itsPassword;
				TS->boardPort(getBoardNr()).send(tp_event);
				TS->boardPort(getBoardNr()).setTimer(TS->timeout());
			} break;
			
			case erase_flash: {
				if ((itsImage == 0) && (itsPassword != 0xfac)) {
					TPErasefSpecEvent tp_event;
					tp_event.opcode = TPERASEFSPEC;
					tp_event.status = 0;
					tp_event.addr = static_cast<uint32>(itsSector * FL_SECTOR_SIZE);
					TS->boardPort(getBoardNr()).send(tp_event);
				} else {
					TPErasefEvent tp_event;
					tp_event.opcode = TPERASEF;
					tp_event.status = 0;
					tp_event.addr = static_cast<uint32>(itsSector * FL_SECTOR_SIZE);
					TS->boardPort(getBoardNr()).send(tp_event);
				}
				TS->boardPort(getBoardNr()).setTimer(5.0); // erase time sector is 500 mSec
			} break;
			
			// stage 2, write flash
			case write_flash: {
				// fill event with data and send
				if ((itsImage == 0) && (itsPassword != 0xfac)) {
					TPWritefSpecEvent tp_event;
					tp_event.opcode = TPWRITEFSPEC;
					tp_event.status = 0;
					tp_event.addr = static_cast<uint32>(itsBlock * FL_BLOCK_SIZE);
					
					//int ptr = itsBlock - (itsImage * FL_BLOCKS_IN_IMAGE);
					for (int tp_an=0; tp_an < 256; tp_an++) {
						tp_event.data[tp_an]  = itsImageData[itsDataPtr]; itsDataPtr++;
						tp_event.data[tp_an] |= (itsImageData[itsDataPtr] << 8); itsDataPtr++; 
						tp_event.data[tp_an] |= (itsImageData[itsDataPtr] << 16); itsDataPtr++; 
						tp_event.data[tp_an] |= (itsImageData[itsDataPtr] << 24); itsDataPtr++;    
					}
					TS->boardPort(getBoardNr()).send(tp_event);
				} else {
					TPWritefEvent tp_event;
					tp_event.opcode = TPWRITEF;
					tp_event.status = 0;
					tp_event.addr = static_cast<uint32>(itsBlock * FL_BLOCK_SIZE);
					
					//int ptr = itsBlock - (itsImage * FL_BLOCKS_IN_IMAGE);
					for (int tp_an=0; tp_an < 256; tp_an++) {
						tp_event.data[tp_an]  = itsImageData[itsDataPtr]; itsDataPtr++;
						tp_event.data[tp_an] |= (itsImageData[itsDataPtr] << 8); itsDataPtr++; 
						tp_event.data[tp_an] |= (itsImageData[itsDataPtr] << 16); itsDataPtr++; 
						tp_event.data[tp_an] |= (itsImageData[itsDataPtr] << 24); itsDataPtr++;    
					}
					TS->boardPort(getBoardNr()).send(tp_event);
				}
				
				TS->boardPort(getBoardNr()).setTimer(5.0);
			} break;
			
			// stage 3, verify flash
			case verify_flash: {
				TPReadfEvent tp_event;
				tp_event.opcode  = TPREADF;
				tp_event.status  = 0;
				tp_event.addr = static_cast<uint32>(itsBlock * FL_BLOCK_SIZE);
				TS->boardPort(getBoardNr()).send(tp_event);
				TS->boardPort(getBoardNr()).setTimer(5.0);
			} break;

			case protect: {
				TPProtectEvent tp_event;
				tp_event.opcode = TPPROTECT;
				tp_event.status = 0;
				TS->boardPort(getBoardNr()).send(tp_event);
				TS->boardPort(getBoardNr()).setTimer(TS->timeout());
			} break;
		 
			default : {
			} break;
		}
}

// ----------------------------------------------------------------------------
void WritefCmd::saveTpAckEvent(GCFEvent& event)
{
	if (event.signal == F_TIMER) {
				itsStatus |= TBB_COMM_ERROR;
				setDone(true);
	} else {
		
		switch (itsStage) {
			
			case unprotect: {
				itsStage = erase_flash;
			} break;
			
			case erase_flash: {
				TPErasefAckEvent tp_ack(event);
				
				if (tp_ack.status == 0) {
					//setSleepTime(0.50);
					itsSector++;
					if (itsSector == ((itsImage + 1) * FL_SECTORS_IN_IMAGE)) {
						itsStage = write_flash;
					}   
				} else {
					itsStatus |= TBB_FLASH_ERROR;
					LOG_DEBUG_STR("Received status > 0 (WritefCmd(erase_flash stage))");
					setDone(true);
				}
			} break;
			
			case write_flash: {
				TPWritefAckEvent tp_ack(event);
					
					if (tp_ack.status == 0) {
						//setSleepTime(0.002);
						itsBlock++;
					} else {
						itsStatus |= TBB_FLASH_ERROR;
						LOG_DEBUG_STR("Received status > 0 (WritefCmd(write_flash stage))");
						setDone(true);
					}
					
					if (itsBlock == ((itsImage + 1) * FL_BLOCKS_IN_IMAGE)) {
						itsBlock  = (itsImage * FL_BLOCKS_IN_IMAGE);
						itsDataPtr = 0; 
						itsStage = verify_flash;    
					}
			} break;
			
			case verify_flash: {
				// check if write-data is read-data
				uint32 testdata;
				
				TPReadfAckEvent tp_ack(event);
				
				if (tp_ack.status == 0) {
					for (int i = 0; i < (FL_BLOCK_SIZE / 4); i++) {
						testdata  = itsImageData[itsDataPtr]; itsDataPtr++;
						testdata |= (itsImageData[itsDataPtr] << 8); itsDataPtr++; 
						testdata |= (itsImageData[itsDataPtr] << 16); itsDataPtr++; 
						testdata |= (itsImageData[itsDataPtr] << 24); itsDataPtr++;    
		 
						if (tp_ack.data[i] != testdata) {
							LOG_DEBUG_STR(formatString("block(%d) uint32(%d) NOT same 0x%08X 0x%08X (WritefCmd(verify_flash stage))",
																			itsBlock,i,tp_ack.data[i],testdata));
							itsStatus |= TBB_FLASH_ERROR;
							if (itsImage == 0) {
								itsStage = protect;
							} else {
								setDone(true);
							}
						}
					}
					itsBlock++;
					if (itsBlock == ((itsImage + 1) * FL_BLOCKS_IN_IMAGE)) {
						if (itsImage == 0) {
							itsStage = protect;
						} else {
							setDone(true);
						}
					}
				} else {
					itsStatus |= TBB_FLASH_ERROR;
					LOG_DEBUG_STR("Received status > 0 (WritefCmd(verify_flash stage))");
					if ((itsImage == 0) && (itsPassword != 0xfac)) {
						itsStage = protect;
					} else {
						setDone(true);
					}
				}
			} break;
			
			case protect: {
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
	TBBWriteImageAckEvent tbb_ack;
	
	if (itsStatus == 0) {
		tbb_ack.status_mask = TBB_SUCCESS;
	} else {
		tbb_ack.status_mask = itsStatus;
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}

// ----------------------------------------------------------------------------
bool WritefCmd::readFiles()
{
	FILE  *itsFile;
	int dataPtr = 0;
	int ch_h, ch_l;
	
	LOG_DEBUG_STR("Opening TP file");
	// load Tp hex file
	itsFile = fopen(itsFileNameTp,"r");
	if (itsFile == 0) {
		LOG_INFO_STR("Error on opening TP file");
		return (false);
	}
	
	LOG_DEBUG_STR("Getting TP file");
	while (1) {
		ch_h = getc(itsFile);
		if (ch_h == EOF) break;
		ch_l = getc(itsFile);
		if (ch_l == EOF) break;
		if ((ch_h == 0x0D) && (ch_l == 0x0A)) { break; }   
		itsImageData[dataPtr] = (charToHex(ch_h) << 4) + charToHex(ch_l);
		dataPtr++;
	}
	LOG_DEBUG_STR("Closing TP file");
	fclose(itsFile);
	
	LOG_DEBUG_STR("Opening MP file");
	// load Mp hex file
	itsFile = fopen(itsFileNameMp,"r");
	if (itsFile == 0) {
		LOG_INFO_STR("Error on opening MP file");
		return (false);
	}
	
	LOG_DEBUG_STR("Getting MP file");   
	while (1) {
		ch_h = getc(itsFile);
		if (ch_h == EOF) break;
		ch_l = getc(itsFile);
		if (ch_l == EOF) break;
		if ((ch_h == 0x0D) && (ch_l == 0x0A)) { break; } 
		itsImageData[dataPtr] = (charToHex(ch_h) << 4) + charToHex(ch_l);
		dataPtr++;
	}
	LOG_DEBUG_STR("Closing MP file");    
	fclose(itsFile);
	
	itsImageSize = dataPtr;
	return (true);
}

// ----------------------------------------------------------------------------
uint8 WritefCmd::charToHex(int ch)
{
	uint8 hex = 0;
	
	if ((ch >= '0') && (ch <= '9')) {
		hex = static_cast<uint8>(ch & 0x0F);
	} 
	else if (((ch >= 'A') && (ch <= 'F')) || ((ch >= 'a') && (ch <= 'f'))) {
		hex = static_cast<uint8>((ch & 0x0F) + 9);
	} 
	return (hex);
}

