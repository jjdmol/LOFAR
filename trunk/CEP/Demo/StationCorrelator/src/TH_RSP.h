//# TH_RSP.h: Transport mechanism for Raw Ethernet 
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

#ifndef TRANSPORT_TH_RSP_H
#define TRANSPORT_TH_RSP_H

// \file TH_RSP.h
// Transport mechanism for Ethernet 

#include <lofar_config.h>
#include <Transport/TransportHolder.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h> 


namespace LOFAR
{

// \addtogroup Transport
// @{

#define MIN_FRAME_LEN 6000

// This class defines the transport mechanism for RAW Ethernet 

class TH_RSP: public TransportHolder
{
public:
  TH_RSP(const string &ifname, 
         const string &rMac, 
         const string &oMac, 
         const uint16 etype  = 0x0000, 
         const bool dhheader = false);
  
  virtual ~TH_RSP();

  // Make an instance of the transportholder
  virtual TH_RSP* make() const;
  
  // Returns if socket initialization was successful 
  virtual bool init();

  // Read the data.
  virtual bool recvBlocking(void* buf, int32 nbytes, int32 tag);
  virtual bool recvVarBlocking(int32 tag);
  virtual bool recvNonBlocking(void* buf, int32 nbytes, int32 tag);
  virtual bool recvVarNonBlocking(int32 tag);

  // Wait for the data to be received
  virtual bool waitForReceived(void* buf, int32 nbytes, int32 tag);

  // Write the data.
  virtual bool sendBlocking(void* buf, int32 nbytes, int32 tag);
  virtual bool sendNonBlocking(void* buf, int32 nbytes, int32 tag);

  // Wait for the data to be sent
  virtual bool waitForSent(void* buf, int32 nbytes, int32 tag);

  // Get the type of transport.
  virtual string getType() const;

  virtual bool isBidirectional() const;
  
 private:  
  int32 itsSocketFD;
  int32 itsMaxdatasize;
  int32 itsMaxframesize;
  
  bool itsDHheader;
  bool itsInitDone;
  
  string itsIfname;
  string itsRemoteMac;
  string itsOwnMac;
  
  char* itsRecvPacket; 
  char* itsSendPacket; 
  char* itsSendPacketData;
  
  uint16 itsEthertype;
  
  struct sockaddr_ll itsSockaddr;

  void Init();
 
 public:

};

inline bool TH_RSP::isBidirectional() const
  { return true; }

// @} // Doxygen endgroup Transport

}

#endif

