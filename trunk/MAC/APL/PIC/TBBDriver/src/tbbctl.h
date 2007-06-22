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
#include <Common/lofar_bitset.h>
#include <Common/lofar_list.h>
#include <Common/lofar_string.h>


namespace LOFAR {
  namespace TbbCtl {
  	
// MAX_N_TBBBOARDS and MAX_N_RCUS come from TBB_protocol.ph

static const int BOARDCMD = 1;
static const int RCUCMD = 2;

// information about the flash memory
static const int FL_SIZE 						= 64 * 1024 *1024; // 64 MB in bytes
static const int FL_N_PAGES 				= 32; // 32 pages in flash
static const int FL_N_SECTORS				= 512; // 512 sectors in flash
static const int FL_N_BLOCKS				= 65536; // 65336 blocks in flash

static const int FL_PAGE_SIZE 			= FL_SIZE / FL_N_PAGES; // 2.097.152 bytes  
static const int FL_SECTOR_SIZE			= FL_SIZE / FL_N_SECTORS; // 131.072 bytes
static const int FL_BLOCK_SIZE 			= FL_SIZE / FL_N_BLOCKS; // 1.024 bytes

static const int FL_SECTORS_IN_PAGE	= FL_PAGE_SIZE / FL_SECTOR_SIZE; // 16 sectors per page
static const int FL_BLOCKS_IN_PAGE	= FL_PAGE_SIZE / FL_BLOCK_SIZE; // 2048 blocks per page

  	
//-----------------------------------------------------------------------------
// class Command :base class for control commands towards the TBBDriver.
//
class Command
{

public:
	virtual ~Command()
	{
		logMessage(cout,formatString("=============================================================================="));
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
			
			if (status & TBB_COMM_ERROR)				str.append(",comm. time-out ");			
			if (status & TBB_SELECT_ERROR)			str.append(",not selectable ");
			if (status & TBB_FLASH_ERROR)				str.append(",flash error ");
			if (status & TBB_ALLOC_ERROR)				str.append(",alloc error ");
			if (status & TBB_RCU_COMM_ERROR)		str.append(",rcu error ");
			
			if (status & TBB_CHANNEL_IN_USE)		str.append(",channel in use ");
			if (status & TBB_INPUT_IN_USE) 			str.append(",input in use ");
			if (status & TBB_BUFFER_TO_LARGE) 	str.append(",buffer to large ");
			if (status & TBB_NO_MP_WRITER)			str.append(",no mp writer ");
			if (status & TBB_RING_FULL)					str.append(",ring full ");
			if (status & TBB_RCU_NOT_FREE)			str.append(",rcu not free ");
			if (status & TBB_RCU_NOT_ALLOCATED)	str.append(",rcu not allocated ");		
			if (status & TBB_RCU_NOT_RECORDING)	str.append(",rcu not recording ");						
							
			if (status & TBB_CRC_ERROR_MP0)			str.append(",crc error mp0 ");
			if (status & TBB_CRC_ERROR_MP1)			str.append(",crc error mp1 ");
			if (status & TBB_CRC_ERROR_MP2)			str.append(",crc error mp2 ");
			if (status & TBB_CRC_ERROR_MP3)			str.append(",crc error mp3 ");
			if (status & TBB_CRC_ERROR_TP)			str.append(",crc error tp ");
			if (status & TBB_ACK_ERROR_TP)			str.append(",ack error tp ");
			if (status & TBB_TIMEOUT_TP_MP)			str.append(",timeout tp-mp ");
			if (status & TBB_TIMEOUT_ETH)				str.append(",timeout eth ");
			
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
		if (itsCmdType == BOARDCMD) maxbits = MAX_N_TBBBOARDS; 
		if (itsCmdType == RCUCMD)   maxbits = MAX_N_RCUS;
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
	
	//--selection is done-------------------------------------	
	void setSelected(bool select)
	{
		itsSelected = select;	
	}
	
	//--check if selection is done----------------------------
	bool getSelected(void) const
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
	
protected:
	explicit Command(GCFPortInterface& port) : 
  	itsPort(port),
  	itsSelected(false),
  	itsCmdDone(false),
		itsCmdType(0)
	{
	}
	
	Command(); // no default construction allowed

protected:
	GCFPortInterface& 			itsPort;
	bool		itsSelected;
	bool		itsCmdDone;
	int			itsCmdType;
	std::list<int>	itsSelection;
		
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
};

//-----------------------------------------------------------------------------
class TrigSetupCmd : public Command
{
	public:
		TrigSetupCmd(GCFPortInterface& port);
		virtual ~TrigSetupCmd() { }
		virtual void send();
		virtual GCFEvent::TResult ack(GCFEvent& e);
		void setLevel(uint32 level) { itsLevel = level; }
		void setMode(uint32 mode) { itsMode = mode; }
		void setFilter(uint32 filter) { itsFilter = filter; }
		void setWindow(uint32 window) { itsWindow = window; }
		void setDummy(uint32 dummy) { itsDummy = dummy; }
	private:
		uint32 itsLevel;
		uint32 itsMode;
		uint32 itsFilter;
		uint32 itsWindow;
		uint32 itsDummy;
};

//-----------------------------------------------------------------------------
class TrigCoefficientCmd : public Command
{
	public:
		TrigCoefficientCmd(GCFPortInterface& port);
		virtual ~TrigCoefficientCmd() { }
		virtual void send();
		virtual GCFEvent::TResult ack(GCFEvent& e);
		void setC0(int16 c0) { itsC0 = c0; }
		void setC1(int16 c1) { itsC1 = c1; }
		void setC2(int16 c2) { itsC2 = c2; }
		void setC3(int16 c3) { itsC3 = c3; }
	private:
		int16 itsC0;
		int16 itsC1;
		int16 itsC2;
		int16 itsC3;
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
		void setImage(uint32 image)  {	itsImage = image;	}
	private:
		uint32	itsImage;
};

//-----------------------------------------------------------------------------
class ErasefCmd : public Command
{
	public:
		ErasefCmd(GCFPortInterface& port);
		virtual ~ErasefCmd() { }
		virtual void send();
		virtual GCFEvent::TResult ack(GCFEvent& e);
		void setPage(int page)  {	itsPage = page;	}
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
		void setPage(int page)  {	itsPage = page;	}	
	private:
		int		itsPage;
};

//-----------------------------------------------------------------------------
class WritefCmd : public Command
{
	public:
		WritefCmd(GCFPortInterface& port);
		virtual ~WritefCmd() { }
		virtual void send();
		virtual GCFEvent::TResult ack(GCFEvent& e);
		void setPage(int page)  {	itsPage = page;	}	
		void setVersion(double version)  {	itsVersion = version;	}	
		void setFileNameTp(char *filename)  {	strcpy(itsFileNameTp,filename);	}
		void setFileNameMp(char *filename)  {	strcpy(itsFileNameMp,filename);	}
	private:
		int itsPage;
		double itsVersion;
		char itsFileNameTp[64];
		char itsFileNameMp[64];
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
		void setMp(uint32 mp)		{	itsMp = mp;	}
		void setStartAddr(uint32 startaddr)  {	itsStartAddr = startaddr;	itsAddr = itsStartAddr;}
		void setStopAddr(uint32 stopaddr)  {	itsStopAddr = stopaddr; }	
	private:
		uint32	itsMp;
		uint32	itsStartAddr;
		uint32 	itsStopAddr;
		uint32	itsAddr;
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
		void setWordLo(uint32 lo)  {	itsWordLo = lo;	}
		void setWordHi(uint32 hi)  {	itsWordHi = hi;	}
	private:
		uint32	itsMp;
		uint32	itsAddr;
		uint32	itsWordLo;
		uint32	itsWordHi;
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
		int32		itsCmdStage;
		uint32	itsMp;
		uint32	itsAddr;
		uint32	itsAddrLine;
		uint32	itsTestPatern;
		uint32	itsWordLo;
		uint32	itsWordHi;
};

//-----------------------------------------------------------------------------
class ReadrCmd : public Command
{
	public:
		ReadrCmd(GCFPortInterface& port);
		virtual ~ReadrCmd() { }
		virtual void send();
		virtual GCFEvent::TResult ack(GCFEvent& e);
	private:
};

//-----------------------------------------------------------------------------
class WriterCmd : public Command
{
	public:
		WriterCmd(GCFPortInterface& port);
		virtual ~WriterCmd() { }
		virtual void send();
		virtual GCFEvent::TResult ack(GCFEvent& e);
	private:
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
		void setAddr(uint32 addr)  { itsAddr = addr; }
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
		int32		itsRcu;
		uint32	itsPages;
		// values used in program
		int32		itsCmdStage;
		uint32	itsPage;
		uint32	itsAddr;
		// data from channelInfoCmd
		char		itsState;		
		uint32	itsStartAddr;
		uint32	itsSize;
		int32		itsBoard;
		int32		itsMp;
		// data from ReadxCmd
		uint32 itsData[512];
		// extracted data from itsData[512]
		int itsStationId;
		int itsRspId;
		int itsRcuId;
		int itsSampleFreq;
		time_t itsTime;
		int itsSampleNr;
		int itsSamplesPerFrame;
		int itsFreqBands;
		int itsTotalSamples;
		int itsTotalBands;
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
	
	void help();
private:
  // ports
  GCFPort	itsServerPort;

  // the command to execute
  Command* 	itsCommand;
	
  // dimensions of the connected hardware
	uint32	itsActiveBoards;	// mask b0 = board0, b1 = board1 ....
	int			itsMemory[MAX_N_TBBBOARDS];
	int			itsMaxBoards;
	int			itsMaxChannels;
					
		
  // commandline parameters
  int   	itsArgc;
  char**	itsArgv;
};

  } // end namespace tbbctl
} // en namespace LOFAR 

#endif /* TBBCTL_H_ */
