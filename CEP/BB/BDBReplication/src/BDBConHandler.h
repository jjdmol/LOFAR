//#  BDBConHandler.h: Handle message from other berkeley db replication nodes
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef BDBCONHANDLER_H
#define BDBCONHANDLER_H

#include <Common/lofar_map.h>
#include <Common/lofar_string.h>
#include <Common/Net/Socket.h>
#include <boost/thread.hpp>
#include <db_cxx.h>
 
#include <MNS/BDBSite.h>

using namespace LOFAR;

class BDBCHThread {
 public:
  // called from outside the thread
  BDBCHThread(const string& hostname, const int port);

  BDBCHThread(const BDBCHThread& other);

  ~BDBCHThread();

  // the callback function for libdb
  static int send(DbEnv *dbenv,
		  const Dbt *control, 
		  const Dbt *rec, 
		  const DbLsn *lsnp,
		  int envid, 
		  u_int32_t flags);
  static void sendOne(const Dbt *control, 
		      const Dbt *rec, 
		      Socket* mySocket);

  void setEnv(DbEnv* DbEnv);

  // signal the thread to stop
  void stop();

  // this contains the loop of the thread
  void operator()();

  // called from listen thread
  void addSite(BDBSite* newSite);
  // dump siteMap to cout
  void printSiteMap();
  // called from inside the ch thread
  void connectTo(const char* otherHostName,
		 int otherPort);

 private:
  void handleMessage(Dbt* rec, Dbt* control, int envId);

  // used from outside and within so protected by a mutex
  static bool theirShouldStop;
  bool shouldStop();

  // the list of sockets should be protected
  // they are used from the static send function and it
  // is probably possible that it is called in different threads
  static boost::mutex theirSiteMapMutex;
  static std::map<int, BDBSite*> theirSiteMap; //int or something else?
  static int theirObjectCounter;
  static int theirLastEnvId;
  int itsEnvId;
  DbEnv* itsDbEnv;
  string itsHostName;
  int itsPort;
};

#endif
