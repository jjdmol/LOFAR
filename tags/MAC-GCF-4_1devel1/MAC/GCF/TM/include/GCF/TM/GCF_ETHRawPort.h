//# GCF_ETHRawPort.h: raw socket connection to capture ethernet frames
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//# $Id$

#ifndef GCF_ETHRAWPORT_H
#define GCF_ETHRAWPORT_H

#include <GCF/GCF_Defines.h>
#include <GCF/TM/GCF_RawPort.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>

#include <string>

//#forward declaration
class GCFTask;
class GTMETHSocket;

/**
 * This class implements the datagram socket communication
 * port. Datagram sockets are an unreliable transport mechanism and this
 * class does not make the transport reliable. This means that calls to
 * send are not guaranteed to transport the data to the peer. What is
 * guaranteed however is that the message sent with the send() method is
 * either received completely, or not received at all.
 *
 * @bug Using the GCFETHRawPort for protocol messages (as slave of the GCFPort)
 * currently doesn't work due to a problem with combining sendv ("gather-write")
 * with multiple regular recv's.`
 */
class GCFETHRawPort : public GCFRawPort
{
  public:

    /// Construction methods
    /** @param protocol NOT USED */    
    explicit GCFETHRawPort (GCFTask& task,
                   string name,
                   TPortType type, 
                   bool transportRawData = false);
    explicit GCFETHRawPort();

    virtual ~GCFETHRawPort();

    // GCFPortInterface methods

  public:

    /**
     * open/close functions
     */
    virtual bool open();
    virtual bool close();
  
    /**
     * send/recv functions
     */
    virtual ssize_t send(GCFEvent& event);
    virtual ssize_t recv(void* buf, size_t count);
  
    // EOF GCFPortInterface methods

  private:

    /**
     * Don't allow copying of the FPort object.
     */
    GCFETHRawPort(const GCFETHRawPort&);
    GCFETHRawPort& operator=(const GCFETHRawPort&);

  public:

    // addr is local address if getType == SPP
    // addr is remote addres if getType == SAP
    void setAddr(const char* ifname,
  	       const char* dest_mac);
  
    /**
     * Set the ethernet type of the ethernet
     * frames.
     * If you don't call this method. Ethernet
     * frames will have type 0x0000.
     */
    void setEtherType(unsigned short type);
  
  protected:
//    friend class GTMSocket;
//    friend class GTMETHSocket;

  private:    
    string _ifname;
    string _destMacStr;
    GTMETHSocket* _pSocket;
    unsigned short _ethertype;
};

#endif /* GCF_ETHRAWPORT_H */
