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


TH_Ethernet::TH_Ethernet(char* ifname, char* rMac, char* oMac, unsigned short etype, bool dhcheck)
     : _ifname(ifname), 
       _remoteMac(rMac),
       _ownMac(oMac), 
       _ethertype(etype),
       _dhcheck(dhcheck) 
{
  LOG_TRACE_FLOW("TH_Ethernet constructor");
  
  _socketFD = -1;
  _initDone = false;

}

TH_Ethernet::~TH_Ethernet()
{
  LOG_TRACE_FLOW("TH_Ethernet destructor");
  
  if (_initDone) {
    free(_sendPacket);
    free(_recvPacket);
    close(_socketFD);
  }
}

TH_Ethernet* TH_Ethernet::make() const
{
  return new TH_Ethernet(_ifname, _remoteMac, _ownMac, _ethertype, _dhcheck);
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
  if (!_initDone) return false;

  int framesize  = 0;
  int bytesinbuf = 0;
  
  struct sockaddr_ll recvSockaddr;
  socklen_t recvSockaddrLen = sizeof(struct sockaddr_ll);
 
  // Repeat recvfrom call until 'buf' is filled with 'nbytes'
  while (bytesinbuf < nbytes) {
    
    framesize = recvfrom(_socketFD, _recvPacket, _maxframesize , 0,
                          (struct sockaddr*)&recvSockaddr, &recvSockaddrLen);
    
    // Packets containing less than MIN_FRAME_LEN bytes will be ignored
    if (framesize > MIN_FRAME_LEN) { 
      // Copy payload, filter header
      // Note that 'buf' cannot contain more than 'nbytes'
      if (bytesinbuf + framesize - (int)sizeof(struct ethhdr) <= nbytes) {
        memcpy((char*)buf + bytesinbuf, _recvPacket + sizeof(struct ethhdr), framesize - sizeof(struct ethhdr));
        bytesinbuf += framesize - sizeof(struct ethhdr);  
      }
      else {
        memcpy((char*)buf + bytesinbuf, _recvPacket + sizeof(struct ethhdr), nbytes - bytesinbuf);
        bytesinbuf += nbytes - bytesinbuf;
      }
    }  
  }
  if (_dhcheck )return (bytesinbuf == nbytes);
  else return false;
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
  if (!_initDone) return false;

  int framesize   = 0;
  int bytesoutbuf = 0;
 
  // Payload at least 46 bytes long 
  if (nbytes < 46) {
    LOG_ERROR_STR("TH_Ethernet::sendBlocking: payload must contain at least 46 bytes.");
    return false;
  }
  
  if (nbytes <= _maxdatasize) {
    //total message fits in one ethernet frame
    memcpy(_sendPacketData, (char*)buf, nbytes);
    framesize = sendto(_socketFD, _sendPacket, nbytes + sizeof(struct ethhdr), 0,
  	  (struct sockaddr*)& _sockaddr, sizeof(struct sockaddr_ll));
    bytesoutbuf = framesize - sizeof(struct ethhdr);
  }
  else {
    // Total message doesn't fit in one ethernet frame
    // Repeat sendto call until 'nbytes' are sent
    while (bytesoutbuf < nbytes) {
      if (nbytes - bytesoutbuf < _maxdatasize)
        memcpy(_sendPacketData, (char*)buf + bytesoutbuf, nbytes - bytesoutbuf);
      else
        memcpy(_sendPacketData, (char*)buf + bytesoutbuf, _maxdatasize);

      framesize = sendto(_socketFD, _sendPacket, _maxframesize, 0,
  	                  (struct sockaddr*)& _sockaddr, sizeof(struct sockaddr_ll));
      bytesoutbuf += framesize - sizeof(struct ethhdr);
    }
  }
  return (bytesoutbuf == nbytes); 
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
  _socketFD = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (_socketFD < 0)
  {
    LOG_ERROR_STR("TH_Ethernet: socket could not be opened.");
    cout << "TH_Ethernet: socket could not be opened" << endl;
    return;
  }

  struct ifreq ifr;
  // Get MTU of specified interface and allocate memory for 
  // send and receivebuffers
  strncpy(ifr.ifr_name, _ifname, IFNAMSIZ);
  if (ioctl(_socketFD, SIOCGIFMTU, &ifr) <0) {
    LOG_ERROR_STR("TH_Ethernet: getting the socket MTU failed.");
    cout << "TH_Ethernet: getting the socket MTU failed" << endl;
    close(_socketFD);
    return; 
  }
  else {
    _maxdatasize  = ifr.ifr_ifru.ifru_mtu;
    _maxframesize = _maxdatasize + ETH_HLEN;
    _recvPacket   = (char*)calloc(_maxframesize, sizeof(char));
    _sendPacket   = (char*)calloc(_maxframesize, sizeof(char));
  }  

  // Make large send and receive buffers
  //((frames/sec)/2) * ((32*1024)+100)
  int val = 262144;
  if (setsockopt(_socketFD, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val)) < 0)
  {
    LOG_WARN("TH_Ethernet: send buffer size not increased, default size will be used.");
  }
  if (setsockopt(_socketFD, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val)) < 0)
  {
    LOG_WARN("TH_Ethernet: receive buffer size not increased, default size will be used.");
  }
  
  char ownMac[ETH_ALEN];
  if (strcmp(_ownMac,"" )== 0) {
    cout << "MacAddress will be extracted from ethernetcard" << endl;
    // Find ownMAC address for specified interface
    // Mac address will be stored ownMac
    strncpy(ifr.ifr_name,_ifname, IFNAMSIZ);
    if (ioctl(_socketFD, SIOCGIFHWADDR, &ifr) < 0)
    {
      LOG_ERROR_STR("TH_Ethernet: MAC address of ethernet device not found.");
      cout << "TH_Ethernet: MAC address of ethernet device not found." << endl;
      close(_socketFD);
      return;
    }  
    for (int i=0;i<ETH_ALEN;i++) ownMac[i] = ifr.ifr_hwaddr.sa_data[i];
  }
  else {
    cout << "User defined Mac address: " << _ownMac << " will be used" << endl; 
    // Convert _ownMac HWADDR string to sll_addr
    unsigned int ohx[ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    sscanf(_ownMac, "%x:%x:%x:%x:%x:%x", 
     &ohx[0],&ohx[1],&ohx[2],&ohx[3],&ohx[4],&ohx[5]);
    for (int i=0;i<ETH_ALEN;i++) ownMac[i]=(char)ohx[i];
  }
  
  // Convert _remoteMac HWADDR string to sll_addr
  char remoteMac[ETH_ALEN];
  unsigned int rhx[ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  sscanf(_remoteMac, "%x:%x:%x:%x:%x:%x", 
     &rhx[0],&rhx[1],&rhx[2],&rhx[3],&rhx[4],&rhx[5]);
  for (int i=0;i<ETH_ALEN;i++) remoteMac[i]=(char)rhx[i];
  
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
  if (setsockopt(_socketFD, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(filter)) < 0)
  {
    LOG_ERROR_STR("TH_Ethernet: Packet filter failure.");
    cout << "TH_Ethernet: Packet filter failure." << endl;
    close(_socketFD);
    return;
  }

  // Fill in packet header for sending messages
  // Now remoteMAC is destMAC and ownMAC is srcMAC
  struct ethhdr* hdr = (struct ethhdr*)_sendPacket;
  memcpy(&hdr->h_dest[0], &remoteMac[0], ETH_ALEN);
  memcpy(&hdr->h_source[0], &ownMac[0], ETH_ALEN);
  hdr->h_proto = htons(_ethertype);

  // Get index number for specified interface
  strncpy(ifr.ifr_name, _ifname, IFNAMSIZ);
  if (ioctl(_socketFD, SIOCGIFINDEX, &ifr) <0)
  {
    LOG_ERROR_STR("TH_Ethernet: Interface index number not found.");
    cout << "TH_Ethernet: Interface index number not found" << endl;
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
    cout << "TH_Ethernet: binding the socket to the interface failed" << endl;
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
    cout << "TH_Ethernet: PROMISCUOUS mode failed" << endl;
    close(_socketFD);
   return;
  }

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

  // Raw ethernet socket successfully initialized
  _initDone = true; 
}

}


