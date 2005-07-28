//#  MISSession.h: 
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

#ifndef NODEMANAGERCLIENT_H
#define NODEMANAGERCLIENT_H

#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <PropertyProxy.h>
#include <MIS_Protocol.ph>
#include <MISSubscription.h>

namespace blitz
{
 template<typename, int > class Array;
}
namespace LOFAR 
{
 namespace AMI
 {  

class MISDaemon;
class MISSubsciption;
/**
*/

class MISSession : public GCF::TM::GCFTask
{
  public:
    MISSession (MISDaemon& daemon);
    virtual ~MISSession ();
    
  public: // member functions
    void subscribed(MISPvssDpSubscriptionResponseEvent& e);
    void valueChanged(MISPvssDpSubscriptionValueChangedAsyncEvent& e);
    void mayDelete(const string& propName);
    static void setCurrentTime(int64& sec, uint32& nsec);
  
  private: // state methods
    GCF::TM::GCFEvent::TResult initial (GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult waiting (GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult setDiagnosis(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult reconfigure(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult getPICStructure(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult subscribe(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult getSubbandStatistics(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult getAntennaCorrelation(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult defaultHandling(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult closing (GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
        
  private: // helper methods
    void genericPingpong(GCF::TM::GCFEvent& e);
    void getGenericIdentity(GCF::TM::GCFEvent& e);
    
  private: // data members      
    typedef map<string /*resource name*/, MISSubscription*> TSubscriptions;
    GCF::TM::GCFTCPPort _missPort;
    GCF::TM::GCFTCPPort _rspDriverPort;
    GCF::TM::GCFTCPPort _acmPort;
    MISDaemon&          _daemon;
    TSubscriptions      _subscriptions;
    PropertyProxy       _propertyProxy;

  private: // admin members
    uint64              _curSeqNr;
    uint64              _curReplyNr;
    bool                _busy;
    list<MISSubscription*>  _subscriptionsGarbage;
};
 } // namespace AMI
} // namespace LOFAR

#endif
