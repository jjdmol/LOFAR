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
  GTMSocket(), _port(port)
{
}

GTMETHSocket::~GTMETHSocket()
{
  close();
}

ssize_t GTMETHSocket::send(void* buf, size_t count)
{
  ssize_t result;
  memcpy(_sendPacketData, (char*)buf, count);
  result = sendto(_socketFD, 
                  _sendPacket, 
                  count + sizeof(struct ethhdr), 0,
                 (struct sockaddr*)&_sockaddr,
                  sizeof(sockaddr));
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

int GTMETHSocket::open(GCFPeerAddr& /*addr*/)
{
  if (_socketFD > -1)
    return 0;
  else
  {
    int socketID = 0;
    // open the raw socket
    socketID = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (socketID < 0) return _socketID;
  
    // find MAC address for specified interface
    struct ifreq ifr;
    strncpy(ifr.ifr_name, _ifname, IFNAMSIZ-1);
    if (ioctl(_socketID, SIOCGIFHWADDR, &ifr) < 0)
    {
      LOFAR_LOG_FATAL(TM_STDOUT_LOGGER, ( 
          "ioctl(SIOCGIFHWADDR)"));
      close();
      return -1;
    }
  
    // print MAC addres for source
    LOFAR_LOG_TRACE(TM_STDOUT_LOGGER, ( 
        "SRC (%s) HWADDR: ", 
        _ifname));
    for (int i = 0; i < ETH_ALEN; i++)
    {
      unsigned int hx = ifr.ifr_hwaddr.sa_data[i] & 0xff;
      printf("%02x", hx);
      if (i < ETH_ALEN - 1) printf(":");
      else                  printf("\n");
    }
  
    // convert HWADDR string to sll_addr
    convertCcp2sllAddr(_destMacStr, destMac);
  
    // print MAC address for destination
    printf("DEST HWADDR: ");
    unsigned int hx;
    for (int i = 0; i < ETH_ALEN; i++)
    {
      hx = destMac[i] & 0xff;
      printf("%02x", hx);
      if (i < ETH_ALEN - 1) printf(":");
      else                  printf("\n");
    }
  
    // fill in packet header for sending messages
    struct ethhdr* hdr = (struct ethhdr*)_sendPacket;
    memcpy(&hdr->h_dest[0],   &destMac[0],                ETH_ALEN);
    memcpy(&hdr->h_source[0], &ifr.ifr_hwaddr.sa_data[0], ETH_ALEN);
    hdr->h_proto = 0x0000;
  
    // get interface index number
    strncpy(ifr.ifr_name, _ifname, IFNAMSIZ-1);
    if (ioctl(socketID, SIOCGIFINDEX, &ifr) < 0)
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
    if (bind(socketID, (struct sockaddr*)&_sockaddr, sizeof(sockaddr)) < 0)
    {
        LOFAR_LOG_FATAL(TM_STDOUT_LOGGER, ( 
            "GCFETHRawPort::open; bind"));
        close();
        return -1;
    }
  
    // enable PROMISCUOUS mode so we catch all packets
    struct packet_mreq so;
    so.mr_ifindex = ifindex;
    so.mr_type = PACKET_MR_PROMISC;
    so.mr_alen = 0;
    memset(&so.mr_address, 0, sizeof(so.mr_address));
    if (setsockopt(socketID, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
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
    
    setFD(socketID);
  
    schedule_connected();
    return (_socketFD < 0 ? -1 : 0);
  }
}

void GTMETHSocket::workProc()
{
  GCFEvent e(F_DISCONNECTED_SIG);
  GCFEvent::TResult status;
  
  ssize_t bytesRead = read(_socketFD, &e, sizeof(e));
  if (bytesRead == 0)
  {
    status = _port.dispatch(e);    
  }
  else if (bytesRead == sizeof(e))
  {
    status = eventReceived(e);
    if (status != GCFEvent::HANDLED)
    {
      LOFAR_LOG_INFO(TM_STDOUT_LOGGER, (
        "Event %s for task %s on port %s not handled or an error occured",
        _port.evtstr(e),
        _port.getTask()->getName().c_str(), 
        _port.getName().c_str()
        ));
    }
  }
}

GCFEvent::TResult GTMETHSocket::eventReceived(const GCFEvent& e)
{
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  char*   event_buf  = 0;
  GCFEvent* full_event = 0;

  event_buf = (char*)malloc(e.length);
  full_event = (GCFEvent*)event_buf;

  memcpy(event_buf, &e, sizeof(e));
  if (e.length - sizeof(e) > 0)
  {
    // recv the rest of the message (payload)
    ssize_t payloadLength = e.length - sizeof(e);
    
    ssize_t count = recv(event_buf + sizeof(e),
                          payloadLength);
    
    if (payloadLength != count)
    {
      LOFAR_LOG_FATAL(TM_STDOUT_LOGGER, (
          "truncated recv on event %s (missing %d bytes)",
          _port.evtstr(e),
          payloadLength - count
          ));
    }
    if (payloadLength != count) // retry to read the rest
    {
      //TODO: Make this retry more secure
      usleep(10);
      count += recv(event_buf + sizeof(e) + count ,
                           payloadLength - count);
      assert(payloadLength == count);
    }
  }

  status = _port.dispatch(*full_event);

  free(event_buf);
  return status;
}
