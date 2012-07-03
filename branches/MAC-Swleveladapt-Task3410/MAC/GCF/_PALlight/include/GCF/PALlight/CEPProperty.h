//#  CEPProperty.h: 
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

#ifndef CEPPROPERTY_H
#define CEPPROPERTY_H

#include <GCF/GCF_PValue.h>
#include <GCF/GCF_Defines.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace CEPPMLlight 
  {
    
class CEPPropertySet;


// This is the owned property of a CEP application itself. 
// Opposite to the Property classes of the RTC-PMLlight and the PML packages, 
// this class knows no oldValue and no access rights. This last difference is 
// consequence of the fact that CEP properties cannot be controlled ( == set from 
// outside, indicated by a PVSS DB value change). 
// A property is therefore always (only) monitorable. 

class CEPProperty
{
  public:
    // @return the property name (without the scope)
    const string& getName () const ;
      
    // @return the property name including the scope of the related property set
    const string getFullName () const;

    // @return a clone of the managed value object. This is done to avoid 
    //         changing the value without using the setValue method. 
    Common::GCFPValue* getValue () const;
   
    // changes the current hold value of the property
    // The new value always can be set by means of a string formated text.
    // The set method converts the data to the right value type.
    // <group>
    bool setValue (const string& value);
    bool setValue (const Common::GCFPValue& value);        
    // </group>

    // @return true if an external process has decided to monitor 
    //         the propertyset containing this property
    bool isMonitoringOn () const;
    
  private:
    friend class CEPPropertySet;
    
    // contructor: is private because only the property set has knowledge about 
    // a property. A property can not exists on its own. 
    CEPProperty (const Common::TPropertyInfo& propertyFields, 
                       CEPPropertySet& propertySet);

    // constructor for a dummy property @see CEPPropertySet
    CEPProperty(CEPPropertySet& propertySet);
    
    virtual ~CEPProperty ();
    
  private: // helper methods
    
  private: 
    // Don't allow copying this object.
    // <group>
    CEPProperty (const CEPProperty&);
    CEPProperty& operator= (const CEPProperty&);  
    // </group>

  private:
    // the name of the property (without scope)
    string            _name;
    // reference to the containing property set
    CEPPropertySet&   _propertySet;
    // the value of the property
    Common::GCFPValue*        _pValue;
};

inline const string& CEPProperty::getName () const 
  { return _name; }

  } // namespace CEPPMLlight
 } // namespace GCF
} // namespace LOFAR
#endif
