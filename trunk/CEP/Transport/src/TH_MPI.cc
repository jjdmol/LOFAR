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

#include <lofar_config.h>

#ifdef HAVE_MPI

#ifdef HAVE_SCAMPI
#define THREAD_SAFE
#endif

#include <TH_MPI.h>
#include <mpi.h>
#include <Common/Debug.h>
#include <Common/lofar_deque.h>
#include <Common/lofar_list.h>

namespace LOFAR
{

#ifndef THREAD_SAFE
pthread_mutex_t TH_MPI::theirMPILock = PTHREAD_MUTEX_INITIALIZER;
#endif
TH_MPI TH_MPI::proto;

TH_MPI::TH_MPI() 
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
#ifndef THREAD_SAFE
    pthread_mutex_lock(&theirMPILock);
#endif
}

void TH_MPI::unlock()
{
#ifndef THREAD_SAFE
    pthread_mutex_unlock(&theirMPILock);
#endif
}

bool TH_MPI::recvBlocking(void* buf, int nbytes, int source, int tag)
{
    int result = MPI_SUCCESS;

    MPI_Status status;

    TRACER2("MPI::recv(" << buf << "," << nbytes << ",....)");
    result = MPI_Recv (buf, nbytes, MPI_BYTE, source, tag,
		       MPI_COMM_WORLD, &status);
    if (MPI_SUCCESS != result) cerr << "result = " << result << endl;

    return (result == MPI_SUCCESS);
}

bool TH_MPI::sendBlocking(void* buf, int nbytes, int destination, int tag)
{
    int result;

    TRACER2("MPI::send(" << buf << "," << nbytes << ",....)");
    lock();
    result = MPI_Send(buf, nbytes, MPI_BYTE, destination, tag,
		      MPI_COMM_WORLD);
    unlock();
  
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

}

#endif
