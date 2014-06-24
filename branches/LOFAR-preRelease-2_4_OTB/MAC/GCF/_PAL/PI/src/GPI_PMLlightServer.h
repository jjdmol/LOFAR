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

#include <GCF/Protocols/PI_Protocol.ph>
#include <GCF/Protocols/PA_Protocol.ph>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace Common
  {
class GCFPValue;
  }
  namespace TM
  {
class GCFEvent;
  }
  namespace PAL
  {
class GPIController;
class GPIPropertySet;

// This class represents and manages the connection with a PIA and the PA. It 
// acts as a PML with no owned properties w.r.t. the PA and as PA w.r.t. the 
// PIA. It manages a list of all registered property sets of one connected PIA.

class GPIPMLlightServer : public TM::GCFTask
{
	public:
		GPIPMLlightServer (GPIController& controller, const string& name, bool transportRawData);
		virtual ~GPIPMLlightServer ();
    
    TM::GCFTCPPort& getClientPort();
    virtual void sendMsgToClient(TM::GCFEvent& msg);
    void sendMsgToPA(TM::GCFEvent& msg);
      
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
     
  protected: // helper methods, also used in derived GPICEPServer class
    GPIPropertySet* findPropertySet(const string& scope) const;
    GPIPropertySet* findPropertySet(unsigned int seqnr) const;
        
	protected: // state methods
		        TM::GCFEvent::TResult initial     (TM::GCFEvent& e, TM::GCFPortInterface& p);
    virtual TM::GCFEvent::TResult operational (TM::GCFEvent& e, TM::GCFPortInterface& p);
            TM::GCFEvent::TResult closing     (TM::GCFEvent& e, TM::GCFPortInterface& p);
    
  private: // (copy)constructors
    GPIPMLlightServer();
    // Don't allow copying of this object.
    // <group>
    GPIPMLlightServer (const GPIPMLlightServer&);
    GPIPMLlightServer& operator= (const GPIPMLlightServer&);
    // </group>

	private: // data members
    // the PMLlight server port
		TM::GCFTCPPort        _clientPort;
    TM::GCFTCPPort        _propertyAgent;
    // PI centre
    GPIController&    _controller;
    typedef map<string /*scope*/, GPIPropertySet*> TPropertySets;
    TPropertySets     _propertySets;

  private: // admin. data members
    typedef map<unsigned short /*seqnr*/, GPIPropertySet*>  TActionSeqList;
    TActionSeqList _actionSeqList;    
    int _timerID;
};

inline TM::GCFTCPPort& GPIPMLlightServer::getClientPort()    
  { return _clientPort; }
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
  
#endif
