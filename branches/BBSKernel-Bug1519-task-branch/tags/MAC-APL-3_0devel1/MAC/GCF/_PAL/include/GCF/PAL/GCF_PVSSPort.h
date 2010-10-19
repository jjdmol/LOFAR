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
#include <GCF/GCF_PVString.h>
#include <Common/lofar_string.h>
#include <set>

using std::set;
using std::string;

// forward declaration
class GCFTask;
class GCFEvent;
class GSAPortService;
class GCFPVBlob;
class GCFPVSSUIMConverter;

// This is the class, which implements the special port with the PVSS message 
// transport protocol. 
// NOTE: This class is intended to use outside of GCF but is not yet ready for use.
class GCFPVSSPort : public GCFRawPort
{
 public:

    // Construction methods
    // @param protocol NOT USED
    explicit GCFPVSSPort (GCFTask& task,
          	    string name,
          	    TPortType type,
                int protocol, 
                bool transportRawData = false);

    explicit GCFPVSSPort ();
  
    virtual ~GCFPVSSPort ();
  
  public:

    void setConverter(GCFPVSSUIMConverter& converter);

    // open/close functions
    // <group>
    virtual bool open ();
    virtual bool close ();
    // </group>
  
    // send/recv functions
    // <group>
    virtual ssize_t send (GCFEvent& event);
    virtual ssize_t recv (void* buf,
                          size_t count);
    // </group>
           
  public: // pvss port specific methods

    void setDestAddr (const string& destDpName);
    const string& getDestAddr() const { return _destDpName;}
    const string& getPortID() { return _portId.getValue();}
    const string& getPortAddr() { return _portAddr.getValue();}
    virtual bool accept(GCFPVSSPort& newPort);

  private:
    // Don't allow copying this object.
    // <group>
    GCFPVSSPort (const GCFPVSSPort&);
    GCFPVSSPort& operator= (const GCFPVSSPort&);
    // </group>
    
  private: // helper methods
    friend class GSAPortService;
    void serviceStarted(bool successfull);
    void setService(GSAPortService& service);
            
    static unsigned int claimPortNr();
    static void releasePortNr(const string& portId);
    
  private: // data members
    GSAPortService*   _pPortService;
    string            _destDpName;
    GCFPVString       _portId;
    string            _remotePortId;
    GCFPVString       _portAddr;
    GCFPVSSUIMConverter* _pConverter;
    
    bool _acceptedPort;
    
    typedef set<unsigned int> TPVSSPortNrs;
    static TPVSSPortNrs _pvssPortNrs;        
};

// Abstract converter class to be able to convert string messages from/for UIM
class GCFPVSSUIMConverter
{
  public:
    virtual bool uimMsgToGCFEvent(unsigned char* buf, unsigned int length, GCFPVBlob& gcfEvent) = 0;
    virtual bool gcfEventToUIMMsg(GCFPVBlob& gcfEvent, GCFPVBlob& uimMsg) = 0;
};

#endif
