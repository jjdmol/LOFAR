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

#include <GCF/GCF_PropertyProxy.h>
#include <GCF/GCF_Defines.h>

class GPMPropertyService;
class GCFPropertySet;
class GCFAnswer;
class GCFEvent;

/** 
 * This is the base class for 2 types of properties. The owned (GCFMyProperty) 
 * and the external (GCFExtProperty) property. This class manages the: 
 *  - name of the property, which points, together with the scopes of the 
 *    related property set, to a property in the SCADA DB 
 *  - answer object, which will be used for responses and indications from the 
 *    SCADA system if specified.  
 * 
 * It uses the GCFPropertyProxy interface and provides the possibility to create 
 * and manage property objects by its name, which refers to one single SCADA 
 * property if existing.
*/

class GCFProperty
{
  public:
    inline const string& getName () const 
      { return propInfo.propName;}
      
    /// @return the given property name including the scope of the related property set
    virtual const string getFullName () const;
    
    /**
     * Asynchronous action
     * Performs a get operation on the SCADA DB
     * @return can be GCF_NO_ERROR or GCF_PML_ERROR
     */ 
    virtual TGCFResult requestValue ();
      
    /**
     * Synchronous (!) action
     * Performs a set operation on the SCADA DB
     * @return can be GCF_NO_ERROR or GCF_PML_ERROR
     */ 
    virtual TGCFResult setValue(const GCFPValue& value);
      
    /**
     * Synchronous (!) action
     * Checks whether the property exists in the SCADA DB or not
     */ 
    virtual bool exists ();
      
    inline virtual void setAnswer (GCFAnswer* pAnswerObj) 
      {_pAnswerObj = pAnswerObj;}
      
  protected:
    GCFProperty (const TProperty& propInfo, GCFPropertySet* pPropertySet);
    virtual ~GCFProperty ();

  protected:
    virtual TGCFResult subscribe ();
    virtual TGCFResult unsubscribe ();
      
    virtual void dispatchAnswer(GCFEvent& answer);  
    
    virtual void subscribed ();
    
    virtual void valueChanged (const GCFPValue& value); 
    
    virtual void valueGet (const GCFPValue& value); 
  
  private:
    friend class GPMPropertyService;
    inline void propSubscribed (const string& propName)
      { assert(propName == getFullName()); subscribed(); }
      
    /** 
     * does nothing (but still in the GCF API)
     * because unsubscribe isn't asynchronous in the SCADA API
     */
    inline void propUnsubscribed (const string& propName)
      { assert(propName == getFullName()); }
      
    inline void propValueGet (const string& propName, 
                              const GCFPValue& value)
      { assert(propName == getFullName()); valueGet(value); }
      
    inline void propValueChanged (const string& propName, 
                                  const GCFPValue& value)
      { assert(propName == getFullName()); valueChanged(value); }
  
  private:
    friend class GCFPropertySetBase;
    inline void resetPropSetRef () 
      { _pPropertySet = 0; }
    
  private:
    GCFProperty();
    //@{ 
    /// Copy contructors. Don't allow copying this object.
    GCFProperty (const GCFProperty&);
    GCFProperty& operator= (const GCFProperty&);
    //@}

  private: // data members
    GCFPropertySetBase* _pPropertySet;
    GCFAnswer*          _pAnswerObj;
    string              _name;
    GPMPropertyService* _pPropService;

  private: // admin. data members
    bool _isBusy;    
    const TProperty&    _propInfo;
};
#endif
