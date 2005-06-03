//#  BDBSite.h: Class that contains information about another Berkeley site
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

#ifndef BDBSITE_H
#define BDBSITE_H

#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <Common/Net/Socket.h>
#include <boost/thread.hpp>
#include <db_cxx.h>
 
using namespace LOFAR;

class BDBReplicator;

class BDBSite {
 public:
  BDBSite(const char* hostName, const int port, Socket* socket);
  ~BDBSite();

  void send(void* buffer, int bufferSize);
  int recv(void* buffer, int bufferSize);

  //  Socket* getSocket();
  Dbt* getConnectionData();
  bool operator==(BDBSite& other);
  friend ostream& operator<<(ostream& os, BDBSite& site);
 private:
  boost::mutex itsSocketMutex;
  string itsHostName;
  int itsPort;
  Socket* itsSocket;
  Dbt itsConnectionData;
  char* itsConnectionDataBuffer;

  ALLOC_TRACER_CONTEXT;
};

//inline Socket* BDBSite::getSocket()
//{ return itsSocket;};
inline Dbt* BDBSite::getConnectionData()
{ return &itsConnectionData; };

#endif
