//# TH_Ethernet.h: Transport mechanism for Ethernet 
//#
//# Copyright (C) 2000, 2001
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
//#
//# $Id$

#ifndef TRANSPORT_TH_ETHERNET_H
#define TRANSPORT_TH_ETHERNET_H

#include <lofar_config.h>
#include <Transport/TransportHolder.h>

#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h> 


namespace LOFAR
{

/**
   This class defines the transport mechanism for RAW Ethernet 
   communication between dataholders.
*/

class TH_Ethernet: public TransportHolder
{
public:
  TH_Ethernet(const char* ifname, 
              const char* remoteMac, 
              unsigned short ethertype = 0x0000);
  
  virtual ~TH_Ethernet();

  // Make an instance of the transportholder
  virtual TH_Ethernet* make() const;
  
  // Returns if socket initialization was successful 
  virtual bool init();

  // Read the data.
  virtual bool recvBlocking(void* buf, int nbytes, int tag);
  virtual bool recvVarBlocking(int tag);
  virtual bool recvNonBlocking(void* buf, int nbytes, int tag);
  virtual bool recvVarNonBlocking(int tag);
  // Wait for the data to be received
  virtual bool waitForReceived(void* buf, int nbytes, int tag);

  // Write the data.
  virtual bool sendBlocking(void* buf, int nbytes, int tag);
  virtual bool sendNonBlocking(void* buf, int nbytes, int tag);
  // Wait for the data to be sent
  virtual bool waitForSent(void* buf, int nbytes, int tag);

  // Get the type of transport.
  virtual string getType() const;

  virtual bool isBidirectional() const;

  
 private:  
  int _socketFD;
  bool _initDone;
  char _ifname[IFNAMSIZ];
  char _remoteMac[ETH_ALEN];
  char _recvPacket[ETH_FRAME_LEN];
  char _sendPacket[ETH_FRAME_LEN];
  char* _sendPacketData;
  unsigned short _ethertype;
  struct sockaddr_ll _sockaddr;

  void Init();
 
 public:

};

inline bool TH_Ethernet::isBidirectional() const
  { return true; }

}

#endif

