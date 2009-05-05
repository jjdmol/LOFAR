//#  -*- mode: c++ -*-
//#
//#  tbbctl.h: command line interface to the TBBDriver
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

#ifndef TBBCTL_H_
#define TBBCTL_H_

#include <APL/TBB_Protocol/TBB_Protocol.ph>

#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>
#include <GCF/TM/GCF_TimerPort.h>
#include <Common/lofar_bitset.h>
#include <Common/lofar_list.h>
#include <Common/lofar_string.h>


namespace LOFAR {
  using GCF::TM::GCFTask;
  using GCF::TM::GCFPort;
  using GCF::TM::GCFTimerPort;
  using GCF::TM::GCFPortInterface;
  namespace TbbCtl {

static const int TBBCTL_VERSION = 210;

// MAX_N_TBBOARDS and MAX_N_RCUS come from TBB_protocol.ph

static const int BOARDCMD = 1;
static const int RCUCMD = 2;

// information about the flash memory
static const int FL_N_IMAGES = 16; // 16 images in flash

// register read/write table
// 0 = not used, 1 = read only, 2 = write only, 3 = read/write
static const int REG_NOT_USED = 0;
static const int REG_READ_ONLY = 1;
static const int REG_WRITE_ONLY = 2;
static const int REG_READ_WRITE = 3;
static const int REG_TABLE_3[8][8] = {
	{3,3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3,3},
	{3,3,0,3,3,3,3,3},	// 6-2, size 2048, to big for 100Mb/s
	{3,3,3,3,3,3,3,3}};
static const int REG_SIZE[8][8] = {
	{1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,0,0},
	{1,1,1,1,2,1,2,0},
	{1,1,1,1,2,1,2,0},
	{1,1,1,1,2,1,2,0},
	{1,1,1,1,2,1,2,0},
	{1,1,0,2,1,0,0,0},	// 6-2, size 2048, to big for 100Mb/s
	{1,1,1,8,4,16,4,16}};

static const int TBB_LISTEN_ONE_SHOT = 0;
static const int TBB_LISTEN_CONTINUES = 1;


// rcu to channel conversion, rcu-0 is on channel-2
static const int TBB_RCU_TABLE[16] = {
	2, 3, 6, 7, 10, 11, 14, 15, 0, 1, 4, 5, 8, 9, 12, 13
};

// rcu to mp conversion, rcu-0 is on mp-0
static const int TBB_MP_TABLE[16] = {
	0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 1, 1, 2, 2, 3, 3
};

// rcu to writer conversion, rcu-0 is on writer-2
static const int TBB_WRITER_TABLE[16] = {
	4, 5, 4, 5, 4, 5, 4, 5, 2, 3, 2, 3, 2, 3, 2, 3
};

// dimensions of the connected hardware
uint32	itsActiveBoards;	// mask b0 = board0, b1 = board1 ....
int		itsMemory[MAX_N_TBBOARDS];
int		itsMaxBoards;
int		itsMaxChannels;


//-----------------------------------------------------------------------------
// class Command :base class for control commands towards the TBBDriver.
//
class Command
{

public:
	virtual ~Command()
	{
		//logMessage(cout,formatString("=============================================================================="));
	}

	// Send the command to the TBBDriver
	virtual void send() = 0;

	// Check the acknowledgement sent by the TBBDriver.
	virtual GCFEvent::TResult ack(GCFEvent& e) = 0;

	//--------------------------------------------------------
	virtual void logMessage(ostream& stream, const string& message)
	{
  	stream << message << endl;
	}

	//-----------------------------------------------------------------------------
	virtual string getDriverErrorStr(uint32 status)
	{
		string str;

		str.clear();

		// all TBB_XXX constants come from TBB_Protocol.ph

		if (!(status & TBB_SUCCESS)) {
			if (status & TBB_NO_BOARD) {
				str.append("board not available ");
				return(str);
			}

			if (status & TBB_COMM_ERROR)        str.append(",comm. time-out ");
			if (status & TBB_SELECT_ERROR)      str.append(",not selectable ");
			if (status & TBB_FLASH_ERROR)       str.append(",flash error ");
			if (status & TBB_ALLOC_ERROR)       str.append(",alloc error ");
			if (status & TBB_RCU_COMM_ERROR)    str.append(",rcu error ");
			if (status & TBB_PROTECT_ERROR)     str.append(",flash protect error ");

			if (status & TBB_CHANNEL_IN_USE)    str.append(",channel in use ");
			if (status & TBB_INPUT_IN_USE)      str.append(",input in use ");
			if (status & TBB_BUFFER_TO_LARGE)   str.append(",buffer to large ");
			if (status & TBB_NO_MP_WRITER)      str.append(",no mp writer ");
			if (status & TBB_RING_FULL)         str.append(",ring full ");
			if (status & TBB_RCU_NOT_FREE)      str.append(",rcu not free ");
			if (status & TBB_RCU_NOT_ALLOCATED) str.append(",rcu not allocated ");
			if (status & TBB_RCU_NOT_RECORDING) str.append(",rcu not recording ");

			if (status & TBB_CRC_ERROR_MP0)     str.append(",crc error mp0 ");
			if (status & TBB_CRC_ERROR_MP1)     str.append(",crc error mp1 ");
			if (status & TBB_CRC_ERROR_MP2)     str.append(",crc error mp2 ");
			if (status & TBB_CRC_ERROR_MP3)     str.append(",crc error mp3 ");
			if (status & TBB_CRC_ERROR_TP)      str.append(",crc error tp ");
			if (status & TBB_ACK_ERROR_TP)      str.append(",ack error tp ");
			if (status & TBB_TIMEOUT_TP_MP)     str.append(",timeout tp-mp ");
			if (status & TBB_TIMEOUT_ETH)       str.append(",timeout eth ");

			char statusstr[64];
			sprintf(statusstr,"unknown ERROR, 0x%08X",status);
			if (str.empty() && status) str.append(statusstr);
		}
		return(str);
	}

	//-----------------------------------------------------------------------------
	virtual string getBoardErrorStr(uint16 status)
	{
		string str;

		str = getDriverErrorStr(((uint32)status << 16));

		return(str);
	}

	//--------------------------------------------------------
  void setCmdType(int type)
	{
		itsCmdType = type;
	}

	//--------------------------------------------------------
	int getMaxSelections(void) const
	{
		int maxbits = 0;
		if (itsCmdType == BOARDCMD) maxbits = itsMaxBoards;
		if (itsCmdType == RCUCMD)   maxbits = itsMaxChannels;
		return(maxbits);
	}

	//--------------------------------------------------------
	void setCmdDone(bool done)
	{
		itsCmdDone = done;
	}

	//--------------------------------------------------------
	bool isCmdDone() const
	{
		return(itsCmdDone);
	}

	//--------------------------------------------------------
	void setCmdSendNext(bool next)
	{
		itsCmdSendNext = next;
	}

	//--------------------------------------------------------
	bool isCmdSendNext() const
	{
		return(itsCmdSendNext);
	}

	//--selection is done-------------------------------------
	void setSelected(bool select)
	{
		itsSelected = select;
	}

	//--check if selection is done----------------------------
	bool isSelectionDone(void) const
	{
		return itsSelected;
	}

	//--set selection-----------------------------------------
	void setSelect(std::list<int> select)
	{
		itsSelection = select;
	}

	//--return true if board or rcu nr is selected------------
	bool isSelected(int nr) const
	{
		bool inList = false;
		std::list<int>::const_iterator it;

		for (it = itsSelection.begin(); it != itsSelection.end(); it++) {
			if (*it == nr) inList = true;
		}
		return(inList);
	}

	//--return selected board---------------------------------
	int getBoard() const
	{
		return(itsSelection.front());
	}

	//--return selected rcu-------------------------------
	int getRcu() const
	{
		return(itsSelection.front());
	}

	//--return selected boards in a mask----------------------
	uint32 getBoardMask() const
	{
		uint32 boardmask = 0;
		std::list<int>::const_iterator it;

		for (it = itsSelection.begin(); it != itsSelection.end(); it++) {
			boardmask |= (1 << *it);
		}
		return(boardmask);
	}

	//--return selected rcus in a mask----------------------
	std::bitset<MAX_N_RCUS> getRcuMask() const
	{
		std::bitset<MAX_N_RCUS> rcus;
		std::list<int>::const_iterator it;

		rcus.reset();
		for (it = itsSelection.begin(); it != itsSelection.end(); it++) {
			rcus.set(*it);
		}
		return(rcus);
	}

	int rcu2ch(int rcunr) const
	{
		return(TBB_RCU_TABLE[rcunr % 16]);
	}

	int rcu2mp(int rcunr) const
	{
		return(TBB_MP_TABLE[rcunr % 16]);
	}

	int rcu2writer(int rcunr) const
	{
		return(TBB_WRITER_TABLE[rcunr % 16]);
	}

	int rcu2board(int rcunr) const
	{
		int board = static_cast<int>(rcunr / 16);
		return(board);
	}

protected:
	explicit Command(GCFPortInterface& port) :
	itsPort(port),
	itsSelected(false),
	itsCmdDone(false),
	itsCmdSendNext(true),
	itsCmdType(0)
	{
	}

	Command(); // no default construction allowed

protected:
	GCFPortInterface& itsPort;
	bool itsSelected;
	bool itsCmdDone;
	bool itsCmdSendNext;
	int  itsCmdType;
	std::list<int> itsSelection;

private:

}; // end class Command


//-----------------------------------------------------------------------------
class AllocCmd : public Command
{
public:
	AllocCmd(GCFPortInterface& port);
	virtual ~AllocCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setAddr(int addr) { itsAddr = addr; }
	void setLength(int length) { itsLength = length; }
private:
	int itsAddr;
		int itsLength;
};

//-----------------------------------------------------------------------------
class ChannelInfoCmd : public Command
{
public:
	ChannelInfoCmd(GCFPortInterface& port);
	virtual ~ChannelInfoCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class FreeCmd : public Command
{
public:
	FreeCmd(GCFPortInterface& port);
	virtual ~FreeCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class RecordCmd : public Command
{
public:
	RecordCmd(GCFPortInterface& port);
	virtual ~RecordCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class StopCmd : public Command
{
public:
	StopCmd(GCFPortInterface& port);
	virtual ~StopCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class TriggerSettingsCmd : public Command
{
public:
	TriggerSettingsCmd(GCFPortInterface& port);
	virtual ~TriggerSettingsCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class TrigReleaseCmd : public Command
{
public:
	TrigReleaseCmd(GCFPortInterface& port);
	virtual ~TrigReleaseCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class TrigGenerateCmd : public Command
{
public:
	TrigGenerateCmd(GCFPortInterface& port);
	virtual ~TrigGenerateCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
	int32 itsTriggerNr;
};

//-----------------------------------------------------------------------------
class TrigSetupCmd : public Command
{
public:
	TrigSetupCmd(GCFPortInterface& port);
	virtual ~TrigSetupCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setLevel(uint16 level) { itsLevel = level; }
	void setStartMode(uint8 mode) { itsStartMode = mode; }
	void setStopMode(uint8 mode) { itsStopMode = mode; }
	void setFilter(uint8 filter) { itsFilter = filter; }
	void setWindow(uint8 window) { itsWindow = window; }
	void setOperatingMode(uint16 mode) { itsOperatingMode = mode; }
	void setTriggerMode(uint16 mode) { itsTriggerMode = mode; }
private:
	uint16 itsLevel;
	uint8 itsStartMode;
	uint8 itsStopMode;
	uint8 itsFilter;
	uint8 itsWindow;
	uint8 itsOperatingMode;
	uint16 itsTriggerMode;
};

//-----------------------------------------------------------------------------
class TrigCoefficientCmd : public Command
{
public:
	TrigCoefficientCmd(GCFPortInterface& port);
	virtual ~TrigCoefficientCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setC0(uint16 c0) { itsC0 = c0; }
	void setC1(uint16 c1) { itsC1 = c1; }
	void setC2(uint16 c2) { itsC2 = c2; }
	void setC3(uint16 c3) { itsC3 = c3; }
private:
	uint16 itsC0;
	uint16 itsC1;
	uint16 itsC2;
	uint16 itsC3;
};

//-----------------------------------------------------------------------------
class TrigInfoCmd : public Command
{
public:
	TrigInfoCmd(GCFPortInterface& port);
	virtual ~TrigInfoCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class ListenCmd : public Command
{
public:
	ListenCmd(GCFPortInterface& port);
	virtual ~ListenCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setListenMode(int32 mode) { itsListenMode = mode; }
private:
	int32 itsCmdStage;
	int32 itsListenMode;
};

//-----------------------------------------------------------------------------
class ReadCmd : public Command
{
public:
	ReadCmd(GCFPortInterface& port);
	virtual ~ReadCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setSecondsTime(uint32 secondstime) { itsSecondsTime = secondstime; }
	void setSampleTime(uint32 sampletime) { itsSampleTime = sampletime; }
	void setPrePages(uint32 prepages) { itsPrePages = prepages; }
	void setPostPages(uint32 postpages) { itsPostPages = postpages; }
private:
	uint32 itsSecondsTime;
	uint32 itsSampleTime;
	uint32 itsPrePages;
	uint32 itsPostPages;
};

//-----------------------------------------------------------------------------
class ReadAllCmd : public Command
{
public:
	ReadAllCmd(GCFPortInterface& port);
	virtual ~ReadAllCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setPages(uint32 pages) { itsPages = pages; }
private:
	int itsStage;
	int itsRcu;
	uint32 itsSecondsTime;
	uint32 itsSampleTime;
	uint32 itsPages;
};

//-----------------------------------------------------------------------------
class ModeCmd : public Command
{
public:
	ModeCmd(GCFPortInterface& port);
	virtual ~ModeCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setRecMode(uint32 recmode) { itsRecMode = recmode; }
private:
	uint32 itsRecMode;
};

//-----------------------------------------------------------------------------
class VersionCmd : public Command
{
public:
	VersionCmd(GCFPortInterface& port);
	virtual ~VersionCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class StatusCmd : public Command
{
public:
	StatusCmd(GCFPortInterface& port);
	virtual ~StatusCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class SizeCmd : public Command
{
public:
	SizeCmd(GCFPortInterface& port);
	virtual ~SizeCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class ClearCmd : public Command
{
public:
	ClearCmd(GCFPortInterface& port);
	virtual ~ClearCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class ResetCmd : public Command
{
public:
	ResetCmd(GCFPortInterface& port);
	virtual ~ResetCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class ConfigCmd : public Command
{
public:
	ConfigCmd(GCFPortInterface& port);
	virtual ~ConfigCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setImage(uint32 image) { itsImage = image; }
private:
	uint32 itsImage;
};

//-----------------------------------------------------------------------------
class ArpCmd : public Command
{
public:
	ArpCmd(GCFPortInterface& port);
	virtual ~ArpCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class ArpModeCmd : public Command
{
public:
	ArpModeCmd(GCFPortInterface& port);
	virtual ~ArpModeCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setMode(uint32 mode) { itsMode = mode; }
private:
	uint32 itsMode;
};

//-----------------------------------------------------------------------------
class StopCepCmd : public Command
{
public:
	StopCepCmd(GCFPortInterface& port);
	virtual ~StopCepCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class CepDelayCmd : public Command
{
public:
	CepDelayCmd(GCFPortInterface& port);
	virtual ~CepDelayCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setDelay(uint32 delay) { itsDelay = delay; }
private:
	uint32 itsDelay;
};

//-----------------------------------------------------------------------------
class TempLimitCmd : public Command
{
public:
	TempLimitCmd(GCFPortInterface& port);
	virtual ~TempLimitCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setLimitHigh(uint32 high) { itsLimitHigh = high; }
	void setLimitLow(uint32 low) { itsLimitLow = low; }
private:
	uint32 itsLimitHigh;
	uint32 itsLimitLow;
};

//-----------------------------------------------------------------------------
class ErasefCmd : public Command
{
public:
	ErasefCmd(GCFPortInterface& port);
	virtual ~ErasefCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setPage(int page) { itsPage = page; }
private:
	int itsPage;
};

//-----------------------------------------------------------------------------
class ReadfCmd : public Command
{
public:
	ReadfCmd(GCFPortInterface& port);
	virtual ~ReadfCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setPage(int page) { itsPage = page; }
private:
	int itsPage;
};

//-----------------------------------------------------------------------------
class WritefCmd : public Command
{
public:
	WritefCmd(GCFPortInterface& port);
	virtual ~WritefCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setPage(int page) { itsPage = page; }
	void setVersion(double version) { itsVersion = version; }
	void setFileNameTp(char *filename) { strcpy(itsFileNameTp,filename); }
	void setFileNameMp(char *filename) { strcpy(itsFileNameMp,filename); }
	void setPassword(uint32 password) { itsPassword = password; }
private:
	int itsPage;
	double itsVersion;
	char itsFileNameTp[64];
	char itsFileNameMp[64];
	uint32 itsPassword;
};

//-----------------------------------------------------------------------------
class ImageInfoCmd : public Command
{
public:
	ImageInfoCmd(GCFPortInterface& port);
	virtual ~ImageInfoCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
};

//-----------------------------------------------------------------------------
class ReadwCmd : public Command
{
public:
	ReadwCmd(GCFPortInterface& port);
	virtual ~ReadwCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setMp(uint32 mp) { itsMp = mp; }
	void setStartAddr(uint32 startaddr) { itsStartAddr = startaddr; itsAddr = itsStartAddr;}
	void setStopAddr(uint32 stopaddr) { itsStopAddr = stopaddr; }
private:
	uint32 itsMp;
	uint32 itsStartAddr;
	uint32 itsStopAddr;
	uint32 itsAddr;
};

//-----------------------------------------------------------------------------
class WritewCmd : public Command
{
public:
	WritewCmd(GCFPortInterface& port);
	virtual ~WritewCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setMp(uint32 mp)		{	itsMp = mp;	}
	void setAddr(uint32 addr)  {	itsAddr = addr;	}
	void setWord(uint32 word, int addr)  {	itsWord[addr] = word;	}
private:
	uint32	itsMp;
	uint32	itsAddr;
	uint32	itsWord[8];
};

//-----------------------------------------------------------------------------
class TestDdrCmd : public Command
{
public:
	TestDdrCmd(GCFPortInterface& port);
	virtual ~TestDdrCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
private:
	int32  itsCmdStage;
	uint32 itsAddrLines;
	uint32 itsMp;
	uint32 itsAddr;
	uint32 itsAddrLine;
	uint32 itsTestPatern;
	uint32 itsWord[8];
	int    itsAddrLineErrors;
	int    itsDataLineErrors;
};

//-----------------------------------------------------------------------------
class ReadrCmd : public Command
{
public:
	ReadrCmd(GCFPortInterface& port);
	virtual ~ReadrCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setMp(uint32 mp) {	itsMp = mp;	}
	void setPid(uint32 pid) {	itsPid = pid;	}
	void setRegId(uint32 regid) {	itsRegId = regid;	}

private:
	int32 itsMp;
	int32 itsPid;
	int32 itsRegId;
};

//-----------------------------------------------------------------------------
class WriterCmd : public Command
{
public:
	WriterCmd(GCFPortInterface& port);
	virtual ~WriterCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setMp(uint32 mp) {	itsMp = mp;	}
	void setPid(uint32 pid) {	itsPid = pid;	}
	void setRegId(uint32 regid) {	itsRegId = regid;	}
	void setData(int data_nr, uint32 data) { itsData[data_nr] = data; }
private:
	int32  itsMp;
	int32  itsPid;
	int32  itsRegId;
	uint32 itsData[16];	// total of 4 x 16 = 64 bytes
};

//-----------------------------------------------------------------------------
class ReadPageCmd : public Command
{
public:
	ReadPageCmd(GCFPortInterface& port);
	virtual ~ReadPageCmd() { }
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setMp(int32 mp)		{	itsMp = mp;	}
	void setStartPage(uint32 startpage)  { itsStartPage = startpage; }
	void setPages(uint32 pages)  { itsPages = pages; }
	static const uint32 PID6 = 6;
	static const uint32 REGID0 = 0;
	static const uint32 REGID1 = 1;
	static const uint32 REGID2 = 2;
	static const uint32 PAGEREAD = 1;
	static const uint32 DATALENGTH = 256;
	static const uint32 DATA1POS = 0;
	static const uint32 DATA2POS = 256;
private:
	// values given by user
	int32  itsRcu;
	uint32 itsStartPage;
	uint32 itsPages;
	// values used in program
	int32  itsCmdStage;
	uint32 itsPage;
	uint32 itsMpSize;
	// data from channelInfoCmd
	char   itsState;
	uint32 itsStartAddr;
	uint32 itsSize;
	int32  itsBoard;
	int32  itsMp;
	// data from ReadxCmd
	uint32 itsData[512];
	// extracted data from itsData[512]
	int    itsStationId;
	int    itsRspId;
	int    itsRcuId;
	int    itsSampleFreq;
	time_t itsTime;
	uint32 itsSampleNr;
	int    itsSamplesPerFrame;
	int    itsFreqBands;
	uint32 itsTotalSamples;
	int    itsTotalBands;
	int    itsBandNr;
	int    itsSliceNr;
};


//-----------------------------------------------------------------------------
// Controller class for tbbctl
// class TBBCtl
//
class TBBCtl : public GCFTask
{
public:

	/**
	* The constructor of the TBBCtl task.
	* @param name The name of the task. The name is used for looking
	* up connection establishment information using the GTMNameService and
	* GTMTopologyService classes.
	*/
	TBBCtl(string name, int argc, char** argv);
	virtual ~TBBCtl();

	// state methods

	/**
	* The initial state. In this state a connection with the TBB
	* driver is attempted. When the connection is established,
	* a transition is made to the connected state.
	*/
	GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);

	/**
	* In this state the command is sent and the acknowledge handled.
	* Any relevant output is printed.
	*/
	GCFEvent::TResult docommand(GCFEvent& e, GCFPortInterface &p);

	/**
	* Start the controller main loop.
	*/
	void mainloop();

private:
	// private methods
	Command* parse_options(int argc, char** argv);
	std::list<int> strtolist(const char* str, int max);
	void logMessage(ostream& stream, const string& message);

	void commandHelp(int level);
private:
	GCFPort       itsServerPort;
	GCFTimerPort* itsCmdTimer;
	Command*      itsCommand; // the command to execute

	// commandline parameters
	int    itsArgc;
	char** itsArgv;
};

	} // end namespace tbbctl
} // en namespace LOFAR

#endif /* TBBCTL_H_ */
