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
#include <Common/lofar_map.h>
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
  int recvBlocking(void* buffer, int bufferSize);

  //  Socket* getSocket();
  Dbt* getConnectionData();
  bool operator==(BDBSite& other);
  friend ostream& operator<<(ostream& os, BDBSite& site);
  void lock();
  void unlock();
  boost::mutex itsMutex;
 private:
  //  boost::mutex::scoped_lock itsLock;
  string itsHostName;
  int itsPort;
  Socket* itsSocket;
  Dbt itsConnectionData;
  char* itsConnectionDataBuffer;

  ALLOC_TRACER_CONTEXT;
};

inline void BDBSite::lock() {
  //  if (itsLock.locked()) cerr<<"BDBSite is already locked"<<endl;
  //  itsLock.lock(); 
};
inline void BDBSite::unlock() {
  //  if (!itsLock.locked()) cerr<<"BDBSite wasn't locked"<<endl;
  //  itsLock.unlock(); 
};
inline Dbt* BDBSite::getConnectionData()
{ return &itsConnectionData; };

class BDBSiteMap {
 public:
  BDBSiteMap();
  ~BDBSiteMap();
  //  BDBSite& getSite(int index);
  void addSite(BDBSite* newSite);

  typedef map<int, BDBSite*>::iterator iterator;

  BDBSite& find(int envid);
  BDBSiteMap::iterator begin();
  BDBSiteMap::iterator end();
  void lock();
  void unlock();
  void print(ostream& os);
  boost::mutex itsMutex;
 private:
  int itsLastEnvId;
  //boost::mutex::scoped_lock itsLock;
  map<int, BDBSite*> itsSiteMap;
};
inline void BDBSiteMap::lock() {
  //  if (itsLock.locked()) 
  //    cerr<<"BDBSiteMap is already locked"<<endl;
  //  itsLock.lock(); 
};
inline void BDBSiteMap::unlock() {
/*   if (!itsLock.locked())  */
/*     cerr<<"BDBSiteMap wasn't locked"<<endl; */
/*   itsLock.unlock();  */
};

inline BDBSite& BDBSiteMap::find(int envid) {
  BDBSite* site = itsSiteMap.find(envid)->second;
  return *site;
};
inline BDBSiteMap::iterator BDBSiteMap::begin() {
  return itsSiteMap.begin(); };
inline BDBSiteMap::iterator BDBSiteMap::end() {
  return itsSiteMap.end(); };

#define MAGIC_VALUE 85

class BDBEnv : public DbEnv
{
 public:
  BDBEnv(int flags, BDBSiteMap& siteMap) : DbEnv(flags), itsSiteMap(siteMap), magicValue(MAGIC_VALUE){};
  BDBSiteMap& itsSiteMap;
  void AssertCorrectInstance() {ASSERTSTR(magicValue = MAGIC_VALUE, "BDBEnv is not a correct instance");};
 private:
  int magicValue;  
};

#endif
