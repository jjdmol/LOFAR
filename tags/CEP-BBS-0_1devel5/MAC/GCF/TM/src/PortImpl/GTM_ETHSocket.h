//#  GTM_ETHSocket.h: base class for all sockets
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

#ifndef GTM_ETHSOCKET_H
#define GTM_ETHSOCKET_H

#include <unistd.h>
//#include <MACIO/GCF_Event.h>
#include "GTM_File.h"

#include <linux/if_packet.h>
#include <linux/if_ether.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {

// forward declaration
class GCFETHRawPort;

/**
 * This class consists of the basic implementation of a ETH socket. 
 */

class GTMETHSocket : public GTMFile
{
  public:
    GTMETHSocket (GCFETHRawPort& port);
    virtual ~GTMETHSocket ();
  
    /**
     * @param ethertype argument to set the type
     * of Ethernet frames being sent.
     */
    virtual int open (const char* ifname,
                      const char* destMac,
		                  unsigned short ethertype = 0x0000);
  
    /**
     * send/recv methods
     */
    virtual ssize_t send (void* buf, size_t count);
    virtual ssize_t recv (void* buf, size_t count);

  private:
    GTMETHSocket ();
    /// Don't allow copying of the GTMETHSocket object.

    GTMETHSocket (const GTMETHSocket&);
    GTMETHSocket& operator= (const GTMETHSocket&);

  private: // helper methods
    void convertCcp2sllAddr(const char* destMacStr,
                            char destMac[ETH_ALEN]);
    
  private:
    char  _recvPacket[ETH_FRAME_LEN];   
    char  _sendPacket[ETH_FRAME_LEN];
    char* _sendPacketData; // pointer to start of packet payload data
    struct sockaddr_ll _sockaddr;
};
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#endif
