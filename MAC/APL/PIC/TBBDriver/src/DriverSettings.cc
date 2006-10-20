//#  DriverSettings.cc: Driver settings
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

//# Includes
#include <Common/LofarLogger.h>
#include <DriverSettings.h>

namespace LOFAR {
  namespace TBB {

//
// Initialize singleton
//
DriverSettings* DriverSettings::theirDriverSettings = 0;

DriverSettings* DriverSettings::instance()
{
	if (theirDriverSettings == 0) {
		theirDriverSettings = new DriverSettings();
	}
	return (theirDriverSettings);
}

//
// Default constructor
//
DriverSettings::DriverSettings() :
	itsMaxBoards(0),
	itsMaxChannels(0),
	itsMpsPerBoard(4),
	itsChannelsPerMp(4),
	itsChannelsPerBoard(16),
	itsTimeOut(0.1),
	itsActiveBoardsMask(0),
	itsChannel(0),
	itsMemorySize(0)
{

}

DriverSettings::~DriverSettings()
{
	if (itsChannel) delete itsChannel;
	if (itsMemorySize) delete itsMemorySize;			
}


void DriverSettings::setBoardPorts(GCFPortInterface* board_ports)
{
	itsBoardPorts	= board_ports; // save address of boards array		
}

//---- setMaxBoards ------------------------------
void DriverSettings::setMaxBoards (int32 maxboards)
{
	itsMaxBoards = maxboards;
	itsMaxChannels = itsChannelsPerBoard * maxboards;
	if (itsChannel) delete itsChannel;
	itsChannel = new ChannelInfo[itsMaxChannels];
		
	int32 boardnr = 0;
	int32 channelnr = 0;
	int32 mpnr = 0;
	
	for (int nr = 0; nr < itsMaxChannels; nr++) {
		itsChannel[nr].Selected = false;
		itsChannel[nr].Active = false;
		itsChannel[nr].BoardNr = boardnr;
		itsChannel[nr].BoardChannelNr = channelnr;
		itsChannel[nr].MpNr = mpnr;
		itsChannel[nr].StartAddr = 0;
		itsChannel[nr].PageLength = 0;
		boardnr++;
		if (boardnr == itsMaxBoards) boardnr = 0;
		channelnr++;
		if (channelnr == itsChannelsPerBoard) channelnr = 0;
		mpnr++;
		if (mpnr == itsMpsPerBoard) mpnr = 0;
	}
	
	if (itsMemorySize) delete itsMemorySize;
	itsMemorySize = new int32[itsMaxBoards];
	
	for (int nr = 0;nr < itsMaxBoards; nr++) {
		itsMemorySize[nr] = 0;
	}
}

//---- setActiveBoards ---------------------------
void DriverSettings::setActiveBoards (uint32 activeboardsmask)
{
	itsActiveBoardsMask = activeboardsmask;
}
//---- set Communication time-out ----------------
void DriverSettings::setTimeOut(double timeout)
{
	itsTimeOut = timeout;
}



  } // namespace PACKAGE
} // namespace LOFAR
