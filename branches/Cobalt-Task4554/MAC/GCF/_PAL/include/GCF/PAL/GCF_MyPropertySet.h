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

#include <GCF/GCF_Defines.h>
#include <GCF/PAL/GCF_PropertySet.h>

#include <Common/lofar_list.h>

namespace LOFAR 
{
 namespace GCF 
 {  
  namespace PAL
  {
class GCFMyProperty;
class GPMController;
class GCFAnswer;

// This class represents a property set of owned (local) properties. It gives 
// the Application the possibility to control (un)loading property sets with 
// properties, which then are freely accessible for other Applications to 
// monitor and/or control them. 
// All responses of the possible actions can be handled by using a 
// specialisation of the GCFAnswer class.

class GCFMyPropertySet : public GCFPropertySet
{
  public:
    typedef enum TDefaultUse
    {
      // must be used on construction if the the default must be loaded from DP
      // in DB (only applicable on permanent property sets)
      USE_DB_DEFAULTS, 
      // must be used on construction if the the defaults set with the 
      // initProperties method must be set to the DP in DB after linking it
      USE_MY_DEFAULTS
    };

    // Constructor
    // Creates an property set with owned properties
    // @param propSet the complete property set in ROM (incl. scope)
    // @param scope explicite specified scope
    // @param category (see GCF/GCF_Defines.h for possible values)
    // @param answerObj the call back object for answers on property set actions
    //                  0 or not specified (second constructor) means no answers
    //                  will be forwarded to user
    // @param defaultUse (see description of TDefaultUse)
    // <group>
    explicit GCFMyPropertySet (const char* name,
                               const char* type, 
                               Common::TPSCategory category,
                               GCFAnswer* pAnswerObj,
                               TDefaultUse defaultUse = USE_MY_DEFAULTS);
                      
    explicit GCFMyPropertySet (const char* name,
                               const char* type, 
                               Common::TPSCategory category,
                               TDefaultUse defaultUse = USE_MY_DEFAULTS);
    // </group>

    virtual ~GCFMyPropertySet ();
        
    // Asynchronous method
    // In fact it registers the scope on the Property Agent.
    // Note that this method also resets all values of the managed list of 
    // properties (loaded into RAM on contruction of the instance) with the 
    // values from the given propSet parameter in the construtor.
    // 
    // @return can be GCF_NO_ERROR, GCF_BUSY, GCF_ALREADY_LOADED, GCF_SCOPE_ALREADY_REG
    //         Note that in case of no GCF_NO_ERROR this action ends synchronous.
    //         Otherwise an anwer of type @link GCFPropSetAnswerEvent @endlink 
    //         will be given.
    Common::TGCFResult enable ();

    // Asynchronous method
    // In fact it unregisters the scope from the Property Agent.
    // Note that this implies too that all properties created by the Property 
    // Agent will be deleted even if there are controllers which are subscribed 
    // on one or more of them.
    // 
    // @return can be GCF_NO_ERROR, GCF_BUSY, GCF_NOT_LOADED.
    //         Note that in case of no GCF_NO_ERROR this action ends synchronous.
    //         Otherwise an anwer of type @link GCFPropSetAnswerEvent @endlink 
    //         will be given.
    Common::TGCFResult disable ();

    bool isEnabled () 
      { return (_state == S_ENABLED || _state == S_LINKING || _state == S_LINKED); }
                                   
    bool isMonitoringOn () 
      { return (_state == S_LINKED); }

    // see for more information GCFMyProperty
    // @param propName can be specified with or without the scope
    // @return a clone of the internal (old) value. Must be deleted by the
    //         application
    // <group>
    Common::GCFPValue* getValue (const string propName); 
    Common::GCFPValue* getOldValue (const string propName);
    // </group>

    bool isTemporary () const 
      { return (_category == Common::PS_CAT_TEMPORARY); }
     
    Common::TPSCategory getCategory () const 
      { return _category; }

    // changes the accessrights of all properties
    void setAllAccessModes(Common::TAccessMode mode, bool on);
    // sets defaults and accessrights to each property
    void initProperties(const Common::TPropertyConfig config[]);
             
  private: // interface methods
    friend class GCFMyProperty;
    void linked (GCFMyProperty& prop);
    
  private: // interface methods
    friend class GPMController;
    void scopeRegistered (Common::TGCFResult result);
    void scopeUnregistered (Common::TGCFResult result);
    bool linkProperties ();
    void unlinkProperties ();
    bool tryLinking ();

  private: // helper methods
    GCFProperty* createPropObject(const Common::TPropertyInfo& propInfo);    
    void wrongState(const char* request);
    
  private:
    GCFMyPropertySet ();
    // Don't allow copying this object.
    // <group>
    GCFMyPropertySet (const GCFMyPropertySet&);
    GCFMyPropertySet& operator= (const GCFMyPropertySet&);  
    // </group>

  private: // data members
    typedef enum TState {S_DISABLED, S_DISABLING, S_ENABLING, S_ENABLED, 
                         S_LINKING, S_LINKED, S_DELAYED_DISABLING};
    TState        _state;
    TDefaultUse   _defaultUse;
    Common::TPSCategory   _category;
    
  private: // administrative members
    unsigned short _counter;
    unsigned short _missing;    
};
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
