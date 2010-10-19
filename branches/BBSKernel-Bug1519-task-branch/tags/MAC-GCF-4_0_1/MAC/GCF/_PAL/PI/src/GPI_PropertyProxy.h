//#  GPI_PropertyProxy.h: the concrete property proxy for the Property Interface 
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

#ifndef GPI_PROPERTYPROXY_H
#define GPI_PROPERTYPROXY_H


#include <GCF/PAL/GCF_PropertyProxy.h>

class GCFPValue;

/**
 * This class provides for the GPISupervisoryServer class the possibility to 
 * (un)subscribe on/from properties, which requested from the Property Agent to 
 * (un)link to a property of the ERTC domain. Furthermore it forwards property 
 * value changes, detected by the SCADA system, to the right 
 * GPISupervisoryServer instance.
 */
class GPIPropertyProxy : public GCFPropertyProxy
{
  public:
    GPIPropertyProxy() {;}
    virtual ~GPIPropertyProxy() {;}
  
  private:
    inline void propSubscribed(const string& /*propName*/) {;}
    inline void propUnsubscribed(const string& /*propName*/) {;}
    inline void propValueGet(const string& /*propName*/, const GCFPValue& /*value*/) {};
    inline void propValueChanged(const string& /*propName*/, const GCFPValue& /*value*/) {;}

    /**
     * Don't allow copying of this object.
     */
    GPIPropertyProxy (const GPIPropertyProxy&);
    GPIPropertyProxy& operator= (const GPIPropertyProxy&);
};    
#endif
