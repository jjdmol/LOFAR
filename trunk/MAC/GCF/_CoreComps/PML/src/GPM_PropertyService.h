//#  GPM_PropertyService.h: 
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

#ifndef GPM_PROPERTYSERVICE_H
#define GPM_PROPERTYSERVICE_H

#include "GPM_Defines.h"
#include <GSA_Service.h>
#include <GCF/GCF_PropertyProxy.h>

class GPMPropertyService : public GSAService
{
  public:
    GPMPropertyService(GCFPropertyProxy& gcfProxy) : _gcfProxy(gcfProxy) {;}
    virtual ~GPMPropertyService() {;}

    inline TSAResult subscribePM(const string& propName)
    {
      return GSAService::subscribe(propName);
    }
    inline TSAResult unsubscribePM(const string& propName)
    {
      return GSAService::unsubscribe(propName);
    }
    inline TSAResult getPM(const string& propName)
    {
      return GSAService::get(propName);
    }
    inline TSAResult setPM(const string& propName, const GCFPValue& value)
    {
      return GSAService::set(propName, value);
    }
    inline bool existsPM(const string& propName)
    {
      return GSAService::exists(propName);
    }

  protected:
    inline void propCreated(const string& /*propName*/) {};
    inline void propDeleted(const string& /*propName*/) {};
    inline void propSubscribed(const string& propName)
    {
      _gcfProxy.propSubscribed(propName);
    }
    inline void propUnsubscribed(const string& propName)
    {
      _gcfProxy.propUnsubscribed(propName);
    }
    inline void propValueGet(const string& propName, const GCFPValue& value)
    {
      _gcfProxy.propValueGet(propName, value);
    }
    inline void propValueChanged(const string& propName, const GCFPValue& value)
    {
      _gcfProxy.propValueChanged(propName, value);
    }

  private:
    GCFPropertyProxy& _gcfProxy;
};
#endif
