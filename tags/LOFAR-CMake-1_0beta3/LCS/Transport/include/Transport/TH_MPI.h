//# TH_MPI.h: Transport mechanism for MPI
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

#ifndef TRANSPORT_TH_MPI_H
#define TRANSPORT_TH_MPI_H

// \file
// Transport mechanism for MPI

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#ifdef HAVE_MPI

#include <Transport/TransportHolder.h>
#include <Common/lofar_deque.h>
#include <Common/lofar_list.h>
#define MPICH_IGNORE_CXX_SEEK
#include <mpi.h>

#ifdef HAVE_MPE
#include <mpe.h>
#endif

#ifndef USE_THREADS    // is we done not have threads available
#ifndef THREAD_SAFE
#define THREAD_SAFE    // we don't have threads; so thread_safety does not have to be added
                       // we proceed as if we have a thread safe implementation already.
#endif // THREAD_SAFE
#endif // USE_THREADS

#ifndef THREAD_SAFE
#include <pthread.h>
#endif

namespace LOFAR
{
// \addtogroup Transport
// @{

// This class defines the transport mechanism for MPI to be
// able to run the simulation in a parallel way on multiple nodes.
// It is the default TransportHolder when compiling with MPI=1.

class TH_MPI: public TransportHolder
{
public:
  TH_MPI(int sourceNode, int targetNode);
  virtual ~TH_MPI();

  // This method does nothing. Use initMPI(..) once at the start of your application!
  bool init();

  void lock();
  void unlock();

  /// Read the data.
  virtual bool recvBlocking(void* buf, int nbytes, int tag, int offset=0, 
			    DataHolder* dh=0);
  virtual int32 recvNonBlocking(void* buf, int32 nbytes, int tag, int32 offset=0, 
			       DataHolder* dh=0);
  /// Wait for the data to be received
  virtual void waitForReceived(void* buf, int nbytes, int tag);

  /// Write the data.
  virtual bool sendBlocking(void* buf, int nbytes, int tag, DataHolder* dh=0);
  virtual bool sendNonBlocking(void* buf, int nbytes, int tag, DataHolder* dh=0);
  /// Wait for the data to be sent
  virtual void waitForSent(void* buf, int nbytes, int tag);

  // Read the total message length of the next message.
  virtual void readTotalMsgLengthBlocking(int tag, int& nrBytes);

  // Read the total message length of the next message.
  virtual bool readTotalMsgLengthNonBlocking(int tag, int& nrBytes);

  // Get the type of transport.
  virtual string getType() const;

  // Can TH_MPI be cloned?
  virtual bool isClonable() const;

  // Create a copy
  virtual TH_MPI* clone() const;

  // Resets all members which are source or destination specific.
  void reset();

  static void initMPI (int& argc, char**&argv);
  static void finalize();
  static void waitForBroadCast();
  static void waitForBroadCast (unsigned long& aVar);
  static void sendBroadCast (unsigned long timeStamp);
  static int getCurrentRank();
  static int getNumberOfNodes();
  static void synchroniseAllProcesses();

  class RecvMsg
  {
  public:
    MPI_Request request;
    char        buffer;
  };

  class RecvHandle
  {
  public:
    RecvHandle(int i, RecvMsg* m) { id = i; msg = m; }
    RecvHandle() { }

    int      id;
    RecvMsg* msg;
  };

private:  
  int itsSourceNode;
  int itsTargetNode;
  int itsMaxSize;
  int itsSource;
  int itsTag;

#ifndef THREAD_SAFE
  static pthread_mutex_t theirMPILock;
#endif

  static int  theirReadcnt;
  static int  theirWritecnt;
  static int  theirBytesRead;
  static int  theirBytesWritten;

 public:

};

inline bool TH_MPI::init()
  { return true;}

inline bool TH_MPI::isClonable() const
  { return true; }

inline void TH_MPI::reset()
  {}

// @} // Doxygen group Transport

}

#endif
#endif
