//#  GCF_PropertySetBase.h:  
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

#ifndef GCF_PROPERTYSETBASE_H
#define GCF_PROPERTYSETBASE_H

#include <GCF/GCF_Defines.h>
#include <GCF/GCF_PropertyBase.h>

class GCFPropertyBase;
class GCFAnswer;
class GCFPValue;

/** 
 * This class is the base class for the 2 types of property set containers. It 
 * implements a number of generic features of a property set (like scope 
 * management, property container).
*/

class GCFPropertySetBase
{
  public:
    virtual ~GCFPropertySetBase ();

    inline const string& getScope () const 
      { return _scope; }

    /**
     * Searches the property specified by the propName param
     * @param propName with or without the scope
     * @returns 0 if not in this property set
     */
    GCFPropertyBase* getProperty (const string propName) const;

    /**
     * Searches the property specified by the propName param
     * @param propName with or without the scope
     * @returns a dummy port if property could not be found
     */
    virtual GCFPropertyBase& operator[] (const string propName);
    
    /**
     * Searches the property specified by the propName param
     * @param propName with or without the scope
     * @returns 0 if not in this property set
     */
    virtual TGCFResult setValue (const string propName, 
                                 const GCFPValue& value);

    virtual void setAnswer (GCFAnswer* pAnswerObj);          

    virtual bool exists (const string propName) const;

  protected:
    GCFPropertySetBase (string scope, 
                        GCFAnswer* pAnswerObj);
    void addProperty(const string& propName, GCFPropertyBase& prop);
    void clearAllProperties();
    inline GCFAnswer* getAnswerObj() const 
      { return _pAnswerObj; }
    
  protected: 
    typedef map<string /*propName*/, GCFPropertyBase*> TPropertyList;
    TPropertyList _properties;

  private: // helper methods
    bool cutScope(string& propName) const;
  
  private:
    GCFPropertySetBase();
    
    /// Don't allow copying this object.
    GCFPropertySetBase (const GCFPropertySetBase&);
    GCFPropertySetBase& operator= (const GCFPropertySetBase&);  

  private:
    GCFAnswer*    _pAnswerObj;
    string        _scope;
    GCFPropertyBase _dummyProperty;
};
#endif
