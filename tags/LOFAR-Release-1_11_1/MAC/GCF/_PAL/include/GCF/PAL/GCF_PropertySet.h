//#  GCF_PropertySet.h:  
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

#ifndef GCF_PROPERTYSET_H
#define GCF_PROPERTYSET_H

#include <GCF/GCF_Defines.h>
#include <GCF/PAL/GCF_Property.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace Common
  {
class GCFPValue;
  }
  namespace PAL
  {
class GCFAnswer;
class GPMController;

// This class is the base class for the 2 types of property set containers. It 
// implements a number of generic features of a property set (like scope 
// management, property container).

class GCFPropertySet
{
  protected: // constructor
    GCFPropertySet (const char* name,
                    const char* type, 
                    GCFAnswer* pAnswerObj);
    
  public:
    virtual ~GCFPropertySet ();

    const string& getScope () const 
      { return _scope; }
    const string getFullScope () const;
    
    const string& getType () const 
      { return _type; }
    GCFAnswer* getAnswerObj() const 
      { return _pAnswerObj; }

    // Searches the property specified by the propName param
    // @param propName with or without the scope
    // @returns 0 if not in this property set
    GCFProperty* getProperty (const string& propName) const;

    // Searches the property specified by the propName param
    // @param propName with or without the scope
    // @returns a dummy port if property could not be found
    virtual GCFProperty& operator[] (const string& propName);
    
    // Changes the property value, if isMonitoringOn is true and property is 
    // readable the value will be forwared to the DP element in DB
    // @param propName with or without the scope
    // @param value can be of type GCFPValue or string (will be converted to 
    //              GCFPValue content)
    // @param wantAnswer setting this parameter to 'true' forces GCF to give an
    //                   answer on this setValue operation. This is very useful
    //                   in case the value must be set on a property of a remote system.
    // @returns GCF_PROP_NOT_IN_SET,  GCF_PROP_WRONG_TYPE, GCF_PROP_NOT_VALID
    // <group>
    Common::TGCFResult setValue (const string& propName, 
                         const Common::GCFPValue& value, 
                         bool wantAnswer = false);

    Common::TGCFResult setValue (const string& propName,
                         const string& value, 
                         bool wantAnswer = false);

    Common::TGCFResult setValueTimed (const string& propName, 
                         const Common::GCFPValue& value, 
                         double timestamp,
                         bool wantAnswer = false);

    Common::TGCFResult setValueTimed (const string& propName,
                         const string& value, 
                         double timestamp,
                         bool wantAnswer = false);
    // </group>

    // changes the answerobject pointer for all properties
    virtual void setAnswer (GCFAnswer* pAnswerObj);          

    virtual bool exists (const string& propName) const;
    
    // Asynchrnous method !
    // Asks the Property Agent to load the apc file for this property set
    // Answer will be indicated to the user by a GCFConfAnswerEvent object
    void configure(const string& apcName);

  protected: // helper methods
    virtual GCFProperty* createPropObject(const Common::TPropertyInfo& propInfo) = 0;
    void dispatchAnswer (unsigned short sig, Common::TGCFResult result);
    void loadPropSetIntoRam();

  private: // helper methods
    bool cutScope(string& propName) const;
    void addProperty(const string& propName, GCFProperty& prop);
    void clearAllProperties();
    
  private: // methods called by GPMController
    friend class GPMController;
    void configured(Common::TGCFResult result, const string& apcName);
    
  private:
    GCFPropertySet();
    
    // Don't allow copying this object.
    // <group> 
    GCFPropertySet (const GCFPropertySet&);
    GCFPropertySet& operator= (const GCFPropertySet&);  
    // </group>

  protected: // data members
    GPMController*      _pController;
    typedef map<string /*propName*/, GCFProperty*> TPropertyList;
    TPropertyList       _properties;

  private: // data members
    GCFAnswer*          _pAnswerObj;
    string              _scope;
    string              _type;
    typedef list<Common::TPropertyInfo> TPropInfoList;
    TPropInfoList       _propSetInfo;

  protected: // helper attributes
    GCFProperty _dummyProperty;
    bool        _isBusy;
};

inline Common::TGCFResult GCFPropertySet::setValue (const string& propName, 
                     const Common::GCFPValue& value, 
                     bool wantAnswer)
{
  return setValueTimed(propName, value, 0.0, wantAnswer);
}

inline Common::TGCFResult GCFPropertySet::setValue (const string& propName,
                     const string& value, 
                     bool wantAnswer)
{
  return setValueTimed(propName, value, 0.0, wantAnswer);
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
