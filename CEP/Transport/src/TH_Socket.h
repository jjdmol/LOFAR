//# TH_Socket.h: POSIX Socket based transportat holder
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

#ifndef CEPFRAME_TH_SOCKET_H
#define CEPFRAME_TH_SOCKET_H

#include <lofar_config.h>
#include <Transport/TransportHolder.h>

// The current (0417-4) version of ACC/Socket makes use of
// log4cplus. Log4cplus is not yet fully implemented in the LOFAR build
// tree. Because of this reason, ACC/Socket generates compiler errors. A
// copy of ACC/Socket has been made, with the log4cplus calls
// removed. This version of Socket has been checked in as
// CEPFrame/src/Socket. After log4cplus is implemented correctly, the
// CEPFrame/Socket class must be removed and ACC/Socket be used.

#include <Transport/Socket.h>

namespace LOFAR
{
  class TH_Socket: public TransportHolder
  {
  public:
    TH_Socket (const std::string &sendhost, const std::string &recvhost, 
	       const int portno);
  
    virtual ~TH_Socket();
  
    virtual TH_Socket* make() const;

  
    virtual bool recvBlocking    (void* buf, int nbytes, int tag);
    virtual bool recvNonBlocking (void* buf, int nbytes, int tag);
    virtual bool waitForReceived (void* buf, int nbytes, int tag);
  
    virtual bool sendBlocking      (void* buf, int nbytes, int tag);
    virtual bool sendNonBlocking   (void* buf, int nbytes, int tag);
    virtual bool waitForSend       (void* buf, int nbytes, int tag);
    virtual bool waitForReceiveAck (void* buf, int nbytes, int tag);
  
    virtual string getType() const;
  
    virtual bool connectionPossible(int srcRank, int dstRank) const;
    virtual bool isBlocking() const { return true; }
  
    static TH_Socket proto;
    
    virtual bool init ();
  
   private:
    std::string itsSendingHostName;
    std::string itsReceivingHostName;
    int itsPortNo;
  
    bool isConnected;
  
    Socket itsSocket, itsDataSocket;
  
    bool ConnectToServer (void);
    bool ConnectToClient (void);
  };
  
}

#endif
