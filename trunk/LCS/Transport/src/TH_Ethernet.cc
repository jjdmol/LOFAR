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

#include <Transport/TH_Ethernet.h>
#include <Transport/BaseSim.h>
#include <Transport/DataHolder.h>
#include <Transport/Transporter.h> 
#include <Common/LofarLogger.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/filter.h>
#include <linux/irda.h>

#include <stdio.h>


namespace LOFAR
{

TH_Ethernet::TH_Ethernet(const string &ifname, 
                         const string &rMac, 
                         const string &oMac, 
                         const uint16 etype, 
                         const bool dhheader)
     : itsIfname(ifname), 
       itsRemoteMac(rMac),
       itsOwnMac(oMac), 
       itsEthertype(etype),
       itsDHheader(dhheader) 
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

TH_Ethernet* TH_Ethernet::make() const
{
  return new TH_Ethernet(itsIfname, itsRemoteMac, itsOwnMac, itsEthertype, itsDHheader);
}

bool TH_Ethernet::init()
{
  Init();
  return itsInitDone;
}

string TH_Ethernet::getType() const
{
  return "TH_Ethernet"; 
}

bool TH_Ethernet::recvBlocking(void* buf, int32 nbytes, int32 tag)
{  
  if (!itsInitDone) {
    return false;
  }
  int32 framesize;
  int32 payloadsize;
  char* endptr;
  char* payloadptr;
  
  struct sockaddr_ll recvSockaddr;
  socklen_t recvSockaddrLen = sizeof(struct sockaddr_ll);
 
  // Pointer to end of buffer
  endptr = (char*)buf + nbytes;

  // if necessary, use offset to prevent that dataholder-header will be overwritten
  if (itsDHheader) {
    (char*)buf += itsDHheaderSize; 
  }

  // Pointer to received data excl. ethernetheader
  payloadptr = itsRecvPacket + sizeof(struct ethhdr);
  
  // Repeat recvfrom call until end of buffer is reached
  while ((char*)buf < endptr) {
    
    // Catch ethernet frame from Socket
    framesize = recvfrom(itsSocketFD, itsRecvPacket, itsMaxframesize , 0,
                          (struct sockaddr*)&recvSockaddr, &recvSockaddrLen);
    
    // Calculate size of payload
    payloadsize = framesize - sizeof(struct ethhdr);

    // Ignore Packets containing less than MIN_FRAME_LEN bytes
    if (framesize <= MIN_FRAME_LEN) {
      continue;
    } 
    
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

bool TH_Ethernet::recvVarBlocking(int32 tag)
{
  // Read dataholder info.
  DataHolder* target = getTransporter()->getDataHolder();
  void* buf = target->getDataPtr();
  int32 size = target->getDataSize();
  
  return recvBlocking(buf, size, tag);
}

bool TH_Ethernet::recvNonBlocking(void* buf, int32 nbytes, int32 tag)
{
  LOG_WARN( "TH_Ethernet::recvNonBlocking() is not implemented. recvBlocking() is used instead." );    
  return recvBlocking(buf, nbytes, tag);
}

bool TH_Ethernet::recvVarNonBlocking(int32 tag)
{
  LOG_WARN( "TH_Ethernet::recvVarNonBlocking() is not implemented. recvVarBlocking() is used instead." );    
  return recvVarBlocking(tag);
}

bool TH_Ethernet::waitForReceived(void*, int32, int32)
{
  LOG_TRACE_RTTI("TH_Ethernet waitForReceived()");
  return true;
}


bool TH_Ethernet::sendBlocking(void* buf, int32 nbytes, int32 tag)
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

bool TH_Ethernet::sendNonBlocking(void* buf, int32 nbytes, int32 tag)
{
  LOG_WARN( "TH_Ethernet::sendNonBlocking() is not implemented." );
  return false;
}

bool TH_Ethernet::waitForSent(void*, int32, int32)
{
  LOG_WARN( "TH_Ethernet::waitForSent() is not implemented." );
  return false;
}

void TH_Ethernet::Init()
{ 
  // Initialize a kernel-level packet filter
  struct sock_fprog filter;
  struct sock_filter mac_filter_insn[]=
    {
      { 0x20, 0, 0, 0x00000002 }, 
      { 0x15, 0, 7, 0x00000000 }, //dstMac [2,3,4,5]
      { 0x28, 0, 0, 0x00000000 }, 
      { 0x15, 0, 5, 0x00000000 }, //dstMac [0,1]
      { 0x20, 0, 0, 0x00000008 },
      { 0x15, 0, 3, 0x00000000 }, //srcMac [2,3,4,5]
      { 0x28, 0, 0, 0x00000006 },
      { 0x15, 0, 1, 0x00000000 }, //srcMac [0,1]
      { 0x6, 0, 0, 0x0000ffff },  //snapshot length
      { 0x6, 0, 0, 0x00000000 }, 
    };
    memset(&filter,0,sizeof(struct sock_fprog));
    filter.len = sizeof(mac_filter_insn) / sizeof(struct sock_filter);
  
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
    itsRecvPacket   = (char*)calloc(itsMaxframesize, sizeof(char));
    itsSendPacket   = (char*)calloc(itsMaxframesize, sizeof(char));
  }  

  // Make large send and receive buffers
  // ((frames/sec)/2) * ((32*1024)+100)
  int32 val = 262144;
  if (setsockopt(itsSocketFD, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val)) < 0)
  {
    LOG_WARN("TH_Ethernet: send buffer size not increased, default size will be used.");
  }
  if (setsockopt(itsSocketFD, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val)) < 0)
  {
    LOG_WARN("TH_Ethernet: receive buffer size not increased, default size will be used.");
  }
  
  char ownMac[ETH_ALEN];
  //if (strcmp(itsOwnMac,"" )== 0) {
  if (itsOwnMac == "") {
    // Find ownMAC address for specified interface
    // Mac address will be stored ownMac
    strncpy(ifr.ifr_name,itsIfname.c_str(), IFNAMSIZ);
    if (ioctl(itsSocketFD, SIOCGIFHWADDR, &ifr) < 0)
    {
      LOG_ERROR_STR("TH_Ethernet: MAC address of ethernet device not found.");
      close(itsSocketFD);
      return;
    }  
    for (int32 i=0;i<ETH_ALEN;i++) ownMac[i] = ifr.ifr_hwaddr.sa_data[i];
  }
  else {
    // Convert _ownMac HWADDR string to sll_addr
    unsigned int ohx[ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    sscanf(itsOwnMac.c_str(), "%x:%x:%x:%x:%x:%x", 
     &ohx[0],&ohx[1],&ohx[2],&ohx[3],&ohx[4],&ohx[5]);
    for (int32 i=0;i<ETH_ALEN;i++) ownMac[i]=(char)ohx[i];
  }
  
  // Convert _remoteMac HWADDR string to sll_addr
  char remoteMac[ETH_ALEN];
  uint32 rhx[ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  sscanf(itsRemoteMac.c_str(), "%x:%x:%x:%x:%x:%x", 
     &rhx[0],&rhx[1],&rhx[2],&rhx[3],&rhx[4],&rhx[5]);
  for (int32 i=0;i<ETH_ALEN;i++) remoteMac[i]=(char)rhx[i];
  
  // Store the MAC addresses in the incoming packet filter, so
  // only packets from remoteMAC (source) to ownMAC (destination) 
  // will be catched
  memcpy(&mac_filter_insn[1].k, ownMac + 2, sizeof(__u32));
  mac_filter_insn[1].k = htonl(mac_filter_insn[1].k);
  memcpy((char*)(&mac_filter_insn[3].k) + 2, ownMac , sizeof(__u16));
  mac_filter_insn[3].k = htonl(mac_filter_insn[3].k);
  memcpy(&mac_filter_insn[5].k, remoteMac + 2, sizeof(__u32));
  mac_filter_insn[5].k = htonl(mac_filter_insn[5].k);
  memcpy((char*)(&mac_filter_insn[7].k) + 2, remoteMac, sizeof(__u16));
  mac_filter_insn[7].k = htonl(mac_filter_insn[7].k);
  filter.filter = mac_filter_insn;
  
  // Set the filter
  if (setsockopt(itsSocketFD, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(filter)) < 0)
  {
    LOG_ERROR_STR("TH_Ethernet: Packet filter failure.");
    close(itsSocketFD);
    return;
  }

  // Fill in packet header for sending messages
  // Now remoteMAC is destMAC and ownMAC is srcMAC
  struct ethhdr* hdr = (struct ethhdr*)itsSendPacket;
  memcpy(&hdr->h_dest[0], &remoteMac[0], ETH_ALEN);
  memcpy(&hdr->h_source[0], &ownMac[0], ETH_ALEN);
  hdr->h_proto = htons(itsEthertype);

  // Get index number for specified interface
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

  // fill in _sockaddr to be used in sendto calls
  // Only the sll_family, sll_addr, sll_halen and
  // sll_ifindex need to be set. The rest should be
  // zero (this is done using memset)
  memset(&itsSockaddr, 0, sizeof(struct sockaddr_ll));
  itsSockaddr.sll_family = PF_PACKET; //raw communication
  memcpy(&itsSockaddr.sll_addr[0], &hdr->h_source[0], ETH_ALEN);
  itsSockaddr.sll_halen = ETH_ALEN;
  itsSockaddr.sll_ifindex = ifindex;

  // Set pointer to user data to be send
  itsSendPacketData = itsSendPacket + sizeof(struct ethhdr);

  // If necessary, determine size of dataholder header
  if (itsDHheader) {
    itsDHheaderSize = getTransporter()->getDataHolder()->getHeaderSize();
  } 

  // Raw ethernet socket successfully initialized
  itsInitDone = true; 
}

}


