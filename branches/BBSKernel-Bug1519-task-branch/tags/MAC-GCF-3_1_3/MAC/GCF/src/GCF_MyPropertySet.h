//#  GCF_MyPropertySet.h:  
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

#ifndef GCF_MYPROPERTYSET_H
#define GCF_MYPROPERTYSET_H

#include <GCF/GCF_PropertySetBase.h>
#include <GCF/GCF_Defines.h>

#include <Common/lofar_list.h>

class GCFMyProperty;
class GPMController;
class GCFAnswer;

/** 
 * This class represents a property set of owned (local) properties. It gives 
 * the Application the possibility to control (un)loading property sets with 
 * properties, which then are freely accessible for other Applications to 
 * monitor and/or control them. 
 * All responses of the possible actions can be handled by using a 
 * specialisation of the GCFAnswer class.
*/

class GCFMyPropertySet : public GCFPropertySetBase
{
  public:
    /**
     * Constructor
     * Creates an property set with owned properties
     * @param propSet the complete property set in ROM (incl. scope)
     * @param answerObj the call back object for answers on property set actions
     */
    GCFMyPropertySet (const TPropertySet& propSet, 
                      GCFAnswer* pAnswerObj = 0);
    /**
     * Constructor
     * Creates an property set with owned properties
     * @param propSet the complete property set in ROM (incl. scope)
     * @param scope explicite specified scope
     * @param answerObj the call back object for answers on property set actions
     */
    GCFMyPropertySet (const TPropertySet& propSet, 
                      const char* scope,
                      GCFAnswer* pAnswerObj = 0);
    virtual ~GCFMyPropertySet ();

    /**
     * Asynchronous method
     * In fact it registers the scope on the Property Agent.
     * Note that this method also resets all values of the managed list of 
     * properties (loaded into RAM on contruction of the instance) with the 
     * values from the given propSet parameter in the construtor.
     * 
     * @return can be GCF_NO_ERROR, GCF_BUSY, GCF_ALREADY_LOADED, GCF_SCOPE_ALREADY_REG
     *         Note that in case of no GCF_NO_ERROR this action ends synchronous.
     *         Otherwise an anwer of type @link GCFMYPropAnswerEvent @endlink 
     *         will be given.
     */
    TGCFResult load ();

    /**
     * Asynchronous method
     * In fact it unregisters the scope from the Property Agent.
     * Note that this implies too that all properties created by the Property 
     * Agent will be deleted even if there are controllers which are subscribed 
     * on one or more of them.
     * 
     * @return can be GCF_NO_ERROR, GCF_BUSY, GCF_NOT_LOADED.
     *         Note that in case of no GCF_NO_ERROR this action ends synchronous.
     *         Otherwise an anwer of type @link GCFMYPropAnswerEvent @endlink 
     *         will be given.
     */
    TGCFResult unload ();

    inline bool isLoaded () 
      { return _isLoaded; }
    
    TGCFResult setValue (const string propName, ///< can be specified with or without the scope
                         const string value);
                         
    inline TGCFResult setValue (const string propName, ///< can be specified with or without the scope
                         const GCFPValue& value)                         
      { return GCFPropertySetBase::setValue(propName, value); }
      
    //@{
    /**
     * @param propName can be specified with or without the scope
     * @return a clone of the internal (old) value. Must be deleted by the
     *         application
     */
    GCFPValue* getValue (const string propName); 
    GCFPValue* getOldValue (const string propName);
    //@}
             
  private: // interface methods
    friend class GCFMyProperty;
    void linked ();
    
  private: // interface methods
    friend class GPMController;
    void scopeRegistered (TGCFResult result);
    void scopeUnregistered (TGCFResult result);
    void linkProperties (list<string>& properties);
    void unlinkProperties (list<string>& properties);

  private: // helper methods
    void retryLinking ();
    void init();
    void dispatchAnswer (unsigned short sig, TGCFResult result);
    
  private:
    GCFMyPropertySet ();
    //@{ 
    /// Copy contructors. Don't allow copying this object.
    GCFMyPropertySet (const GCFMyPropertySet&);
    GCFMyPropertySet& operator= (const GCFMyPropertySet&);  
    //@}

  private: // attribute members
    bool _isLoaded;

  private: // administrative members
    bool _isBusy;
    list<string> _tempLinkList;
    unsigned short _counter;
    unsigned short _missing;
    const TPropertySet& _propSet;
        
};
#endif
