//#  GPI_PMLlightServer.h: representation of a Supervisory Server in a ERTC env.
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

#ifndef GPI_PMLLIGTHSERVER_H
#define GPI_PMLLIGTHSERVER_H

#include <GPI_Defines.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <Common/lofar_list.h>

class GCFEvent;
class GPIController;
class GCFPValue;
class GPIPropertySet;

/**
 * This class represents and manages the connection with a Supervisory Server 
 * (part of ERTC framework). It acts as a PML with no owned properties. The 
 * properties and scopes are 'managed' by the SS and its connected ERTC tasks.
 */
class GPIPMLlightServer : public GCFTask
{
	public:
		GPIPMLlightServer (GPIController& controller);
		virtual ~GPIPMLlightServer ();
    GCFTCPPort& getPort()    {return _plsPort;}
    GCFTCPPort& getPAPort()  {return _propertyAgent;}
      
	private: // helper methods
    GPIPropertySet* findPropertySet(string& scope);
    GPIPropertySet* findPropertySet(unsigned int seqnr);
    void replyMsgToPA(GCFEvent& e);
        
	private: // state methods
		GCFEvent::TResult initial     (GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult operational (GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult closing     (GCFEvent& e, GCFPortInterface& p);
    
  private: // helper methods
    GPIPMLlightServer();
    /**
     * Don't allow copying of this object.
     */
    GPIPMLlightServer (const GPIPMLlightServer&);
    GPIPMLlightServer& operator= (const GPIPMLlightServer&);

	private: // data members
		GCFTCPPort        _plsPort;
    GCFTCPPort        _propertyAgent;
    GPIController&    _controller;
    typedef map<string /*scope*/, GPIPropertySet*> TPropSetRegister;
    TPropSetRegister    _propSetRegister;

    typedef map<unsigned short /*seqnr*/, GPIPropertySet*>  TActionSeqList;
    TActionSeqList _actionSeqList;    

    
  private: // admin. data members
};

#endif
