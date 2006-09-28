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
				
				virtual void saveTbbEvent(GCFEvent& event, uint32 activeboards);
											
				virtual void sendTpEvent(GCFPortInterface& port);

				virtual void saveTpAckEvent(GCFEvent& event, int32 boardnr);

				virtual void sendTbbAckEvent(GCFPortInterface* clientport);
				
				virtual void portError(int32 boardnr);
				
				virtual uint32 getSendMask();
				
				virtual uint32 getRecvMask();
				
				virtual bool done();
				
				virtual bool waitAck();
      
			private:
				uint32	itsSendMask;  // mask indicates the boards to communicate with
				uint32	itsRecvMask;  // mask indicates the boards handled
				uint32  itsErrorMask;  // mask indicates the not responding boards
				uint32	itsBoardsMask;	// Installed boards mask
				
				TPVersionEvent			*itsTPE;
				TPVersionEvent			*itsTPackE;
				TBBVersionEvent			*itsTBBE;
				TBBVersionackEvent	*itsTBBackE;
				
				uint32	itsBoardStatus[MAX_N_TBBBOARDS];
				uint32	itsBoardId[MAX_N_TBBBOARDS];
				uint32	itsSwVersion[MAX_N_TBBBOARDS];
				uint32	itsBoardVersion[MAX_N_TBBBOARDS];
				uint32	itsTpVersion[MAX_N_TBBBOARDS];
				uint32	itsMp0Version[MAX_N_TBBBOARDS];
				uint32	itsMp1Version[MAX_N_TBBBOARDS];
				uint32	itsMp2Version[MAX_N_TBBBOARDS];
				uint32	itsMp3Version[MAX_N_TBBBOARDS];
		};
	} // end TBB namespace
} // end LOFAR namespace

#endif /* VERSIONCMD_H_ */
