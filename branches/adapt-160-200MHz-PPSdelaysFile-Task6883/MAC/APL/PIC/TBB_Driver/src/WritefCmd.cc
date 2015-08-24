//#  WritefCmd.cc: implementation of the WritefCmd class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
#include <cstdio>

#include "WritefCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using namespace TBB;


//--Constructors for a WritefCmd object.----------------------------------------
WritefCmd::WritefCmd():
		itsStage(idle),itsImage(0),itsSector(0),itsBlock(0),itsImageSize(0),
		itsDataPtr(0),itsPassword(0)
{
	TS = TbbSettings::instance();
	itsImageData = new uint8[TS->flashImageSize()];
	memset(itsImageData,0xFF,TS->flashImageSize());
	
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
	
	setBoard(tbb_event.board);
	
	// copy filename
	memcpy(itsFileNameTp,tbb_event.filename_tp,64);
	memcpy(itsFileNameMp,tbb_event.filename_mp,64);
	
	LOG_DEBUG_STR(formatString("TP file: %s",itsFileNameTp));
	LOG_DEBUG_STR(formatString("MP file: %s",itsFileNameMp));
	
	if (readFiles()) {
		LOG_DEBUG_STR("Image files are read");
	} else {
		setStatus(0, TBB_FLASH_FILE_NOT_FIND);
		setDone(true);
	}
	
	// save Image info in last block
	time_t write_time;
	time(&write_time);
				
	// print write date and used TP and MP filename
	char info[256];
	memset(info,0,sizeof info);
				
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
				
	snprintf(info,sizeof info," %s %s ",tp_name,mp_name);
	LOG_DEBUG_STR(formatString("ImageInfo: %s",info));
	
	int addr = (TS->flashBlocksInImage() - 1) * TS->flashBlockSize();
	memset(&itsImageData[addr],0x00,TS->flashBlockSize());
	
	memcpy(&itsImageData[addr],&tbb_event.version,sizeof(uint32));
	addr += sizeof(uint32); 
	memcpy(&itsImageData[addr],&write_time,sizeof(uint32));
	addr += sizeof(uint32); 
	memcpy(&itsImageData[addr],info,sizeof(info)); 
			 

	// set start of image, 1 image-set = 2 pages
	itsImage  = tbb_event.image;
	itsSector = (itsImage * TS->flashSectorsInImage());
	itsBlock  = (itsImage * TS->flashBlocksInImage()); 
	
	itsPassword = tbb_event.password;
	
	if (itsImage == 0) {
	    if (itsPassword == 0xfac) {
		    itsStage = unprotect;
		}
		else {
		    setStatus(0, TBB_FLASH_BAD_PASSWORD);
			setDone(true);
		}
	} 
	else {
		itsStage = erase_flash;
	}
	
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void WritefCmd::sendTpEvent()
{
	switch (itsStage) {

		case unprotect: {
			TPUnprotectEvent tp_event;
			tp_event.opcode = oc_UNPROTECT;
			tp_event.status = 0;
			tp_event.password = 0xad001234;
			TS->boardPort(getBoardNr()).send(tp_event);
			TS->boardPort(getBoardNr()).setTimer(TS->timeout());
		} break;
		
		case erase_flash: {
			if ((itsImage == 0) && (itsPassword == 0xfac)) {
				TPErasefSpecEvent tp_event;
				tp_event.opcode = oc_ERASEF_SPEC;
				tp_event.status = 0;
				tp_event.addr = static_cast<uint32>(itsSector * TS->flashSectorSize());
				TS->boardPort(getBoardNr()).send(tp_event);
				TS->setBoardUsed(getBoardNr());
			} else {
				TPErasefEvent tp_event;
				tp_event.opcode = oc_ERASEF;
				tp_event.status = 0;
				tp_event.addr = static_cast<uint32>(itsSector * TS->flashSectorSize());
				TS->boardPort(getBoardNr()).send(tp_event);
				TS->setBoardUsed(getBoardNr());
			}
			TS->boardPort(getBoardNr()).setTimer(5.0); // erase time sector is 500 mSec
		} break;
		
		// stage 2, write flash
		case write_flash: {
			// fill event with data and send
			if ((itsImage == 0) && (itsPassword == 0xfac)) {
				TPWritefSpecEvent tp_event;
				tp_event.opcode = oc_WRITEF_SPEC;
				tp_event.status = 0;
				tp_event.addr = static_cast<uint32>(itsBlock * TS->flashBlockSize());
				
				for (int tp_an=0; tp_an < 256; tp_an++) {
					tp_event.data[tp_an]  = itsImageData[itsDataPtr]; itsDataPtr++;
					tp_event.data[tp_an] |= (itsImageData[itsDataPtr] << 8); itsDataPtr++; 
					tp_event.data[tp_an] |= (itsImageData[itsDataPtr] << 16); itsDataPtr++; 
					tp_event.data[tp_an] |= (itsImageData[itsDataPtr] << 24); itsDataPtr++;    
				}
				TS->boardPort(getBoardNr()).send(tp_event);
				TS->setBoardUsed(getBoardNr());
			} else {
				TPWritefEvent tp_event;
				tp_event.opcode = oc_WRITEF;
				tp_event.status = 0;
				tp_event.addr = static_cast<uint32>(itsBlock * TS->flashBlockSize());
				
				for (int tp_an=0; tp_an < 256; tp_an++) {
					tp_event.data[tp_an]  = itsImageData[itsDataPtr]; itsDataPtr++;
					tp_event.data[tp_an] |= (itsImageData[itsDataPtr] << 8); itsDataPtr++; 
					tp_event.data[tp_an] |= (itsImageData[itsDataPtr] << 16); itsDataPtr++; 
					tp_event.data[tp_an] |= (itsImageData[itsDataPtr] << 24); itsDataPtr++;    
				}
				TS->boardPort(getBoardNr()).send(tp_event);
				TS->setBoardUsed(getBoardNr());
			}
			
			TS->boardPort(getBoardNr()).setTimer(5.0);
		} break;
		
		// stage 3, verify flash
		case verify_flash: {
			TPReadfEvent tp_event;
			tp_event.opcode = oc_READF;
			tp_event.status = 0;
			tp_event.addr = static_cast<uint32>(itsBlock * TS->flashBlockSize());
			TS->boardPort(getBoardNr()).send(tp_event);
			TS->setBoardUsed(getBoardNr());
			TS->boardPort(getBoardNr()).setTimer(5.0);
		} break;

		case protect: {
			TPProtectEvent tp_event;
			tp_event.opcode = oc_PROTECT;
			tp_event.status = 0;
			TS->boardPort(getBoardNr()).send(tp_event);
			TS->setBoardUsed(getBoardNr());
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
		setStatus(0, TBB_TIME_OUT);
		setDone(true);
	} else {
		
		switch (itsStage) {
			
			case unprotect: {
				TPUnprotectAckEvent tp_ack(event);
				LOG_DEBUG_STR(formatString("Received UnprotectAck from boardnr[%d]", getBoardNr()));
				if (tp_ack.status != 0) {
					setStatus(0, (TBB_FLASH_BAD_PASSWORD | (tp_ack.status << 24)));
					setDone(true);
				} else {
					itsStage = erase_flash;
				}
			} break;
			
			case erase_flash: {
				TPErasefAckEvent tp_ack(event);
				LOG_DEBUG_STR(formatString("Received ErasefAck from boardnr[%d]", getBoardNr()));
				if (tp_ack.status != 0) {
					setStatus(0, (TBB_FLASH_ERASE_ERROR | (tp_ack.status << 24)));
					setDone(true);
				} else {
					itsSector++;
					if (itsSector == ((itsImage + 1) * TS->flashSectorsInImage())) {
						itsStage = write_flash;
					}   
				}
			} break;
			
			case write_flash: {
				TPWritefAckEvent tp_ack(event);
				LOG_DEBUG_STR(formatString("Received WritefAck from boardnr[%d]", getBoardNr()));	
				if (tp_ack.status != 0) {
					setStatus(0, (TBB_FLASH_WRITE_ERROR | (tp_ack.status << 24)));
					setDone(true);
				} else {
					itsBlock++;
					if (itsBlock == ((itsImage + 1) * TS->flashBlocksInImage())) {
						itsBlock  = (itsImage * TS->flashBlocksInImage());
						itsDataPtr = 0; 
						itsStage = verify_flash;    
					}
				}
			} break;
			
			case verify_flash: {
				// check if write-data is read-data
				uint32 testdata;
				
				TPReadfAckEvent tp_ack(event);
				LOG_DEBUG_STR(formatString("Received ReadfAck from boardnr[%d]", getBoardNr()));	
				if (tp_ack.status != 0) {
					setStatus(0, (TBB_FLASH_VERIFY_ERROR | (tp_ack.status << 24)));
					if (itsImage == 0) {
						itsStage = protect;
					} else {
						setDone(true);
					}
				} else {
					for (int i = 0; i < (TS->flashBlockSize() / 4); i++) {
						testdata = static_cast<uint32>(itsImageData[itsDataPtr])
						         + (static_cast<uint32>(itsImageData[itsDataPtr+1]) << 8)
						         + (static_cast<uint32>(itsImageData[itsDataPtr+2]) << 16)
						         + (static_cast<uint32>(itsImageData[itsDataPtr+3]) << 24);
						itsDataPtr += 4;
						/*
						testdata  = itsImageData[itsDataPtr]; itsDataPtr++;
						testdata |= (itsImageData[itsDataPtr] << 8); itsDataPtr++; 
						testdata |= (itsImageData[itsDataPtr] << 16); itsDataPtr++; 
						testdata |= (itsImageData[itsDataPtr] << 24); itsDataPtr++;    
						*/
						if (tp_ack.data[i] != testdata) {
							LOG_DEBUG_STR(formatString("block(%d) uint32(%d) NOT same 0x%08X 0x%08X (WritefCmd(verify_flash stage))",
																			itsBlock,i,tp_ack.data[i],testdata));
							setStatus(0, TBB_FLASH_VERIFY_ERROR);
							if (itsImage == 0) {
								itsStage = protect;
							} else {
								setDone(true);
							}
						}
					}
					itsBlock++;
					if (itsBlock == ((itsImage + 1) * TS->flashBlocksInImage())) {
						if (itsImage == 0) {
							itsStage = protect;
						} else {
							setDone(true);
						}
					}
				} 
			} break;
			
			case protect: {
				TPProtectAckEvent tp_ack(event);
				LOG_DEBUG_STR(formatString("Received ProtectAck from boardnr[%d]", getBoardNr()));
				if (tp_ack.status != 0) {
					setStatus(0, (tp_ack.status << 24));
				}
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
	
	tbb_ack.status_mask = getStatus(0);
	
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

