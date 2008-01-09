//#  WanLSPropertyProxy.cc: 
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

#include <lofar_config.h>

#include <GCF/GCF_PVDouble.h>
#include "MACScheduler_Defines.h"
#include "WanLSPropertyProxy.h"

using namespace LOFAR;
using namespace LOFAR::GCF;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{

namespace GSO
{
INIT_TRACER_CONTEXT(WanLSPropertyProxy,LOFARLOGGER_PACKAGE);

WanLSPropertyProxy::WanLSPropertyProxy(const string& propset) : 
  GCFPropertyProxy(),
  m_propSetName(propset),
  m_allocated(0.0),
  m_capacity(0.0)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,m_propSetName.c_str());

  setPropValue(m_propSetName + string(".") + string(MS_LOGICALSEGMENT_PROPNAME_ALLOCATED),GCFPVDouble(0.0));
  setPropValue(m_propSetName + string(".") + string(MS_LOGICALSEGMENT_PROPNAME_CHANGEALLOCATED),GCFPVDouble(0.0));

  subscribeProp(m_propSetName + string(".") + string(MS_LOGICALSEGMENT_PROPNAME_ALLOCATED));
  subscribeProp(m_propSetName + string(".") + string(MS_LOGICALSEGMENT_PROPNAME_CHANGEALLOCATED));

  requestPropValue(m_propSetName + string(".") + string(MS_LOGICALSEGMENT_PROPNAME_CAPACITY));
}

WanLSPropertyProxy::~WanLSPropertyProxy()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,m_propSetName.c_str());

  unsubscribeProp(m_propSetName + string(".") + string(MS_LOGICALSEGMENT_PROPNAME_ALLOCATED));
  unsubscribeProp(m_propSetName + string(".") + string(MS_LOGICALSEGMENT_PROPNAME_CHANGEALLOCATED));
}

double WanLSPropertyProxy::getCapacity() const
{
  return m_capacity;
}

void WanLSPropertyProxy::updateCapacity()
{
  requestPropValue(m_propSetName + string(".") + string(MS_LOGICALSEGMENT_PROPNAME_CAPACITY));
}

void WanLSPropertyProxy::propSubscribed(const string& propName)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,propName.c_str());
}

void WanLSPropertyProxy::propSubscriptionLost(const string& propName)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,propName.c_str());
}

void WanLSPropertyProxy::propUnsubscribed(const string& propName)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,propName.c_str());
}

void WanLSPropertyProxy::propValueGet(const string& propName, const GCFPValue& value)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,propName.c_str());

  // note: check for changeAllocated first
  if (propName.find(MS_LOGICALSEGMENT_PROPNAME_CHANGEALLOCATED) != string::npos)
  {
    const GCFPVDouble& dblRef(static_cast<const GCFPVDouble&>(value));
    double allocationChange = dblRef.getValue();
    LOG_DEBUG(formatString("LogicalSegment %s allocated: %f; change: %f",propName.c_str(),m_allocated,allocationChange));
    m_allocated += allocationChange;
    setPropValue(m_propSetName + string(".") + string(MS_LOGICALSEGMENT_PROPNAME_ALLOCATED),GCFPVDouble(m_allocated));
  }
  else if (propName.find(MS_LOGICALSEGMENT_PROPNAME_ALLOCATED) != string::npos)
  {
    const GCFPVDouble& dblRef(static_cast<const GCFPVDouble&>(value));
    m_allocated = dblRef.getValue();
    LOG_DEBUG(formatString("LogicalSegment %s allocated: %f",propName.c_str(),m_allocated));
  }
  else if (propName.find(MS_LOGICALSEGMENT_PROPNAME_CAPACITY) != string::npos)
  {
    const GCFPVDouble& dblRef(static_cast<const GCFPVDouble&>(value));
    m_capacity = dblRef.getValue();
    LOG_TRACE_FLOW(formatString("LogicalSegment %s capacity: %f",propName.c_str(),m_capacity));
  }
}

void WanLSPropertyProxy::propValueChanged(const string& propName, const GCFPValue& value)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,propName.c_str());

  propValueGet(propName,value);
}

void WanLSPropertyProxy::propValueSet(const string& propName)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,propName.c_str());
}

  } // namespace GSO
} // namespace LOFAR
