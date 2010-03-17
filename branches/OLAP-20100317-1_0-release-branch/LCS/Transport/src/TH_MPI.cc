//# TH_MPI.cc: Transport mechanism for MPI
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

#ifdef HAVE_MPI

#ifdef HAVE_SCAMPI
#define THREAD_SAFE
#endif

#include <Transport/TH_MPI.h>
#include <Transport/BaseSim.h>
#include <Transport/DataHolder.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_deque.h>
#include <Common/lofar_list.h>

namespace LOFAR
{

#ifndef THREAD_SAFE
pthread_mutex_t TH_MPI::theirMPILock = PTHREAD_MUTEX_INITIALIZER;
#endif

TH_MPI::TH_MPI(int sourceNode, int targetNode)
  : itsSourceNode(sourceNode),
    itsTargetNode(targetNode)
{
  LOG_TRACE_FLOW("TH_MPI constructor");
}

TH_MPI::~TH_MPI()
{
  LOG_TRACE_FLOW("TH_MPI destructor");
}

string TH_MPI::getType() const
{
    return "TH_MPI";
}

TH_MPI* TH_MPI::clone() const
{
  return new TH_MPI(itsSourceNode, itsTargetNode);
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

bool TH_MPI::recvBlocking(void* buf, int nbytes, int tag, int, DataHolder*)
{
  LOG_TRACE_RTTI_STR("TH_MPI recvBlocking(" << buf << "," 
		     << nbytes << ", MPI_BYTE, " << itsSourceNode 
		     << ", " << tag << ",... )");
  int result = MPI_SUCCESS;
  
  MPI_Status status;
  
  result = MPI_Recv (buf, nbytes, MPI_BYTE, itsSourceNode, tag,
		     MPI_COMM_WORLD, &status);
  if (MPI_SUCCESS != result) {
    LOG_ERROR_STR( "TH_MPI::recvBlocking result = " << result );
  }
  return (result == MPI_SUCCESS);
}

int32 TH_MPI::recvNonBlocking(void* buf, int32 nbytes, int tag, int32, DataHolder*)
{
  LOG_WARN( "TH_MPI::recvNonBlocking() is not implemented. recvBlocking() is used instead." );    
  return (recvBlocking(buf, nbytes, tag) ? nbytes : 0);
}

void TH_MPI::waitForReceived(void*, int, int)
{
  LOG_TRACE_RTTI("TH_MPI waitForReceived()");
  // Iwait
}

bool TH_MPI::sendBlocking(void* buf, int nbytes, int tag, DataHolder*)
{
  LOG_TRACE_RTTI_STR("TH_MPI::sendBlocking(" << buf << "," 
		     << nbytes << ",MPI_BYTE, " << itsTargetNode 
		     << ", " << tag << ", MPI_COMM_WORLD)");
  int result;
  
  lock();
  result = MPI_Send(buf, nbytes, MPI_BYTE, itsTargetNode, tag,
		    MPI_COMM_WORLD);
  unlock();
  
  return (result == MPI_SUCCESS);
}

bool TH_MPI::sendNonBlocking(void* buf, int nbytes, int tag, DataHolder*)
{
  LOG_WARN( "TH_MPI::sendNonBlocking() is not implemented. The sendBlocking() method is used instead." );    
  return sendBlocking(buf, nbytes, tag);
}

void TH_MPI::readTotalMsgLengthBlocking(int tag, int& nrBytes)
{
  LOG_TRACE_RTTI( "TH_MPI::readTotalMsgLengthBlocking" );
  int result = MPI_SUCCESS;

  MPI_Status status;

  LOG_TRACE_STAT("MPI::probe ....");
  result = MPI_Probe (itsSourceNode, tag, MPI_COMM_WORLD, &status);
  MPI_Get_count(&status, MPI_BYTE, &nrBytes);
}

bool TH_MPI::readTotalMsgLengthNonBlocking(int tag, int& nrBytes)
{
  LOG_WARN( "TH_MPI::readTotalMsgLengthNonBlocking() is not implemented. The blocking method is used instead." );
  readTotalMsgLengthBlocking(tag, nrBytes);
  return true;

//   LOG_TRACE_RTTI( "TH_MPI::readTotalMsgLengthBlocking" );
//   int result = MPI_SUCCESS;

//   MPI_Status status;

//   LOG_TRACE_STAT("MPI::probe ....");
//   result = MPI_IProbe (itsSourceNode, tag, MPI_COMM_WORLD, &status);
//   if (result == MPI_SUCCESS)
//   nrBytes = status.count;
//   //iprobe
}

void TH_MPI::waitForSent(void*, int, int)
{
  LOG_TRACE_RTTI("TH_MPI waitForSent()");
  // Iwait
}

void TH_MPI::waitForBroadCast()
{
  LOG_TRACE_RTTI("TH_MPI waitForBroadCast()");
  /// Wait for a broadcast message with MPI
  unsigned long timeStamp;
  MPI_Bcast(&timeStamp, 1, MPI_UNSIGNED_LONG,
	    CONTROLLER_NODE, MPI_COMM_WORLD);
  LOG_TRACE_STAT_STR( "Broadcast received timestamp " << timeStamp
		      << " rank=  " << getCurrentRank());
}

void TH_MPI::waitForBroadCast(unsigned long& aVar)
{
  LOG_TRACE_RTTI( "TH_MPI waitForBroadCast(..)" );
  /// Wait for a broadcast message with MPI
  MPI_Bcast(&aVar, 1, MPI_UNSIGNED_LONG, CONTROLLER_NODE, MPI_COMM_WORLD);
  LOG_TRACE_STAT_STR(" Broadcast received timestamp " << aVar
		     << " rank=  " << getCurrentRank());
}

void TH_MPI::sendBroadCast(unsigned long timeStamp)
{
  LOG_TRACE_RTTI( "TH_MPI SendBroadCast" );
  /// Send a broadcast timestamp
  MPI_Bcast(&timeStamp, 1, MPI_UNSIGNED_LONG, CONTROLLER_NODE, MPI_COMM_WORLD);
  LOG_TRACE_STAT_STR( "Broadcast sent timestamp " << timeStamp ); 
}

int TH_MPI::getCurrentRank()
{
  LOG_TRACE_RTTI( "TH_MPI getCurrentRank()" );
  int rank;

  ///  Get the current node 
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    
  return rank;
}

int TH_MPI::getNumberOfNodes()
{
  LOG_TRACE_RTTI( "TH_MPI getNumberOfNodes()" );
  int size;

  /// get the Number of nodes
  MPI_Comm_size (MPI_COMM_WORLD, &size);
  
  return size;
}

void TH_MPI::initMPI(int& argc, char **&argv)
{
  LOG_TRACE_RTTI( "TH_MPI init()" );
  int initialized = 0;
  
  /// Initialize the MPI communication
  MPI_Initialized(&initialized);
  if (!initialized)
  {
    MPI_Init (&argc,&argv);
  }
}

void TH_MPI::finalize()
{
  LOG_TRACE_RTTI( "TH_MPI finalize()" );
  int finalized = 0;
  
  /// Finalize the MPI communication
  MPI_Finalized(&finalized);
  if (!finalized)
  {
    MPI_Finalize();
  }
}

void TH_MPI::synchroniseAllProcesses()
{
  LOG_TRACE_RTTI( "TH_MPI synchroniseAllProcesses()" );
  MPI_Barrier(MPI_COMM_WORLD);
  LOG_TRACE_STAT("Synchronised...");
}

}

#endif
