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
#include <SAL/GSA_Service.h>
//#include "GCF_PropertyProxy.h"

class GCFPropertyProxy;

class GPMPropertyProxy : public GSAService
{
  public:
    GPMPropertyProxy(GCFPropertyProxy& gcfProxy);
    ~GPMPropertyProxy();

/*    GPMPropertyProxy(GCFPropertyProxy& gcfProxy) : _gcfProxy(gcfProxy) {;}
    virtual ~GPMPropertyProxy() {;}

    inline TSAResult subscribe(const string& propName)
    {
      return GSAService::subscribe(propName);
    }
    inline TSAResult unsubscribe(const string& propName)
    {
      return GSAService::unsubscribe(propName);
    }
    inline TSAResult get(const string& propName)
    {
      return GSAService::get(propName);
    }
    inline TSAResult set(const string& propName, const GCFPValue& value)
    {
      return GSAService::set(propName, value);
    }
    inline bool exists(const string& propName)
    {
      return GSAService::exists(propName);
    }
*/
    TPMResult subscribePM(const string& propName);
    TPMResult unsubscribePM(const string& propName);
    TPMResult getPM(const string& propName);
    TPMResult setPM(const string& propName, const GCFPValue& value);
    bool existsPM(const string& propName);

  protected:
/*    inline void propCreated(string& propName) {};
    inline void propDeleted(string& propName) {};
    inline void propSubscribed(string& propName)
    {
      _gcfProxy.propSubscribed(propName);
    }
    inline void propUnsubscribed(string& propName)
    {
      _gcfProxy.propUnsubscribed(propName);
    }
    inline void propValueGet(string& propName, GCFPValue& value)
    {
      _gcfProxy.propValueGet(propName, value);
    }
    inline void propValueChanged(string& propName, GCFPValue& value)
    {
      _gcfProxy.propValueChanged(propName, value);
    }
*/
    void propCreated(const string& propName);
    void propDeleted(const string& propName);
    void propSubscribed(const string& propName);
    void propUnsubscribed(const string& propName);
    void propValueGet(const string& propName, const GCFPValue& value);
    void propValueChanged(const string& propName, const GCFPValue& value);

  private:
    GCFPropertyProxy& _gcfProxy;
};
#endif
