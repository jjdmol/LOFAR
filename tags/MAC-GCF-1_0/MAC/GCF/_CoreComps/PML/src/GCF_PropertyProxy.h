//#  GCF_PropertyProxy.h: abstract class provides the possibility to 
//#                       (un)subscribe from/on, set or get property (values)  
//#                       in the SCADA DB and handle their responses. 
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

#include <lofar_config.h>
#ifdef HAVE_LOFAR_PML
#include <GCFCommon/GCF_Defines.h>
#else
#include <GCF_Defines.h>
#endif

/** 
  This abstract class provides the possibility to (un)subscribe from/on, set or 
  get property (values) in the SCADA DB and handle their responses. Opposite to 
  classes based on the GCFPropertyBase class this class has no knowledge about 
  the property itself but only which actions on properties are possible. So it 
  could happen that APL can perform multiple subscriptions on a certain property 
  by means of the same proxy. Then multiple value-changed indications on one 
  value change in the SCADA DB will be received. In most of the cases this is 
  not wanted. Always use this class with cautions.
*/
class GPMPropertyProxy;
class GCFPValue;

class GCFPropertyProxy
{
  public:
    GCFPropertyProxy ();
    virtual ~GCFPropertyProxy ();

    TGCFResult subscribe (const string& propName);
    TGCFResult unsubscribe (const string& propName);
    TGCFResult get (const string& propName);
    TGCFResult set (const string& propName, 
                    const GCFPValue& value);
    bool exists (const string& propName);

  protected:
    friend class GPMPropertyProxy;
    virtual void propSubscribed (const string& propName) = 0;
    virtual void propUnsubscribed (const string& propName) = 0;
    virtual void propValueGet (const string& propName, 
                               const GCFPValue& value) = 0;
    virtual void propValueChanged (const string& propName, 
                                   const GCFPValue& value) = 0;
  
  private:
    /**
    * Don't allow copying this object.
    */
    GCFPropertyProxy (const GCFPropertyProxy&);
    GCFPropertyProxy& operator= (const GCFPropertyProxy&);
  
    GPMPropertyProxy* _pPMProxy;
};
#endif
