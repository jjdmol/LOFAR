//# TH_MPI.h: Transport mechanism for MPI
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

#ifndef CEPFRAME_TH_MPI_H
#define CEPFRAME_TH_MPI_H

#include <lofar_config.h>

#ifdef HAVE_MPI

#include "CEPFrame/TransportHolder.h"
#include "CEPFrame/CyclicBuffer.h"
#include <Common/lofar_deque.h>
#include <Common/lofar_list.h>
#include <mpi.h>

#include <pthread.h>

namespace LOFAR
{

/**
   This class defines the transport mechanism for MPI to be
   able to run the simulation in a parallel way on multiple nodes.
   It is the default TransportHolder when compiling with MPI=1.
*/

class TH_MPI: public TransportHolder
{
public:
  TH_MPI();
  virtual ~TH_MPI();

  virtual TH_MPI* make() const;

  virtual void* allocate (size_t size);
  virtual void  deallocate (void*& ptr);

  static void* readerThread(void* theObject);

  void lock();
  void unlock();

  /// Read the data.
  virtual bool recv(void* buf, int nbytes, int source, int tag);

  /// Write the data.
  virtual bool send(void* buf, int nbytes, int destination, int tag);

  /// Get the type of transport.
  virtual string getType() const;

  virtual bool isBlocking() const { return true; }

  static void init (int argc, const char *argv[]);
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

  CyclicBuffer<RecvMsg*> itsMsgQueue;
  bool                   itsFirstCall;
  MPI_Request            itsRequest;
  int                    itsMaxSize;
  
  pthread_t itsReaderThread;
#ifndef THREAD_SAFE
  static pthread_mutex_t theirMPILock;
#endif
  int itsSource;
  int itsTag;

  static int  theirReadcnt;
  static int  theirWritecnt;
  static int  theirBytesRead;
  static int  theirBytesWritten;

 public:
  /// Declare static prototype variable that
  /// can be used to pass to functions requiring
  /// a TransportHolder prototype.
  static TH_MPI proto;

};

}

#endif
#endif
