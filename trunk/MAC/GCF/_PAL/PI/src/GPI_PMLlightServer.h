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

#include <PI_Protocol.ph>
#include <PA_Protocol.ph>

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
		GPIPMLlightServer (GPIController& controller, const string& name, bool transportRawData);
		virtual ~GPIPMLlightServer ();
    
    GCFTCPPort& getClientPort()    {return _plsPort;}
    //GCFTCPPort& getPAPort()  {return _propertyAgent;}      
    virtual void sendMsgToClient(GCFEvent& msg);
    void sendMsgToPA(GCFEvent& msg);
      
  protected: // event handle methods
    void registerPropSet(const PIRegisterScopeEvent& requestIn);
    void propSetRegistered(const PAScopeRegisteredEvent& responseIn);
    void unregisterPropSet(const PIUnregisterScopeEvent& requestIn);
    void propSetUnregistered(const PAScopeUnregisteredEvent& responseIn);
    void linkPropSet(const PALinkPropSetEvent& requestIn);
    void propSetLinked(const PIPropSetLinkedEvent& responseIn);
    void unlinkPropSet(const PAUnlinkPropSetEvent& requestIn);
    void propSetUnlinked(const PIPropSetUnlinkedEvent& responseIn);
    void valueSet(const PIValueSetEvent& indication);        
     
	private: // helper methods
    GPIPropertySet* findPropertySet(const string& scope) const;
    GPIPropertySet* findPropertySet(unsigned int seqnr) const;
        
	protected: // state methods
		        GCFEvent::TResult initial     (GCFEvent& e, GCFPortInterface& p);
    virtual GCFEvent::TResult operational (GCFEvent& e, GCFPortInterface& p);
            GCFEvent::TResult closing     (GCFEvent& e, GCFPortInterface& p);
    
  private: // (copy)constructors
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

  private: // admin. data members
    typedef map<unsigned short /*seqnr*/, GPIPropertySet*>  TActionSeqList;
    TActionSeqList _actionSeqList;    
    int _timerID;
};

#endif
