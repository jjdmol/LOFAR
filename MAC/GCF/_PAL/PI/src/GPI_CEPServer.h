//#  GPI_CEPServer.h: representation of a Supervisory Server in a ERTC env.
//#
//#  Copyright (C) 2002-2003
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

#ifndef GPI_CEPSERVER_H
#define GPI_CEPSERVER_H

#include <GPI_PMLlightServer.h>
#include <GPI_TH_Port.h>
#include <DH_PIProtocol.h>

using LOFAR::GPITH_Port;
using LOFAR::DH_PIProtocol;


/**
 */
 
class GPICEPServer: public GPIPMLlightServer
{
	public:
		GPICEPServer (GPIController& controller);
		virtual ~GPICEPServer ();
   
    virtual void sendMsgToClient(GCFEvent& msg);        
      
	private: // helper methods
    void convertToGCFEvent();
    void convertToBlob();
        
	private: // state methods
    GCFEvent::TResult operational (GCFEvent& e, GCFPortInterface& p);
    
  private: // (copy)constructors
    GPICEPServer();
    /**
     * Don't allow copying of this object.
     */
    GPICEPServer (const GPICEPServer&);
    GPICEPServer& operator= (const GPICEPServer&);

	private: // data members
    GPITH_Port      _thPort;
    DH_PIProtocol   _dhProto;
        
  private: // admin. data members
    char*         _valueBuf;
    unsigned int  _upperboundValueBuf;
};

#endif
