//# TH_Mem.h: In-memory transport mechanism
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

#ifndef TRANSPORT_TH_MEM_H
#define TRANSPORT_TH_MEM_H

// \file TH_Mem.h
// In-memory transport mechanism

#include <lofar_config.h>

#include <Transport/TransportHolder.h>
#include <Common/lofar_map.h>

#ifdef USE_THREADS
#include <pthread.h>
#endif

namespace LOFAR
{
// \addtogroup Transport
// @{

//# Forward declarations.
class DataHolder;

// This class defines the transport mechanism between data holders
// that have been connected using the TH_Mem prototype. This can
// only be done when both data holders reside within the same address
// space. It uses memcpy to transport the data.
//
// The match between send and receive is done using a map.  This map
// keeps track of all messages that have been sent through the
// TH_Mem::send function. The tag argument is mapped to the sending DataHolder.
// The assumption of this implementation is that the tag is unique for
// communication between each pair of connected DataHolders and that
// there is no need to queue multiple sends (i.e. a send will always be
// followed by the matching receive before the next send is done).

class TH_Mem: public TransportHolder
{
public:
  TH_Mem();
  virtual ~TH_Mem();

  // method to make a TH_Mem instance
  virtual TH_Mem* make() const;

  // Receive fixed sized data. This call does the actual data transport
  // by memcpy'ing the data from the sender.
  virtual bool recvNonBlocking(void* buf, int nbytes, int tag);

  // Send fixed sized data.
  // It does not really send, because the recv is doing the memcpy.
  // This call only records the buf, nbytes, destination and tag
  // which can be matched by the recv call.
  // The only thing it does is setting the status.
  virtual bool sendNonBlocking(void* buf, int nbytes, int tag);

  // Receive variable sized data. This call does the actual data transport
  // by memcpy'ing the data from the sender.
  virtual bool recvVarNonBlocking(int tag);

  // Receive fixed size data. This call does the actual data transport
  // by memcpy'ing the data from the sender and sending out a
  // received notification.
  virtual bool recvBlocking(void* buf, int nbytes, int tag);

  // Send fixed size data.
  // It does not really send, because the recv is doing the memcpy.
  // This call only records the buf, nbytes, destination and tag
  // which can be matched by the recv call.
  // The only things it does are setting the status and waiting for
  // a notification of the receiver.
  virtual bool sendBlocking(void* buf, int nbytes, int tag);

  // Receive variable size data. This call does the actual data transport
  // by memcpy'ing the data from the sender and sending out a
  // received notification.
  virtual bool recvVarBlocking(int tag);

  // Get the type of transport.
  virtual string getType() const;

  virtual bool connectionPossible(int srcRank, int dstRank) const;

  virtual bool isBidirectional() const;
 
  // \name Static functions which are the same as those in TH_ShMem and TH_MPI.
  // They don't do anything. In this way templating on TH type can be done.
  // <group>
  static void init (int argc, const char *argv[]);
  static void finalize();
  static void waitForBroadCast();
  static void waitForBroadCast (unsigned long& aVar);
  static void sendBroadCast (unsigned long timeStamp);
  static int getCurrentRank();
  static int getNumberOfNodes();
  static void synchroniseAllProcesses();
  // </group>

 private:

  // Initializes condition variables needed for blocking communication
  void initConditionVariables(int tag);

  // The map from tag to source DataHolder object.
  static map<int, DataHolder*> theSources;

#ifdef USE_THREADS
  // \name Maps which hold condition variables.
  // @{
  static map<int, pthread_cond_t> dataAvailable;
  static map<int, pthread_cond_t> dataReceived;  
  // @}

  // Mutex for access to messages map
  static pthread_mutex_t theirMapLock;
#endif

  bool        itsFirstSendCall;
  bool        itsFirstRecvCall;
  DataHolder* itsDataSource;

  bool        itsFirstCall;
};

inline bool TH_Mem::isBidirectional() const
  { return true; }

// @} // Doxygen endgroup Transport

}

#endif
