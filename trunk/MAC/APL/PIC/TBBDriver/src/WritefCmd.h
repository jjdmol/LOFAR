//#  -*- mode: c++ -*-
//#
//#  WritefCmd.h: III
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

#ifndef WRITEFCMD_H_
#define WRITEFCMD_H_

#include <Common/LofarTypes.h>
#include <GCF/TM/GCF_Control.h>

#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include "TP_Protocol.ph"

#include "Command.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
	namespace TBB {
		
class WritefCmd : public Command 
{
public:
	enum flashStage {idle, unprotect, protect, erase_flash, write_flash, verify_flash};
	
	// Constructors for a WritefCmd object.
	WritefCmd();

	// Destructor for WritefCmd.
	virtual ~WritefCmd();
	
	virtual bool isValid(GCFEvent& event);
	
	virtual void saveTbbEvent(GCFEvent& event);
						
	virtual void sendTpEvent();

	virtual void saveTpAckEvent(GCFEvent& event);

	virtual void sendTbbAckEvent(GCFPortInterface* clientport);
	
	bool readFiles();
				
	uint8 charToHex(int ch);
	
private:
	TbbSettings *TS;
	
	flashStage	itsStage;
	
	int32		itsImage;
	int32		itsSector;
	int32		itsBlock;
	int32		itsImageSize;
	int32		itsDataPtr;
	char		itsFileNameTp[64];
	char		itsFileNameMp[64];
	uint32	itsPassword;
	uint8		*itsImageData; // data from hex files
	
	// return status
	uint32	itsStatus;
};

	} // end TBB namespace
} // end LOFAR namespace

#endif /* WRITEFCMD_H_ */
