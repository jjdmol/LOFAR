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

namespace LOFAR {
	using namespace TBB_Protocol;
  namespace TBB {

		class WritefCmd : public Command 
		{
			public:
				// Constructors for a WritefCmd object.
				WritefCmd();
	  
				// Destructor for WritefCmd.
				virtual ~WritefCmd();
				
				virtual bool isValid(GCFEvent& event);
				
				virtual void saveTbbEvent(GCFEvent& event, int32 boards);
									
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
				
				TPWritefEvent			*itsTPE;
				TPWritefEvent			*itsTPackE;
				TBBWritefEvent		*itsTBBE;
				TBBWritefackEvent	*itsTBBackE;
				
				// variables holding data from tp cmd
				uint32	itsBoardStatus;
				uint32	itsAddr;
				uint32	itsData[256];
		};
	} // end TBB namespace
} // end LOFAR namespace

#endif /* WRITEFCMD_H_ */
