//#  GCF_PropertyProxy.h: 
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

#ifndef GCF_PROPERTYPROXY_H
#define GCF_PROPERTYPROXY_H

class GPMPropertyProxy;

class GCFPropertyProxy
{
  public:
    GCFPropertyProxy();
    virtual ~GCFPropertyProxy();

    inline TPMResult subscribe(const string& propName)
    {
      return GSAService::subscribe(propName);
    }
    inline TPMResult unsubscribe(const string& propName);
    {
      return GSAService::unsubscribe(propName);
    }
    inline TPMResult get(const string& propName)
    {
      return GSAService::get(propName);
    }
    inline TPMResult set(const string& propName, const GCFPValue& value);
    {
      return GSAService::set(propName, value);
    }
    bool exists(const string& propName);
    {
      return GSAService::exists(propName);
    }

  protected:
    friend class GPMPropertyProxy;
    virtual void propSubscribed(string& propName) = 0;
    virtual void propUnsubscribed(string& propName) = 0;
    virtual void propValueGet(string& propName, GCFPValue& value) = 0;
    virtual void propValueChanged(string& propName, GCFPValue& value) = 0;
  
  private:
    GPMPropertyProxy* _pPMProxy;
};
#endif