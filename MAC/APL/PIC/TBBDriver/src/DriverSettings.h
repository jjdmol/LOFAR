//#  DriverSettings.h: Global station settings
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

#include <GCF/TM/GCF_Control.h>
//# Includes
//#include <Common/LofarTypes.h>


namespace LOFAR {
  namespace TBB {

struct ChannelInfo
{
	bool		Selected;
	bool		Allocated;
	bool		Active;
	int32		BoardNr;
	int32		InputNr;
	int32		MpNr;
	uint32	StartAddr;
	uint32	PageSize;
};
		
		
// forward declaration
class TBBDriver;

// class_description
// ...
class DriverSettings
{
public:
	DriverSettings ();
	~DriverSettings();

	static DriverSettings* instance();

	int32 maxBoards();
	int32 maxChannels();
	int32 nrMpsPerBoard();
	int32 nrChannelsPerMp();
	int32 nrChannelsPerBoard();
	//uint32 channelMask(int32 boardnr);
	uint32 activeBoardsMask();
	double timeout();
	GCFPortInterface& boardPort(int32 boardnr);
	
	//void setChannelMask(int32 boardnr, uint32 channelmask);

	bool getChSelected(int32 channelnr);
	bool getChAllocated(int32 channelnr);
	bool getChActive(int32 channelnr);
	int32 getChBoardNr(int32 channelnr);
	int32 getChInputNr(int32 channelnr);
	int32 getChMpNr(int32 channelnr);
	uint32 getChStartAddr(int32 channelnr);
	uint32 getChPageSize(int32 channelnr);

	void setChSelected(int32 channelnr, bool select);
	void setChAllocated(int32 channelnr, bool allocated);
	void setChActive(int32 channelnr, bool active);
	void setChStartAddr(int32 channelnr, uint32 startaddr);
	void setChPageSize(int32 channelnr, uint32 pagesize);
	
	uint32 getMemorySize(int32 boardnr);
	void setMemorySize(int32 boardnr,uint32 pages);
	
	friend class TBBDriver;

protected:	// note TBBDriver must be able to set them
	void setMaxBoards (int32 maxboards);
	void setActiveBoards (uint32 activeboardsmask);
	void setTimeOut(double timeout);
	void setBoardPorts(GCFPortInterface* board_ports);
	
private:
	// Copying is not allowed
	DriverSettings(const DriverSettings&	that);
	DriverSettings& operator=(const DriverSettings& that);

	//# --- Datamembers ---
	int32	itsMaxBoards;	// constants
	int32	itsMaxChannels;
	int32	itsMpsPerBoard;
	int32	itsChannelsPerMp;
	int32 itsChannelsPerBoard;
	double	itsTimeOut;

	uint32	itsActiveBoardsMask;		// values depend on OPERATION_MODE
	
	//uint32			*itsChannelMask;
	ChannelInfo	*itsChannel;
	uint32	*itsMemorySize;
		
	GCFPortInterface*		itsBoardPorts; // array of tbb board ports
				
	static DriverSettings* theirDriverSettings;
};
	
//# --- inline functions ---
inline	int32 DriverSettings::maxBoards()	{ return (itsMaxBoards);   }
inline	int32 DriverSettings::maxChannels()	{ return (itsMaxChannels);   }
inline	int32 DriverSettings::nrMpsPerBoard()	{ return (itsMpsPerBoard);   }
inline	int32 DriverSettings::nrChannelsPerMp()	{ return (itsChannelsPerMp);   }
inline	int32 DriverSettings::nrChannelsPerBoard()	{ return (itsChannelsPerBoard);   }
inline	uint32 DriverSettings::activeBoardsMask()	{ return (itsActiveBoardsMask);   }
inline	double DriverSettings::timeout()	{ return (itsTimeOut);   }
inline	GCFPortInterface& DriverSettings::boardPort(int32 boardnr)	{ return (itsBoardPorts[boardnr]); }

//---- inline functions for channel information ------------
inline	bool DriverSettings::getChSelected(int32 channelnr) { return (itsChannel[channelnr].Selected); }
inline	bool DriverSettings::getChAllocated(int32 channelnr) { return (itsChannel[channelnr].Allocated); }
inline	bool DriverSettings::getChActive(int32 channelnr) { return (itsChannel[channelnr].Active); };
inline	int32 DriverSettings::getChBoardNr(int32 channelnr) { return (itsChannel[channelnr].BoardNr); }
inline	int32 DriverSettings::getChInputNr(int32 channelnr) { return (itsChannel[channelnr].InputNr); }
inline	int32 DriverSettings::getChMpNr(int32 channelnr) { return (itsChannel[channelnr].MpNr); }
inline	uint32 DriverSettings::getChStartAddr(int32 channelnr) { return (itsChannel[channelnr].StartAddr); }
inline	uint32 DriverSettings::getChPageSize(int32 channelnr) { return (itsChannel[channelnr].PageSize); }

inline	void DriverSettings::setChSelected(int32 channelnr, bool select) { itsChannel[channelnr].Selected = select; }
inline	void DriverSettings::setChAllocated(int32 channelnr, bool allocated) { itsChannel[channelnr].Allocated = allocated; }
inline	void DriverSettings::setChActive(int32 channelnr, bool active){ itsChannel[channelnr].Active = active; }
inline	void DriverSettings::setChStartAddr(int32 channelnr, uint32 startaddr){ itsChannel[channelnr].StartAddr = startaddr; }
inline	void DriverSettings::setChPageSize(int32 channelnr, uint32 pagesize){ itsChannel[channelnr].PageSize = pagesize; }

//---- inline functions for board information ------------
inline	uint32 DriverSettings::getMemorySize(int32 boardnr) { return (itsMemorySize[boardnr]); }
inline	void DriverSettings::setMemorySize(int32 boardnr,uint32 pages) { itsMemorySize[boardnr] = pages; }

  } // namespace TBB
} // namespace LOFAR

#endif
