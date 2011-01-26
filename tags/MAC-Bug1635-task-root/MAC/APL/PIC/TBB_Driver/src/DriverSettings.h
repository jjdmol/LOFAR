//#  TbbSettings.h: Global station settings
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

#ifndef LOFAR_RSP_STATIONSETTINGS_H
#define LOFAR_RSP_STATIONSETTINGS_H

#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include "TP_Protocol.ph"
#include <GCF/TM/GCF_Control.h>
#include <Common/LofarTypes.h>
#include <time.h>



namespace LOFAR {
	using GCF::TM::GCFPortInterface;
	namespace TBB {

static const int DRIVER_VERSION = 232;

enum BoardStateT {noBoard,
				  setImage1, image1Set,
				  clearBoard, boardCleared,
				  enableWatchdog, watchdogEnabled,
				  enableArp, arpEnabled,
				  freeBoard, boardFreed,
				  boardReady,
				  boardError};

// info for all channels
struct ChannelInfo
{
	uint32 Status;
	char   State;
	int32  RcuNr;
	int32  BoardNr;
	int32  InputNr;
	int32  MpNr;
	uint32 StartAddr;
	uint32 PageSize;
	// settings for the trigger system
	bool   TriggerReleased;
	bool   Triggered;
	uint32 TriggerLevel;
	uint32  TriggerStartMode;
	uint32  TriggerStopMode;
	uint32  FilterSelect;
	uint32  DetectWindow;
	uint32  TriggerMode;
	uint32  OperatingMode;
	uint16 Filter[2][4];
	string dstIpCep;
	string dstMacCep;
};

struct BoardInfo
{
	GCFPortInterface* port;
	bool   used;
	BoardStateT boardState;
	time_t setupWaitTime;
	int32  setupRetries;
	bool   setupCmdDone;
	uint32 memorySize;
	uint32 imageNr;
	bool   freeToReset;
	string dstMac;
	string srcIpCep;
	string srcMacCep;
	string dstIpCep;  // base can be overuled by channel settings
	string dstMacCep;  // base can be overuled by channel settings
};

// forward declaration
class TBBDriver;


class TbbSettings
{
public:
	TbbSettings ();
	~TbbSettings();

	static TbbSettings* instance();
	
	int32 driverVersion();
	int32 maxBoards();
	int32 maxChannels();
	int32 nrMpsOnBoard();
	int32 nrChannelsOnMp();
	int32 nrChannelsOnBoard();
	int32 flashMaxImages();
	int32 flashSectorsInImage();
	int32 flashBlocksInImage();
	int32 flashImageSize();
	int32 flashSectorSize();
	int32 flashBlockSize();
	uint32 activeBoardsMask();
	int32 maxRetries();
	double timeout();
	
	GCFPortInterface& boardPort(int32 boardnr);
	int32 port2Board(GCFPortInterface* port);
		
	uint32 getChStatus(int32 channelnr);
	char getChState(int32 channelnr);
	int32 getChRcuNr(int32 channelnr);
	int32 getChBoardNr(int32 channelnr);
	int32 getChInputNr(int32 channelnr);
	int32 getChMpNr(int32 channelnr);
	int32 getFirstChannelNr(int32 board, int32 mp);
	uint32 getChStartAddr(int32 channelnr);
	uint32 getChPageSize(int32 channelnr);
	bool isChTriggerReleased(int32 channelnr);
	bool isChTriggered(int32 channelnr);
	uint32 getChTriggerLevel(int32 channelnr);
	uint32 getChTriggerStartMode(int32 channelnr);
	uint32 getChTriggerStopMode(int32 channelnr);
	uint32 getChFilterSelect(int32 channelnr);
	uint32 getChDetectWindow(int32 channelnr);
	uint32 getChTriggerMode(int32 channelnr);
	uint32 getChOperatingMode(int32 channelnr);
    uint32 getChFilterCoefficient(int32 channelnr, int32 filter, int32 coef_nr);
	
	string getIfName();
	string getDstMac(int32 boardnr);
	
	string getSrcIpCep(int32 boardnr);
	string getDstIpCep(int32 channelnr);
	string getSrcMacCep(int32 boardnr);
	string getDstMacCep(int32 channelnr);
	
	int32  saveTriggersToFile();
	bool   isRecording();
	
	BoardStateT getBoardState(int32 boardnr);
	void setBoardState(int32 boardnr, BoardStateT boardstate);
	
	int32 getSetupWaitTime(int32 boardnr);
	void setSetupWaitTime(int32 boardnr, int32 waittime);
	
	int32 getSetupRetries(int32 boardnr);
	void resetSetupRetries(int32 boardnr);
	void incSetupRetries(int32 boardnr);
	
	bool isSetupCmdDone(int32 boardnr);
	void setSetupCmdDone(int32 boardnr, bool state);
	
	bool boardSetupNeeded();
	void clearBoardSetup();
	void setActiveBoardsMask (uint32 activeboardsmask);
	void setActiveBoard (int32 boardnr);
	void resetActiveBoard (int32 boardnr);

	void setChStatus(int32 channelnr, uint32 status);
	void setChState(int32 channelnr, char state);
	void setChStartAddr(int32 channelnr, uint32 startaddr);
	void setChPageSize(int32 channelnr, uint32 pagesize);
	void setChTriggered(int32 channelnr, bool triggered);
	void setChTriggerReleased(int32 channelnr, bool released);
	void setChTriggerLevel(int32 channelnr, uint32 level);
	void setChTriggerStartMode(int32 channelnr, uint32 mode);
	void setChTriggerStopMode(int32 channelnr, uint32 mode);
	void setChFilterSelect(int32 channelnr, uint32 filter_select);
	void setChDetectWindow(int32 channelnr, uint32 detect_window);
	void setChTriggerMode(int32 channelnr, uint32 trigger_mode);
	void setChOperatingMode(int32 channelnr, uint32 operating_mode);
    void setChFilterCoefficient(int32 channelnr, int32 filter, int32 coef_nr, uint16 coef);
	
	void setSrcIpCep(int32 boardnr, string ip);
	void setDstIpCep(int32 channelnr, string ip);
	void setSrcMacCep(int32 boardnr, string mac);
	void setDstMacCep(int32 channelnr, string mac);
	void setDestination(int32 channelnr, char *storage);
	
	
	void clearRcuSettings(int32 boardnr);
	
	void convertRcu2BrdCh(int32 rcunr, int32 *boardnr, int32 *channelnr);
	void convertCh2Rcu(int32 channelnr, int32 *rcunr);
	int32 convertRcuToChan(int32 rcunr);
	int32 convertRcuToBoard(int32 rcunr);
	int32 convertChanToRcu(int32 channelnr);
	bool isBoardActive(int32 boardnr);
	bool isBoardReady(int32 boardnr);
	bool isBoardUsed(int32 boardnr);
	void resetBoardUsed();
	void logChannelInfo(int32 channel);
	
	void setSetupNeeded(bool state);
	bool isSetupNeeded();
	
	uint32 getMemorySize(int32 boardnr);
	void setMemorySize(int32 boardnr,uint32 pages);
	
	uint32 getImageNr(int32 boardnr);
	void setImageNr(int32 boardnr, uint32 image);
	bool getFreeToReset(int32 boardnr);
	void setFreeToReset(int32 boardnr, bool reset);
	
friend class TBBDriver;

protected:	// note TBBDriver must be able to set them
	void getTbbSettings();
	void setMaxBoards (int32 maxboards);
	void setMaxRetries(int32 retries);
	void setTimeOut(double timeout);
	void setBoardPorts(int board, GCFPortInterface* board_ports);
	
private:
	// Copying is not allowed
	TbbSettings(const TbbSettings&	that);
	TbbSettings& operator=(const TbbSettings& that);

	// --- Datamembers ---  
	int32  itsDriverVersion;
	int32  itsMaxBoards;	// constants
	int32  itsMaxChannels;
	int32  itsMpsOnBoard;
	int32  itsChannelsOnMp;
	int32  itsChannelsOnBoard;
	int32  itsFlashMaxImages;
	int32  itsFlashSectorsInImage;
	int32  itsFlashBlocksInImage;
	int32  itsFlashImageSize;
	int32  itsFlashSectorSize;
	int32  itsFlashBlockSize;
	int32  itsMaxRetries;
	double itsTimeOut;
	int32  itsSaveTriggersToFile;
	int32  itsRecording;
	
	// mask with active boards
	uint32 itsActiveBoardsMask;
	
	BoardInfo   *itsBoardInfo;
	ChannelInfo *itsChannelInfo;
	bool        itsBoardSetup;
	string      itsIfName;
	bool        itsSetupNeeded;
	
	static TbbSettings *theirTbbSettings;
};
	
//# --- inline functions ---
inline	int32 TbbSettings::driverVersion() { return (itsDriverVersion); }
inline	int32 TbbSettings::maxBoards() { return (itsMaxBoards); }
inline	int32 TbbSettings::maxChannels() { return (itsMaxChannels); }
inline	int32 TbbSettings::nrMpsOnBoard() { return (itsMpsOnBoard); }
inline	int32 TbbSettings::nrChannelsOnMp() { return (itsChannelsOnMp); }
inline	int32 TbbSettings::nrChannelsOnBoard() { return (itsChannelsOnBoard); }
inline	int32 TbbSettings::flashMaxImages() { return (itsFlashMaxImages); }
inline	int32 TbbSettings::flashSectorsInImage() { return (itsFlashSectorsInImage); }
inline	int32 TbbSettings::flashBlocksInImage() { return (itsFlashBlocksInImage); }
inline	int32 TbbSettings::flashImageSize() { return (itsFlashImageSize); }
inline	int32 TbbSettings::flashSectorSize() { return (itsFlashSectorSize); }
inline	int32 TbbSettings::flashBlockSize() { return (itsFlashBlockSize); }
inline	uint32 TbbSettings::activeBoardsMask()	{ return (itsActiveBoardsMask);   }
inline	int32 TbbSettings::maxRetries()	{ return (itsMaxRetries);   }
inline	double TbbSettings::timeout()	{ return (itsTimeOut);   }
inline	GCFPortInterface& TbbSettings::boardPort(int32 boardnr)	{ 
            itsBoardInfo[boardnr].used = true;
            return (*itsBoardInfo[boardnr].port);
        }

inline	BoardStateT TbbSettings::getBoardState(int32 boardnr) { return (itsBoardInfo[boardnr].boardState); }
inline  bool TbbSettings::boardSetupNeeded() { return (itsBoardSetup); }
inline  void TbbSettings::clearBoardSetup() { itsBoardSetup = false; }

inline  int32 TbbSettings::getSetupWaitTime(int32 boardnr) { 
			if (time(NULL) >= itsBoardInfo[boardnr].setupWaitTime) { return(0); }
			return(static_cast<int32>(itsBoardInfo[boardnr].setupWaitTime - time(NULL)));
		}
inline  void TbbSettings::setSetupWaitTime(int32 boardnr, int32 waittime) { 
            itsBoardInfo[boardnr].setupWaitTime = time(NULL) + waittime;
        }
	
inline  int32 TbbSettings::getSetupRetries(int32 boardnr) { return(itsBoardInfo[boardnr].setupRetries); }
inline  void TbbSettings::resetSetupRetries(int32 boardnr) { itsBoardInfo[boardnr].setupRetries = 0; }
inline  void TbbSettings::incSetupRetries(int32 boardnr) { ++itsBoardInfo[boardnr].setupRetries; }

inline  bool TbbSettings::isSetupCmdDone(int32 boardnr) { 
            if (boardnr == -1) {
                for (int bnr = 0; bnr < itsMaxBoards; bnr++) {
                    if ( itsBoardInfo[bnr].setupCmdDone == false ) { return(false); }
                }
                return(true);
            }
            else {
                return(itsBoardInfo[boardnr].setupCmdDone);
            }
        }
inline  void TbbSettings::setSetupCmdDone(int32 boardnr, bool state) { itsBoardInfo[boardnr].setupCmdDone = state; }

//---- inline functions for channel information ------------
inline	uint32 TbbSettings::getChStatus(int32 channelnr) { return (itsChannelInfo[channelnr].Status); }
inline	char TbbSettings::getChState(int32 channelnr) { return (itsChannelInfo[channelnr].State); }
inline	int32 TbbSettings::getChRcuNr(int32 channelnr) { return (itsChannelInfo[channelnr].RcuNr); }
inline	int32 TbbSettings::getChBoardNr(int32 channelnr) { return (itsChannelInfo[channelnr].BoardNr); }
inline	int32 TbbSettings::getChInputNr(int32 channelnr) { return (itsChannelInfo[channelnr].InputNr); }
inline	int32 TbbSettings::getChMpNr(int32 channelnr) { return (itsChannelInfo[channelnr].MpNr); }
inline	uint32 TbbSettings::getChStartAddr(int32 channelnr) { return (itsChannelInfo[channelnr].StartAddr); }
inline	uint32 TbbSettings::getChPageSize(int32 channelnr) { return (itsChannelInfo[channelnr].PageSize); }
inline	bool TbbSettings::isChTriggered(int32 channelnr) { return (itsChannelInfo[channelnr].Triggered); }	
inline	bool TbbSettings::isChTriggerReleased(int32 channelnr) { return (itsChannelInfo[channelnr].TriggerReleased); }
inline	uint32 TbbSettings::getChTriggerLevel(int32 channelnr) { return (itsChannelInfo[channelnr].TriggerLevel); }
inline	uint32 TbbSettings::getChTriggerStartMode(int32 channelnr) { return (itsChannelInfo[channelnr].TriggerStartMode); }
inline	uint32 TbbSettings::getChTriggerStopMode(int32 channelnr) { return (itsChannelInfo[channelnr].TriggerStopMode); }
inline	uint32 TbbSettings::getChFilterSelect(int32 channelnr) { return (itsChannelInfo[channelnr].FilterSelect); }
inline	uint32 TbbSettings::getChDetectWindow(int32 channelnr) { return (itsChannelInfo[channelnr].DetectWindow); }
inline	uint32 TbbSettings::getChTriggerMode(int32 channelnr) { return (itsChannelInfo[channelnr].TriggerMode); }
inline	uint32 TbbSettings::getChOperatingMode(int32 channelnr) { return (itsChannelInfo[channelnr].OperatingMode); }
inline	uint32 TbbSettings::getChFilterCoefficient(int32 channelnr, int32 filter, int32 coef_nr) { return (itsChannelInfo[channelnr].Filter[filter][coef_nr]); }
inline	string TbbSettings::getIfName() { return(itsIfName); }
inline	string TbbSettings::getDstMac(int32 boardnr) { return(itsBoardInfo[boardnr].dstMac); }
inline	string TbbSettings::getSrcIpCep(int32 boardnr) { return(itsBoardInfo[boardnr].srcIpCep); }
inline	string TbbSettings::getDstIpCep(int32 channelnr) { 
            if (itsChannelInfo[channelnr].dstIpCep.empty()) { return(itsBoardInfo[getChBoardNr(channelnr)].dstIpCep); }
            else { return(itsChannelInfo[channelnr].dstIpCep); }
        }
inline	string TbbSettings::getSrcMacCep(int32 boardnr) { return(itsBoardInfo[boardnr].srcMacCep); }
inline	string TbbSettings::getDstMacCep(int32 channelnr) {
            if (itsChannelInfo[channelnr].dstMacCep.empty()) { return(itsBoardInfo[getChBoardNr(channelnr)].dstMacCep); }
            else { return(itsChannelInfo[channelnr].dstMacCep); }
        }
inline  int32 TbbSettings::saveTriggersToFile() { return(itsSaveTriggersToFile); }
inline  bool TbbSettings::isRecording() { return(static_cast<bool>(itsRecording)); }

inline	void TbbSettings::setChStatus(int32 channelnr, uint32 status){ itsChannelInfo[channelnr].Status = status; }
inline	void TbbSettings::setChState(int32 channelnr, char state){
	 		itsChannelInfo[channelnr].State = state;
	 		if (state == 'R') { itsRecording |= (1 << itsChannelInfo[channelnr].BoardNr); }
	 		else { itsRecording &= ~(1 << itsChannelInfo[channelnr].BoardNr); }
	 	}
inline	void TbbSettings::setChStartAddr(int32 channelnr, uint32 startaddr){ itsChannelInfo[channelnr].StartAddr = startaddr; }
inline	void TbbSettings::setChPageSize(int32 channelnr, uint32 pagesize){ itsChannelInfo[channelnr].PageSize = pagesize; }
inline	void TbbSettings::setChTriggered(int32 channelnr, bool triggered){ itsChannelInfo[channelnr].Triggered = triggered; }
inline	void TbbSettings::setChTriggerReleased(int32 channelnr, bool release){ itsChannelInfo[channelnr].TriggerReleased = release; }
inline	void TbbSettings::setChTriggerLevel(int32 channelnr, uint32 level){ itsChannelInfo[channelnr].TriggerLevel = level; }
inline	void TbbSettings::setChTriggerStartMode(int32 channelnr, uint32 mode){ itsChannelInfo[channelnr].TriggerStartMode = mode; }
inline	void TbbSettings::setChTriggerStopMode(int32 channelnr, uint32 mode){ itsChannelInfo[channelnr].TriggerStopMode = mode; }
inline	void TbbSettings::setChFilterSelect(int32 channelnr, uint32 select){ itsChannelInfo[channelnr].FilterSelect = select; }
inline	void TbbSettings::setChDetectWindow(int32 channelnr, uint32 window){ itsChannelInfo[channelnr].DetectWindow = window; }
inline	void TbbSettings::setChTriggerMode(int32 channelnr, uint32 trigger_mode){ itsChannelInfo[channelnr].TriggerMode = trigger_mode; }
inline	void TbbSettings::setChOperatingMode(int32 channelnr, uint32 operating_mode){ itsChannelInfo[channelnr].OperatingMode = operating_mode; }
inline	void TbbSettings::setChFilterCoefficient(int32 channelnr, int32 filter, int32 coef_nr, uint16 coef){ itsChannelInfo[channelnr].Filter[filter][coef_nr] = coef; }

inline	void TbbSettings::setSrcIpCep(int32 boardnr, string ip) { itsBoardInfo[boardnr].srcIpCep = ip; }
inline	void TbbSettings::setDstIpCep(int32 channelnr, string ip) { itsChannelInfo[channelnr].dstIpCep = ip; }
inline	void TbbSettings::setSrcMacCep(int32 boardnr, string mac) { itsBoardInfo[boardnr].srcMacCep = mac; }
inline	void TbbSettings::setDstMacCep(int32 channelnr, string mac) { itsChannelInfo[channelnr].dstMacCep = mac; }
//---- inline functions for board information ------------
inline	uint32 TbbSettings::getMemorySize(int32 boardnr) { return (itsBoardInfo[boardnr].memorySize); }
inline	void TbbSettings::setMemorySize(int32 boardnr,uint32 pages) { itsBoardInfo[boardnr].memorySize = pages; }
inline	uint32 TbbSettings::getImageNr(int32 boardnr) { return (itsBoardInfo[boardnr].imageNr); }
inline	void TbbSettings::setImageNr(int32 boardnr,uint32 image) { itsBoardInfo[boardnr].imageNr = image; }
inline	bool TbbSettings::getFreeToReset(int32 boardnr) { return (itsBoardInfo[boardnr].freeToReset); }
inline	void TbbSettings::setFreeToReset(int32 boardnr, bool reset) { itsBoardInfo[boardnr].freeToReset = reset; }
inline	bool TbbSettings::isBoardReady(int32 boardnr) { return(itsBoardInfo[boardnr].boardState == boardReady); }
inline  bool TbbSettings::isBoardUsed(int32 boardnr) { return(itsBoardInfo[boardnr].used); }
inline  void TbbSettings::resetBoardUsed() {
            for (int boardnr = 0; boardnr < itsMaxBoards; boardnr++) {
                itsBoardInfo[boardnr].used = false;
            }
        }
inline  void TbbSettings::setSetupNeeded(bool state) { itsSetupNeeded = state; }
inline  bool TbbSettings::isSetupNeeded() { return(itsSetupNeeded); }
	 
	} // namespace TBB
} // namespace LOFAR

#endif
