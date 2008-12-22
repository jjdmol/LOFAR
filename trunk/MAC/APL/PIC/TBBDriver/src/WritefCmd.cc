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
    itsStage(idle),itsImage(0),itsSector(0),itsBlock(0),itsImageSize(0),itsDataPtr(0),itsBoardStatus(0)
{
  TS          = TbbSettings::instance();
  itsTPE      = new TPWritefEvent();
  itsTPackE   = 0;
  itsTBBE     = 0;
  itsTBBackE  = new TBBWriteImageAckEvent();
  itsImageData  = new uint8[FL_IMAGE_SIZE];
  
  itsTBBackE->status_mask = 0;
  setWaitAck(true);
}
    
//--Destructor for WritefCmd.---------------------------------------------------
WritefCmd::~WritefCmd()
{
  delete itsTPE;
  delete itsTBBackE;
  if (itsTBBE) delete itsTBBE;
  delete [] itsImageData; 
}

// ----------------------------------------------------------------------------
bool WritefCmd::isValid(GCFEvent& event)
{
  if ((event.signal == TBB_WRITE_IMAGE) 
    || (event.signal == TP_ERASEF_ACK)
    || (event.signal == TP_WRITEF_ACK)
    || (event.signal == TP_READF_ACK)) {
    return(true);
  }
  return(false);
}

// ----------------------------------------------------------------------------
void WritefCmd::saveTbbEvent(GCFEvent& event)
{
  itsTBBE     = new TBBWriteImageEvent(event);
  
  itsTBBackE->status_mask = 0;
  if (TS->isBoardActive(itsTBBE->board)) {  
    setBoardNr(itsTBBE->board);
  } else {
    itsTBBackE->status_mask |= TBB_NO_BOARD ;
    setDone(true);
  }
  
  // copy filename
  memcpy(itsFileNameTp,itsTBBE->filename_tp,sizeof(char) * 64);
  memcpy(itsFileNameMp,itsTBBE->filename_mp,sizeof(char) * 64);
  
  LOG_DEBUG_STR(formatString("TP file: %s",itsFileNameTp));
  LOG_DEBUG_STR(formatString("MP file: %s",itsFileNameMp));
  
  if (readFiles()) {
    LOG_DEBUG_STR("Image files are read");
  } else {
    itsTBBackE->status_mask |= TBB_FLASH_ERROR;
    setDone(true);
  }
  
  // set start of image, 1 image-set = 2 pages
  itsImage  = itsTBBE->image;
  itsSector = (itsImage * FL_SECTORS_IN_IMAGE);
  itsBlock  = (itsImage * FL_BLOCKS_IN_IMAGE); 
  
  // initialize TP send frame
  itsTPE->opcode      = TPWRITEF;
  itsTPE->status      = 0;
  
  itsStage = erase_flash;
}

// ----------------------------------------------------------------------------
void WritefCmd::sendTpEvent()
{
    switch (itsStage) {
      
      // stage 1, erase flash
      case erase_flash: {
        TPErasefEvent *erasefEvent = new TPErasefEvent();
        erasefEvent->opcode = TPERASEF;
        erasefEvent->status = 0;
        erasefEvent->addr = static_cast<uint32>(itsSector * FL_SECTOR_SIZE);
        TS->boardPort(getBoardNr()).send(*erasefEvent);
        TS->boardPort(getBoardNr()).setTimer(1.0); // erase time sector is 500 mSec
        delete erasefEvent;
      } break;
      
      // stage 2, write flash
      case write_flash: {
        // fill event with data and send
        itsTPE->addr = static_cast<uint32>(itsBlock * FL_BLOCK_SIZE);
        
        //int ptr = itsBlock - (itsImage * FL_BLOCKS_IN_IMAGE);
        for (int tp_an=0; tp_an < 256; tp_an++) {
          itsTPE->data[tp_an]  = itsImageData[itsDataPtr]; itsDataPtr++;
          itsTPE->data[tp_an] |= (itsImageData[itsDataPtr] << 8); itsDataPtr++; 
          itsTPE->data[tp_an] |= (itsImageData[itsDataPtr] << 16); itsDataPtr++; 
          itsTPE->data[tp_an] |= (itsImageData[itsDataPtr] << 24); itsDataPtr++;    
        }
        TS->boardPort(getBoardNr()).send(*itsTPE);
        TS->boardPort(getBoardNr()).setTimer(1.0);
      } break;
      
      // stage 3, verify flash
      case verify_flash: {
        TPReadfEvent *readfEvent = new TPReadfEvent();
        readfEvent->opcode  = TPREADF;
        readfEvent->status  = 0;
        readfEvent->addr = static_cast<uint32>(itsBlock * FL_BLOCK_SIZE);
        TS->boardPort(getBoardNr()).send(*readfEvent);
        TS->boardPort(getBoardNr()).setTimer(1.0);
        delete readfEvent;
      } break;
      
      case write_info: {
        // save Image info in last block
        itsBlock = (itsImage * FL_BLOCKS_IN_IMAGE) + (FL_BLOCKS_IN_IMAGE - 1);
        itsTPE->addr = static_cast<uint32>(itsBlock * FL_BLOCK_SIZE);
        for (int i = 0; i < 256; i++) {
          itsTPE->data[i]  = 0;
        }
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
        
        itsTPE->data[0] = static_cast<uint32>(itsTBBE->version);
        itsTPE->data[1] = static_cast<uint32>(write_time);
        memcpy(&itsTPE->data[2],info,sizeof(info)); 
        
        TS->boardPort(getBoardNr()).send(*itsTPE);
        TS->boardPort(getBoardNr()).setTimer(1.0);
        LOG_DEBUG_STR("Writing image info");
        LOG_DEBUG_STR(formatString("%u %u",itsTPE->data[0],itsTPE->data[1]));
      } break;
      
      case verify_info: {
        TPReadfEvent *readfEvent = new TPReadfEvent();
        readfEvent->opcode  = TPREADF;
        readfEvent->status  = 0;
        itsBlock = (itsImage * FL_BLOCKS_IN_IMAGE) + (FL_BLOCKS_IN_IMAGE - 1);
        readfEvent->addr = static_cast<uint32>(itsBlock * FL_BLOCK_SIZE);
        TS->boardPort(getBoardNr()).send(*readfEvent);
        TS->boardPort(getBoardNr()).setTimer(1.0);
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
  } else {
    
    switch (itsStage) {
      
      case erase_flash: {
        TPErasefAckEvent *erasefAckEvent = new TPErasefAckEvent(event);
        
        if (erasefAckEvent->status == 0) {
          setSleepTime(0.50);
          itsSector++;
          if (itsSector == ((itsImage + 1) * FL_SECTORS_IN_IMAGE)) {
            itsStage = write_flash;
          }   
        } else {
          itsTBBackE->status_mask |= TBB_FLASH_ERROR;
          LOG_DEBUG_STR("Received status > 0 (WritefCmd(erase_flash stage))");
          setDone(true);
        }
        delete erasefAckEvent;
      } break;
      
      case write_flash: {
        itsTPackE = new TPWritefAckEvent(event);
          
          if (itsTPackE->status == 0) {
            setSleepTime(0.002);
            itsStage = verify_flash;    
          } else {
            itsTBBackE->status_mask |= TBB_FLASH_ERROR;
            LOG_DEBUG_STR("Received status > 0 (WritefCmd(write_flash stage))");
            setDone(true);
          }
          delete itsTPackE;
      } break;
      
      case verify_flash: {
        // check if write-data is read-data
        bool same = true;
        
        TPReadfAckEvent *readfAckEvent = new TPReadfAckEvent(event);
        
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
            itsTBBackE->status_mask |= TBB_FLASH_ERROR;
            setDone(true);
          }
          
          int nextByte = ((itsBlock - (itsImage * FL_BLOCKS_IN_IMAGE)) * FL_BLOCK_SIZE); 
          if (nextByte > itsImageSize) {
            //setDone(true);
            itsStage = write_info;
          }
        } else {
          itsTBBackE->status_mask |= TBB_FLASH_ERROR;
          LOG_DEBUG_STR("Received status > 0 (WritefCmd(verify_flash stage))");
          setDone(true);
        }       
        delete readfAckEvent;
      } break;
      
      case write_info: {
        itsTPackE = new TPWritefAckEvent(event);
          
        if (itsTPackE->status == 0) {
          setSleepTime(0.002);
          itsStage = verify_info;   
        } else {
          itsTBBackE->status_mask |= TBB_FLASH_ERROR;
          LOG_DEBUG_STR(formatString("Received status > 0 (0x%08X) (WritefCmd(write_info stage))",itsTPackE->status));
          setDone(true);
        }
        delete itsTPackE;
      } break;
      
      case verify_info: {
      // check if write-data is read-data
        bool same = true;
        
        TPReadfAckEvent *readfAckEvent = new TPReadfAckEvent(event);
        
        if (readfAckEvent->status == 0) {
          for (int i = 0; i < 64; i++) {
            if (readfAckEvent->data[i] != itsTPE->data[i]) {
              LOG_DEBUG_STR(formatString("image info %d not same 0x%08X 0x%08X (WritefCmd(verify_info stage))",i,readfAckEvent->data[i],itsTPE->data[i]));
              same = false; 
            }
          }
          if (!same) {
            itsTBBackE->status_mask |= TBB_FLASH_ERROR; 
          }
        } else {
          itsTBBackE->status_mask |= TBB_FLASH_ERROR;
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
  
  if (clientport->isConnected()) { clientport->send(*itsTBBackE); }
}

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


uint8 WritefCmd::charToHex(int ch)
{
  if ((ch >= '0') && (ch <= '9')) {
    return (ch & 0x0F);
  } 
  
  if (((ch >= 'A') && (ch <= 'F')) || ((ch >= 'a') && (ch <= 'f'))) {
    return ((ch & 0x0F) + 9);
  }
  return (0);
}

