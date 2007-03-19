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
//#include <GCF/TM/GCF_ETHRawPort.h>
//#include <GCF/TM/GCF_DevicePort.h>
#include <Common/LofarTypes.h>



namespace LOFAR {
  namespace TBB {

// info for all channels
struct ChannelInfo
{
	bool		Selected;
	uint32	Status;
	char		State;
	int32		RcuNr;
	int32		BoardNr;
	int32		InputNr;
	int32		MpNr;
	uint32	StartAddr;
	uint32	PageSize;
};

struct BoardInfo
{
	GCFPortInterface* port;
	uint32	memorySize;
	string	srcIp;
	string	dstIp;
	string	srcMac;
	string	dstMac; 	
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
	int32 flashMaxPages();
	int32 flashSectorsInPage();
	int32 flashBlocksInPage();
	int32 flashPageSize();
	int32 flashPageSectorSize();
	int32 flashPageBlockSize();
	uint32 activeBoardsMask();
	int32 maxRetries();
	double timeout();
	GCFPortInterface& boardPort(int32 boardnr);

	uint32 getChStatus(int32 channelnr);
	char getChState(int32 channelnr);
	bool isChSelected(int32 channelnr);
	int32 getChRcuNr(int32 channelnr);
	int32 getChBoardNr(int32 channelnr);
	int32 getChInputNr(int32 channelnr);
	int32 getChMpNr(int32 channelnr);
	uint32 getChStartAddr(int32 channelnr);
	uint32 getChPageSize(int32 channelnr);
	
	string getIfName();
	string getSrcIp(int32 boardnr);
	string getDstIp(int32 boardnr);
	string getSrcMac(int32 boardnr);
	string getDstMac(int32 boardnr);

	void setChSelected(int32 channelnr, bool selected);
	void setChStatus(int32 channelnr, uint32 status);
	void setChState(int32 channelnr, char state);
	void setChStartAddr(int32 channelnr, uint32 startaddr);
	void setChPageSize(int32 channelnr, uint32 pagesize);
	
	void convertRcu2Ch(int32 rcunr, int32 *boardnr, int32 *channelnr);
	void convertCh2Rcu(int32 channelnr, int32 *rcunr);
	bool isBoardActive(int32 boardnr);
	bool isBoardSelected(int32 boardnr);
	void logChannelInfo(int32 channel);
		
	uint32 getMemorySize(int32 boardnr);
	void setMemorySize(int32 boardnr,uint32 pages);
	
	friend class TBBDriver;

protected:	// note TBBDriver must be able to set them
	void getTbbSettings();
	void setConversionTable(int32 rcu, int32 channel); 
	void setMaxBoards (int32 maxboards);
	void setActiveBoards (uint32 activeboardsmask);
	void setMaxRetries(int32 retries);
	void setTimeOut(double timeout);
	void setBoardPorts(int board, GCFPortInterface* board_ports);
	void clearRcuSettings(int32 boardnr);
	
private:
	// Copying is not allowed
	TbbSettings(const TbbSettings&	that);
	TbbSettings& operator=(const TbbSettings& that);

	// --- Datamembers ---
	int32	itsDriverVersion;
	int32	itsMaxBoards;	// constants
	int32	itsMaxChannels;
	int32	itsMpsOnBoard;
	int32	itsChannelsOnMp;
	int32 itsChannelsOnBoard;
	int32 itsFlashMaxPages;
	int32 itsFlashSectorsInPage;
	int32 itsFlashBlocksInPage;
	int32 itsFlashPageSize;
	int32 itsFlashPageSectorSize;
	int32 itsFlashPageBlockSize;
	int32	itsMaxRetries;
	double	itsTimeOut;

	// mask with active boards
	uint32	itsActiveBoardsMask;
	
	int32				*itsRcu2ChTable;
	int32				*itsCh2RcuTable;
	BoardInfo		*itsBoardInfo;
	ChannelInfo	*itsChannelInfo;
	string			itsIfName;
	
	static TbbSettings	*theirTbbSettings;
};


	
//# --- inline functions ---
inline	int32 TbbSettings::driverVersion() { return (itsDriverVersion); }
inline	int32 TbbSettings::maxBoards() { return (itsMaxBoards); }
inline	int32 TbbSettings::maxChannels() { return (itsMaxChannels); }
inline	int32 TbbSettings::nrMpsOnBoard() { return (itsMpsOnBoard); }
inline	int32 TbbSettings::nrChannelsOnMp() { return (itsChannelsOnMp); }
inline	int32 TbbSettings::nrChannelsOnBoard() { return (itsChannelsOnBoard); }
inline	int32 TbbSettings::flashMaxPages() { return (itsFlashMaxPages); }
inline	int32 TbbSettings::flashSectorsInPage() { return (itsFlashSectorsInPage); }
inline	int32 TbbSettings::flashBlocksInPage() { return (itsFlashBlocksInPage); }
inline	int32 TbbSettings::flashPageSize() { return (itsFlashPageSize); }
inline	int32 TbbSettings::flashPageSectorSize() { return (itsFlashPageSectorSize); }
inline	int32 TbbSettings::flashPageBlockSize() { return (itsFlashPageBlockSize); }
inline	uint32 TbbSettings::activeBoardsMask()	{ return (itsActiveBoardsMask);   }
inline	int32 TbbSettings::maxRetries()	{ return (itsMaxRetries);   }
inline	double TbbSettings::timeout()	{ return (itsTimeOut);   }
//inline	GCFPortInterface& TbbSettings::boardPort(int32 boardnr)	{ return (itsBoardPorts[boardnr]); }
inline	GCFPortInterface& TbbSettings::boardPort(int32 boardnr)	{ return (*itsBoardInfo[boardnr].port); }

//---- inline functions for channel information ------------
inline	bool TbbSettings::isChSelected(int32 channelnr) { return (itsChannelInfo[channelnr].Selected); }
inline	uint32 TbbSettings::getChStatus(int32 channelnr) { return (itsChannelInfo[channelnr].Status); }
inline	char TbbSettings::getChState(int32 channelnr) { return (itsChannelInfo[channelnr].State); }
inline	int32 TbbSettings::getChRcuNr(int32 channelnr) { return (itsChannelInfo[channelnr].RcuNr); }
inline	int32 TbbSettings::getChBoardNr(int32 channelnr) { return (itsChannelInfo[channelnr].BoardNr); }
inline	int32 TbbSettings::getChInputNr(int32 channelnr) { return (itsChannelInfo[channelnr].InputNr); }
inline	int32 TbbSettings::getChMpNr(int32 channelnr) { return (itsChannelInfo[channelnr].MpNr); }
inline	uint32 TbbSettings::getChStartAddr(int32 channelnr) { return (itsChannelInfo[channelnr].StartAddr); }
inline	uint32 TbbSettings::getChPageSize(int32 channelnr) { return (itsChannelInfo[channelnr].PageSize); }
inline	string TbbSettings::getIfName() { return(itsIfName); }
inline	string TbbSettings::getSrcIp(int32 boardnr) { return(itsBoardInfo[boardnr].srcIp); }
inline	string TbbSettings::getDstIp(int32 boardnr) { return(itsBoardInfo[boardnr].dstIp); }
inline	string TbbSettings::getSrcMac(int32 boardnr) { return(itsBoardInfo[boardnr].srcMac); }
inline	string TbbSettings::getDstMac(int32 boardnr) { return(itsBoardInfo[boardnr].dstMac); }

inline	void TbbSettings::setChSelected(int32 channelnr, bool selected) { itsChannelInfo[channelnr].Selected = selected; }
inline	void TbbSettings::setChStatus(int32 channelnr, uint32 status){ itsChannelInfo[channelnr].Status = status; }
inline	void TbbSettings::setChState(int32 channelnr, char state){ itsChannelInfo[channelnr].State = state; }
inline	void TbbSettings::setChStartAddr(int32 channelnr, uint32 startaddr){ itsChannelInfo[channelnr].StartAddr = startaddr; }
inline	void TbbSettings::setChPageSize(int32 channelnr, uint32 pagesize){ itsChannelInfo[channelnr].PageSize = pagesize; }

//---- inline functions for board information ------------
inline	uint32 TbbSettings::getMemorySize(int32 boardnr) { return (itsBoardInfo[boardnr].memorySize); }
inline	void TbbSettings::setMemorySize(int32 boardnr,uint32 pages) { itsBoardInfo[boardnr].memorySize = pages; }

  } // namespace TBB
} // namespace LOFAR

#endif
