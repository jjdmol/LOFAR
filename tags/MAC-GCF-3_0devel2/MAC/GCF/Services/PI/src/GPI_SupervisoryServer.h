//#  GPI_SupervisoryServer.h: representation of a Supervisory Server in a ERTC env.
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

#ifndef GPI_SUPERVISORYSERVER_H
#define GPI_SUPERVISORYSERVER_H

#include <GPI_Defines.h>
#include <GCF/GCF_Task.h>
#include <GCF/GCF_TCPPort.h>
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
class GPISupervisoryServer : public GCFTask
{
	public:
		GPISupervisoryServer (GPIController& controller);
		virtual ~GPISupervisoryServer ();
    inline GCFTCPPort& getPort()    {return _ssPort;}
    inline GCFTCPPort& getPAPort()  {return _propertyAgent;}
      
	private: // helper methods
    GPIPropertySet* findPropertySet(char* pScopeData, string& scope);
    void replyMsgToPA(GCFEvent& e, const string& scope);
        
	private: // state methods
		GCFEvent::TResult initial     (GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult operational (GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult closing     (GCFEvent& e, GCFPortInterface& p);
    
  private: // helper methods
    GPISupervisoryServer();
    /**
     * Don't allow copying of this object.
     */
    GPISupervisoryServer (const GPISupervisoryServer&);
    GPISupervisoryServer& operator= (const GPISupervisoryServer&);

	private: // data members
		GCFTCPPort        _ssPort;
    GCFTCPPort        _propertyAgent;
    GPIController&    _controller;
    typedef map<string /*scope*/, GPIPropertySet*> TScopeRegister;
    TScopeRegister    _scopeRegister;
    
  private: // admin. data members
    static const unsigned int MAX_BUF_SIZE = 5000;
    char              _buffer[MAX_BUF_SIZE];
};

#endif
