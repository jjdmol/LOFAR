//#  GCF_PropertyBase.h:  
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

#ifndef GCF_PROPERTYBASE_H
#define GCF_PROPERTYBASE_H

#include <GCF/GCF_PropertyProxy.h>
#include <GCF/GCF_Defines.h>

class GCFPropertySetBase;
class GCFAnswer;
class GCFEvent;

/** 
 * This is the base class for 2 types of properties. The owned (GCFMyProperty) 
 * and the external (GCFProperty) property. This class manages the: 
 *  - name of the property, which points, together with the scopes of the 
 *    related property set, to a property in the SCADA DB 
 *  - answer object, which will be used for responses and indications from the 
 *    SCADA system if specified.  
 * 
 * It uses the GCFPropertyProxy interface and provides the possibility to create 
 * and manage property objects by its name, which refers to one single SCADA 
 * property if existing.
*/

class GCFPropertyBase : public GCFPropertyProxy
{
  public:
    inline const string& getName () const 
      { return _name;}
      
    /// @return the given property name including the scope of the related property set
    virtual const string getFullName () const;
    
    /**
     * Asynchronous action
     * Performs a get operation on the SCADA DB
     * @return can be GCF_NO_ERROR or GCF_PML_ERROR
     */ 
    inline virtual TGCFResult requestValue () 
      { return requestPropValue(getFullName()); }
      
    /**
     * Synchronous (!) action
     * Performs a set operation on the SCADA DB
     * @return can be GCF_NO_ERROR or GCF_PML_ERROR
     */ 
    inline virtual TGCFResult setValue(const GCFPValue& value)
      { return setPropValue(getFullName(), value); }
      
    /**
     * Synchronous (!) action
     * Checks whether the property exists in the SCADA DB or not
     */ 
    inline virtual bool exists () 
      { return GCFPropertyProxy::exists(getFullName()); }
      
    inline virtual void setAnswer (GCFAnswer* pAnswerObj) 
      {_pAnswerObj = pAnswerObj;}
      
  protected:
    GCFPropertyBase (string propName, 
                     GCFPropertySetBase* pPropertySet) : 
                      _pPropertySet(pPropertySet),
                      _pAnswerObj(0),
                      _name(propName)
                      {;}
    virtual ~GCFPropertyBase ();

  protected:
    inline virtual TGCFResult subscribe ()
      { return subscribeProp(getFullName()); }
     
    inline virtual TGCFResult unsubscribe ()
      { return unsubscribeProp(getFullName()); }
      
    virtual void dispatchAnswer(GCFEvent& answer);  
    
    virtual void subscribed ();
    
    virtual void valueChanged (const GCFPValue& value); 
    
    virtual void valueGet (const GCFPValue& value); 
  
  private:
    inline TGCFResult subscribeProp (const string& propName)
      { return GCFPropertyProxy::subscribeProp (propName); }
    
    inline TGCFResult unsubscribeProp (const string& propName)
      { return GCFPropertyProxy::unsubscribeProp (propName); }
      
    inline TGCFResult requestPropValue (const string& propName)
      { return GCFPropertyProxy::requestPropValue (propName); }

    inline TGCFResult setPropValue (const string& propName, 
                           const GCFPValue& value)
      { return GCFPropertyProxy::setPropValue (propName, value); }

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
    GCFPropertyBase();
    //@{ 
    /// Copy contructors. Don't allow copying this object.
    GCFPropertyBase (const GCFPropertyBase&);
    GCFPropertyBase& operator= (const GCFPropertyBase&);
    //@}

  private: // data members
    GCFPropertySetBase* _pPropertySet;
    GCFAnswer*          _pAnswerObj;
    string              _name;
};
#endif
