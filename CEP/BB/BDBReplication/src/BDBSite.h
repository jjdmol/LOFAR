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

class BDBSiteMap {
 public:
  BDBSiteMap();
  ~BDBSiteMap();
  BDBSite& getSite(int index);
  void addSite(int index, BDBSite& newSite);
  map<int, BDBSite*>::iterator beginIterator();
  map<int, BDBSite*>::iterator getEnd();
  void destroyIterator();
 private:
  void lock();
  boost::mutex itsMutex;
  boost::mutex::scoped_lock itsLock;
  void unlock();
  map<int, BDBSite*> itsSiteMap;
};
inline void BDBSiteMap::lock() {
  itsLock.lock(); };
inline void BDBSiteMap::unlock() {
  itsLock.unlock(); };

inline BDBSite& BDBSiteMap::getSite (int index) {
  lock();
  BDBSite* site = itsSiteMap[index];
  unlock();
  return *site;
};
inline void BDBSiteMap::addSite (int index, BDBSite& newSite) {
  lock(); itsSiteMap[index] = &newSite; unlock();};
inline map<int, BDBSite*>::iterator BDBSiteMap::getEnd() {
  return itsSiteMap.end(); };
inline map<int, BDBSite*>::iterator BDBSiteMap::beginIterator() {
  while (itsLock.locked()) {
    LOG_TRACE_FLOW("BDBSiteMap is locked, maybe you forgot to call destroyIterator?");
    sleep(1);
  }
  lock(); return itsSiteMap.begin(); };
inline void BDBSiteMap::destroyIterator() {
  unlock(); };

#endif
