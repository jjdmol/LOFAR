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

#ifndef TRANSPORT_TH_MPI_H
#define TRANSPORT_TH_MPI_H

#include <lofar_config.h>

#ifdef HAVE_MPI

#include <Transport/TransportHolder.h>
#include <Common/lofar_deque.h>
#include <Common/lofar_list.h>
#include <mpi.h>

#ifndef THREAD_SAFE
#include <pthread.h>
#endif

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
  TH_MPI(int sourceNode, int targetNode);
  virtual ~TH_MPI();

  virtual TH_MPI* make() const;

  void lock();
  void unlock();

  /// Read the data.
  virtual bool recvBlocking(void* buf, int nbytes, int tag);
  virtual bool recvVarBlocking(int tag);
  virtual bool recvNonBlocking(void* buf, int nbytes, int tag);
  virtual bool recvVarNonBlocking(int tag);
  /// Wait for the data to be received
  virtual bool waitForReceived(void* buf, int nbytes, int tag);

  /// Write the data.
  virtual bool sendBlocking(void* buf, int nbytes, int tag);
  virtual bool sendNonBlocking(void* buf, int nbytes, int tag);
  /// Wait for the data to be sent
  virtual bool waitForSent(void* buf, int nbytes, int tag);

  /// Get the type of transport.
  virtual string getType() const;

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

}

#endif
#endif
