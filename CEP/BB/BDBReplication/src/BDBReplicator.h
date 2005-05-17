//#  BDBReplicator.h: Handle replication of a Berkeley DB database
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

#ifndef BDBREPLICATOR_H
#define BDBREPLICATOR_H

#include <map>
#include <Common/lofar_string.h>
#include <Common/Net/Socket.h>
#include <boost/thread.hpp>
#include <db_cxx.h>

#include <BDBReplication/BDBConHandler.h>
#include <BDBReplication/BDBListener.h>
 
using namespace LOFAR;


class BDBReplicator {
 public:
  // called from outside the thread
  BDBReplicator(const string& DbEnvName,
		const string& hostName, 
		const int port, 
		const string& masterHostname,
		const int masterPort,
		const bool isMaster);

  void startReplication();

  DbEnv* getDbEnv();

  ~BDBReplicator();

 private:
  bool itsReplicationStarted;
  string itsDbEnvName;
  string itsHostName;
  int itsPort;
  // connection data for this master (ip and port)
  string itsMasterHostName;
  int itsMasterPort;
  bool itsIsMaster;

  BDBCHThread itsCHThreadObject;
  boost::thread* itsCHThread;
  BDBListenThread itsLThreadObject;
  boost::thread* itsLThread;
  DbEnv* itsDbEnv;
};

inline DbEnv* BDBReplicator::getDbEnv()
{ return itsDbEnv; };
#endif
