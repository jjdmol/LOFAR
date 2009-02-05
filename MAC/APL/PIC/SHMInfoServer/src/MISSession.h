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

#include <Common/lofar_bitset.h>
#include <Common/LofarConstants.h>  //MAXMOD
#include <GCF/TM/GCF_Control.h>
#include <MACIO/MACServiceInfo.h>

//#include <PropertyProxy.h>
//#include <MISSubscription.h>

#include <MIS_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>

namespace blitz {
	template<typename, int > class Array;
}

namespace LOFAR {
	using MACIO::GCFEvent;
	namespace AMI {  

class MISDaemon;
//class MISSubsciption;
/**
*/

class MISSession : public GCF::TM::GCFTask
{
public:
    MISSession (MISDaemon& daemon);
    virtual ~MISSession ();
    
	// member functions
//    void subscribed(MISPvssDpSubscriptionResponseEvent& e);
//    void valueChanged(MISPvssDpSubscriptionValueChangedAsyncEvent& e);
    void mayDelete(const string& propName);
    static void setCurrentTime(int64& sec, uint32& nsec);
  
private: 
	// state methods
    GCFEvent::TResult initial_state                (GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCFEvent::TResult waiting_state                (GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCFEvent::TResult setDiagnosis_state           (GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCFEvent::TResult reconfigure_state            (GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCFEvent::TResult getPICStructure_state        (GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCFEvent::TResult subscribe_state              (GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCFEvent::TResult getSubbandStatistics_state   (GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCFEvent::TResult getAntennaCorrelation_state  (GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCFEvent::TResult getRspStatus_state           (GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCFEvent::TResult closing_state                (GCFEvent& e, GCF::TM::GCFPortInterface& p);
        
    GCFEvent::TResult defaultHandling              (GCFEvent& e, GCF::TM::GCFPortInterface& p);

	// helper methods
    void genericPingpong      (GCFEvent& e);
    void getGenericIdentity   (GCFEvent& e);
    void setDiagnosis         (GCFEvent& e);
    void subscribe            (GCFEvent& e);
    void getSubbandStatistics (GCFEvent& e);
    //MAXMOD
    void getAntennaCorrelation(GCFEvent& e);
    void getRspStatus         (GCFEvent& e);

	// data members      
//    typedef map<string /*resource name*/, MISSubscription*> TSubscriptions;
    GCF::TM::GCFTCPPort _missPort;
    GCF::TM::GCFTCPPort _rspDriverPort;
    GCF::TM::GCFTCPPort _acmPort;
    MISDaemon&          _daemon;
//    TSubscriptions      _subscriptions;
//    PropertyProxy       _propertyProxy;

	// admin members
    uint64                  _curSeqNr;
    uint64                  _curReplyNr;
    bool                    _busy;
//    list<MISSubscription*>  _subscriptionsGarbage;
    GCFEvent*      _pRememberedEvent;
    uint16                  _nrOfRCUs;
    bitset<MEPHeader::MAX_N_RCUS> _allRCUSMask;    
    //bitset<MAX_RCUS> _allRCUSMask;
    bitset<MAX_N_RSPBOARDS> _allRSPSMask;    
};

 } // namespace AMI
} // namespace LOFAR

#endif
