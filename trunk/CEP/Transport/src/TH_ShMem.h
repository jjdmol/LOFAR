//# TH_ShMem.h: In-memory transport mechanism between different processes
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

#ifndef LIBTRANSPORT_TH_SHMEM_H
#define LIBTRANSPORT_TH_SHMEM_H

#include <lofar_config.h>

//# Needed for shmem
#define USE_PUBLIC_MALLOC_WRAPPERS
#define USE_DL_PREFIX

#include <Transport/TransportHolder.h>
#include <Common/Allocator.h>
#include <Common/lofar_map.h>
#include <Common/shmem/shmem_alloc.h>

namespace LOFAR
{

/**
   This class defines the transport mechanism between data holders
   that have been connected using a TH_ShMem object. This can
   only be done when both data holder reside within the same address
   space.  It uses memcpy to transport the data.
  
   The match between send and receive is done using a map.  This map
   keeps track of all messages that have been sent through the
   TH_Mem::send function. The tag argument is mapped to the private Msg
   class which keeps record of the address of the buffer that should be
   sent, the number of bytes to send and the tag of the send.  The
   assumption of this implementation is that the tag is unique for
   communication between each pair of connected DataHolders and that
   there is no need to queue multiple sends (i.e. a send will always be
   followed by the matching receive before the next send is done).
*/

class TH_ShMem: public TransportHolder
{
public:
  // Create an object to send data from source to destination.
  // Both must be running on the same node.
  TH_ShMem(int sourceNode, int targetNode);

  virtual ~TH_ShMem();

  /// method to make a TH_ShMem instance; used for prototype pattern
  virtual TH_ShMem* make() const;

  /**
     Receive the data. This call does the actual data transport
     by memcpy'ing the data from the sender.
  */
  void initRecv(void* buf, int tag);
  virtual bool recvBlocking(void* buf, int nbytes, int tag);
  virtual bool recvVarBlocking(int tag);
  virtual bool recvNonBlocking(void* buf, int nbytes, int tag);
  virtual bool recvVarNonBlocking(int tag);

  // Wait for the data to be received
  virtual bool waitForReceived(void* bug, int nbytes, int tag);

  /**
     Send the data.
     It does not really send, because the recv is doing the memcpy.
     This call only records the buf, nbytes, destination and tag
     which can be matched by the recv call.
     The only thing it does is setting the status.
  */
  void initSend(void* buf, int tag);
  virtual bool sendBlocking(void* buf, int nbytes, int tag);
  virtual bool sendVarBlocking(int tag);
  virtual bool sendNonBlocking(void* buf, int nbytes, int tag);
  virtual bool sendVarNonBlocking(int tag);

  virtual bool waitForSent(void* buf, int nbytes, int tag);

  /// Get the type of transport.
  virtual string getType() const;

  // Get the type of BlobString needed from the transport holder.
  virtual BlobStringType blobStringType() const;

  // A data buffer can not grow.
  virtual bool canDataGrow() const;

  virtual bool connectionPossible(int srcRank, int dstRank) const;
 
  static void   init (int argc, const char *argv[]);
  static void   finalize();
  static void   waitForBroadCast();
  static void   waitForBroadCast (unsigned long& aVar);
  static void   sendBroadCast (unsigned long timeStamp);
  static int    getCurrentRank();
  static int    getNumberOfNodes();
  static void   synchroniseAllProcesses();
  static string getHostName(int rank);

 private:

  // The class ShMemAllocator allocates and deallocates the shared memory.
  class ShMemAllocator: public Allocator
  {
  public:
      ShMemAllocator()
        {}

      virtual ~ShMemAllocator();

      // Clone the allocator.
      virtual ShMemAllocator* clone() const;

      // Allocate shared memory.
      virtual void* allocate (size_t size);
      // Deallocate shared memory.
      virtual void  deallocate (void* ptr);
  };


  /**
     The class ShMemHandle keeps information on a remote shared
     memory segment that needs to be accessible from the local context.
  */
  class ShMemHandle
  {
  public:
      ShMemHandle();

      void set(int shmid, size_t offset);

      int    getShmid()    { return itsShmid;  }
      size_t getOffset()   { return itsOffset; }

  private:
      int    itsShmid;
      size_t itsOffset;
  };

  class CommContext
  {
  public:

      void setArgs(void* buf, int remote, int tag);
      bool matchArgs(void* buf, int remote, int tag);

  public:
      ShMemHandle handle;
      void*       buf;
      int         remote;
      int         tag;
  };

  /**
     The class Buf keeps track of the shared memory buffer
     that is used in shared memory communication.
  */
  class ShMemBuf
  {
  public:
      ShMemBuf();
      void init(void);
      
      int           getMagicCookie();
      bool          matchMagicCookie();
      int           getRemote();
      int           getTag();
      shmem_cond_t* getRecvCompleteCondition();
      shmem_cond_t* getSendReadyCondition();
      void*         getDataAddress();
      
      static ShMemBuf* toBuf(void* ptr);

  private:
      unsigned int itsMagicCookie;
      shmem_cond_t itsRecvCompleteCondition;
      shmem_cond_t itsSendReadyCondition;
      void*        itsBuf;
  };

  int       itsSourceNode;
  int       itsTargetNode;
  bool      itsFirstCall;
  ShMemBuf* itsSendBuf;
  ShMemBuf* itsRecvBuf;

  CommContext itsSendContext;
  CommContext itsRecvContext;

  static char* hostNames;
};

}

#endif
