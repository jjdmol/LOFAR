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

#include <PA_Protocol.ph>
#include "PI_Protocol.ph"
#include "GPI_Defines.h"

#include <Common/lofar_list.h>
#include <GCF/PAL/GCF_PropertyProxy.h>

class GCFPValue;
class GPIPMLlightServer;

/**
 * This class provides for the GPISupervisoryServer class the possibility to 
 * (un)subscribe on/from properties, which requested from the Property Agent to 
 * (un)link to a property of the ERTC domain. Furthermore it forwards property 
 * value changes, detected by the SCADA system, to the right 
 * GPISupervisoryServer instance.
 */
class GPIPropertySet : public GCFPropertyProxy
{
  public:
    GPIPropertySet(GPIPMLlightServer& pls) : 
      _pls(pls), 
      _state(S_DISABLED),
      _counter(0),
      _missing(0),
      _tmpPIResult(PI_NO_ERROR) {}
    virtual ~GPIPropertySet() {assert(_state == S_DISABLED);}

    void enable(PIRegisterScopeEvent& requestIn);
    void retryEnable();
    void enabled(TPAResult result);
    
    void disable(PIUnregisterScopeEvent& requestIn);
    void disabled(TPAResult result);

    void linkPropSet(PALinkPropSetEvent& requestIn);
    bool propSetLinkedInRTC(PIPropSetLinkedEvent& responseIn);
    bool trySubscribing();

    void unlinkPropSet(PAUnlinkPropSetEvent& requestIn);
    void propSetUnlinkedInRTC(PIPropSetUnlinkedEvent& responseIn);
    
    const string& getScope() const {return _scope;}
    
  private:
    void propSubscribed(const string& propName);
    void propSubscriptionLost(const string& /*propName*/) {;}
    void propUnsubscribed(const string& /*propName*/) {;}
    void propValueGet(const string& /*propName*/, const GCFPValue& /*value*/) {;}
    void propValueChanged(const string& propName, const GCFPValue& value);
  
  private: //helper methods
    void sendMsgToPA(GCFEvent& msg);
    void sendMsgToRTC(GCFEvent& msg);
    void wrongState(const char* request);
    
    void propSetLinkedInPI(TPAResult result);
    void propSetUnlinkedInPI(TPAResult result);
  private:
    GPIPropertySet();
    /**
     * Don't allow copying of this object.
     */
    GPIPropertySet (const GPIPropertySet&);
    GPIPropertySet& operator= (const GPIPropertySet&);
  
    
  private:
    GPIPMLlightServer& _pls;
    string _scope;
    string _type;
    bool _isTemporary;
    unsigned long _savedSeqnr;
    
    typedef enum TSTATE {S_DISABLED, S_DISABLING, S_ENABLING, S_ENABLED, 
                         S_LINKING, S_LINKED, S_UNLINKING, S_DELAYED_DISABLING};
    TSTATE _state;
    
  private:
    unsigned int  _counter;
    unsigned int  _missing;
    TPIResult     _tmpPIResult;
    list<string>  _propsSubscribed;
    
};    
#endif
