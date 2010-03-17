//#  GCF_DevicePort.h: TCP connection to a remote process
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

#ifndef GCF_DEVICEPORT_H
#define GCF_DEVICEPORT_H

#include <GCF/TM/GCF_RawPort.h>
#include <Common/lofar_string.h>

namespace LOFAR {
 class MACIO::GCFEvent;
 namespace GCF {
  namespace TM {

// forward declaration
class GCFTask;
class GTMDevice;

/**
 * This is the class, which implements the special port for opening and work with 
 * a file or device. It uses the 'file' pattern to do this. It can only act as a SAP (client).
 */
class GCFDevicePort : public GCFRawPort
{
  public: // constructors && destructors

    /// params see constructor of GCFPortInterface  
    /// type is always SAP  
    explicit GCFDevicePort (GCFTask& task,
          	    const string& name,
                int   protocol, 
                const string& deviceName,
                bool  transportRawData = false);

    /** @param deviceName name of the file/device to open
     * GCFPortInterface params are:
     * pTask => 0
     * name => ""
     * type => SAP
     * protocol => 0
     * transportRawData => false
     */ 
    explicit GCFDevicePort (const string& deviceName);
    /** default constructor
     * GCFPortInterface params are:
     * pTask => 0
     * name => ""
     * type => SAP
     * protocol => 0
     * transportRawData => false
     */ 
    GCFDevicePort ();

    /// destructor
    virtual ~GCFDevicePort ();
  
  private:
    /// Don't allow copying this object.
    GCFDevicePort (const GCFDevicePort&);
    GCFDevicePort& operator= (const GCFDevicePort&);

  public: // GCFPortInterface overloaded/defined methods
    
   /**
     * open/close methods
     */
    virtual bool open ();
    virtual bool close ();
  
    /**
     * send/recv methods
     */
    virtual ssize_t send (GCFEvent& event);
    virtual ssize_t recv (void* buf,
                          size_t count);

  public: // GCFDevicePort specific methods
    void setDeviceName (const string& deviceName);
    
  private: // data members
    bool                _devNameIsSet;
    GTMDevice*          _pDevice;
    string              _deviceName;
};
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#endif
