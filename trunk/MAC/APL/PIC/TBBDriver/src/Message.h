//#  -*- mode: c++ -*-
//#
//#  Message.h: TBB Driver Message class
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

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <Common/LofarTypes.h>
#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
  namespace TBB {

    class Message
    {
    public:
					
			static const double TIME_OUT = 0.5; 
						
			// Constructor for Message
			Message(){};
	  
      // Destructor for Message.
      virtual ~Message(){};
			
			virtual bool isValid(GCFEvent& event) = 0;
				
			virtual void saveTpEvent(GCFEvent& event) = 0;
				
			virtual void makeTbbEvent() = 0;
						
			virtual void sendTbbEvent(GCFPortInterface& port) = 0;
			
			virtual void sendTpEvent(GCFPortInterface& port) = 0;
			
    private:
									
    };
	} // end TBB namespace
} // end LOFAR namespace
     
#endif /* MESSAGE_H_ */
