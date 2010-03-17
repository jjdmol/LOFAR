//# TH_Mem.h: In-memory transport mechanism
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef TRANSPORT_TH_MEM_H
#define TRANSPORT_TH_MEM_H

// \file
// In-memory transport mechanism

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

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

  virtual bool init();  

  /**
     Receive fixed sized data. This call does the actual data transport
     by memcpy'ing the data from the sender.
     Note: when using these non-blocking methods be very careful not to change 
     the data between a sendNonBlocking and recvNonBlocking call! 
  */
  virtual int32 recvNonBlocking(void* buf, int32 nbytes, int tag, 
			       int32 offset=0, DataHolder* dh=0);
  /**
     Send fixed sized data.
     It does not really send, because the recv is doing the memcpy.
     This call only records the buf, nbytes, destination and tag
     which can be matched by the recv call.
     The only thing it does is setting the status.
  */
  virtual bool sendNonBlocking(void* buf, int nbytes, int tag, DataHolder* dh=0);

  /**
     Receive fixed size data. This call does the actual data transport
     by memcpy'ing the data from the sender and sending out a
     received notification.
  */
  virtual bool recvBlocking(void* buf, int nbytes, int tag, int nrBytesRead=0, 
			    DataHolder* dh=0);

 /**
     Send fixed size data.
     It does not really send, because the recv is doing the memcpy.
     This call only records the buf, nbytes, destination and tag
     which can be matched by the recv call.
     The only things it does are setting the status and waiting for
     a notification of the receiver.
  */
  virtual bool sendBlocking(void* buf, int nbytes, int tag, DataHolder* dh=0);

  // Read the total message length of the next message.
  virtual void readTotalMsgLengthBlocking(int tag, int& nrBytes);

  // Read the total message length of the next message.
  virtual bool readTotalMsgLengthNonBlocking(int tag, int& nrBytes);

  // Get the type of transport.
  virtual string getType() const;

  virtual bool isClonable() const;

  virtual TH_Mem* clone() const;

  // Resets all members which are source or destination specific.
  virtual void reset();

  // NB: The following method doesn't do what you expect! It does not
  // guarantee the buffer contents has been safely sent.
  // Your application should make sure write/read are called alternately.
  virtual void waitForSent(void* buf, int nbytes, int tag);

  // This method does nothing, because the memcpy is always done by recvNonBlocking
  virtual void waitForReceived(void* buf, int nbytes, int tag);

  // Static functions which are the same as those in TH_ShMem and TH_MPI.
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

  /**
     The map from tag to source DataHolder object.
   */
  static map<int, DataHolder*> theirSources;

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
  DataHolder* itsDataSource;   // Pointer to current source DataHolder

  bool        itsFirstCall;
};

inline bool TH_Mem::init()
{ return true; }

inline bool TH_Mem::isClonable() const
{ return true; }

// @} // Doxygen endgroup Transport

}

#endif
