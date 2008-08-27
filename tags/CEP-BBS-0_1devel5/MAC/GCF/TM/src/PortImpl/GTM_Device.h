//#  GTM_Device.h: base class for all sockets
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

#ifndef GTM_DEVICE_H
#define GTM_DEVICE_H

#include <unistd.h>
//#include <MACIO/GCF_Event.h>
#include "GTM_File.h"
#include <Common/lofar_string.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {

// forward declaration
class GCFDevicePort;

/**
 * This class consists of the basic implementation of a device (driver). 
 */

class GTMDevice : public GTMFile
{
  public:
    GTMDevice (GCFDevicePort& port);
    virtual ~GTMDevice ();
  
    /**
     * open/close methods
     */
    virtual bool open (const string& deviceName);
  
    /**
     * send/recv methods
     */
    virtual ssize_t send (void* buf, size_t count);
    virtual ssize_t recv (void* buf, size_t count);

  private:
    /// default contructor
    GTMDevice ();

    /// Don't allow copying of the GTMDevice object.
    GTMDevice (const GTMDevice&);
    GTMDevice& operator= (const GTMDevice&);
};
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#endif
