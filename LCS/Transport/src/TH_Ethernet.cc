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

#include <stdio.h>

namespace LOFAR
{


TH_Ethernet::TH_Ethernet(const char* ifname, 
              const char* destMac, 
              unsigned short ethertype)
{
  LOG_TRACE_FLOW("TH_Ethernet constructor");
  
  sprintf(_ifname, ifname);
  sprintf(_destMac, destMac);
  _ethertype = ethertype;

  _socketFD = -1;
  _initDone = false;
}

TH_Ethernet::~TH_Ethernet()
{
  LOG_TRACE_FLOW("TH_Ethernet destructor");
  
  if (_initDone) close(_socketFD);
}

TH_Ethernet* TH_Ethernet::make() const
{
    return new TH_Ethernet(_ifname, _destMac, _ethertype);
}

bool TH_Ethernet::init()
{
  Init();
  return _initDone;
}


string TH_Ethernet::getType() const
{
    return "TH_Ethernet";
}

bool TH_Ethernet::recvBlocking(void* buf, int nbytes, int tag)
{  
  int length = 0;
  
  struct sockaddr_ll recvSockaddr;
  socklen_t recvSockaddrLen = sizeof(struct sockaddr_ll);

  length = recvfrom(_socketFD, _recvPacket, nbytes + sizeof(struct ethhdr) , 0,
                (struct sockaddr*)&recvSockaddr, &recvSockaddrLen);
  
  if (length <= 0) return false; 
  else {
  
    //cout << "TH_Ethernet -> bytes read = " << length << endl;
    
    //string PacketStr = ""; 
    //for (int i=0; i<length; i++) PacketStr += _recvPacket[i];
    //cout << "TH_Ethernet -> received packet = " << PacketStr << endl;
    
    // Filter ethernet-header
    memcpy(buf, _recvPacket + sizeof(struct ethhdr), nbytes);

    return true;
  }
}

bool TH_Ethernet::recvVarBlocking(int tag)
{
  // Read dataholder info.
  DataHolder* target = getTransporter()->getDataHolder();
  void* buf = target->getDataPtr();
  int32 size = target->getDataSize();
  
  if (!recvBlocking(buf, size, tag)) return false;  
}

bool TH_Ethernet::recvNonBlocking(void* buf, int nbytes, int tag)
{
  LOG_WARN( "TH_Ethernet::recvNonBlocking() is not implemented. recvBlocking() is used instead." );    
  return recvBlocking(buf, nbytes, tag);
}

bool TH_Ethernet::recvVarNonBlocking(int tag)
{
  LOG_WARN( "TH_Ethernet::recvVarNonBlocking() is not implemented. recvVarBlocking() is used instead." );    
  return recvVarBlocking(tag);
}

bool TH_Ethernet::waitForReceived(void*, int, int)
{
  LOG_TRACE_RTTI("TH_Ethernet waitForReceived()");
  return true;
}

bool TH_Ethernet::sendBlocking(void* buf, int nbytes, int tag)
{
  int length = 0;
  int count  = nbytes;
 
  memcpy(_sendPacketData, (char*)buf, nbytes);
  
  //Make payload at least 46 bytes long
  //Payload cannot contain more than 1500 bytes 
  if (count < ETH_ZLEN - ETH_HLEN) count = ETH_ZLEN - ETH_HLEN;
  else if (count > ETH_DATA_LEN) {
    LOG_ERROR_STR("TH_Ethernet::sendBlocking: payload contains more than 1500 bytes.");
    return false;
  }
  
  //string packetStr = ""; 
  //for (int i=0;i<count + sizeof(struct ethhdr);i++) packetStr += _sendPacket[i];
  //cout << "TH_Ethernet ->  sent packet = " << packetStr << endl;
    
  length = sendto(_socketFD, _sendPacket, count + sizeof(struct ethhdr), 0,
  	  (struct sockaddr*)& _sockaddr, sizeof(struct sockaddr_ll));

  //cout << "TH_Ethernet -> bytes sent = " << length << endl;
  
  return (length > 0); 
}

bool TH_Ethernet::sendNonBlocking(void* buf, int nbytes, int tag)
{
  LOG_WARN( "TH_Ethernet::sendNonBlocking() is not implemented." );
  return false;
}

bool TH_Ethernet::waitForSent(void*, int, int)
{
  LOG_WARN( "TH_Ethernet::waitForSent() is not implemented." );
  return false;
}

void TH_Ethernet::Init()
{ 
  // Initialize a kernel-level packet filter
  // filtering source Mac-address and destination Mac-addres
  struct sock_fprog filter;
  struct sock_filter mac_filter_insn[]=
    {
      {0x20,0,0,0x00000002},
      {0x15,0,7,0x00000000}, //dstMac [2,3,4,5]
      {0x28,0,0,0x00000000},
      {0x15,0,5,0x00000000}, //dstMac [0,1]
      {0x20,0,0,0x00000008},
      {0x15,0,3,0x00000000}, //srcMac [2,3,4,5]
      {0x28,0,0,0x00000006},
      {0x15,0,1,0x00000000}, //srcMac [0,1]
      {0x6,0,0,0x0000ffff},  //snapshot length (0x0000ffff=all data)
      {0x6,0,0,0x00000000}     
    };
  memset(&filter,0,sizeof(struct sock_fprog));
  filter.len = sizeof(mac_filter_insn) / sizeof(struct sock_filter);

  char destMac[ETH_ALEN];
    
  // Open the raw socket
  _socketFD = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (_socketFD < 0)
  {
    LOG_ERROR_STR("TH_Ethernet: socket could not be opened.");
    return;
  }
    
  // Make large send and receive buffers
  int val = 262144;
  if (setsockopt(_socketFD, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val)) < 0)
  {
    LOG_WARN("TH_Ethernet: send buffer size not increased, default size will be used.");
  }
  if (setsockopt(_socketFD, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val)) < 0)
  {
    LOG_WARN("TH_Ethernet: receive buffer size not increased, default size will be used.");
  }

  // Find ownMAC address for specified interface
  // Mac address will be stored in ifr.ifr_hwaddr.sa_data
  struct ifreq ifr;
  strncpy(ifr.ifr_name,_ifname, IFNAMSIZ);
  if (ioctl(_socketFD, SIOCGIFHWADDR, &ifr) < 0)
  {
    LOG_ERROR_STR("TH_Ethernet: MAC address of ethernet device not found.");
    close(_socketFD);
    return;
  }  
    
  // Convert _destMac HWADDR string to sll_addr
  unsigned int hx[ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
 
  sscanf(_destMac, "%X:%X:%X:%X:%X:%X", 
     &hx[0],&hx[1],&hx[2],&hx[3],&hx[4],&hx[5]);
    
  for (int i=0;i<ETH_ALEN;i++)
  {
    destMac[i]=(char)hx[i];
  }

  // Store the destMAC and ownMAC in the packet filter so only
  // the raw ethernet packets from srcMAC to ownMAC will be catched
  memcpy(&mac_filter_insn[1].k, ifr.ifr_hwaddr.sa_data + 2, sizeof(__u32));
  mac_filter_insn[1].k = htonl(mac_filter_insn[1].k);
  memcpy((char*)(&mac_filter_insn[3].k) + 2, ifr.ifr_hwaddr.sa_data, sizeof(__u16));
  mac_filter_insn[3].k = htonl(mac_filter_insn[3].k);
  memcpy(&mac_filter_insn[5].k, destMac + 2, sizeof(__u32));
  mac_filter_insn[5].k = htonl(mac_filter_insn[5].k);
  memcpy((char*)(&mac_filter_insn[7].k) + 2, destMac, sizeof(__u16));
  mac_filter_insn[7].k = htonl(mac_filter_insn[7].k);
  filter.filter = mac_filter_insn;
  if (setsockopt(_socketFD, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(struct sock_fprog)) < 0)
  {
    LOG_ERROR_STR("TH_Ethernet: Packet filter failure.");
    close(_socketFD);
  }

  // Fill in packet header for sending messages
  struct ethhdr* hdr = (struct ethhdr*)_sendPacket;
  memcpy(&hdr->h_dest[0], &destMac[0], ETH_ALEN);
  memcpy(&hdr->h_source[0], &ifr.ifr_hwaddr.sa_data[0], ETH_ALEN);
  hdr->h_proto = htons(_ethertype);

  // Get interface index number
  strncpy(ifr.ifr_name, _ifname, IFNAMSIZ);
  if (ioctl(_socketFD, SIOCGIFINDEX, &ifr) <0)
  {
    LOG_ERROR_STR("TH_Ethernet: Interface index number not found.");
    close(_socketFD);
    return;
  }
  int ifindex = ifr.ifr_ifindex;

  // Bind the socket to the interface
  // For bind only the sll_protocol and 
  // sll_ifindex fields of the socketaddr_ll are used
  memset(&_sockaddr, 0, sizeof(struct sockaddr_ll));
  _sockaddr.sll_family = AF_PACKET;
  _sockaddr.sll_protocol = htons(ETH_P_ALL);
  _sockaddr.sll_ifindex = ifindex;
  _sockaddr.sll_hatype = ARPHRD_ETHER; 
  if (bind(_socketFD, (struct sockaddr*)&_sockaddr, sizeof(struct sockaddr_ll)) < 0)
  {
    LOG_ERROR_STR("TH_Ethernet: binding the socket to the interface failed.");
    close(_socketFD);
    return;
  }

  // Enable PROMISCUOUS mode so we catch all packets
  struct packet_mreq so;
  so.mr_ifindex = ifindex;
  so.mr_type = PACKET_MR_PROMISC;
  so.mr_alen = 0;
  memset(&so.mr_address, 0, sizeof(so.mr_address));
  if (setsockopt(_socketFD, SOL_PACKET, PACKET_ADD_MEMBERSHIP, 
     (void*)&so, sizeof(struct packet_mreq)) <0)
  {
     LOG_ERROR_STR("TH_Ethernet: PROMISCUOUS mode failed.");
    close(_socketFD);
    return;
  }

  try {
    // fill in _sockaddr to be used in sendto calls
    // Only the sll_family, sll_addr, sll_halen and
    // sll_ifindex need to be set. The rest should be
    // zero (this is done using memset)
    memset(&_sockaddr, 0, sizeof(struct sockaddr_ll));
    _sockaddr.sll_family = PF_PACKET; //raw communication
    memcpy(&_sockaddr.sll_addr[0], &hdr->h_source[0], ETH_ALEN);
    _sockaddr.sll_halen = ETH_ALEN;
    _sockaddr.sll_ifindex = ifindex;

    // Set pointer to user data
    _sendPacketData = _sendPacket + sizeof(struct ethhdr);

    // Initialize the data
    memset(_sendPacketData,0, ETH_DATA_LEN);

    _initDone = true;
  } 
  catch (std::exception& x) {
    LOG_ERROR_STR("TH_Ethernet: unexpected exception: %s" << x.what()); 
    close(_socketFD);
  }
  
}

}


