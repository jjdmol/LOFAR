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

class GPMPropertyService;
class GCFPropertySet;
class GCFAnswer;
class GCFEvent;
class GCFPValue;

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
    virtual TGCFResult requestValue ();
      
    // Synchronous (!) action
    // Performs a set operation on the SCADA DB
    // @return can be GCF_NO_ERROR or GCF_PML_ERROR
    virtual TGCFResult setValue(const GCFPValue& value, bool wantAnswer = false);
      
    // Synchronous (!) action
    // Performs a set operation on the SCADA DB
    // @return can be GCF_NO_ERROR or GCF_PML_ERROR
    virtual TGCFResult setValue(const string& value, bool wantAnswer = false);

    // Synchronous (!) action
    // Checks whether the property exists in the SCADA DB or not
    virtual bool exists ();
      
    virtual void setAnswer (GCFAnswer* pAnswerObj) 
      {_pAnswerObj = pAnswerObj;}
      
  protected:
    GCFProperty (const TPropertyInfo& propInfo, GCFPropertySet* pPropertySet);
    virtual ~GCFProperty ();

    virtual TGCFResult subscribe ();
    virtual TGCFResult unsubscribe ();
      
    virtual void dispatchAnswer(GCFEvent& answer);  
    
    virtual void subscribed ();
    
    virtual void subscriptionLost () {};

    virtual void valueChanged (const GCFPValue& value); 
    
    virtual void valueGet (const GCFPValue& value); 
  
    virtual void valueSet (); 

  protected: // helper attr.
    bool _isBusy;    

  private:
    friend class GPMPropertyService;
    void propSubscribed (const string& propName)
      { assert(propName == getFullName()); subscribed(); }
   
   void propSubscriptionLost (const string& propName)
      { assert(propName == getFullName()); subscriptionLost(); }
    
    // does nothing (but still in the GCF API)
    // because unsubscribe isn't asynchronous in the SCADA API
    void propUnsubscribed (const string& propName)
      { assert(propName == getFullName()); }
      
    void propValueGet (const string& propName, 
                              const GCFPValue& value)
      { assert(propName == getFullName()); valueGet(value); }
      
    void propValueChanged (const string& propName, 
                                  const GCFPValue& value)
      { assert(propName == getFullName()); valueChanged(value); }
  
    void propValueSet (const string& propName)
      { assert(propName == getFullName()); valueSet(); }

  private:
    friend class GCFPropertySet;
    // Normally a property object can only constructed by a property set object.
    // The constructor and destructor are therefor protected. At this way the 
    // property objects are protected from deleting by others than property 
    // sets. 
    // There is one exception: a GCFExtProperty can be instantiated by a user.
    // In that case the instance is not related to a property set and should 
    // therefor be able to delete by the user. This conflicts with the above 
    // description. Therefor the property set objects gets by this method the 
    // possibility to uncouple the properties before deleting them (in destructor
    // of GCFPropertySet). So the destruction of a property is made dependent on
    // the value of the pointer to a property set. If 0 it may be deleted 
    // otherwise we get an assert!!!
    void resetPropSetRef () 
      { _pPropertySet = 0; }
    
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
    TPropertyInfo  _propInfo;

  private: // admin. data members
};
#endif
