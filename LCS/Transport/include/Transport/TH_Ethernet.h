//# TH_Ethernet.h: Transport mechanism for Raw Ethernet 
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$


#ifndef LOFAR_TRANSPORT_TH_ETHERNET_H
#define LOFAR_TRANSPORT_TH_ETHERNET_H

#ifndef HAVE_BGL
#ifndef USE_NO_TH_ETHERNET

// \file
// Transport mechanism for Ethernet 

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

// This class defines the transport mechanism for RAW Ethernet 

class TH_Ethernet: public TransportHolder
{
public:
  TH_Ethernet(const string &ifname, 
              const string &srcMac, 
              const string &dstMac, 
              const uint16 etype  = 0x0000,
	      const int receiveBufferSize = -1,
	      const int sendBufferSize = -1); 
  
  virtual ~TH_Ethernet();

  // Returns if socket initialization was successful 
  bool init();

  /// Read the data.
  virtual bool recvBlocking(void* buf, int nbytes, int tag, int offset=0, 
			    DataHolder* dh=0);
  virtual int32 recvNonBlocking(void* buf, int32 nbytes, int tag, int32 offset=0, 
			       DataHolder* dh=0);
  /// Wait for the data to be received
  virtual void waitForReceived(void* buf, int nbytes, int tag);
  /// Write the data.

  virtual bool sendBlocking(void* buf, int nbytes, int tag, DataHolder* dh=0);
  virtual bool sendNonBlocking(void* buf, int nbytes, int tag, DataHolder* dh=0);
  /// Wait for the data to be sent
  virtual void waitForSent(void* buf, int nbytes, int tag);

  // Read the total message length of the next message.
  virtual void readTotalMsgLengthBlocking(int tag, int& nrBytes);

  // Read the total message length of the next message.
  virtual bool readTotalMsgLengthNonBlocking(int tag, int& nrBytes);

  // Get the type of transport.
  virtual string getType() const;

  // Is TH_Ethernet clonable?
  virtual bool isClonable() const;

  // Make an instance of the transportholder
  virtual TH_Ethernet* clone() const;

  void reset();

 protected:  
  int32 itsSocketFD;
  int32 itsMaxdatasize;
  int32 itsMaxframesize;
  int32 itsDHheaderSize;
  
  bool itsInitDone;
  
  string itsIfname;
  string itsSrcMac;
  string itsDstMac;
  
  char* itsRecvPacket; 
  char* itsSendPacket; 
  char* itsSendPacketData;
  
  uint16 itsEthertype;

  struct sockaddr_ll itsSockaddr;

  void initSocket();
  void initBuffers();
  void initIncomingFilter();
  void initSendHeader();
  void bindToIF();

  int itsRecvBufferSize;
  int itsSendBufferSize;

  bool itsUsePromiscuousReceive;
};

inline bool TH_Ethernet::isClonable() const
  { return true; }


inline void TH_Ethernet::reset()
  {}

// @} // Doxygen endgroup Transport

}

#endif
#endif

#endif
