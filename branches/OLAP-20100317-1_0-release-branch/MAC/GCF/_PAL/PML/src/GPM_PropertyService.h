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
#include <GCF/PAL/GCF_Property.h>

namespace LOFAR {
 namespace GCF {
  namespace PAL {
   
class GPMPropertyService : public GSAService
{
public:
    GPMPropertyService(GCFProperty& gcfProperty);
    virtual ~GPMPropertyService();

    Common::TGCFResult subscribeProp(const string& propName);
    Common::TGCFResult unsubscribeProp(const string& propName);
    Common::TGCFResult requestPropValue(const string& propName);
    Common::TGCFResult setPropValue(const string& propName, 
                                    const Common::GCFPValue& value, 
                                    double timestamp, 
                                    bool wantAnswer);
        
protected:
    void dpCreated(const string& /*propName*/);
    void dpDeleted(const string& /*propName*/);
    void dpeSubscribed(const string& propName);
    void dpeSubscriptionLost (const string& propName);
    void dpeUnsubscribed(const string& propName);
    void dpeValueGet(const string& propName, const Common::GCFPValue& value);
    void dpeValueChanged(const string& propName, const Common::GCFPValue& value);
    void dpeValueSet(const string& propName);
    
private:
    GCFProperty& _gcfProperty;
};


inline GPMPropertyService::GPMPropertyService(GCFProperty& gcfProperty) : 
  _gcfProperty(gcfProperty) 
{}

inline GPMPropertyService::~GPMPropertyService() 
{}
    
inline Common::TGCFResult GPMPropertyService::subscribeProp(const string& propName)
{
  return (dpeSubscribe(propName) == SA_NO_ERROR ? Common::GCF_NO_ERROR : Common::GCF_PML_ERROR);
}

inline Common::TGCFResult GPMPropertyService::unsubscribeProp(const string& propName)
{
  return (dpeUnsubscribe(propName) == SA_NO_ERROR ? Common::GCF_NO_ERROR : Common::GCF_PML_ERROR);
}

inline Common::TGCFResult GPMPropertyService::requestPropValue(const string& propName)
{
  return (dpeGet(propName) == SA_NO_ERROR ? Common::GCF_NO_ERROR : Common::GCF_PML_ERROR);
}

inline Common::TGCFResult GPMPropertyService::setPropValue(const string& propName, 
                                                   const Common::GCFPValue& value, 
                                                   double timestamp, 
                                                   bool wantAnswer)
{
  return (dpeSet(propName, value, timestamp, wantAnswer) == SA_NO_ERROR ? Common::GCF_NO_ERROR : Common::GCF_PML_ERROR);
}

inline void GPMPropertyService::dpCreated(const string& /*propName*/) 
{}

inline void GPMPropertyService::dpDeleted(const string& /*propName*/) 
{}

inline void GPMPropertyService::dpeSubscribed(const string& propName)
{
  _gcfProperty.propSubscribed(propName);
}

inline void GPMPropertyService::dpeSubscriptionLost (const string& propName)
{
  _gcfProperty.propSubscriptionLost(propName);
}     

inline void GPMPropertyService::dpeUnsubscribed(const string& propName)
{
  _gcfProperty.propUnsubscribed(propName);
}

inline void GPMPropertyService::dpeValueGet(const string& propName, const Common::GCFPValue& value)
{
  _gcfProperty.propValueGet(propName, value);
}

inline void GPMPropertyService::dpeValueChanged(const string& propName, const Common::GCFPValue& value)
{
  _gcfProperty.propValueChanged(propName, value);
}

inline void GPMPropertyService::dpeValueSet(const string& propName)
{
  _gcfProperty.propValueSet(propName);
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

#endif
