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

class GPMPropertyProxy : public GSAService
{
  public:
    GPMPropertyProxy(GCFPropertyProxy& gcfProxy) : _gcfProxy(gcfProxy) {;}
    virtual ~GPMPropertyProxy() {;}

    inline TSAResult subscribePM(const string& propName)
    {
      return GSAService::dpeSubscribe(propName);
    }
    inline TSAResult unsubscribePM(const string& propName)
    {
      return GSAService::dpeUnsubscribe(propName);
    }
    inline TSAResult getPM(const string& propName)
    {
      return GSAService::dpeGet(propName);
    }
    inline TSAResult setPM(const string& propName, const GCFPValue& value)
    {
      return GSAService::dpeSet(propName, value);
    }

  protected:
    inline void dpCreated(const string& /*propName*/) {};
    inline void dpDeleted(const string& /*propName*/) {};
    inline void dpeSubscribed(const string& propName)
    {
      _gcfProxy.propSubscribed(propName);
    }
    inline void dpeUnsubscribed(const string& propName)
    {
      _gcfProxy.propUnsubscribed(propName);
    }
    inline void dpeValueGet(const string& propName, const GCFPValue& value)
    {
      _gcfProxy.propValueGet(propName, value);
    }
    inline void dpeValueChanged(const string& propName, const GCFPValue& value)
    {
      _gcfProxy.propValueChanged(propName, value);
    }

  private:
    GCFPropertyProxy& _gcfProxy;
};
#endif

