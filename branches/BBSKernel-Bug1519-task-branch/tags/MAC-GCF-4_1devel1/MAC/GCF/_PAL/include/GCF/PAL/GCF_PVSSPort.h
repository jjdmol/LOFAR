//#  GCF_PVSSPort.h: PVSS connection to a remote process
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

#ifndef GCF_PVSSPORT_H
#define GCF_PVSSPORT_H

#include <GCF/TM/GCF_RawPort.h>
#include <Common/lofar_string.h>

// forward declaration
class GCFTask;
class GCFEvent;
class GSAPortService;

/**
 * This is the class, which implements the special port with the PVSS message 
 * transport protocol. 
 */
class GCFPVSSPort : public GCFRawPort
{
 public:

    /// Construction methods
    /** @param protocol NOT USED */    
    explicit GCFPVSSPort (GCFTask& task,
          	    string name,
          	    TPortType type,
                int protocol, 
                bool transportRawData = false);
    explicit GCFPVSSPort ();
  
    virtual ~GCFPVSSPort ();
  
  public:

    /**
     * open/close functions
     */
    virtual bool open ();
    virtual bool close ();
  
    /**
     * send/recv functions
     */
    virtual ssize_t send (GCFEvent& event);
    virtual ssize_t recv (void* buf,
                          size_t count);
                          
  public: // pvss port specific methods

    const string getOwnAddr() const;
    void setDestAddr (const string& destDpName);

  private:
    /**
     * Don't allow copying this object.
     */
    GCFPVSSPort (const GCFPVSSPort&);
    GCFPVSSPort& operator= (const GCFPVSSPort&);
    
    friend class GSAPortService;
    void serviceStarted(bool successfull);
        
  private:
    GSAPortService*   _pPortService;
    string          _destDpName;
};

#endif
