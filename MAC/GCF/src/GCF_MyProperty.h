//#  GCF_MyProperty.h: 
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

#ifndef GCF_MYPROPERTY_H
#define GCF_MYPROPERTY_H

#include <GCF/GCF_PValue.h>
#include <GCF/GCF_Defines.h>
#include <GCF/GCF_PropertyBase.h>

class GCFMyPropertySet;

/**
 * This class represents a property set of owned (local) properties. It gives 
 * the Application the possibility to control (un)loading property sets with 
 * properties, which then are freely accessible for other Applications to 
 * monitor and/or control them. 
 * 
 * All responses of the possible actions can be handled by using a 
 * specialisation of the GCFAnswer class. 
 * 
 * This container class keeps properties belonging to a 'scope' (root) together. 
 * A scope means the path in the SCADA DB, where the properties should be 
 * created by GCF. This scope will be used to "(un)register" itself in the PA.  
 * The property set is also responsible to (un)link properties on request of the 
 * PA via the GPMController.
 */

class GCFMyProperty : public GCFPropertyBase
{
  public:
    GCFPValue* getValue () const;
    GCFPValue* getOldValue () const;
   
    //@{
    /** They both first copys the current local (owned) value
     * to the old value followed by setting the changing the current
     * with the parameter 'value'. In case the property is linked
     * and the property is readable then the value in SCADA DB will be 
     * updated too.
     * @return can be GCF_PROP_WRONG_TYPE, GCF_PROP_NOT_VALID
     */
    TGCFResult setValue (const string value);
    TGCFResult setValue (const GCFPValue& value);        
    //@}
    inline bool isLinked () const {return _isLinked;}
    //@{
    /** Access mode can be: GCF_READABLE_PROP and/or GCF_WRITABLE_PROP
     * NOTE: If both modes are set and the property is linked, the setValue 
     * method call results immediate in a F_VCHANGEMSG_SIG answer event. This 
     * on its turn copies the current to the old value and the changed value to
     * current value. This means that the current value from before the setValue 
     * will not be available after the answer event is received (old value is 
     * then the same as the new value). On the other hand this construction can 
     * been seen as a asynchronous 'set' action.
     */ 
    void setAccessMode (TAccessMode mode, bool on);
    bool testAccessMode (TAccessMode mode) const;   
    //@}
    
  private:
    friend class GCFMyPropertySet;
    
    GCFMyProperty (const TProperty& propertyFields, 
                   GCFMyPropertySet& propertySet);
    virtual ~GCFMyProperty ();
    
    bool link ();
    void unlink ();    
    
  private: // overrides base class methods
    void subscribed ();

    /** 
     * normally this method should never appear
     * but if so (in case of calling the method requestValue method of the base 
     * class GCFPropertyBase) it acts as the valueChanged method
     */
    void valueGet (const GCFPValue& value);
    void valueChanged (const GCFPValue& value);
           
  private: 
    //@{ 
    /// Copy contructors. Don't allow copying this object.
    GCFMyProperty (const GCFMyProperty&);
    GCFMyProperty& operator= (const GCFMyProperty&);  
    //@}

  private:
    TAccessMode       _accessMode;
    GCFPValue*        _pCurValue;
    GCFPValue*        _pOldValue;
    bool              _isLinked;
    GCFMyPropertySet& _propertySet;
    bool              _isBusy;
};
#endif
