//#  PropertyProxy.cc: 
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

#include <GCF/GCF_PValue.h>

#include "THPVSSBridge.h"
#include "PropertyProxy.h"

using namespace LOFAR;
using namespace LOFAR::GCF;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{
  namespace MACTest
  {
    
PropertyProxy::PropertyProxy(THPVSSBridge& bridge) : GCFPropertyProxy(),
  m_theBridge(bridge)
{
}

PropertyProxy::~PropertyProxy()
{
}

void PropertyProxy::propSubscribed(const string& propName)
{
  m_theBridge.proxyPropSubscribed(propName);
}

void PropertyProxy::propSubscriptionLost(const string& propName)
{
  m_theBridge.proxyPropSubscriptionLost(propName);
}

void PropertyProxy::propUnsubscribed(const string& propName)
{
  m_theBridge.proxyPropUnsubscribed(propName);
}

void PropertyProxy::propValueGet(const string& propName, const GCFPValue& value)
{
  m_theBridge.proxyPropValueGet(propName,value.getValueAsString());
}

void PropertyProxy::propValueChanged(const string& propName, const GCFPValue& value)
{
  m_theBridge.proxyPropValueChanged(propName,value.getValueAsString());
}

void PropertyProxy::propValueSet(const string& propName)
{
  m_theBridge.proxyPropValueSet(propName);
}

  } // namespace MACTest
} // namespace LOFAR
