//#  GPI_PropertyProxy.h: 
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

#ifndef GPI_PROPERTYPROXY_H
#define GPI_PROPERTYPROXY_H

#include <PML/GCF_PropertyProxy.h>
#include <SAL/GSA_Service.h>

class GCFPValue;
class GPISupervisoryServer;

class GPIPropertyProxy: public GSAService
{
  public:
    inline GPIPropertyProxy(GPISupervisoryServer& ss) : _ss(ss) {};
    virtual ~GPIPropertyProxy() {;}
    
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

    inline TSAResult subscribe(const string& propName)
    {
      return GSAService::subscribe(propName);
    }
    
    inline TSAResult unsubscribe(const string& propName)
    {
      return GSAService::unsubscribe(propName);
    }

  protected:
    inline void propCreated(const string& /*propName*/) {};
    inline void propDeleted(const string& /*propName*/) {};
    void propSubscribed(const string& propName);
    void propUnsubscribed(const string& propName);
    void propValueGet(const string& /*propName*/, const GCFPValue& /*value*/) {;}
    void propValueChanged(const string& propName, const GCFPValue& value);
  
  private:

    GPISupervisoryServer& _ss;
};
#endif
