//#  GCF_ExtPropertySet.h:  
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

#ifndef GCF_EXTPROPERTYSET_H
#define GCF_EXTPROPERTYSET_H

#include <GCF/GCF_Defines.h>
#include <GCF/PAL/GCF_PropertySet.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace PAL
  {
class GPMController;

// This class represents a property set of properties specified in an APC. It 
// gives the Application the possibility to access properties of for instance a 
// just loaded APC. 
// This class knows no asynchronous actions. Therefore no relation with a 
// GCFAnswer instances is necessary. But there is a possibility to set a 
// GCFAnswer instance for each managed property with one method.  
// This container class keeps properties belonging to a 'scope' (root) together. 
// A scope means the path in the SCADA DB, where the properties should be 
// created by GCF. 

class GCFExtPropertySet : public GCFPropertySet
{
  public:
    // By means of this contructor a property set will be loaded, which has the
    // same structure as in the APC specified by the apcName param
    GCFExtPropertySet (const char* name,
                       const char* type, 
                       GCFAnswer* pAnswerObj = 0);
    virtual ~GCFExtPropertySet ();

    // Asynchronous method
    // In fact it registers the scope on the Property Agent.
    // Note that this method also resets all values of the managed list of 
    // properties (loaded into RAM on contruction of the instance) with the 
    // values from the given propSet parameter in the construtor.
    // 
    // @return can be GCF_NO_ERROR, GCF_BUSY, GCF_ALREADY_LOADED
    //         Note that in case of no GCF_NO_ERROR this action ends synchronous.
    //         Otherwise an anwer of type @link GCFPropSetAnswerEvent @endlink 
    //         will be given.
    Common::TGCFResult load ();

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
    Common::TGCFResult unload ();

    // Asynchronous request (results in a response via the GCFAnswer object)
    // @return GCF_NO_ERROR, GCF_PROP_NOT_IN_SET, GCF_PML_ERROR (see for more 
    //         info in the logging of the SAL of GCF
    Common::TGCFResult requestValue (const string propName) const;

    // Asynchronous request (results in a response via the GCFAnswer object)
    // @return GCF_NO_ERROR, GCF_PROP_NOT_IN_SET, GCF_BUSY, GCF_ALREADY_SUBSCRIBED,
    //         GCF_PML_ERROR (see for more info in the logging of the SAL of GCF)
    Common::TGCFResult subscribeProp (const string propName) const;

    // Asynchronous request (results in a response via the GCFAnswer object)
    // @return GCF_NO_ERROR, GCF_PROP_NOT_IN_SET, GCF_BUSY, GCF_NOT_SUBSCRIBED, 
    //         GCF_PML_ERROR (see for more info in the logging of the SAL of GCF)
    Common::TGCFResult unsubscribeProp (const string propName) const;
    
    bool isPropSubscribed (const string propName) const;

    bool isLoaded() const { return _isLoaded;} 
    
  private:
    friend class GPMController;
    void loaded(Common::TGCFResult result);
    
    void unloaded(Common::TGCFResult result);
    void serverIsGone();
    
  private:
    GCFExtPropertySet();
    // Copy contructors. Don't allow copying this object.
    // <group>
    GCFExtPropertySet (const GCFExtPropertySet&);
    GCFExtPropertySet& operator= (const GCFExtPropertySet&);     
    // </group>
    
  private: // helper methods
    GCFProperty* createPropObject(const Common::TPropertyInfo& propInfo);
      
  private: // data members
    bool _isLoaded;
};
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif

