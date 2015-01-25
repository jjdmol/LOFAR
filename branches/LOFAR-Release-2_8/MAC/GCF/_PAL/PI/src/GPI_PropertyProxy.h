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

// This specialized property proxy class provides for all in the controller 
// registered PMLlight servers (objects of classes, which derives from the 
// GPIPMLlightServer) the possibility to forward property value changes received 
// from a PIA to the PVSS system in a very fast, short and simple way.

class GPIPropertyProxy : public GCFPropertyProxy
{
  public:
    GPIPropertyProxy() {}
    virtual ~GPIPropertyProxy() {}
  
  private:
    void propSubscribed(const string& /*propName*/) {}
    void propSubscriptionLost(const string& /*propName*/) {}
    void propUnsubscribed(const string& /*propName*/) {}
    void propValueGet(const string& /*propName*/, const Common::GCFPValue& /*value*/) {}
    void propValueChanged(const string& /*propName*/, const Common::GCFPValue& /*value*/) {}
    void propValueSet(const string& /*propName*/) {}

    // Don't allow copying of this object.
    // <group>
    GPIPropertyProxy (const GPIPropertyProxy&);
    GPIPropertyProxy& operator= (const GPIPropertyProxy&);
    // </group>
};    
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
