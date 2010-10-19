//# TH_ShMem.cc: In-memory transport mechanism
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

#include <lofar_config.h>

#ifdef HAVE_SHMEM

#include <Transport/TH_ShMem.h>

#include <Transport/BaseSim.h>
#include <Transport/DataHolder.h>
#include <Blob/BlobStringType.h>
#include <Common/LofarLogger.h>
#include <Common/shmem/shmem_alloc.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sched.h>

namespace LOFAR
{

#define TH_SHMEM_MAGIC_COOKIE 0xfefabade

// magic cookie for TH_ShMem communication acknowledgement
#define TH_SHMEM_ACK_MAGIC       777
#define TH_SHMEM_SENDREADY_MAGIC 345

TH_ShMem::TH_ShMem(TransportHolder* th)
  : itsHelperTH    (th),
    itsFirstCall   (true),
    itsReset       (true),
    itsCommContext (0),
    itsSendBuf     (0),
    itsRecvBuf     (0)
{
  LOG_TRACE_FLOW("TH_ShMem constructor");
  ASSERTSTR(itsHelperTH != 0, "Give a valid TransportHolder in TH_ShMem constructor.");
  ASSERTSTR(itsHelperTH->getType() != getType(), "TH_ShMem helper TransportHolder cannot be TH_ShMem!");
  itsPrevSendHandle.set(0, 0);
}

TH_ShMem::~TH_ShMem()
{
  LOG_TRACE_FLOW("TH_ShMem destructor");

  if (itsRecvBuf)  // Receiving side
  {
    if (itsCommContext)
    {
      // disconnect from CommContext in shared memory
      LOG_TRACE_STAT_STR( "shmem disconnect " << itsCommContext << ' '
			  << itsCommHandle.getOffset());
      shmem_disconnect(itsCommContext,
		       itsCommHandle.getOffset());    
    }
  }
  else if (itsCommContext) // On sending side
  {
    // free commContext memory
    LOG_TRACE_STAT_STR( "shmem disconnect " << itsCommContext << ' '
			<< itsCommHandle.getOffset());
    shmem_free(itsCommContext);    
  }

  delete itsHelperTH;   // Note that TH_ShMem cleans up its helper TransportHolder!
}

string TH_ShMem::getType() const
{
  return "TH_ShMem";
}

TH_ShMem* TH_ShMem::clone() const
{
  ASSERTSTR (itsHelperTH->isClonable(), 
	     "Cannot clone TH_ShMem because its helper "
	     << itsHelperTH->getType() << " is not clonable.");
  TransportHolder* helperClone = itsHelperTH->clone();
  return new TH_ShMem(helperClone);
}

bool TH_ShMem::init()
{ 
  LOG_TRACE_FLOW( "TH_ShMem init()" );
    
  // initialize use of share memory allocator
  shmem_init();

  return itsHelperTH->init();
}


BlobStringType TH_ShMem::blobStringType() const
{
  return BlobStringType (false, ShMemAllocator());
}

bool TH_ShMem::canDataGrow() const
{
  return false;
}

TH_ShMem::ShMemAllocator::~ShMemAllocator()
{}

TH_ShMem::ShMemAllocator* TH_ShMem::ShMemAllocator::clone() const
{
  return new TH_ShMem::ShMemAllocator();
}

void TH_ShMem::reset()
{
  // To Do: a.o a shmem disconnect like in destructor
  itsReset = true;
}

void* TH_ShMem::ShMemAllocator::allocate (size_t size, unsigned int alignment)
{
  // Make sure alignment is power of 2.
  int align = 1;
  if (alignment > 0) {
    align = alignment;
    ASSERT (align <= 256  &&  (align & (align-1)) == 0);
  }
  // Allocate buffer header plus data space
  // Allocate some extra bytes to be sure that the data can be aligned
  // correctly and that the nr of extra bytes can be stored in the byte
  // just before the returned buffer.
  ShMemBuf* buf = (ShMemBuf*)shmem_malloc(sizeof(TH_ShMem::ShMemBuf) +
					  size + align);
  LOG_TRACE_STAT_STR( " alloc " << (void*)buf << ' ' << size);
  // Initialize header.
  buf->init (align);
  // Get address of first buffer element.
  void* data = buf->getDataAddress();
  LOG_TRACE_STAT_STR ("TH_ShMem:: alloc ptr " << data);
  return data;
}

void TH_ShMem::ShMemAllocator::deallocate(void* ptr)
{
  if (ptr) {
    void* buf = TH_ShMem::ShMemBuf::toBuf(ptr);
    LOG_TRACE_STAT_STR("shmem dealloc " << ptr << ' ' << buf);
    shmem_free(buf);
  }
}

void TH_ShMem::recvCommContext(int tag)
{
  LOG_TRACE_STAT_STR("TH_Shmem::recvCommContext");
  // Receive the handle of the commcontext in shared memory
  bool result = itsHelperTH->recvBlocking(&itsCommHandle, 
					  sizeof(ShMemHandle), tag, 0, 0);  
  ASSERTSTR(result, "TH_ShMem Receiving of commcontext handle failed");
 
  LOG_TRACE_STAT_STR( "TH_ShMem received commcontext " << itsCommHandle.getShmid()
		      << ' ' << itsCommHandle.getOffset());

  // connect to commContext
  itsCommContext = (CommContext*)shmem_connect(itsCommHandle.getShmid(),
					       itsCommHandle.getOffset());
  ASSERT(itsCommContext != 0);
  LOG_TRACE_STAT_STR ("shmem connected to commcontext " << itsCommContext);

  LOG_TRACE_STAT( "recvCommContext fully done" );
}

bool TH_ShMem::recvBlocking(void* buf, int nbytes, int tag, int nrBytesRead, DataHolder*)
{ 
  LOG_TRACE_RTTI("TH_ShMem recvBlocking()");
  if (itsFirstCall)
  {
    LOG_TRACE_STAT_STR ("TH_ShMem::recv firstCall");
    itsFirstCall = false;
    recvCommContext(tag);
  }

  if (itsReset)
  {
    itsRecvBuf = ShMemBuf::toBuf(buf);
    // Check if buffer was allocated with shmem_malloc
    ASSERT (itsRecvBuf->matchMagicCookie());
    itsReset = false;
  }

  if (nrBytesRead <= 0)
  {
    // wait until sender is in send routine
    shmem_cond_wait(itsCommContext->getSendReadyCondition());
  }

  // If handle of sending buffer has changed connect to new sending buffer
  if (!itsCommContext->matchHandle(itsPrevSendHandle))
  {
    // connect to sending buffer
    itsSendBuf = (ShMemBuf*)shmem_connect(itsCommContext->handle.getShmid(),
					  itsCommContext->handle.getOffset());
    LOG_TRACE_STAT_STR ("shmem connected to sending buf " << itsSendBuf << ' '
			<< itsCommContext->handle.getOffset());
    
    ASSERT (itsSendBuf != 0);

    itsPrevSendHandle.set(itsCommContext->handle.getShmid(), 
			  itsCommContext->handle.getOffset());
  }

  char* dataPtr = static_cast<char*>(itsSendBuf->getDataAddress()) + nrBytesRead;
  // do the memcpy
  memcpy(buf, dataPtr, nbytes);

  int totalSize = DataHolder::getDataLength (itsSendBuf->getDataAddress());
  if ((nbytes + nrBytesRead) == totalSize)
  {
    // signal send that transfer is complete
    shmem_cond_signal(itsCommContext->getRecvCompleteCondition());
  }

  return true;
}

int32 TH_ShMem::recvNonBlocking(void* buf, int32 nbytes, int tag, int32 offset, DataHolder* dh)
{
  LOG_WARN( "TH_ShMem::recvNonBlocking() is not implemented. recvBlocking() is used instead.");    
  recvBlocking(buf, nbytes, tag, offset, dh);
  return nbytes;
} 

void TH_ShMem::waitForReceived(void*, int, int)
{
  LOG_TRACE_RTTI("TH_ShMem waitForReceived");
}

void TH_ShMem::sendCommContext(int tag)
{
   // allocate CommContext
  itsCommContext = 
    (TH_ShMem::CommContext*)shmem_malloc(sizeof(TH_ShMem::CommContext)); 
  itsCommContext->init();

   // get shared memory handle of commContext and store this in itsCommHandle
  itsCommHandle.set(shmem_id(itsCommContext),
		    shmem_offset(itsCommContext));

  // send itsCommHandle using itsHelperTH 
  LOG_TRACE_STAT_STR ("Sending commcontext handle " << itsCommHandle.getShmid() << ' ' 
		      << itsCommHandle.getOffset());
  bool result = itsHelperTH->sendBlocking(&itsCommHandle, sizeof(ShMemHandle), tag, 0);

  ASSERTSTR(result, "Sending of commcontext handle failed");
}

bool TH_ShMem::sendBlocking(void* buf, int nbytes, int tag, DataHolder* dh)
{
  LOG_TRACE_RTTI("TH_ShMem sendBlocking");
  DBGASSERT (nbytes == dh->getDataSize()
	     && buf == dh->getDataPtr());

  if (itsFirstCall)
  {
    itsFirstCall = false;
    // Allocate commContext and send it to receiver
    sendCommContext(tag);
    DBGASSERTSTR(0 == itsRecvBuf, "itsRecvBuf not 0");
  }
  if (itsReset)
  {
    // Get the whole allocated ShMemBuf
    itsSendBuf = ShMemBuf::toBuf(buf);
    ASSERT (itsSendBuf->matchMagicCookie());

    // Set the handle of the new send buffer in the comm context 
    itsCommContext->handle.set(shmem_id(itsSendBuf), shmem_offset(itsSendBuf)); 

    itsReset = false;
  } 

  // signal that the send is ready
  shmem_cond_signal(itsCommContext->getSendReadyCondition());
        
  // wait for receiver to complete
  shmem_cond_wait(itsCommContext->getRecvCompleteCondition());

  LOG_TRACE_STAT ("TH_ShMem::sendBlocking done");
  return true;

}

bool TH_ShMem::sendNonBlocking(void* buf, int nbytes, int tag, DataHolder* dh)
{
  LOG_WARN( "TH_ShMem::sendNonBlocking() is not implemented. The sendBlocking() method is used instead." );    
  sendBlocking(buf, nbytes, tag, dh);
  return true;
}

void TH_ShMem::waitForSent(void*, int, int)
{
  LOG_TRACE_RTTI("TH_ShMem waitForSent()");
}

TH_ShMem::ShMemHandle::ShMemHandle()
  : itsShmid(0), itsOffset(0)
{}

void TH_ShMem::ShMemHandle::set(int shmid, size_t offset)
{
  itsShmid = shmid;
  itsOffset = offset;
}

void TH_ShMem::CommContext::init()
{
  shmem_cond_init(&itsRecvCompleteCondition);
  shmem_cond_init(&itsSendReadyCondition);
}

void TH_ShMem::CommContext::setHandle(ShMemHandle hdl)
{
  handle.set(hdl.getShmid(), hdl.getOffset());
}

bool TH_ShMem::CommContext::matchHandle(ShMemHandle& hdl)
{
  return (handle.getShmid() == hdl.getShmid() && handle.getOffset() == hdl.getOffset());
}

shmem_cond_t* TH_ShMem::CommContext::getRecvCompleteCondition(void)
{
  return &itsRecvCompleteCondition;
}

shmem_cond_t* TH_ShMem::CommContext::getSendReadyCondition(void)
{
  return &itsSendReadyCondition;
}

TH_ShMem::ShMemBuf::ShMemBuf()
{}

void TH_ShMem::ShMemBuf::init (int align)
{
  itsMagicCookie = TH_SHMEM_MAGIC_COOKIE;
  // Skip header.
  uchar* data = (uchar*)this + sizeof(ShMemBuf);
  // Align the buffer as needed.
  ptrdiff_t ptr = (ptrdiff_t(data) & ~(align-1)) + align;
  uchar* newbuf = (uchar*)ptr;
  ptrdiff_t ptrd = newbuf-data;
  DBGASSERT (ptrd > 0  &&  ptrd <= align);
  // Store the nr of bytes 'shifted'. This nr is > 0, so subtract 1
  // to be sure that 256 byte alignment also fits.
  *(newbuf-1) = ptrd-1;
  itsNrSkip = ptrd + sizeof(ShMemBuf);
}

TH_ShMem::ShMemBuf* TH_ShMem::ShMemBuf::toBuf (void* ptr)
{
  uchar* buf = (uchar*)ptr;
  // First take possible alignment bytes into account.
  int nalign = 1 + *(buf-1);
  buf -= nalign;
  ShMemBuf* result = (ShMemBuf*)(buf - sizeof(ShMemBuf));
  DBGASSERT (nalign == int(result->itsNrSkip - sizeof(ShMemBuf)));
  return result;
}

int TH_ShMem::ShMemBuf::getMagicCookie(void)
{
  return itsMagicCookie;
}

bool TH_ShMem::ShMemBuf::matchMagicCookie(void)
{
  return TH_SHMEM_MAGIC_COOKIE == itsMagicCookie;
}


}

#endif
