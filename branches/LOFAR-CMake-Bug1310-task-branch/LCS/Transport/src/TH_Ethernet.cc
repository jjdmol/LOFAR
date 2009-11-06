//# TH_Ethernet.cc: Transport mechanism for RAW Ethernet
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

#include <lofar_config.h>

#ifndef HAVE_BGL
#ifndef USE_NO_TH_ETHERNET

#include <Transport/TH_Ethernet.h>
#include <Transport/BaseSim.h>
#include <Transport/DataHolder.h> 
#include <Common/LofarLogger.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <asm/types.h>
#include <stdio.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/filter.h>
#ifdef __linux__
#include <sys/sysctl.h>
#endif

#include <stdio.h>
#include <stdlib.h>

namespace LOFAR
{

TH_Ethernet::TH_Ethernet(const string &ifname, 
                         const string &srcMac, 
                         const string &dstMac, 
                         const uint16 etype,
			 const int receiveBufferSize,
			 const int sendBufferSize) 
     : itsIfname(ifname), 
       itsSrcMac(srcMac),
       itsDstMac(dstMac), 
       itsEthertype(etype),
       itsRecvBufferSize(receiveBufferSize),
       itsSendBufferSize(sendBufferSize),
       itsUsePromiscuousReceive(false)
{
  LOG_TRACE_FLOW("TH_Ethernet constructor");
  
  itsSocketFD = -1;
  itsInitDone = false;

}

TH_Ethernet::~TH_Ethernet()
{
  LOG_TRACE_FLOW("TH_Ethernet destructor");
  
  if (itsInitDone) {
    free(itsSendPacket);
    free(itsRecvPacket);
    close(itsSocketFD);
  }
}

TH_Ethernet* TH_Ethernet::clone() const
{
  return new TH_Ethernet(itsIfname, itsSrcMac, itsDstMac, itsEthertype, itsRecvBufferSize, itsSendBufferSize);
}

bool TH_Ethernet::init()
{
  initSocket();
  initBuffers();
  initIncomingFilter();
  initSendHeader();
  bindToIF();

  // Raw ethernet socket successfully initialized
  itsInitDone = true; 

  return itsInitDone;
}

string TH_Ethernet::getType() const
{
  return "TH_Ethernet"; 
}

bool TH_Ethernet::recvBlocking(void* buf, int nbytes, int, int, DataHolder*)
{  
  if (!itsInitDone) {
    return false;
  }
  int32 framesize;
  int32 payloadsize;
  char* endptr;
  char* payloadptr;
  
  // Pointer to end of buffer
  endptr = (char*)buf + nbytes;

  // Pointer to received data excl. ethernetheader
  payloadptr = itsRecvPacket + sizeof(struct ethhdr);
  
  // Repeat recvfrom call until end of buffer is reached
  while ((char*)buf < endptr) {
    
    // Catch ethernet frame from Socket
    framesize = recv(itsSocketFD, itsRecvPacket, itsMaxframesize , 0);
    
#define MIN_FRAME_LEN 200
     // Ignore Packets containing less than MIN_FRAME_LEN bytes
    if (framesize <= MIN_FRAME_LEN) {
      continue;
    } 

    // Calculate size of payload
    payloadsize = framesize - sizeof(struct ethhdr);

    // Copy payload, filter header
    // Note that 'buf' cannot contain more than 'nbytes'
    if ((char*)buf + payloadsize <= endptr) {
      memcpy((char*)buf , payloadptr, payloadsize);
      buf = (char*)buf + payloadsize;  
    }
    else {
      memcpy((char*)buf, payloadptr, endptr - (char*)buf);
      buf = endptr;
    }  
  }
  return true;
}

int32 TH_Ethernet::recvNonBlocking(void* buf, int32 nbytes, int tag, int32, DataHolder*)
{
  LOG_WARN( "TH_Ethernet::recvNonBlocking() is not implemented. recvBlocking() is used instead." );    
  return (recvBlocking(buf, nbytes, tag) ? nbytes : 0);
}

void TH_Ethernet::waitForReceived(void*, int32, int32)
{
  LOG_TRACE_RTTI("TH_Ethernet waitForReceived()");
}


bool TH_Ethernet::sendBlocking(void* buf, int nbytes, int, DataHolder*)
{
  if (!itsInitDone) return false;

  int32 framesize;
  int32 bytesoutbuf = 0;
 
  // Payload at least 46 bytes long 
  if (nbytes < 46) {
    LOG_ERROR_STR("TH_Ethernet::sendBlocking: payload must contain at least 46 bytes.");
    return false;
  }
  
  if (nbytes <= itsMaxdatasize) {
    //total message fits in one ethernet frame
    memcpy(itsSendPacketData, (char*)buf, nbytes);
    framesize = sendto(itsSocketFD, itsSendPacket, nbytes + sizeof(struct ethhdr), 0,
  	  (struct sockaddr*)& itsSockaddr, sizeof(struct sockaddr_ll));
    return ((framesize - (int32)sizeof(struct ethhdr)) == nbytes);
  }
  else {
    // Total message doesn't fit in one ethernet frame
    // Repeat sendto call until 'nbytes' are sent
    while (bytesoutbuf < nbytes) {
      if (nbytes - bytesoutbuf < itsMaxdatasize) {
        memcpy(itsSendPacketData, (char*)buf + bytesoutbuf, nbytes - bytesoutbuf);
      } 
      else {
        memcpy(itsSendPacketData, (char*)buf + bytesoutbuf, itsMaxdatasize);
      }

      framesize = sendto(itsSocketFD, itsSendPacket, itsMaxframesize, 0,
  	                  (struct sockaddr*)& itsSockaddr, sizeof(struct sockaddr_ll));
      bytesoutbuf += framesize - sizeof(struct ethhdr);
    }
    return true;
  }
}

bool TH_Ethernet::sendNonBlocking(void*, int, int, DataHolder*)
{
  LOG_WARN( "TH_Ethernet::sendNonBlocking() is not implemented." );
  return false;
}

void TH_Ethernet::readTotalMsgLengthBlocking(int, int&)
{
  LOG_WARN( "TH_Ethernet::readTotalMsgLengthBlocking() is not implemented." );
}

bool TH_Ethernet::readTotalMsgLengthNonBlocking(int, int&)
{ 
  LOG_WARN( "TH_Ethernet::readTotalMsgLengthNonBlocking() is not implemented." );
  return false;
}


void TH_Ethernet::waitForSent(void*, int32, int32)
{
  LOG_WARN( "TH_Ethernet::waitForSent() is not implemented." );
}

void TH_Ethernet::initSocket() {
  // Open the raw socket
  itsSocketFD = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (itsSocketFD < 0)
  {
    LOG_ERROR_STR("TH_Ethernet: socket could not be opened.");
    return;
  }

  struct ifreq ifr;
  // Get MTU of specified interface and allocate memory for 
  // send and receivebuffers
  strncpy(ifr.ifr_name, itsIfname.c_str(), IFNAMSIZ);
  if (ioctl(itsSocketFD, SIOCGIFMTU, &ifr) <0) {
    LOG_ERROR_STR("TH_Ethernet: getting the socket MTU failed.");
    close(itsSocketFD);
    return; 
  }
  else {
    itsMaxdatasize  = ifr.ifr_ifru.ifru_mtu;
    itsMaxframesize = itsMaxdatasize + ETH_HLEN;
    itsRecvPacket   = new char[itsMaxframesize];
    itsSendPacket   = new char[itsMaxframesize];
  }  

  // Find dstMAC address for specified interface
  // Mac address will be stored dstMac
  strncpy(ifr.ifr_name,itsIfname.c_str(), IFNAMSIZ);
  if (ioctl(itsSocketFD, SIOCGIFHWADDR, &ifr) < 0)
  {
    LOG_ERROR_STR("TH_Ethernet: MAC address of ethernet device not found.");
    close(itsSocketFD);
    return;
  }  
  char dstMac[18];
  unsigned char* sd = (unsigned char*) ifr.ifr_hwaddr.sa_data;
  snprintf(dstMac, sizeof(dstMac), "%02x:%02x:%02x:%02x:%02x:%02x", sd[0], sd[1], sd[2], sd[3], sd[4], sd[5]);
  if (itsDstMac == "") {
    itsDstMac = dstMac;
  }
  if (strcasecmp(itsDstMac.c_str(), dstMac) == 0) {
    itsUsePromiscuousReceive = false;
    LOG_INFO_STR("TH_Ethernet: not using promiscuous mode");
  } else {
    itsUsePromiscuousReceive = true;
    LOG_INFO_STR("TH_Ethernet: using promiscuous mode: reading from "<<itsDstMac<<" while mac of "<<itsIfname<<" is "<<dstMac);
  }
}

void TH_Ethernet::initBuffers() {
  // set the size of the kernel level socket buffer
  // use -1 in the constructor to leave it untouched
  if (itsRecvBufferSize != -1) {
#ifdef __linux__
    int name[] = { CTL_NET, NET_CORE, NET_CORE_RMEM_MAX };
    int value;
    size_t valueSize = sizeof(value);
    // check the max buffer size of the kernel
    sysctl(name, sizeof(name)/sizeof(int), &value, &valueSize, 0, 0);
    if (itsRecvBufferSize > value) {
      // if the max size is not large enough increase it
      if (sysctl(name, sizeof(name)/sizeof(int), 0, 0, &itsRecvBufferSize, sizeof(itsRecvBufferSize)) < 0){
	LOG_WARN("TH_Ethernet: could not increase max socket receive buffer");
      };
    }
#endif
    // now set the buffer for our socket
    if (setsockopt(itsSocketFD, SOL_SOCKET, SO_RCVBUF, &itsRecvBufferSize, sizeof(itsRecvBufferSize)) < 0)
    {
      LOG_WARN("TH_Ethernet: receive buffer size could not be set, default size will be used.");
    }
  }    
  if (itsSendBufferSize != -1) {
#ifdef __linux__
    int name[] = { CTL_NET, NET_CORE, NET_CORE_WMEM_MAX };
    int value;
    size_t valueSize = sizeof(value);
    // check the max buffer size of the kernel
    sysctl(name, sizeof(name)/sizeof(int), &value, &valueSize, 0, 0);
    if (itsSendBufferSize > value) {
      // if the max size is not large enough increase it
      if (sysctl(name, sizeof(name)/sizeof(int), 0, 0, &itsSendBufferSize, sizeof(itsSendBufferSize)) < 0){ 
	LOG_WARN("TH_Ethernet: could not increase max socket send buffer");
      };
    }
#endif
    if (setsockopt(itsSocketFD, SOL_SOCKET, SO_RCVBUF, &itsSendBufferSize, sizeof(itsSendBufferSize)) < 0)
    {
      LOG_WARN("TH_Ethernet: send buffer size could not be set, default size will be used.");
    }
  }    
}

void TH_Ethernet::initIncomingFilter() {
  // Convert itsDstMac HWADDR string to sll_addr
  unsigned int dstMac[ETH_ALEN] = {0, 0, 0, 0, 0, 0};
  sscanf(itsDstMac.c_str(), "%x:%x:%x:%x:%x:%x", 
	 &dstMac[0],&dstMac[1],&dstMac[2],&dstMac[3],&dstMac[4],&dstMac[5]);
  uint32 dstMacHigh = htonl(((dstMac[5]*256 + dstMac[4])*256+dstMac[3])*256+dstMac[2]);
  uint16 dstMacLow = htons(dstMac[1]*256 + dstMac[0]);

  // Convert _srcMac HWADDR string to sll_addr
  unsigned int srcMac[ETH_ALEN] = {0, 0, 0, 0, 0, 0};
  sscanf(itsSrcMac.c_str(), "%x:%x:%x:%x:%x:%x", 
	 &srcMac[0],&srcMac[1],&srcMac[2],&srcMac[3],&srcMac[4],&srcMac[5]);
  uint32 srcMacHigh = htonl(((srcMac[5]*256 + srcMac[4])*256+srcMac[3])*256+srcMac[2]);
  uint16 srcMacLow = htons(srcMac[1]*256 + srcMac[0]);

  uint16 ethType = htons(itsEthertype);

  // Initialize a kernel-level packet filter
  struct sock_filter mac_filter_insn[]=
    {
      BPF_STMT(BPF_LD + BPF_H + BPF_ABS , 12),               // load half word from byte 2
      BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, ethType, 0 ,9),    // compare to ethtype, if not equal jump to the last line (return 0)
      BPF_STMT(BPF_LD + BPF_W + BPF_ABS , 2),                // load whole word from byte 2
      BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, dstMacHigh, 0, 7), // compare to dstMac[2-], if not equal jump to the last line (return 0)
      BPF_STMT(BPF_LD + BPF_H + BPF_ABS , 0),                // load half word from byte 0
      BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, dstMacLow, 0, 5),  // compare to dstMac[0-1], if not equal jump to the last line (return 0)
      BPF_STMT(BPF_LD + BPF_W + BPF_ABS , 8),                // load whole word from byte 8
      BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, srcMacHigh, 0, 3), // compare to srcMac[2-], if not equal jump to the last line (return 0)
      BPF_STMT(BPF_LD + BPF_H + BPF_ABS , 6),                // load half word from byte 6
      BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, srcMacLow, 0, 1),  // compare to srcMac[0-1], if not equal jump to the last line (return 0)
      BPF_STMT(BPF_RET + BPF_K          , 65535),            // right packet so return everythin
      BPF_STMT(BPF_RET + BPF_K          , 0),                // wrong packet so return zero
    };
  struct sock_fprog filter;
  memset(&filter,0,sizeof(struct sock_fprog));
  filter.len = sizeof(mac_filter_insn) / sizeof(struct sock_filter);
  filter.filter = mac_filter_insn;
  
  // Set the filter
  if (setsockopt(itsSocketFD, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(filter)) < 0)
  {
    LOG_ERROR_STR("TH_Ethernet: Packet filter failure.");
    close(itsSocketFD);
    return;
  }
}

void TH_Ethernet::initSendHeader() {

  // Fill in packet header for sending messages
  struct ethhdr* hdr = (struct ethhdr*)itsSendPacket;

  unsigned char* dstMac = hdr->h_dest;//[ETH_ALEN] = {0, 0, 0, 0, 0, 0};
  sscanf(itsDstMac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
	 &dstMac[0],&dstMac[1],&dstMac[2],&dstMac[3],&dstMac[4],&dstMac[5]);

  unsigned char* srcMac = hdr->h_source;
  sscanf(itsSrcMac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
	 &srcMac[0],&srcMac[1],&srcMac[2],&srcMac[3],&srcMac[4],&srcMac[5]);

  hdr->h_proto = htons(itsEthertype);

  // Set pointer to user data to be send
  itsSendPacketData = itsSendPacket + sizeof(struct ethhdr);
}

void TH_Ethernet::bindToIF(){

  // Get index number for specified interface
  struct ifreq ifr;
  strncpy(ifr.ifr_name, itsIfname.c_str(), IFNAMSIZ);
  if (ioctl(itsSocketFD, SIOCGIFINDEX, &ifr) <0)
  {
    LOG_ERROR_STR("TH_Ethernet: Interface index number not found.");
    close(itsSocketFD);
    return;
  }
  int32 ifindex = ifr.ifr_ifindex;

  // Bind the socket to the interface
  // For bind only the sll_protocol and 
  // sll_ifindex fields of the socketaddr_ll are used
  memset(&itsSockaddr, 0, sizeof(struct sockaddr_ll));
  itsSockaddr.sll_family = AF_PACKET;
  itsSockaddr.sll_protocol = htons(ETH_P_ALL);
  itsSockaddr.sll_ifindex = ifindex;
  itsSockaddr.sll_hatype = ARPHRD_ETHER; 
  if (bind(itsSocketFD, (struct sockaddr*)&itsSockaddr, sizeof(struct sockaddr_ll)) < 0)
  {
    LOG_ERROR_STR("TH_Ethernet: binding the socket to the interface failed.");
    close(itsSocketFD);
    return;
  }
  // fill in _sockaddr to be used in sendto calls
  // Only the sll_family, sll_addr, sll_halen and
  // sll_ifindex need to be set. The rest should be
  // zero (this is done using memset)
  memset(&itsSockaddr, 0, sizeof(struct sockaddr_ll));
  itsSockaddr.sll_family = PF_PACKET; //raw communication
  struct ethhdr* hdr = (struct ethhdr*)itsSendPacket;
  memcpy(&itsSockaddr.sll_addr[0], &hdr->h_dest[0], ETH_ALEN);
  itsSockaddr.sll_halen = ETH_ALEN;
  itsSockaddr.sll_ifindex = ifindex;

  if (itsUsePromiscuousReceive){
    // Enable PROMISCUOUS mode so we catch all packets
    struct packet_mreq so;
    so.mr_ifindex = ifindex;
    so.mr_type = PACKET_MR_PROMISC;
    so.mr_alen = 0;
    memset(&so.mr_address, 0, sizeof(so.mr_address));
    if (setsockopt(itsSocketFD, SOL_PACKET, PACKET_ADD_MEMBERSHIP, 
		   (void*)&so, sizeof(struct packet_mreq)) <0)
    {
      LOG_ERROR_STR("TH_Ethernet: PROMISCUOUS mode failed.");
      close(itsSocketFD);
      return;
    }
  }
}

}

#endif
#endif
