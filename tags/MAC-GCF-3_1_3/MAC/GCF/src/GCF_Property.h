//#  GCF_Property.h: 
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

#ifndef GCF_PROPERTY_H
#define GCF_PROPERTY_H

#include <GCF/GCF_PValue.h>
#include <GCF/GCF_Defines.h>
#include <GCF/GCF_PropertyBase.h>

class GCFPropertySet;

/**
 * This class represents an external property, which are normally owned by an 
 * opposite Application than this representation is created. (Note that owned 
 * properties can also be represented by this class in the same Application). 
 * The name of the property only consists of alphanumeric characters and the 
 * '_'. This last character will be used as separator between different levels 
 * in a virtual tree. The name can be (but not necessarily) relative to a scope, 
 * which then is managed by a property set container. In that case the 
 * GCFPropertySet manages the property instance too. Otherwise the property 
 * instance exists independent and must therefore be managed by the APL. 
 * All responses of the possible actions or value-changed indications can be 
 * handled by using a specialisation of the GCFAnswer class.
 */
class GCFProperty : public GCFPropertyBase
{
  public:
    /**
     * Creates an instance of the property class, which can be used independent
     * of a GCFPropertySet object.
     * @param propName it only can mapped to a property in the SCADA DB if the
     *                 scope is included 
     */
    GCFProperty (string propName);
    virtual ~GCFProperty () {;}

    inline bool isSubscribed () const {return _isSubscribed;}
    /**
     * Asynchronous action
     * @return GCF_NO_ERROR, GCF_BUSY, GCF_ALREADY_SUBSCRIBED, GCF_PML_ERROR
     */
    TGCFResult subscribe();
    /**
     * Synchronous (!) action
     * @return GCF_NO_ERROR, GCF_BUSY, GCF_NOT_SUBSCRIBED, GCF_PML_ERROR
     */
    TGCFResult unsubscribe();

    /**
     * Checks whether the property exists in the SCADA DB or not
     * Note that in case the property does not exists the property will
     * be marked as "not subscribed".
     */
    bool exists();

  private:
    friend class GCFPropertySet;
    GCFProperty();
    /**
     * Creates an instance of the property class, which only can exists in 
     * combination with a GCFPropertySet object.
     */
    GCFProperty (string propName, 
                 GCFPropertySet& pPropertySet);
    
  private: // overrides base class methods
    void subscribed ();

  private:
    //@{ 
    /// Copy contructors. Don't allow copying this object.
    GCFProperty (const GCFProperty&);
    GCFProperty& operator= (const GCFProperty&);  
    //@}

  private: // data members
    bool _isSubscribed;

  private: // admin. data members
    bool _isBusy;
           
};
#endif
