//#  MISSubscription.h: 
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

#ifndef MISSUBSCRIPTION_H
#define MISSUBSCRIPTION_H

#include <GCF/PAL/GCF_PropertyProxy.h>

namespace LOFAR 
{
  namespace AMI
  {
class MISSession;

class MISSubscription : public GCF::PAL::GCFPropertyProxy
{
  public:
    MISSubscription (MISSession& session, const string& propName, uint64 replyNr, bool onlyOnce);
    virtual ~MISSubscription ();

    void subscribe();
    void unsubscribe();
    
  private:
    void propSubscribed (const string& propName);
    void propSubscriptionLost (const string& propName);
    void propUnsubscribed (const string& /*propName*/) {}
    void propValueGet (const string& propName, 
                       const GCF::Common::GCFPValue& value);
    void propValueChanged (const string& propName, 
                           const GCF::Common::GCFPValue& value);
    void propValueSet (const string& /*propName*/) {}
    
  private:
    MISSubscription();

    // Don't allow copying this object.
    // <group>
    MISSubscription (const MISSubscription&);
    MISSubscription& operator= (const MISSubscription&);  
    // </group>

  private: // data members
    MISSession&   _session;
    const string  _propName;
    uint64        _curReplySeqNr;
    bool          _onlyOnce;
  
  private:
    GCF::Common::GCFPValue* _pFirstValue;
    
    ALLOC_TRACER_CONTEXT
};

inline MISSubscription::~MISSubscription ()
{
}

 } // namespace AMI
} // namespace LOFAR
#endif
