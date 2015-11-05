//#  GPM_PropertyProxy.h: 
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

#ifndef GPM_PROPERTYPROXY_H
#define GPM_PROPERTYPROXY_H

#include "GPM_Defines.h"
#include <GSA_Service.h>
#include <GCF/PAL/GCF_PropertyProxy.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace PAL
  {
class GPMPropertyProxy : public GSAService
{
  public:
    GPMPropertyProxy(GCFPropertyProxy& gcfProxy);
    virtual ~GPMPropertyProxy();

    TSAResult subscribePM(const string& propName);
    TSAResult unsubscribePM(const string& propName);
    TSAResult getPM(const string& propName);
    TSAResult setPM(const string& propName, const Common::GCFPValue& value, 
                    double timestamp, bool wantAnswer);
    TSAResult dpQuerySubscribeSinglePM(const string& queryWhere, 
                                       const string& queryFrom);
    TSAResult dpQueryUnsubscribePM(uint32 /*queryId*/);

  protected:
    void dpCreated(const string& /*propName*/);
    void dpDeleted(const string& /*propName*/);
    void dpeSubscribed(const string& propName);
    void dpeSubscriptionLost (const string& propName);
    void dpeUnsubscribed(const string& propName);
    void dpeValueGet(const string& propName, const Common::GCFPValue& value);
    void dpeValueChanged(const string& propName, const Common::GCFPValue& value);
    void dpeValueSet(const string& propName);
    void dpQuerySubscribed(uint32 queryId);
    
  private:
    GCFPropertyProxy& _gcfProxy;
};

inline GPMPropertyProxy::GPMPropertyProxy(GCFPropertyProxy& gcfProxy) : 
    _gcfProxy(gcfProxy) 
{}

inline GPMPropertyProxy::~GPMPropertyProxy() 
{}

inline TSAResult GPMPropertyProxy::subscribePM(const string& propName)
{
  return GSAService::dpeSubscribe(propName);
}

inline TSAResult GPMPropertyProxy::unsubscribePM(const string& propName)
{
  return GSAService::dpeUnsubscribe(propName);
}

inline TSAResult GPMPropertyProxy::getPM(const string& propName)
{
  return GSAService::dpeGet(propName);
}

inline TSAResult GPMPropertyProxy::setPM(const string& propName, 
                                         const Common::GCFPValue& value, 
                                         double timestamp,
                                         bool wantAnswer)
{
  return GSAService::dpeSet(propName, value, timestamp, wantAnswer);
}

inline TSAResult GPMPropertyProxy::dpQuerySubscribeSinglePM(const string& queryWhere, 
                                                            const string& queryFrom)
{
  return GSAService::dpQuerySubscribeSingle(queryWhere, queryFrom);
}

inline TSAResult GPMPropertyProxy::dpQueryUnsubscribePM(uint32 queryId)
{
  return GSAService::dpQueryUnsubscribe(queryId);
}

inline void GPMPropertyProxy::dpCreated(const string& /*propName*/) 
{}

inline void GPMPropertyProxy::dpDeleted(const string& /*propName*/)
{}

inline void GPMPropertyProxy::dpeSubscribed(const string& propName)
{
  _gcfProxy.propSubscribed(propName);
}

inline void GPMPropertyProxy::dpeSubscriptionLost (const string& propName)
{
  _gcfProxy.propSubscriptionLost(propName);
}     

inline void GPMPropertyProxy::dpeUnsubscribed(const string& propName)
{
  _gcfProxy.propUnsubscribed(propName);
}

inline void GPMPropertyProxy::dpeValueGet(const string& propName, const Common::GCFPValue& value)
{
  _gcfProxy.propValueGet(propName, value);
}

inline void GPMPropertyProxy::dpeValueChanged(const string& propName, const Common::GCFPValue& value)
{
  _gcfProxy.propValueChanged(propName, value);
}

inline void GPMPropertyProxy::dpeValueSet(const string& propName)
{
  _gcfProxy.propValueSet(propName);
}

inline void GPMPropertyProxy::dpQuerySubscribed(uint32 queryId)
{
  _gcfProxy.dpQuerySubscribed(queryId);
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif

