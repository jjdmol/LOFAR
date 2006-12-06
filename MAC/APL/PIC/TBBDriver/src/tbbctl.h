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
	virtual string getErrorStr(uint32 status)
	{
		string str;
		
		str.clear();
		str.append("=ERROR= ");
		if (status & NO_BOARD)				str.append(";Board not available ");
		if (status & COMM_ERROR)			str.append(";Comm. time-out ");			
		if (status & SELECT_ERROR)		str.append(";Not selectable ");
		if (status & CMD_ERROR)				str.append(";Command failed ");
		if (status & ALLOC_ERROR)			str.append(";Alloc error ");
		if (status & CHANNEL_IN_USE)	str.append(";Channel In Use ");
		if (status & INPUT_IN_USE) 		str.append(";Input In Use ");
		if (status & BUFFER_TO_LARGE) str.append(";Buffer To Large ");
		if (status & NO_MP_WRITER)		str.append(";No Mp Writer ");
		if (status & RING_FULL)				str.append(";Ring Full ");
		if (status & CRC_ERROR_MP0)		str.append(";CRC error MP0 ");
		if (status & CRC_ERROR_MP1)		str.append(";CRC error MP1 ");
		if (status & CRC_ERROR_MP2)		str.append(";CRC error MP2 ");
		if (status & CRC_ERROR_MP3)		str.append(";CRC error MP3 ");
		if (status & CRC_ERROR_TP)		str.append(";CRC error TP ");
		if (status & ACK_ERROR_TP)		str.append(";ACK error TP ");
		if (status & TIMEOUT_TP_MP)		str.append(";TIMEOUT TP-MP ");
		return str;
	}
	
	
	//--------------------------------------------------------
  void setMax(int max)
	{
		itsMax = max;	
	}
	
	//--------------------------------------------------------
	int getMax(void) const
	{
		return itsMax;	
	}
	
	//--------------------------------------------------------	
	void setSelected(bool select)
	{
		itsSelected = select;	
	}
	
	//--------------------------------------------------------
	bool getSelected(void) const
	{
		return itsSelected;	
	}
	
	//--------------------------------------------------------
	void setSelect(std::list<int> select)
	{
		itsSelect = select;
		
		int32 boardnr = 0;
		int32 channelnr = 0;
		std::list<int>::const_iterator it;
		for (it = itsSelect.begin(); it != itsSelect.end(); ++it) {
			channelnr = *it;
			boardnr = (int32)(channelnr / 16.);
			channelnr -= boardnr * 16; 
			itsChannelMask[boardnr] |= (1 << *it);	
		}
	}
	
	
	//--------------------------------------------------------
	bool isSelected(int nr) const
	{
		bool inList = false;
		
		std::list<int>::const_iterator it;
		for (it = itsSelect.begin(); it != itsSelect.end(); ++it) {
			if(*it == nr) {
				inList = true;	
				break;
			}
		}	
		return inList;	
	}
	
	int32 getChannel()
	{
		return itsSelect.front();
	}
	
	int32 getBoard()
	{
		return itsSelect.front();
	}
	
	//--------------------------------------------------------
	uint32 getMask()
	{
		uint32 mask = 0;
		std::list<int>::const_iterator it;
		for (it = itsSelect.begin(); it != itsSelect.end(); ++it) {
			mask |= (1 << *it);	
		}
		return mask;
	}
		
	//--------------------------------------------------------
	uint32 getChannelMask(int32 boardnr)
	{
		return itsChannelMask[boardnr];
	}
	
	//--------------------------------------------------------
	void setCmdDone(bool done)
	{
			itsCmdDone = done;
	}
	
	//--------------------------------------------------------
	bool isCmdDone()
	{
			return itsCmdDone;
	}
		
	
protected:
	explicit Command(GCFPortInterface& port) : 
  	itsPort(port),
  	itsMax(0),
		itsSelected(false),
  	itsSelect(0),
  	itsCmdDone(false)
	{
		for(int32 nr = 0; nr < MAX_N_TBBBOARDS;nr++)
			itsChannelMask[nr] = 0; 
	}
	
	Command(); // no default construction allowed

protected:
	GCFPortInterface& itsPort;
	int								itsMax;
	bool							itsSelected;
	std::list<int>		itsSelect; 
	uint32						itsChannelMask[MAX_N_TBBBOARDS];
	bool							itsCmdDone;
	
		
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
class TrigclrCmd : public Command
{
	public:
		TrigclrCmd(GCFPortInterface& port);
		virtual ~TrigclrCmd() { }
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
	private:
};

//-----------------------------------------------------------------------------
class UdpCmd : public Command
{
	public:
		UdpCmd(GCFPortInterface& port);
		virtual ~UdpCmd() { }
		virtual void send();
		virtual GCFEvent::TResult ack(GCFEvent& e);
	private:
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
	private:
};

//-----------------------------------------------------------------------------
class ReadfCmd : public Command
{
	public:
		ReadfCmd(GCFPortInterface& port);
		virtual ~ReadfCmd() { }
		virtual void send();
		virtual GCFEvent::TResult ack(GCFEvent& e);
	private:
};

//-----------------------------------------------------------------------------
class WritefCmd : public Command
{
	public:
		WritefCmd(GCFPortInterface& port);
		virtual ~WritefCmd() { }
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
		void setMp(uint32 mp)		{	itsMp = mp;	}
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
		int32 itsCmdStage;
		uint32 itsPage;
		uint32 itsMp;
		uint32 itsAddr;
		uint32 itsPages;
		uint32 itsData[512];
		
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
