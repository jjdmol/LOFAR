//#  GTM_ETHSocket.cc: base class for all sockets
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

#include "GTM_ETHSocket.h"
#include "GTM_SocketHandler.h"
#include "GCF_ETHRawPort.h"
#include <GCF/GCF_Task.h>
#include <GTM_Defines.h>
#include <GCF/GCF_TMProtocols.h>
#include <GCF/GCF_PeerAddr.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

//#required includes according to man packet(7)
#include <sys/socket.h>
#include <features.h>    /* for the glibc version number */
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>     /* the L2 protocols */
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>   /* The L2 protocols */
#endif


GTMETHSocket::GTMETHSocket(GCFETHRawPort& port) :
  GTMSocket(port)
{
}

GTMETHSocket::~GTMETHSocket()
{
  close();
}

ssize_t GTMETHSocket::send(void* buf, size_t count)
{
  ssize_t result = 0;
  memcpy(_sendPacketData, (char*)buf, count);
  result = sendto(_socketFD, 
                  _sendPacket, 
                  count + sizeof(struct ethhdr), 0,
                 (struct sockaddr*)&_sockaddr,
                  sizeof(struct sockaddr_ll));
  return result - sizeof(struct ethhdr);
}

ssize_t GTMETHSocket::recv(void* buf, size_t count)
{
  ssize_t result = -1;

  if (count < ETH_DATA_LEN) return -1;

  struct sockaddr_ll recvSockaddr;
  socklen_t recvSockaddrLen = sizeof(struct sockaddr_ll);
  result = recvfrom(_socketFD, _recvPacket, ETH_FRAME_LEN,
       0, (struct sockaddr*)&recvSockaddr, &recvSockaddrLen);

  if (result < 0) return -1;

  memcpy(buf, _recvPacket + sizeof(struct ethhdr), ETH_DATA_LEN);

  return result - sizeof(struct ethhdr);
}

int GTMETHSocket::open(const char* ifname,
		       const char* destMacStr)
{
  if (_socketFD > -1)
    return 0;
  else
  {
    int socketFD = 0;
    char destMac[ETH_ALEN];
    
    // open the raw socket
    socketFD = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (socketFD < 0)
    {
    	LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, ( 
    			"open(PF_PACKET): %s", 
          strerror(errno)));
    	return socketFD;
    }

    // make large send/recv buffers
    int val = 262144;
    if (setsockopt(socketFD, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val)) < 0) 
    {
      LOFAR_LOG_WARN(TM_STDOUT_LOGGER, (
          "setsockopt(SO_RCVBUF): %s", 
          strerror(errno)));	
    }
    if (setsockopt(socketFD, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val)) < 0) 
    {
      LOFAR_LOG_WARN(TM_STDOUT_LOGGER, (
          "setsockopt(SO_SNDBUF): %s", 
          strerror(errno)));  	
    }

   // find MAC address for specified interface
    struct ifreq ifr;
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
    if (ioctl(socketFD, SIOCGIFHWADDR, &ifr) < 0)
    {
      LOFAR_LOG_FATAL(TM_STDOUT_LOGGER, ( 
          "ioctl(SIOCGIFHWADDR): %s", 
          strerror(errno)));
      close();
      return -1;
    }
  
    string macAddress;
    unsigned int hx;
    char macPart[3];
    // print MAC addres for source
    for (int i = 0; i < ETH_ALEN; i++)
    {
      hx = ifr.ifr_hwaddr.sa_data[i] & 0xff;
      sprintf(macPart, "%02x", hx);
      macAddress += macPart;
      if (i < ETH_ALEN - 1) macAddress += ':';
    }
    LOFAR_LOG_TRACE(TM_STDOUT_LOGGER, ( 
        "SRC (%s) HWADDR: %s", 
        ifname, 
        macAddress.c_str()));
    
    // convert HWADDR string to sll_addr
    convertCcp2sllAddr(destMacStr, destMac);
  
    // print MAC address for destination
    printf("DEST HWADDR: ");
    macAddress = "";
    for (int i = 0; i < ETH_ALEN; i++)
    {
      hx = destMac[i] & 0xff;
      sprintf(macPart, "%02x", hx);
      macAddress += macPart;
      if (i < ETH_ALEN - 1) macAddress += ':';
    }
    LOFAR_LOG_TRACE(TM_STDOUT_LOGGER, ( 
        "DEST HWADDR: %s", 
        macAddress.c_str()));
  
    // fill in packet header for sending messages
    struct ethhdr* hdr = (struct ethhdr*)_sendPacket;
    memcpy(&hdr->h_dest[0],   &destMac[0],                ETH_ALEN);
    memcpy(&hdr->h_source[0], &ifr.ifr_hwaddr.sa_data[0], ETH_ALEN);
    hdr->h_proto = 0x0000;
  
    // get interface index number
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
    if (ioctl(socketFD, SIOCGIFINDEX, &ifr) < 0)
    {
        LOFAR_LOG_FATAL(TM_STDOUT_LOGGER, ( 
            "ioctl(SIOCGIFINDEX)"));
        close();
        return -1;
    }
    int ifindex = ifr.ifr_ifindex;
  
    // bind the socket to the interface
    // for bind only the sll_protocol and sll_ifindex
    // fields of the sockaddr_ll are used
    memset(&_sockaddr, 0, sizeof(struct sockaddr_ll));
    _sockaddr.sll_family = AF_PACKET;
    _sockaddr.sll_protocol = htons(ETH_P_ALL);
    _sockaddr.sll_ifindex = ifindex;
    _sockaddr.sll_hatype = ARPHRD_ETHER;
    if (bind(socketFD, (struct sockaddr*)&_sockaddr,
	     sizeof(struct sockaddr_ll)) < 0)
    {
        LOFAR_LOG_FATAL(TM_STDOUT_LOGGER, ( 
            "GCFETHRawPort::open; bind : %s",
            strerror(errno)));
        close();
        return -1;
    }
  
    // enable PROMISCUOUS mode so we catch all packets
    struct packet_mreq so;
    so.mr_ifindex = ifindex;
    so.mr_type = PACKET_MR_PROMISC;
    so.mr_alen = 0;
    memset(&so.mr_address, 0, sizeof(so.mr_address));
    if (setsockopt(socketFD, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
       (void*)&so, sizeof(struct packet_mreq)) < 0)
    {
    }
  
    //
    // fill in _sockaddr to be used in sendto calls
    //
    // Only the sll_family, sll_addr, sll_halen
    // and sll_ifindex need to be set. The rest
    // should be zero (this is done using memset).
    //
    memset(&_sockaddr, 0, sizeof(struct sockaddr_ll));
    _sockaddr.sll_family = PF_PACKET; // raw communication
    memcpy(&_sockaddr.sll_addr[0], &hdr->h_source[0], ETH_ALEN);
    _sockaddr.sll_halen = ETH_ALEN;
    _sockaddr.sll_ifindex = ifindex;
  
    // set pointer to user data
    _sendPacketData = _sendPacket + sizeof(struct ethhdr);
  
    // initialize the data
    memset(_sendPacketData, 0, ETH_DATA_LEN);
    
    setFD(socketFD);
  
    return (_socketFD < 0 ? -1 : 0);
  }
}

void GTMETHSocket::convertCcp2sllAddr(const char* destMacStr,
              char destMac[ETH_ALEN])
{
  unsigned int hx[ETH_ALEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  sscanf(destMacStr, "%x:%x:%x:%x:%x:%x",
   &hx[0], &hx[1], &hx[2], &hx[3], &hx[4], &hx[5]);
   
  for (int i = 0; i < ETH_ALEN; i++)
  {
      destMac[i] = (char)hx[i];
  }
}
