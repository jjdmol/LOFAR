//#  -*- mode: c++ -*-
//#
//#  CDOWrite.h: Synchronize CEP data output settings with hardware
//#
//#  Copyright (C) 2002-2004
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

#ifndef CDOWRITE_H_
#define CDOWRITE_H_

#include <Common/LofarTypes.h>
#include <APL/RSP_Protocol/MEPHeader.h>
#include <net/ethernet.h>

#include "SyncAction.h"

namespace LOFAR {
  namespace RSP {

    class CDOWrite : public SyncAction
    {
    public:
      /**
       * Constructors for a CDOWrite object.
       */
      CDOWrite(GCFPortInterface& board_port, int board_id);
	  
      /* Destructor for CDOWrite. */
      virtual ~CDOWrite();

      /**
       * Write subband selection info.
       */
      virtual void sendrequest();

      /**
       * Read the board status.
       */
      virtual void sendrequest_status();

      /**
       * Handle the READRES message.
       */
      virtual GCFEvent::TResult handleack(GCFEvent& event, GCFPortInterface& port);

    private:
      //private methods

      /**
       * Convert a string containing a Ethernet MAC address
       * to an array of 6 bytes.
       */
      void string2mac(const char* macstring, uint8 mac[ETH_ALEN]);

			/**
       * Convert a string containing an IP address
       * to an array of 4 bytes.
       */
			void string2ip(const char* ipstring, uint8 ip[IP_ALEN]);
			
			/**
       * Convert a string containing an IP address
       * to an uint32.
       */
      uint32 string2ip_uint32(const char* ipstring);

      /**
       * Setup an appropriate UDP/IP header
       */
      void setup_udpip_header(uint32 l_srcip, uint32 l_dstip);

      /**
       * Compute the 16-bit 1-complements checksum for the IP header.
       */
      uint16 compute_ip_checksum(void* addr, int count);

    private:

      // private type
      typedef struct
      {
	struct { // IP header Ref RFC791
	  //                        Field Index   Explanation
	  unsigned ihl:4;           /* [1]  Internet Header Length, length of IP packet header in 32 bit words
	                             *      Suggested value = 6 (incl. padding)
				     */
	  unsigned version:4;       /* [0]  Format of IP packet header (Version = 4 indicates IPv4) */
	  unsigned dscp:8;          /* [2]  Differentiated Services Code Point
	                             *      Suggested DSCP value = 0x00 (meaning Best Effort)
				     */
	  unsigned total_length:16; /* [3]  Length of the packet including the header */
	  unsigned seqnr:16;        /* [4]  Identification (sequence number, used for reassembly) */
	  unsigned flags_offset:16; /* [5,6]  First 3 bits are for flags which control fragmentation 
				     *        Suggested falgs avlue = 0x2 (do not fragment, last fragment)
				     *        Fragment offset should be set to 0
				     */
	  unsigned ttl:8;           /* [7]  Time To Live, when decremented to zero, the datagram is discarded
				     *      Suggested ttl value = 128
				     *      Meaning: allow 128 hops
				     */
	  unsigned protocol:8;      /* [8]  Protocol specifies the next encapsulated protocol
				     *      Suggested protocol value = 17
				     *      Meaning: UDP
				     */
	  unsigned hdrchksum:16;    /* [9]  Header checksum, 16 bit one's complement checksum of IP header and IP options */
	  unsigned srcip:32;        /* [10] Source IP-address */
	  unsigned dstip:32;        /* [11] Destinatino IP-address */
	} ip;
	struct { // UDP header
	  uint16 srcport;  /* The port number of the sender.
			    * Suggested srcport value = 0x10FA(4346) + index of RSP board
			    */
	  uint16 dstport;  /* The port this packet is addresse to
			    * Suggested dstport value = 0x10FA(4346) + index of RSP board
			    */
	  uint16 length;   /* length in bytes of the UDP header (8 bytes) + data (minimum value 8) */
	  uint16 checksum; /* Checksum of IP header, UDP header and data
			    * Suggested checksum value = 0 (meaning that check summing is disabled)
			    */
	} udp;
      } UDPIPType;

    private:

      // private data
      EPA_Protocol::MEPHeader m_hdr;
      UDPIPType               m_udpip_hdr;
      uint8                   m_srcmac[ETH_ALEN];
      uint8                   m_dstmac[ETH_ALEN];
    };
  };
};
     
#endif /* CDOWRITE_H_ */
