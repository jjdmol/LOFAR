//#  -*- mode: c++ -*-
//#
//#  VersionCmd.h: Get TBB board versions
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

#ifndef VERSIONCMD_H_
#define VERSIONCMD_H_

#include <Common/LofarTypes.h>
#include <GCF/TM/GCF_Control.h>

#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include "TP_Protocol.ph"

#include "Command.h"
#include "DriverSettings.h"

namespace LOFAR {
	using namespace TBB_Protocol;
  namespace TBB {

		class VersionCmd : public Command 
		{
			public:
				// Constructors for a GetVersions object.
				VersionCmd();
	  
				// Destructor for GetVersions.
				virtual ~VersionCmd();
				
				virtual bool isValid(GCFEvent& event);
				
				virtual void saveTbbEvent(GCFEvent& event);
									
				virtual void sendTpEvent();

				virtual void saveTpAckEvent(GCFEvent& event);

				virtual void sendTbbAckEvent(GCFPortInterface* clientport);
      
			private:
				TbbSettings *TS;
				
				TPVersionEvent			*itsTPE;
				TPVersionAckEvent		*itsTPackE;
				TBBVersionEvent			*itsTBBE;
				TBBVersionAckEvent	*itsTBBackE;
		};
	} // end TBB namespace
} // end LOFAR namespace

#endif /* VERSIONCMD_H_ */
