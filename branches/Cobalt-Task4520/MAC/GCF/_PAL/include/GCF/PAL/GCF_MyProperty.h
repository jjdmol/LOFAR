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
#include <GCF/PAL/GCF_Property.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace PAL
  {
class GCFMyPropertySet;

// This is the owned property itself. It manages an old value and the current 
// value. The current value is (if linked) synchronised with the value in the 
// SCADA DB. The old value becomes the current value if the value is the SCADA 
// DB was changed. The old value is introduced because the owner of the property 
// will only be notified about the changed (current) value and is not able to 
// decide about whether or not changing the (current) value. At this way it is 
// possible to compare the changed value with the original value of a property 
// (Note that changed values can be equal to the original current value). The 
// name of the property only consists of alphanumeric characters and the '_'. 
// This last character will be used as separator between different levels in a 
// virtual tree. The name should always be relative to the scope, which is 
// managed by its property set container (GCFMyPropertySet). 
// Furthermore this class handles the access rights based on the accessMode 
// (see Terms). Note that only the GCFMyPropertySet class is able to create 
// instances of this class. 
// All value-changed indications can be handled by using a specialisation of the 
// GCFAnswer class. No actions of this class results in responses. Because the 
// current value is always synchronised with the SCADA property (if linked) 
// the get(Old)Value methods are simply synchronous actions.

class GCFMyProperty : public GCFProperty
{
  public:
    // returns pointer to a clone of the (old) value
    // <group>    
    Common::GCFPValue* getValue () const;
    Common::GCFPValue* getOldValue () const;
    // </group>    
   
    // They both first copys the current local (owned) value
    // to the old value followed by setting the changing the current
    // with the parameter 'value'. In case the property is linked
    // and the property is readable then the value in SCADA DB will be 
    // updated too.
    // @return can be GCF_PROP_WRONG_TYPE, GCF_PROP_NOT_VALID
    // <group>
    Common::TGCFResult setValueTimed (const string& value, double timestamp, bool wantAnswer = false);
    Common::TGCFResult setValueTimed (const Common::GCFPValue& value, double timestamp, bool wantAnswer = false);
    // </group>
    
    bool isMonitoringOn () const {return _isLinked && !_isBusy;}
    
    // Access mode can be: GCF_READABLE_PROP and/or GCF_WRITABLE_PROP
    // NOTE: If both modes are set and the property is linked, the setValue 
    // method call results immediate in a F_VCHANGEMSG_SIG answer event. This 
    // on its turn copies the current to the old value and the changed value to
    // current value. This means that the current value from before the setValue 
    // will not be available after the answer event is received (old value is 
    // then the same as the new value). On the other hand this construction can 
    // been seen as a asynchronous 'set' action.
    // <group>
    void setAccessMode (Common::TAccessMode mode, bool on);
    bool testAccessMode (Common::TAccessMode mode) const;   
    // </group>
    
  private:
    friend class GCFMyPropertySet;
    
    GCFMyProperty (const Common::TPropertyInfo& propertyFields, 
                   GCFMyPropertySet& propertySet);
    virtual ~GCFMyProperty ();
    
    // @param setDefault true => use my defaults (set value), false => use DB defaults (get value)
    // @return true if is an asynchronous action, false if not
    bool link (bool setDefault, Common::TGCFResult& result); 
    
    void unlink ();    
    
  private: // overrides base class methods
    void subscribed ();
    void subscriptionLost ();

    // normally this method should never appear
    // but if so (in case of calling the method requestValue method of the base 
    // class GCFPropertyBase) it acts as the valueChanged method
    void valueGet (const Common::GCFPValue& value);

    void valueChanged (const Common::GCFPValue& value);
           
  private: 
    // Don't allow copying this object.
    // <group>
    GCFMyProperty (const GCFMyProperty&);
    GCFMyProperty& operator= (const GCFMyProperty&);  
    // </group>

  private: // data members
    Common::TAccessMode       _accessMode;
    Common::GCFPValue*        _pCurValue;
    Common::GCFPValue*        _pOldValue;
    bool              _isLinked;
    GCFMyPropertySet& _propertySet;
    
  private: // adminstrative members
    bool              _changingAccessMode;
};
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
