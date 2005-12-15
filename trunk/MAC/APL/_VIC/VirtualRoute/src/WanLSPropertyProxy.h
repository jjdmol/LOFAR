//#  WanLSPropertyProxy.h: 
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

#ifndef WANLSPROPERTYPROXY_H
#define WANLSPROPERTYPROXY_H

//# Common Includes
#include <Common/lofar_string.h>

#include <GCF/PAL/GCF_PropertyProxy.h>

namespace LOFAR
{
namespace AVR
{

class WanLSPropertyProxy : public GCF::PAL::GCFPropertyProxy
{
  public:
    WanLSPropertyProxy(const string& propset);
    virtual ~WanLSPropertyProxy();

    double getCapacity() const;
    void changeAllocated(const double allocated);

  protected:    
    virtual void propSubscribed(const string& propName);
    virtual void propSubscriptionLost(const string& propName);
    virtual void propUnsubscribed(const string& propName);
    virtual void propValueGet(const string& propName, const GCF::Common::GCFPValue& value);
    virtual void propValueChanged(const string& propName, const GCF::Common::GCFPValue& value);
    virtual void propValueSet(const string& propName);
  
  private:
    string m_propSetName;
    double m_capacity;

    ALLOC_TRACER_CONTEXT  
};

 } // namespace AVR
} // namespace LOFAR
#endif
