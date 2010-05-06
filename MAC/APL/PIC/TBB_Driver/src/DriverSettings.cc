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
#include <cstdio>

using namespace LOFAR;
	//using namespace GCFCommon;
using namespace TBB;

// rcu to channel conversion, rcu-0 is on channel-2
static const int RCU_TO_CH_TABLE[16] = {2, 3, 6, 7, 10, 11, 14, 15, 0, 1, 4, 5, 8, 9, 12, 13};

// channel to rcu conversion, ch-0 is rcu-8
static const int CH_TO_RCU_TABLE[16] = {8, 9, 0, 1, 10, 11, 2, 3, 12, 13, 4, 5, 14, 15, 6, 7};

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
	itsDriverVersion(DRIVER_VERSION),  // set version of TBBDriver.cc
	itsMaxBoards(0),                   // max.number of boards on 1 driver 
	itsMaxChannels(0),                 // max.number of channels on 1 driver
	itsMpsOnBoard(4),                  // number of MPs on 1 board
	itsChannelsOnMp(4),                // number of channels on 1 MP
	itsChannelsOnBoard(16),            // number of channels on 1 board
	// Flash memory size is 64 MB, divided in 512 sectors or 65536 blocks
	itsFlashMaxImages(16),             // max.number of images in flash
	itsFlashSectorsInImage(32),        // number of sectors in 1 image (512 / 16)
	itsFlashBlocksInImage(4096),       // number of blocks in 1 image (65536 / 16)
	itsFlashImageSize(4194304),        // size of 1 image in bytes ((64x1024x1024) / 16)
	itsFlashSectorSize(131072),        // size of 1 sector in bytes ((64x1024x1024) / 512)
	itsFlashBlockSize(1024),           // size of 1 block in bytes ((64x1024x1024) / 65536) 
	
	itsMaxRetries(5),                  // max.number of retries for each command
	itsTimeOut(0.2),                   // response timeout
	itsSaveTriggersToFile(0),          // save trigger info to a file
	itsRecording(0),                   // if > 0 then recording is active
	itsActiveBoardsMask(0),            // mask with active boards
	itsBoardInfo(0),
	itsChannelInfo(0)                  // Struct with channel info
{
}

TbbSettings::~TbbSettings()
{
	if (itsBoardInfo) delete itsBoardInfo;
	if (itsChannelInfo) delete itsChannelInfo;
	if (theirTbbSettings) delete theirTbbSettings;	
}

//---- get Tbb settings loaded from config file ---
void TbbSettings::getTbbSettings()
{ 
	bool configOK = true;
	
	int32 n_tbboards = MAX_N_TBBOARDS;
	try { n_tbboards = globalParameterSet()->getInt32("RS.N_TBBOARDS"); }
	catch (...) { LOG_INFO_STR(formatString("RS.N_TBBOARDS not found")); }
	LOG_INFO_STR(formatString("RS.N_TBBOARDS=%d", n_tbboards));
	// setMaxBoards() must be set 2e
	setMaxBoards(n_tbboards);
	//setMaxBoards(MAX_N_TBBOARDS);
	
	try { itsSaveTriggersToFile = globalParameterSet()->getInt32("TBBDriver.SAVE_TRIGGERS_TO_FILE"); }
	catch (...) { LOG_INFO_STR(formatString("TBBDriver.SAVE_TRIGGERS_TO_FILE not found")); }
	
	try { itsTimeOut = globalParameterSet()->getDouble("TBBDriver.TP_TIMEOUT"); }
	catch (...) { LOG_INFO_STR(formatString("TBBDriver.TP_TIMEOUT not found")); configOK = false;}
	
	try { itsMaxRetries = globalParameterSet()->getInt32("TBBDriver.TP_RETRIES"); }
	catch (...) { LOG_INFO_STR(formatString("TBBDriver.TP_RETRIES not found")); configOK = false; }
		
	try { itsIfName = globalParameterSet()->getString("TBBDriver.IF_NAME"); }
	catch (...) { LOG_INFO_STR(formatString("TBBDriver.IF_NAME not found")); configOK = false; }
		
	char dstmac[64];
	char dstipcep[64];
	char srcipcep[64];
	char srcmaccep[64];
	char dstmaccep[64];
	
	for (int boardnr = 0; boardnr < itsMaxBoards; boardnr++) {
		snprintf(dstmac, 64, "TBBDriver.MAC_ADDR_%d", boardnr);
		
		snprintf(srcipcep,  64, "TBBDriver.SRC_IP_ADDR_%d", boardnr);
		snprintf(dstipcep,  64, "TBBDriver.DST_IP_ADDR_%d", boardnr);
		snprintf(srcmaccep, 64, "TBBDriver.SRC_MAC_ADDR_%d", boardnr);
		snprintf(dstmaccep, 64, "TBBDriver.DST_MAC_ADDR_%d", boardnr);
		
		try { itsBoardInfo[boardnr].dstMac = globalParameterSet()->getString(dstmac); }
		catch (APSException&) { LOG_INFO_STR(formatString("%s not found",dstmac)); configOK = false;}
		
		try { itsBoardInfo[boardnr].srcIpCep = globalParameterSet()->getString(srcipcep); }
		catch (APSException&) { LOG_INFO_STR(formatString("%s not found",srcipcep)); configOK = false; }
		
		try { itsBoardInfo[boardnr].dstIpCep = globalParameterSet()->getString(dstipcep); }
		catch (APSException&) {	LOG_INFO_STR(formatString("%s not found",dstipcep)); configOK = false; }
		
		try { itsBoardInfo[boardnr].srcMacCep = globalParameterSet()->getString(srcmaccep); }
		catch (APSException&) { LOG_INFO_STR(formatString("%s not found",dstmac)); configOK = false;}
		
		try { itsBoardInfo[boardnr].dstMacCep = globalParameterSet()->getString(dstmaccep); }
		catch (APSException&) { LOG_INFO_STR(formatString("%s not found",dstmac)); configOK = false;}
		
		LOG_INFO_STR(formatString("Board %d:",boardnr));
		LOG_INFO_STR(formatString("Control port: Mac = '%s'"
															,itsBoardInfo[boardnr].dstMac.c_str()));
		LOG_INFO_STR(formatString("CEP port    : Src Ip = '%s', Src Mac = '%s'"
															,itsBoardInfo[boardnr].srcIpCep.c_str()
															,itsBoardInfo[boardnr].srcMacCep.c_str()));
		LOG_INFO_STR(formatString("            : Dst Ip = '%s', Dst Mac = '%s'"
															,itsBoardInfo[boardnr].dstIpCep.c_str()
															,itsBoardInfo[boardnr].dstMacCep.c_str()));
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
		itsChannelInfo[ch].TriggerLevel = 2047;
		itsChannelInfo[ch].TriggerStartMode = 0;
		itsChannelInfo[ch].TriggerStopMode = 0;
		itsChannelInfo[ch].FilterSelect = 0;
		itsChannelInfo[ch].DetectWindow = 0;
		itsChannelInfo[ch].TriggerMode = 0;
		itsChannelInfo[ch].OperatingMode = 0;
                for (int f = 0; f < 2; f++) {
                    for (int c = 0; c < 4; c++) {
			itsChannelInfo[ch].Filter[f][c] = 0;
                    }
		}
	}
	
	itsBoardSetup  = false;
		
	if (itsBoardInfo) delete itsBoardInfo;
	itsBoardInfo = new BoardInfo[itsMaxBoards];
	
	for (int nr = 0;nr < itsMaxBoards; nr++) {
		itsBoardInfo[nr].boardState = noBoard;
		itsBoardInfo[nr].setupWaitTime = 0;
		itsBoardInfo[nr].setupRetries = 0;
		itsBoardInfo[nr].setupCmdDone = true;
		itsBoardInfo[nr].memorySize = 0;
		itsBoardInfo[nr].imageNr = 0;
		itsBoardInfo[nr].freeToReset = true;
		itsBoardInfo[nr].dstMac = "";
		itsBoardInfo[nr].srcIpCep = "";
		itsBoardInfo[nr].dstIpCep = "";
		itsBoardInfo[nr].srcMacCep = "";
		itsBoardInfo[nr].dstMacCep = "";
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

void TbbSettings::convertRcu2Ch(int32 rcunr, int32 *boardnr, int32 *channelnr)
{
	int32 board;	// board 0 .. 11
	int32 channel;	// channel 0 .. 15
	
	board = (int32)(rcunr / itsChannelsOnBoard);
	channel = RCU_TO_CH_TABLE[rcunr % itsChannelsOnBoard];
	*boardnr = board;
	*channelnr = channel;
}

void TbbSettings::convertCh2Rcu(int32 channelnr, int32 *rcunr)
{
	int32 boardnr;
	int32 rcu;
	
	boardnr = (int32)(channelnr / itsChannelsOnBoard);
	rcu = CH_TO_RCU_TABLE[channelnr % itsChannelsOnBoard] + (boardnr * itsChannelsOnBoard);
	*rcunr = rcu;
}

bool TbbSettings::isBoardActive(int32 boardnr)
{
	if (itsActiveBoardsMask & (1 << boardnr)) return (true);
	return (false);
}

int32 TbbSettings::getFirstChannelNr(int32 board, int32 mp)
{
	return((board * itsChannelsOnBoard) + (mp * itsChannelsOnMp));
}


void TbbSettings::clearRcuSettings(int32 boardnr)
{
	for (int cn = 0; cn < itsChannelsOnBoard; cn++) {
		itsChannelInfo[(boardnr * 16) + cn].Status = 0;
		itsChannelInfo[(boardnr * 16) + cn].State = 'F';
		itsChannelInfo[(boardnr * 16) + cn].StartAddr = 0;
		itsChannelInfo[(boardnr * 16) + cn].PageSize = 0;
		
		itsChannelInfo[(boardnr * 16) + cn].TriggerReleased = false;
		itsChannelInfo[(boardnr * 16) + cn].Triggered = false;
		itsChannelInfo[(boardnr * 16) + cn].TriggerLevel = 2047;
		itsChannelInfo[(boardnr * 16) + cn].TriggerStartMode = 0;
		itsChannelInfo[(boardnr * 16) + cn].TriggerStopMode = 0;
		itsChannelInfo[(boardnr * 16) + cn].FilterSelect = 0;
		itsChannelInfo[(boardnr * 16) + cn].DetectWindow = 0;
		itsChannelInfo[(boardnr * 16) + cn].TriggerMode = 0;
		itsChannelInfo[(boardnr * 16) + cn].OperatingMode = 0;
                for (int f = 0; f < 2; f++) {
                    for (int c = 0; c < 4; c++) {
                        itsChannelInfo[cn].Filter[f][c] = 0;
                    }
                }
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
