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
#include <GCF/GCF_PropertySetBase.h>

/**
 * This class represents a property set of properties specified in an APC. It 
 * gives the Application the possibility to access properties of for instance a 
 * just loaded APC. 
 * This class knows no asynchronous actions. Therefore no relation with a 
 * GCFAnswer instances is necessary. But there is a possibility to set a 
 * GCFAnswer instance for each managed property with one method.  
 * This container class keeps properties belonging to a 'scope' (root) together. 
 * A scope means the path in the SCADA DB, where the properties should be 
 * created by GCF. 
*/

class GCFPropertySet : public GCFPropertySetBase
{
  public:
    /**
     * By means of this contructor a property set will be loaded, which has the
     * same structure as in the APC specified by the apcName param
     */ 
    GCFPropertySet (string apcName, 
                    string scope, 
                    GCFAnswer* pAnswerObj = 0);
    virtual ~GCFPropertySet () {;}

    /**
     * Asynchronous request (results in a response via the GCFAnswer object)
     * @return GCF_NO_ERROR, GCF_PROP_NOT_IN_SET, GCF_PML_ERROR (see for more 
     *         info in the logging of the SAL of GCF
     */
    TGCFResult requestValue (const string propName) const;
    /**
     * Asynchronous request (results in a response via the GCFAnswer object)
     * @return GCF_NO_ERROR, GCF_PROP_NOT_IN_SET, GCF_BUSY, GCF_ALREADY_SUBSCRIBED,
     *         GCF_PML_ERROR (see for more info in the logging of the SAL of GCF)
     */
    TGCFResult subscribe (const string propName) const;
    /**
     * Asynchronous request (results in a response via the GCFAnswer object)
     * @return GCF_NO_ERROR, GCF_PROP_NOT_IN_SET, GCF_BUSY, GCF_NOT_SUBSCRIBED, 
     *         GCF_PML_ERROR (see for more info in the logging of the SAL of GCF)
     */
    TGCFResult unsubscribe (const string propName) const;

  private:
    GCFPropertySet();
    //@{ 
    /// Copy contructors. Don't allow copying this object.
    GCFPropertySet (const GCFPropertySet&);
    GCFPropertySet& operator= (const GCFPropertySet&);  
    //@}
};
#endif
