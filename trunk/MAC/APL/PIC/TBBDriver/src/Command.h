//#  -*- mode: c++ -*-
//#
//#  Command.h: TBB Driver command class
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

#ifndef COMMAND_H_
#define COMMAND_H_

#include <Common/LofarTypes.h>
#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
  namespace TBB {
		
		enum CmdTypes {BoardCmd,ChannelCmd}; 
				
		class Command
    {
    public:
						
			// Constructor for Command
			Command(){};
	  
      // Destructor for Command.
      virtual ~Command(){};
			
			virtual bool isValid(GCFEvent& event) = 0;
				
			virtual void saveTbbEvent(GCFEvent& event) = 0;
											
			virtual bool sendTpEvent(int32 boardnr, int32 channelnr) = 0;
			
			virtual void saveTpAckEvent(GCFEvent& event, int32 boardnr) = 0;

			virtual void sendTbbAckEvent(GCFPortInterface* clientport) = 0;
			
			virtual uint32 getBoardMask() = 0;
				
			virtual bool waitAck() = 0;
			
			virtual CmdTypes getCmdType() = 0;
			
    private:
												
    };
	} // end TBB namespace
} // end LOFAR namespace
     
#endif /* COMMAND_H_ */
