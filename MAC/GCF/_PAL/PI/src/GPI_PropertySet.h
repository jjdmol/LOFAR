//#  GPI_PropertySet.h: the concrete property proxy for the Property Interface 
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

#ifndef GPI_PROPERTYSET_H
#define GPI_PROPERTYSET_H

#include <GCF/Protocols/PI_Protocol.ph>
#include <GCF/Protocols/PA_Protocol.ph>
#include "GPI_Defines.h"

#include <GCF/PAL/GCF_PropertyProxy.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace Common
  {
class GCFPValue;
  }
  namespace PAL
  {
class GPIPMLlightServer;

// This class represents a registered property sets. The idea of this class is 
// the same as the GPAPropertySet, with some minor differences. 

class GPIPropertySet : public GCFPropertyProxy
{
  public:
    GPIPropertySet(GPIPMLlightServer& server) : 
      _server(server),       
      _scope(""),
      _type(""),
      _category(Common::PS_CAT_TEMPORARY),
      _state(S_DISABLED),
      _counter(0),
      _missing(0),
      _tmpPIResult(PI_NO_ERROR) 
      {}
      
    virtual ~GPIPropertySet();

    // handling the enable request of a PI client
    void enable(const PIRegisterScopeEvent& requestIn);
    // retry handling the enable request of a PI client if property agent 
    // was not available at the initial enable request time
    void retryEnable();
    // PA responses the enable request
    void enabled(TPAResult result);
    
    // handling the disable request of a PI client
    void disable(const PIUnregisterScopeEvent& requestIn);
    void disabling(unsigned int seqnr);
    // PA responses the disable request
    void disabled(const PAScopeUnregisteredEvent& responseIn);

    // handling the link request of the PA
    void linkPropSet(const PALinkPropSetEvent& requestIn);
    // handling the link request of the PA
    void linkCEPPropSet(const PALinkPropSetEvent& requestIn);
    // PI client reponses the link request
    // @return false if no subscription could be made
    bool propSetLinkedInClient(const PIPropSetLinkedEvent& responseIn);
    // if the subscription could not be made in the propSetLinkedInClient method
    // in the server a timer will be started, which enables the possibility
    // to retry the subscription (until this succeeds)
    bool trySubscribing();

    // handling the unlink request of the PA
    void unlinkPropSet(const PAUnlinkPropSetEvent& requestIn);
    // handling the unlink request of the PA
    void unlinkCEPPropSet(const PAUnlinkPropSetEvent& requestIn);
    // PI client reponses the unlink request
    void propSetUnlinkedInClient(const PIPropSetUnlinkedEvent& responseIn);
    
    const string& getScope() const {return _scope;}
    
  private:
    void propSubscribed(const string& propName);
    void propSubscriptionLost(const string& /*propName*/) {}
    void propUnsubscribed(const string& /*propName*/) {}
    void propValueGet(const string& /*propName*/, const Common::GCFPValue& /*value*/) {}
    void propValueChanged(const string& propName, const Common::GCFPValue& value);
    void propValueSet(const string& /*propName*/) {}
  
  private: //helper methods
    TPAResult unsubscribeAllProps();
    void sendMsgToPA(TM::GCFEvent& msg);
    void sendMsgToClient(TM::GCFEvent& msg);
    void wrongState(const char* request);
    
    void propSetLinkedInPI(TPAResult result);
    void propSetUnlinkedInPI(TPAResult result);
  
  private:
    GPIPropertySet();
    //Don't allow copying of this object.
    // <group>
    GPIPropertySet (const GPIPropertySet&);
    GPIPropertySet& operator= (const GPIPropertySet&);
    // </group>  
    
  private: // data members
    GPIPMLlightServer& _server;
    string _scope;
    string _type;
    Common::TPSCategory _category;
    
    typedef enum TState {S_DISABLED, S_DISABLING, S_DELAYED_DISABLING, 
                         S_ENABLING, S_ENABLED, 
                         S_LINKING, S_LINKING_IN_CLIENT, S_LINKED, 
                         S_UNLINKING};
    TState _state;
    
  private: // adminstrative members
    unsigned long _savedSeqnr;
    unsigned int  _counter;
    unsigned int  _missing;
    TPIResult     _tmpPIResult;
    list<string>  _propsSubscribed;
    
};    

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

#include "GPI_PMLlightServer.h"

using namespace LOFAR::GCF;

inline void PAL::GPIPropertySet::sendMsgToPA(TM::GCFEvent& msg) 
  { _server.sendMsgToPA(msg); }
  
inline void PAL::GPIPropertySet::sendMsgToClient(TM::GCFEvent& msg)  
  { _server.sendMsgToClient(msg); }

#endif
