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
   only be done when both data holders reside within the same address
   space.  It uses memcpy to transport the data.

   It uses a variable in shared memory to communicate the address in
   shared memory to copy the data from. The address of this variable
   has to be communicated once to the receiver. For this another
   transport mechanism must be used.
*/

class TH_ShMem: public TransportHolder
{
public:
  // Create an object to send data from source to destination.
  // Both must be running on the same node. 
  // The TransportHolder th is used once to communicate the 
  // handle of the shared memory region used in communication 
  // between source and destination.
  // NB: TH_ShMem becomes owner of this th and takes care of its
  // destruction!
  TH_ShMem(TransportHolder* th);

  virtual ~TH_ShMem();

  // This init does nothing. Initialization occurs in the first
  // send and receive call.
  bool init();

  /**
     Receive the data. This call does the actual data transport
     by memcpy'ing the data from the sender.
  */
  void recvCommContext(int tag);
  virtual bool recvBlocking(void* buf, int32 nbytes, int tag, 
			    int32 offset=0, DataHolder* dh=0);
  virtual int32 recvNonBlocking(void* buf, int32 nbytes, int tag, 
				int32 offset=0, DataHolder* dh=0);

  // Wait for the data to be received
  virtual void waitForReceived(void* bug, int nbytes, int tag);

  /**
     Send the data.
     It does not really send, because the recv is doing the memcpy.
     This call only records the buf, nbytes, destination and tag
     which can be matched by the recv call.
     The only thing it does is setting the status.
  */
  void sendCommContext(int tag);
  virtual bool sendBlocking(void* buf, int nbytes, int tag, DataHolder* dh=0);
  virtual bool sendNonBlocking(void* buf, int nbytes, int tag, DataHolder* dh=0);

  virtual void waitForSent(void* buf, int nbytes, int tag);

  /// Get the type of transport.
  virtual string getType() const;

  /// Is TH_ShMem clonable?
  virtual bool isClonable() const;

  /// Return a copy of this transportholder
  virtual TH_ShMem* clone() const;

  // Resets all members which are source or destination specific.
  virtual void reset();

  // Get the type of BlobString needed from the transport holder.
  virtual BlobStringType blobStringType() const;

  // A data buffer can not grow.
  virtual bool canDataGrow() const;

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

  // Class CommContext keeps track of the sending shared memory buffer
  // and semaphores to synchronize sender and receiver.
  class CommContext
  {
  public:
      void init(void);
      void setHandle(ShMemHandle hdl);
      bool matchHandle(ShMemHandle& hdl);
      shmem_cond_t* getRecvCompleteCondition();
      shmem_cond_t* getSendReadyCondition();

  public:
      ShMemHandle handle;                      // Handle of the sending ShMemBuf
      shmem_cond_t itsRecvCompleteCondition;   // use semaphore in shared memory to synchronize the sender and receiver
      shmem_cond_t itsSendReadyCondition;
  };

  /**
     The class ShMemBuf is a buffer in shared memory.
  */                                                                                                                                                                                                                                    
  class ShMemBuf
  {
  public:
      ShMemBuf();
      void init(void);
      
      int           getMagicCookie();
      bool          matchMagicCookie();
      void*         getDataAddress();   
      static ShMemBuf* toBuf(void* ptr);

  private:
      unsigned int itsMagicCookie;    // Identifies this as a shared memory buffer
      void*        itsBuf;
  };

  TransportHolder* itsHelperTH;       // TransportHolder, used once to communicate the shared memory handle

  bool             itsFirstCall;     // First time send/recv is called on this TH?
  bool             itsReset;         // Has reset been called?

  ShMemHandle      itsCommHandle;    // Handle of the CommContext 
  CommContext*     itsCommContext;   // Communication context in shared memory

  ShMemBuf*        itsSendBuf;       // Sending shared memory buffer
  ShMemBuf*        itsRecvBuf;       // Receiving shared memory buffer,
                                     // (only used by receiving TH_ShMem)

  ShMemHandle      itsPrevSendHandle; // Handle of the previous sending buffer 
};

inline bool TH_ShMem::isClonable() const
  { return itsHelperTH->isClonable(); }


}

#endif
