//#  CEPPropertySet.h:  
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

#ifndef CEPPROPERTYSET_H
#define CEPPROPERTYSET_H

#include <GCF/GCF_Defines.h>

#include <Common/lofar_list.h>
#include <Common/lofar_map.h>
#include <GCF/PALlight/CEPProperty.h>
#include <GCF/Mutex.h>

class GCFPValue;

namespace LOFAR 
{
 namespace GCF 
 {
  namespace CEPPMLlight 
  {

class PIClient;

// This class represents a property set of owned (local) properties on a CEP 
// application. It gives the CEP application the possibility to control 
// (enable/disable) property sets with properties, which then are freely 
// accessible for other Applications to (only !) monitor them. “Control” will 
// already be done by ACC. 
//
// The property set is also responsible to (un)link properties on request of 
// the PA (via PI) via the PIClient.
// Opposite to the PropertySet classes in the PML package this class supports no
// 'configure' (@see GCFPropertySet) action. On the other hand it now must 
// forward value changes (set) in the Property to the PIClient. 

class CEPPropertySet
{
  public:
    typedef list<Common::TPropertyInfo> TPropInfoList;
    
    // Constructor
    // Creates an property set with owned properties.
    // By means of the 'type' param a file with information (sturcture of property set)
    // will be loaded. This file ('types_<type>.dpl.tmp') should be available 
    // at place where the application with this property set runs. It should be generated at 
    // preprocess time of an application.
    explicit CEPPropertySet (const char* name,
                             const char* type, 
                             Common::TPSCategory category);

    virtual ~CEPPropertySet ();

    // @return the scope (Instance name)
    const string& getScope () const;

    // @return the type
    const string& getType () const;
      
    // @return true if is marked as a temporary property set
    bool isTemporary () const;
     
    // @return the category data member value
    Common::TPSCategory getCategory () const;

    // @param propName with or without the scope
    // @return true if property with 'propName' exists in this set
    bool exists (const string& propName) const;

    // @param propName with or without the scope
    // @return pointer to property with 'propName' if found, otherwise 0
    CEPProperty* getProperty (const string& propName) const;

    // @param propName with or without the scope
    // @return reference to a property with 'propName' if found, otherwise a dummy
    CEPProperty& operator[] (const string& propName);

    // Asynchronous method
    // In fact it registers the scope on the Property Agent (via Property Interface).
    // It should be called by the user of this property set, if the containing 
    // properties must be monitorable by external processes (MAC applications). 
    // At the moment the isEnabled() method returns true this asynchronous 
    // sequence behind this method has finished successfully.
    // 
    // @return true if request is performed successfull 
    bool enable ();

    // Asynchronous method
    // In fact it unregisters the scope on the Property Agent (via Property Interface).
    // It should be called by the user of this property set, if the containing 
    // properties may not be monitorable by external processes (MAC applications) anymore. 
    // At the moment the isEnabled() method returns false this asynchronous 
    // sequence behind this method has finished successfully.
    // 
    // @return true if request is performed successfull 
    bool disable ();

    // @return true if the property set is ready for monitoring requests (enabled)
    bool isEnabled ();  
    
    // @return true if there is an external process, which has decided to monitor
    //         one or more properties of this property set 
    bool isMonitoringOn();                            
  
    // @param propName with or without the scope
    // @return false if 'propName' was not found in property set or value could 
    //         not be converted in the property value object
    bool setValue (const string& propName,
                   const string& value);
  
    // @param propName with or without the scope
    // @return false if 'propName' was not found in property set or value type 
    //         is not equal to the value type in the property object
    bool setValue (const string& propName, 
                   const Common::GCFPValue& value);
                   
    // @param propName with or without the scope
    // @return pointer of cloned value object of the addressed property or 0 
    //         if not found
    Common::GCFPValue* getValue (const string& propName);             
               
  private: // methods called by CEPProperty
    friend class CEPProperty;
    // will be invoked by CEPProperty if the value of the property has changed
    // and the property set is in monitoring state
    void valueSet(const string& propName, const Common::GCFPValue& value);
    
  private: // methods called by PIClient
    friend class PIClient;
    // response of an enable method call invoked by PIClient
    void scopeRegistered (bool succeed);
    // response of an disable method call invoked by PIClient
    void scopeUnregistered (bool succeed);
    // requests of the Property Agent (via the Property Interface) invoked by PIClient
    // <group>
    void linkProperties ();    
    void unlinkProperties ();
    // </group>
    void connectionLost();

  private: // helper methods    
    void addProperty(const string& propName, CEPProperty& prop);
    void clearAllProperties();
    bool cutScope(string& propName) const;
    void wrongState(const char* request);
    void readTypeFile(TPropInfoList& propInfos);
    
  private: // copy constructors
    CEPPropertySet();
    // Don't allow copying this object.
    // <group>
    CEPPropertySet (const CEPPropertySet&);
    CEPPropertySet& operator= (const CEPPropertySet&);  
    // </group>

  private: // attribute members
    // the scope (Instance name)
    string  _scope;     
    // the type
    string  _type;   
    // temporary, permanent or permanent autoloading on enable?
    Common::TPSCategory _category;
    
    typedef map<string /*propName*/, CEPProperty*> TPropertyList;
    TPropertyList       _properties;

  private: // administrative members
    
    typedef enum TState {S_DISABLED, S_DISABLING, S_ENABLING, S_ENABLED, 
                         S_LINKED};
    TState _state;
    // will be used in the operator[] if property not found
    CEPProperty         _dummyProperty; 
    // pointer to PIClient singleton
    PIClient*           _pClient;
    GCF::Thread::Mutex  _stateMutex;
};

inline const string& CEPPropertySet::getScope () const 
  { return _scope; }

inline const string& CEPPropertySet::getType () const 
  { return _type; }
  
inline bool CEPPropertySet::isTemporary () const 
  { return (_category == Common::PS_CAT_TEMPORARY); }
     
inline Common::TPSCategory CEPPropertySet::getCategory () const 
  { return _category; }
  
inline bool CEPPropertySet::isEnabled () 
  { GCF::Thread::Mutex::Lock(_stateMutex);
    return (_state == S_ENABLED || _state == S_LINKED); 
  }
  
inline bool CEPPropertySet::isMonitoringOn()
  { GCF::Thread::Mutex::Lock(_stateMutex);
    return _state == S_LINKED; 
  }

  } // namespace CEPPMLlight
 } // namespace GCF
} // namespace LOFAR
#endif
