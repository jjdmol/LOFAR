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

#include <GCF/GCF_Defines.h>

// This abstract class provides the possibility to (un)subscribe from/on, set or 
// get property (values) in the SCADA DB and handle their responses. Opposite to 
// classes based on the GCFPropertyBase class this class has no knowledge about 
// the property itself but only which actions on properties are possible. So it 
// could happen that APL can perform multiple subscriptions on a certain property 
// by means of the same proxy. Then multiple value-changed indications on one 
// value change in the SCADA DB will be received. In most of the cases this is 
// not wanted. Always use this class with cautions.

namespace LOFAR {
 namespace GCF {
  namespace Common {
	class Common::GCFPValue;
  }
  namespace PAL {
   class GPMPropertyProxy;

class GCFPropertyProxy
{
public:
    GCFPropertyProxy ();
    virtual ~GCFPropertyProxy ();

    // Asynchronous request (results in response 'propSubscribed' and 
    // indication(s) 'propValueChanged')
    // Note that subscription could be made multiple times on the same 
    // property, but this results in multiple 'propValueChanged' indications too
    // 
    // @returns GCF_NO_ERROR, GCF_PML_ERROR (see in logging whats wrong)
    virtual Common::TGCFResult subscribeProp (const string& propName);

    // Synchronous (!) request
    // Note that subscription could be made multiple times on the same 
    // property. They had to be undone by multipe unsubscription too to avoid 
    // more 'propValueChanged' indications
    // 
    // @returns GCF_NO_ERROR, GCF_PML_ERROR (see in logging whats wrong)
    virtual Common::TGCFResult unsubscribeProp (const string& propName);

    // Asynchronous request (results in response 'propValueGet')
    // 
    // @returns GCF_NO_ERROR, GCF_PML_ERROR (see in logging whats wrong)
    virtual Common::TGCFResult requestPropValue (const string& propName);

    // Synchronous (!) request
    // 
    // @returns GCF_NO_ERROR, GCF_PML_ERROR (see in logging whats wrong)
    virtual Common::TGCFResult setPropValue (const string& propName, 
                                     const Common::GCFPValue& value, 
                                     bool wantAnswer = false);

    // @returns GCF_NO_ERROR, GCF_PML_ERROR (see in logging whats wrong)
    virtual Common::TGCFResult setPropValueTimed (const string& propName, 
                                     const Common::GCFPValue& value, 
                                     double timestamp,
                                     bool wantAnswer = false);

    virtual Common::TGCFResult setPropValue (const string& propName, 
                                     const string& value, 
                                     bool wantAnswer = false);

    // @returns GCF_NO_ERROR, GCF_PML_ERROR (see in logging whats wrong)
    virtual Common::TGCFResult setPropValueTimed (const string& propName, 
                                     const string& value, 
                                     double timestamp,
                                     bool wantAnswer = false);

    virtual Common::TGCFResult dpQuerySubscribeSingle(const string& queryWhere, 
                                                      const string& queryFrom);
    virtual Common::TGCFResult dpQueryUnsubscribe(uint32 queryId);

protected:
    friend class GPMPropertyProxy;

    // Response on 'subscribeProp' request
    virtual void propSubscribed (const string& propName) = 0;

    // Indication that the subscription is lost
    virtual void propSubscriptionLost (const string& propName) = 0;

    // Response on 'unsubscribeProp' request (NOT USED)
    virtual void propUnsubscribed (const string& propName) = 0;

    // Response on 'requestPropValue' request
    virtual void propValueGet (const string& propName, 
                               const Common::GCFPValue& value) = 0;

    // Indication after propSubscribed is received successfully
    virtual void propValueChanged (const string& propName, 
                                   const Common::GCFPValue& value) = 0;
  
    // Response on 'setPropValue' request, only if wantAnswer was set to true
    virtual void propValueSet (const string& propName) = 0;

    virtual void dpQuerySubscribed(uint32 queryId);

private:
    // Don't allow copying this object.
    // <group>
    GCFPropertyProxy (const GCFPropertyProxy&);
    GCFPropertyProxy& operator= (const GCFPropertyProxy&);
    // </group>
  
    GPMPropertyProxy* _pPMProxy;
};

inline Common::TGCFResult GCFPropertyProxy::setPropValue (const string& propName, 
                                     const Common::GCFPValue& value, 
                                     bool wantAnswer)
{
  return setPropValueTimed(propName, value, 0.0, wantAnswer);
}

inline Common::TGCFResult GCFPropertyProxy::setPropValue (const string& propName, 
                                     const string& value, 
                                     bool wantAnswer)
{
  return setPropValueTimed(propName, value, 0.0, wantAnswer);
}

inline void GCFPropertyProxy::dpQuerySubscribed(uint32 /*queryId*/)
{
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
