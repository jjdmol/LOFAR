//#  GCF_Property.h:  
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

#ifndef GCF_PROPERTY_H
#define GCF_PROPERTY_H

#include <GCF/GCF_Defines.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace Common
  {
class GCFPValue;
  }
  namespace TM
  {
class GCFEvent;
  }
  namespace PAL
  {
class GPMPropertyService;
class GCFPropertySet;
class GCFAnswer;

// This is the base class for 2 types of properties. The owned (GCFMyProperty) 
// and the external (GCFExtProperty) property. This class manages the: 
//  - name of the property, which points, together with the scopes of the 
//    related property set, to a property in the SCADA DB 
//  - answer object, which will be used for responses and indications from the 
//    SCADA system if specified.  
// 
// It uses the GCFPropertyProxy interface and provides the possibility to create 
// and manage property objects by its name, which refers to one single SCADA 
// property if existing.

class GCFProperty
{
  public:
    const string getName () const 
      { return _propInfo.propName;}
      
    // @return the given property name including the scope of the related property set
    virtual const string getFullName () const;
    
    // Asynchronous action
    // Performs a get operation on the SCADA DB
    // @return can be GCF_NO_ERROR or GCF_PML_ERROR
    virtual Common::TGCFResult requestValue ();
      
    // (A)Synchronous (!) action
    // Performs a set operation on the SCADA DB
    // @return can be GCF_NO_ERROR or GCF_PML_ERROR
    virtual Common::TGCFResult setValue(const Common::GCFPValue& value, bool wantAnswer = false);
      
    // (A)Synchronous (!) action
    // Performs a set operation on the SCADA DB
    // @return can be GCF_NO_ERROR or GCF_PML_ERROR
    virtual Common::TGCFResult setValue(const string& value, bool wantAnswer = false);

    // (A)Synchronous (!) action
    // Performs a set operation on the SCADA DB with a timestamp
    // @return can be GCF_NO_ERROR or GCF_PML_ERROR
    virtual Common::TGCFResult setValueTimed(const Common::GCFPValue& value, 
                                             double timestamp, 
                                             bool wantAnswer = false);
      
    // (A)Synchronous (!) action
    // Performs a set operation on the SCADA DB with a timestamp
    // @return can be GCF_NO_ERROR or GCF_PML_ERROR
    virtual Common::TGCFResult setValueTimed(const string& value, 
                                             double timestamp, 
                                             bool wantAnswer = false);

    // Synchronous (!) action
    // Checks whether the property exists in the SCADA DB or not
    virtual bool exists ();
      
    virtual void setAnswer (GCFAnswer* pAnswerObj) 
      {_pAnswerObj = pAnswerObj;}
      
  protected:
    GCFProperty (const Common::TPropertyInfo& propInfo, GCFPropertySet* pPropertySet);
    virtual ~GCFProperty ();

    virtual Common::TGCFResult subscribe ();
    virtual Common::TGCFResult unsubscribe ();
      
    virtual void dispatchAnswer(TM::GCFEvent& answer);  
    
    virtual void subscribed ();
    
    virtual void subscriptionLost () {};

    virtual void valueChanged (const Common::GCFPValue& value); 
    
    virtual void valueGet (const Common::GCFPValue& value); 
  
    virtual void valueSet (); 

  protected: // helper attr.
    bool _isBusy;    

  private:
    friend class GPMPropertyService;
    void propSubscribed (const string& propName)
      { ASSERT(propName == getFullName()); subscribed(); }
   
   void propSubscriptionLost (const string& propName)
      { ASSERT(propName == getFullName()); subscriptionLost(); }
    
    // does nothing (but still in the GCF API)
    // because unsubscribe isn't asynchronous in the SCADA API
    void propUnsubscribed (const string& propName)
      { ASSERT(propName == getFullName()); }
      
    void propValueGet (const string& propName, 
                              const Common::GCFPValue& value)
      { ASSERT(propName == getFullName()); valueGet(value); }
      
    void propValueChanged (const string& propName, 
                                  const Common::GCFPValue& value)
      { ASSERT(propName == getFullName()); valueChanged(value); }
  
    void propValueSet (const string& propName)
      { ASSERT(propName == getFullName()); valueSet(); }

  private:
    friend class GCFPropertySet;
    // Normally a property object can only constructed by a property set object.
    // The constructor and destructor are therefore protected to avoid unlegal 
    // con-/destructions. So the property set objects and the containing property 
    // objects have a strong dependency to eachother. 
    // There is one exception: a GCFExtProperty can be instantiated by a user.
    // In that case the instance is not related to any property set (independent) 
    // and must be  able to be deleted by the user. This conflicts with the above 
    // statement. Therefore the property set objects gets, by means of this 
    // method, the possibility to make all containing properties independent 
    // just right before deleting them (in destructor of the GCFPropertySet). 
    // Then the destruction of a property could be made dependent on
    // the value of the _isIndependedProp. If true it may be deleted 
    // otherwise we get an ASSERT!!!
    void resetPropSetRef () 
      { _isIndependedProp = true; }
    
  private:
    GCFProperty();
    // Don't allow copying this object.
    // <group>
    GCFProperty (const GCFProperty&);
    GCFProperty& operator= (const GCFProperty&);
    // </group>

  private: // data members
    GCFPropertySet*       _pPropertySet;
    GCFAnswer*            _pAnswerObj;
    GPMPropertyService*   _pPropService;
    Common::TPropertyInfo  _propInfo;

  private: // admin. data members
    bool                  _isIndependedProp;
};

inline Common::TGCFResult GCFProperty::setValue (const string& value, bool wantAnswer)
{
  return setValueTimed(value, 0.0, wantAnswer);
}

inline Common::TGCFResult GCFProperty::setValue (const Common::GCFPValue& value, bool wantAnswer)
{
  return setValueTimed(value, 0.0, wantAnswer);
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
