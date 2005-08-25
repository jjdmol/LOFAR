//#  MISSubscription.cc: Implementation of the Virtual MISSubscription task
//#
//#  Copyright (C) 2002-2004
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

#include <lofar_config.h>

#include "MISSubscription.h"
#include "MISSession.h"
#include <MIS_Protocol.ph>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/GCF_PValue.h>

namespace LOFAR
{

using namespace GCF::Common;
using namespace GCF::PAL;
  
  namespace AMI
  {
    
INIT_TRACER_CONTEXT(MISSubscription, LOFARLOGGER_PACKAGE);

     
MISSubscription::MISSubscription (MISSession& session, const string& propName, uint64 replyNr, bool onlyOnce) :
  GCFPropertyProxy(),
  _session(session),
  _propName(propName),
  _curReplySeqNr(replyNr), 
  _onlyOnce(onlyOnce),
  _pFirstValue(0),
  _isSubscribed(false)  
{
}

MISSubscription::~MISSubscription ()
{
  if (_isSubscribed && GCFPVSSInfo::propExists(_propName))
  {
    unsubscribeProp(_propName);
  }
  if (_pFirstValue) delete _pFirstValue;
}

void MISSubscription::subscribe()
{
  string response("ACK");
  if (GCFPVSSInfo::propExists(_propName))
  {
    if (requestPropValue(_propName) != GCF_NO_ERROR)    
    {
      response = "NAK (could not retrieve first value)";
    }
  }
  else
  {
    response = "NAK (DPE does not exist)";
  }  
  
  if (response != "ACK")
  {   
    MISPvssDpSubscriptionResponseEvent nak;
    nak.replynr = _curReplySeqNr;
    MISSession::setCurrentTime(nak.timestamp_sec, nak.timestamp_nsec);
    nak.response = response;
    _session.subscribed(nak);    
    _session.mayDelete(_propName);
  }
}

void MISSubscription::unsubscribe(uint64 seqnr)
{
  string response("ACK");
  if (GCFPVSSInfo::propExists(_propName))
  {
    if (unsubscribeProp(_propName) != GCF_NO_ERROR)
    {
      response = "NAK (could not unsubscribe)";
    }
  }
  else
  {
    response = "NAK (DPE does not exist)";
  }  
  _isSubscribed = false;
  MISPvssDpSubscriptionResponseEvent resp;
  resp.replynr = seqnr;
  MISSession::setCurrentTime(resp.timestamp_sec, resp.timestamp_nsec);
  resp.response = response;
  _session.subscribed(resp);    
  _session.mayDelete(_propName);
}

void MISSubscription::propSubscribed (const string& propName)
{
  ASSERTSTR(propName.find(_propName) < string::npos, "Propnames should be the same");
  MISPvssDpSubscriptionResponseEvent ack;
  ack.replynr = _curReplySeqNr;
  MISSession::setCurrentTime(ack.timestamp_sec, ack.timestamp_nsec);
  ack.response = "ACK";
  ASSERTSTR(_pFirstValue, "Value was not requested from PVSS");
  ack.dptype = _pFirstValue->getTypeName();
  _isSubscribed = true;
  _session.subscribed(ack);    
  propValueChanged(_propName, *_pFirstValue);  
}

void MISSubscription::propValueChanged (const string& propName, const GCFPValue& newValue)
{
  ASSERTSTR(propName.find(_propName) < string::npos, "Propnames should be the same");
  MISPvssDpSubscriptionValueChangedAsyncEvent vce;
  vce.replynr = _curReplySeqNr;
  MISSession::setCurrentTime(vce.timestamp_sec, vce.timestamp_nsec);
  timeval ts = GCFPVSSInfo::getLastEventTimestamp();
  vce.payload_timestamp_sec = ts.tv_sec;
  vce.payload_timestamp_nsec = ts.tv_usec * 1000;
  vce.value = newValue.getValueAsString();
  _session.valueChanged(vce);
}

void MISSubscription::propValueGet (const string& propName, const GCFPValue& newValue)
{
  ASSERTSTR(propName.find(_propName) < string::npos, "Propnames should be the same");
  _pFirstValue = newValue.clone();
  if (_onlyOnce)
  {
    propSubscribed (_propName);
    _isSubscribed = false;
    _session.mayDelete(_propName);    
  }
  else
  {
    if (subscribeProp(_propName) != GCF_NO_ERROR)
    {
      MISPvssDpSubscriptionResponseEvent nak;
      nak.replynr = _curReplySeqNr;
      MISSession::setCurrentTime(nak.timestamp_sec, nak.timestamp_nsec);
      nak.response = "NAK (could not subscribe)";
      _session.subscribed(nak);    
      _session.mayDelete(_propName);
    }
  }
}

void MISSubscription::propSubscriptionLost(const string& propName)
{
  ASSERTSTR(propName.find(_propName) < string::npos, "Propnames should be the same");
  LOG_DEBUG(formatString (
      "Lost subscription of %s",
      propName.c_str()));
  _isSubscribed = false;
  _session.mayDelete(_propName);
}
  } // namespace AMI
} // namespace LOFAR
