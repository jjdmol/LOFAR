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

#include <Common/lofar_map.h>
class GCFAnswer;
class GCFPValue;
class GPMController;

/** 
 * This class is the base class for the 2 types of property set containers. It 
 * implements a number of generic features of a property set (like scope 
 * management, property container).
*/

class GCFPropertySet
{
  public:
    virtual ~GCFPropertySet ();

    const string& getScope () const 
      { return _scope; }
    const char* getType () const 
      { return _propSetInfo.typeName; }
    const bool isTemporary () const 
      { return _propSetInfo.isTemporary; }
    GCFAnswer* getAnswerObj() const 
      { return _pAnswerObj; }

    /**
     * Searches the property specified by the propName param
     * @param propName with or without the scope
     * @returns 0 if not in this property set
     */
    GCFProperty* getProperty (const string propName) const;

    /**
     * Searches the property specified by the propName param
     * @param propName with or without the scope
     * @returns a dummy port if property could not be found
     */
    virtual GCFProperty& operator[] (const string propName);
    
    /**
     * Searches the property specified by the propName param
     * @param propName with or without the scope
     * @returns 0 if not in this property set
     */
    TGCFResult setValue (const string propName, 
                                 const GCFPValue& value);

    TGCFResult setValue (const string propName, ///< can be specified with or without the scope
                         const string value);

    virtual void setAnswer (GCFAnswer* pAnswerObj);          

    virtual bool exists (const string propName) const;
    void configure(const string apcName);

  protected:
    GCFPropertySet (const char* name, 
                    const TPropertySet& typeInfo,
                    GCFAnswer* pAnswerObj);
    
  protected: // helper methods
    virtual GCFProperty* createPropObject(const TProperty& propInfo) = 0;
    void dispatchAnswer (unsigned short sig, TGCFResult result);
    void loadPropSetIntoRam();
    
  protected: // helper attributes
    typedef map<string /*propName*/, GCFProperty*> TPropertyList;
    TPropertyList   _properties;
    bool            _isBusy;
    GPMController*  _pController;

  private: // methods called by GPMController
    friend class GPMController;
    void configured(TGCFResult result, const string& apcName);
    
  private: // helper methods
    bool cutScope(string& propName) const;
    void addProperty(const string& propName, GCFProperty& prop);
    void clearAllProperties();
  
  private: // default/copy constructors
    GCFPropertySet();
    
    /// Don't allow copying this object.
    GCFPropertySet (const GCFPropertySet&);
    GCFPropertySet& operator= (const GCFPropertySet&);  

  private:
    GCFAnswer*          _pAnswerObj;
    string              _scope;
    GCFProperty         _dummyProperty;
    const TPropertySet& _propSetInfo;    
};
#endif
