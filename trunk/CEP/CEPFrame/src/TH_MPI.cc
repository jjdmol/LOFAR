//  TH_MPI.cc: Transport mechanism for MPI
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
///////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MPI

#ifdef HAVE_SCAMPI
#define THREAD_SAFE
#endif

#include "CEPFrame/TH_MPI.h"
#include "mpi.h"
#include "CEPFrame/StepRep.h"
#include "Common/Debug.h"
#include "Common/lofar_deque.h"
#include "Common/lofar_list.h"

#include <pthread.h>

// static ints to count nr of reads/writes
#ifndef THREAD_SAFE
pthread_mutex_t TH_MPI::theirMPILock = PTHREAD_MUTEX_INITIALIZER;
#endif
int TH_MPI::theirReadcnt=0;
int TH_MPI::theirWritecnt=0;
int TH_MPI::theirBytesRead=0; 
int TH_MPI::theirBytesWritten=0; 
TH_MPI TH_MPI::proto;

// Don't use cyclic buffer by default
//#define USE_CYCLIC_BUFFER
#define CYCLIC_BUFFER_SIZE 20
#ifdef USE_CYCLIC_BUFFER
#define READER_THREAD
#endif

TH_MPI::TH_MPI() :
    itsFirstCall(true),
    itsRequest(MPI_REQUEST_NULL)
{
}

TH_MPI::~TH_MPI()
{
}

TH_MPI* TH_MPI::make() const
{
    return new TH_MPI();
}

string TH_MPI::getType() const
{
    return "TH_MPI";
}

void* TH_MPI::allocate (size_t size)
{
  itsMaxSize = size;
  return TransportHolder::allocate(size);
}

void  TH_MPI::deallocate (void*& ptr)
{
  TransportHolder::deallocate(ptr);
}

void TH_MPI::lock()
{
#ifdef READER_THREAD
#ifndef THREAD_SAFE
    pthread_mutex_lock(&theirMPILock);
#endif
#endif
}

void TH_MPI::unlock()
{
#ifdef READER_THREAD
#ifndef THREAD_SAFE
    pthread_mutex_unlock(&theirMPILock);
#endif
#endif
}

void* TH_MPI::readerThread(void* theObject)
{
#ifdef READER_THREAD
    TH_MPI* thisObject = (TH_MPI*)theObject;
    MPI_Status status;
    int result;

    // initiate itsMsgQueue for all elements of the buffer
    while (1)
    {
	int id;

	// get buffer from cyclic buffer
	RecvMsg* msg = thisObject->itsMsgQueue.GetWriteLockedDataItem(&id);

	// wait for new message
	thisObject->lock();
	result = MPI_Recv(&(msg->buffer), thisObject->itsMaxSize, MPI_BYTE,
			  thisObject->itsSource, thisObject->itsTag,
			  MPI_COMM_WORLD, &status);
	thisObject->unlock();
	if (MPI_SUCCESS != result) cerr << "error " << result << endl;

	// return buffer to cyclicbuffer
	thisObject->itsMsgQueue.WriteUnlockElement(id);
    }
    
    // should never get here!
    return NULL;
#else
    theObject = theObject;
    return NULL;
#endif
}

bool TH_MPI::recv(void* buf, int nbytes, int source, int tag)
{
    int result = MPI_SUCCESS;

#ifndef USE_CYCLIC_BUFFER

    MPI_Status status;

    //TRACER3("MPI::recv(" << buf << "," << nbytes << ",....)");
    result = MPI_Recv (buf, nbytes, MPI_BYTE, source, tag,
		       MPI_COMM_WORLD, &status);
    if (MPI_SUCCESS != result) cerr << "result = " << result << endl;

#else

    RecvMsg*   msg;

    AssertStr(nbytes <= itsMaxSize, "msg size larger than maximum allowed");

    if (itsFirstCall)
    {
	// allocate the cyclic buffer
	for (int i=0; i<CYCLIC_BUFFER_SIZE; i++)
	{
	    msg = (RecvMsg*)malloc(sizeof(RecvMsg)+itsMaxSize);
	    itsMsgQueue.AddBufferElement(msg);
	}

	itsSource = source;
	itsTag    = tag;
	pthread_create(&itsReaderThread, NULL, readerThread, this);

	itsFirstCall = false;
    }

    AssertStr(itsSource == source && itsTag == tag,
	      "source or tag mismatch");

    int id;
    msg = itsMsgQueue.GetReadDataItem(&id);
    memcpy(buf, &(msg->buffer), nbytes);
    itsMsgQueue.ReadUnlockElement(id);

    theirReadcnt++;
    theirBytesRead += nbytes;

#endif
  
    return (result == MPI_SUCCESS);
}

bool TH_MPI::send(void* buf, int nbytes, int destination, int tag)
{
    // non-blocking MPI send
    int result;

    //TRACER3("MPI::send(" << buf << "," << nbytes << ",....)");
    lock();
    result = MPI_Send(buf, nbytes, MPI_BYTE, destination, tag,
		      MPI_COMM_WORLD);
    unlock();
    theirWritecnt++;
    theirBytesWritten += nbytes;

    return (result == MPI_SUCCESS);
}

void TH_MPI::waitForBroadCast()
{
    /// Wait for a broadcast message with MPI
    unsigned long timeStamp;
    MPI_Bcast(&timeStamp, 1, MPI_UNSIGNED_LONG,
	      CONTROLLER_NODE, MPI_COMM_WORLD);
    TRACER2("Broadcast received timestamp " <<timeStamp
	    << " rank=  " << getCurrentRank());
}

void TH_MPI::waitForBroadCast(unsigned long& aVar)
{
    TRACER2("wait for broadcast");
    /// Wait for a broadcast message with MPI
    MPI_Bcast(&aVar, 1, MPI_UNSIGNED_LONG, CONTROLLER_NODE, MPI_COMM_WORLD);
    TRACER2("Broadcast received timestamp " << aVar
	    << " rank=  " << getCurrentRank());
}


void TH_MPI::sendBroadCast(unsigned long timeStamp)
{
    TRACER2("send broadcast");
    /// Send a broadcast timestamp
    MPI_Bcast(&timeStamp, 1, MPI_UNSIGNED_LONG, CONTROLLER_NODE, MPI_COMM_WORLD);
    TRACER2("Broadcast sent timestamp " <<timeStamp); 
}

int TH_MPI::getCurrentRank()
{
    int rank;

    ///  Get the current node 
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);

    return rank;
}

int TH_MPI::getNumberOfNodes()
{
    int size;

    /// get the Number of nodes
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    return size;
}

void TH_MPI::init(int argc, const char *argv[])
{
    int initialized = 0;
  
    /// Initialize the MPI communication
    MPI_Initialized(&initialized);
    if (!initialized)
    {
      MPI_Init (&argc,(char***)&argv);
    }
}

void TH_MPI::finalize()
{
    /// finalize the MPI communication
    MPI_Finalize();
}

void TH_MPI::synchroniseAllProcesses()
{
    TRACER2("Synchronise all");
    MPI_Barrier(MPI_COMM_WORLD);
    TRACER2("Synchronised...");
}

#endif
