//#  TbbSettings.cc: Driver settings
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/ParameterSet.h>
#include <Common/Exceptions.h>
#include <MACIO/MACServiceInfo.h>
#include <DriverSettings.h>

using namespace LOFAR;
	//using namespace GCFCommon;
using namespace TBB;


//
// Initialize singleton
//
TbbSettings* TbbSettings::theirTbbSettings = 0;

TbbSettings* TbbSettings::instance()
{
	if (theirTbbSettings == 0) {
		theirTbbSettings = new TbbSettings();
	}
	return(theirTbbSettings);
}

//
// Default constructor
//
TbbSettings::TbbSettings() :
	itsDriverVersion(DRIVER_VERSION),				// set version of TBBDriver.cc
	itsMaxBoards(0),							// max.number of boards on 1 driver 
	itsMaxChannels(0),						// max.number of channels on 1 driver
	itsMpsOnBoard(4),							// number of MPs on 1 board
	itsChannelsOnMp(4),						// number of channels on 1 MP
	itsChannelsOnBoard(16),				// number of channels on 1 board
	itsFlashMaxPages(32),					// max.number of pages in flash on 1 board
	itsFlashSectorsInPage(16),		// number of sectors in 1 page
	itsFlashBlocksInPage(2048),		// number of blocks in 1 page
	itsFlashPageSize(2097152),		// size of 1 page in bytes
	itsFlashPageSectorSize(131072),// size of 1 sector in bytes
	itsFlashPageBlockSize(1024),	// size of 1 block in bytes
	itsMaxRetries(5),							// max.number of retries for each command
	itsTimeOut(0.2),							// response timeout
	itsSaveTriggersToFile(0),			// save trigger info to a file
	itsActiveBoardsMask(0),				// mask with active boards
	itsRcu2ChTable(0),						// conversion table Rcu-number to Channnel-number
	itsCh2RcuTable(0),						// conversion table Channel-number to Rcu-number
	itsBoardInfo(0),
	itsChannelInfo(0) 								// Struct with channel info
{
	itsRcu2ChTable = new int32[itsChannelsOnBoard];
	itsCh2RcuTable = new int32[itsChannelsOnBoard];
}

TbbSettings::~TbbSettings()
{
	if (itsRcu2ChTable) delete itsRcu2ChTable;
	if (itsCh2RcuTable) delete itsCh2RcuTable;
	if (itsBoardInfo) delete itsBoardInfo;
	if (itsChannelInfo) delete itsChannelInfo;
	if (theirTbbSettings) delete theirTbbSettings;	
}

//---- get Tbb settings loaded from config file ---
void TbbSettings::getTbbSettings()
{ 
  bool configOK = true;
  
  // the conversion table must be set 1e
  char chname[64];
  for (int32 channelnr = 0; channelnr < itsChannelsOnBoard; channelnr++) {
  	snprintf(chname, 64, "TBBDriver.TBB_CH_%d", channelnr);	
  	try { setConversionTable(globalParameterSet()->getInt32(chname), channelnr); }
  	catch (...) { LOG_INFO_STR(formatString("%s not found",chname)); configOK = false; } 
  }
  
  // setMaxBoards() must be set 2e
  setMaxBoards(MAX_N_TBBOARDS);
  
  try { itsSaveTriggersToFile = globalParameterSet()->getInt32("TBBDriver.SAVE_TRIGGERS_TO_FILE"); }
  catch (...) { LOG_INFO_STR(formatString("TBBDriver.SAVE_TRIGGERS_TO_FILE not found")); }
  
  try { itsTimeOut = globalParameterSet()->getDouble("TBBDriver.TP_TIMEOUT"); }
  catch (...) { LOG_INFO_STR(formatString("TBBDriver.TP_TIMEOUT not found")); configOK = false;}
  
  try { itsMaxRetries = globalParameterSet()->getInt32("TBBDriver.TP_RETRIES"); }
  catch (...) { LOG_INFO_STR(formatString("TBBDriver.TP_RETRIES not found")); configOK = false; }
    
  try { itsIfName = globalParameterSet()->getString("TBBDriver.IF_NAME"); }
  catch (...) { LOG_INFO_STR(formatString("TBBDriver.IF_NAME not found")); configOK = false; }
    
  char dstip[64];
  char srcip[64];
  char srcmac[64];
  char dstmac[64];
  for (int boardnr = 0; boardnr < itsMaxBoards; boardnr++) {
  	snprintf(srcip,  64, "TBBDriver.SRC_IP_ADDR_%d", boardnr);
  	snprintf(dstip,  64, "TBBDriver.DST_IP_ADDR_%d", boardnr);
		snprintf(srcmac, 64, "TBBDriver.MAC_ADDR_%d", boardnr);
		snprintf(dstmac, 64, "TBBDriver.DST_MAC_ADDR_%d", boardnr);
		
		try { itsBoardInfo[boardnr].srcIp = globalParameterSet()->getString(srcip); }
  	catch (APSException&) { LOG_INFO_STR(formatString("%s not found",srcip)); configOK = false; }
		
		try { itsBoardInfo[boardnr].dstIp = globalParameterSet()->getString(dstip); }
  	catch (APSException&) {	LOG_INFO_STR(formatString("%s not found",dstip)); configOK = false; }
		
		try { itsBoardInfo[boardnr].srcMac = globalParameterSet()->getString(srcmac); }
  	catch (APSException&) { LOG_INFO_STR(formatString("%s not found",srcmac)); }
		
		try { itsBoardInfo[boardnr].dstMac = globalParameterSet()->getString(dstmac); }
  	catch (APSException&) { LOG_INFO_STR(formatString("%s not found",dstmac)); }
		
		LOG_INFO_STR(formatString("Board %d:",boardnr));
		LOG_INFO_STR(formatString("Control port: Mac = '%s'"
															,itsBoardInfo[boardnr].srcMac.c_str()));
		LOG_INFO_STR(formatString("CEP port    : Src Ip = '%s'"
															,itsBoardInfo[boardnr].srcIp.c_str()));
		LOG_INFO_STR(formatString("            : Dst Ip = '%s', Dst Mac = '%s'"
															,itsBoardInfo[boardnr].dstIp.c_str()
															,itsBoardInfo[boardnr].dstMac.c_str()));		
  }
}
  
  

//---- setBoardPorts ------------------------------
void TbbSettings::setBoardPorts(int board, GCFPortInterface* board_ports)
{
	itsBoardInfo[board].port = board_ports;
}

//---- getBoardNr ------------------------------
int32 TbbSettings::port2Board(GCFPortInterface* port)
{
	for (int32 board = 0; board < itsMaxBoards; board++) {
		if (itsBoardInfo[board].port == port) {
			return(board);
		}
	}
	return(-1); // not a board port	
} 



//---- setMaxBoards ------------------------------
void TbbSettings::setMaxBoards (int32 maxboards)
{
	itsMaxBoards = maxboards;
	itsMaxChannels = itsChannelsOnBoard * maxboards;
	
	if (itsChannelInfo) delete itsChannelInfo;
	itsChannelInfo = new ChannelInfo[itsMaxChannels];
				
	int32 boardnr = 0;
	int32 inputnr = 0;
	int32 mpnr = 0;
	
	for (int32 ch = 0; ch < itsMaxChannels; ch++) {
		itsChannelInfo[ch].Selected = false;
		itsChannelInfo[ch].Status = 0;
		itsChannelInfo[ch].State = 'F';
		convertCh2Rcu(ch,&itsChannelInfo[ch].RcuNr);
		itsChannelInfo[ch].BoardNr = boardnr;
		itsChannelInfo[ch].InputNr = inputnr;
		itsChannelInfo[ch].MpNr = mpnr;
		itsChannelInfo[ch].StartAddr = 0;
		itsChannelInfo[ch].PageSize = 0;
		inputnr++;
		if (inputnr == itsChannelsOnBoard) {
			inputnr = 0;
			boardnr++;
		}
		mpnr = (int32)(inputnr / 4);
		
		// initialize filter settings
		itsChannelInfo[ch].TriggerReleased = false;
		itsChannelInfo[ch].Triggered = false;
		itsChannelInfo[ch].TriggerLevel = 0;
		itsChannelInfo[ch].TriggerStartMode = 0;
		itsChannelInfo[ch].TriggerStopMode = 0;
		itsChannelInfo[ch].FilterSelect = 0;
		itsChannelInfo[ch].DetectWindow = 0;
		itsChannelInfo[ch].OperatingMode = 0;
		for (int i = 0; i < 4; i++) {
			itsChannelInfo[ch].Coefficient[i] = 0;
		}
	}
	
	itsBoardSetup  = false;
	itsTriggerMode = 0;	
		
	if (itsBoardInfo) delete itsBoardInfo;
	itsBoardInfo = new BoardInfo[itsMaxBoards];
	
	for (int nr = 0;nr < itsMaxBoards; nr++) {
		itsBoardInfo[nr].boardState = setImage1;
		itsBoardInfo[nr].memorySize = 0;
		itsBoardInfo[nr].imageNr = 0;
	   itsBoardInfo[nr].freeToReset = true;
      itsBoardInfo[nr].srcIp = "";
		itsBoardInfo[nr].dstIp = "";
		itsBoardInfo[nr].srcMac = "";
		itsBoardInfo[nr].dstMac = "";
	}
}

void TbbSettings::setBoardState(int32 boardnr, BoardStateT boardstate)
{
	itsBoardInfo[boardnr].boardState = boardstate; 
	if ((boardstate > noBoard) && (boardstate < boardReady)) {
		itsBoardSetup = true;
	}
}

//---- setActiveBoardsMask -----------------------
void TbbSettings::setActiveBoardsMask (uint32 activeboardsmask)
{
	// clear rcu setting for boards not active anymore
	uint32 mask;
	mask = (~activeboardsmask) & itsActiveBoardsMask;
	for (int bn = 0; bn < itsMaxBoards; bn++) {
		if (mask & (1 << bn)) {
			clearRcuSettings(bn);	
		}
	}
	itsActiveBoardsMask = activeboardsmask;
}

//---- setActiveBoard ---------------------------
void TbbSettings::setActiveBoard (int32 boardnr)
{
	itsActiveBoardsMask |= (1 << boardnr);
}

//---- resetActiveBoards ---------------------------
void TbbSettings::resetActiveBoard (int32 boardnr)
{
	clearRcuSettings(boardnr);
	itsActiveBoardsMask &= ~(1 << boardnr);
}
	
//---- set Communication retries ----------------
void TbbSettings::setMaxRetries(int32 retries)
{
	itsMaxRetries = retries;
}

//---- set Communication time-out ----------------
void TbbSettings::setTimeOut(double timeout)
{
	itsTimeOut = timeout;
}

//---- set RCU2CH conversion table ----------------
void TbbSettings::setConversionTable(int32 rcu, int32 channel)
{
	itsRcu2ChTable[rcu] = channel;
	itsCh2RcuTable[channel] = rcu;
	//LOG_INFO_STR(formatString("table rcu.%d = chan.%d",rcu, channel));
}

void TbbSettings::convertRcu2Ch(int32 rcunr, int32 *boardnr, int32 *channelnr)
{
	int32 board;		// board 0 .. 11
	int32 channel;	// channel 0 .. 15
	
	board = (int32)(rcunr / itsChannelsOnBoard);
	channel = itsRcu2ChTable[rcunr - (board * itsChannelsOnBoard)];
	*boardnr = board;
	*channelnr = channel;	
}

void TbbSettings::convertCh2Rcu(int32 channelnr, int32 *rcunr)
{
	int32 boardnr;
	int32 rcu;
	
	boardnr = (int32)(channelnr / itsChannelsOnBoard);
	rcu = itsCh2RcuTable[(channelnr - (boardnr * itsChannelsOnBoard))] + (boardnr * itsChannelsOnBoard);
	*rcunr = rcu;
}

bool TbbSettings::isBoardActive(int32 boardnr)
{
	if (itsActiveBoardsMask & (1 << boardnr)) return (true);
	return (false);
}

bool TbbSettings::isBoardSelected(int32 boardnr)
{
	bool active = false;
	
	for (int cn = 0; cn < itsChannelsOnBoard; cn++) {
		if (itsChannelInfo[(boardnr * itsChannelsOnBoard) + cn].Selected) active = true;		
	}
	return (active);
}


void TbbSettings::clearRcuSettings(int32 boardnr)
{
	for (int cn = 0; cn < itsChannelsOnBoard; cn++) {
		itsChannelInfo[(boardnr * 16) + cn].Selected = false;
		itsChannelInfo[(boardnr * 16) + cn].Status = 0;
		itsChannelInfo[(boardnr * 16) + cn].State = 'F';
		itsChannelInfo[(boardnr * 16) + cn].StartAddr = 0;
		itsChannelInfo[(boardnr * 16) + cn].PageSize = 0;	
	}		
}

void TbbSettings::logChannelInfo(int32 channel)
{
		LOG_DEBUG_STR(formatString("Channel %d ,Rcu %d = status[0x%04X] state[%c] addr[%u] pages[%u]"
					, channel
					, itsChannelInfo[channel].RcuNr
					, itsChannelInfo[channel].Status
					, itsChannelInfo[channel].State
					, itsChannelInfo[channel].StartAddr
					, itsChannelInfo[channel].PageSize));
}
