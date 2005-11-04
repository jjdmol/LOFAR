//#  BDBSyncer.h: synchronizes master and slaves
//#
//#  Copyright (C) 2005
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

#ifndef LOFAR_BDBREPLICATIONBDBSYNCER_H
#define LOFAR_BDBREPLICATIONBDBSYNCER_H

// \file BDBSyncer.h
// synchronizes master and slaves

//# Includes
#include <db_cxx.h>
#include <boost/thread.hpp>
#include <map>
#include <BDBReplication/BDBSite.h>

namespace LOFAR 
{
  namespace BDBReplication 
  {

    // \addtogroup BDBReplication
    // @{

    using std::map;
    class BDBMessage;

    // Description of class.
    class BDBSyncer
    {
    public:
      BDBSyncer(DbEnv* env);
      virtual ~BDBSyncer();

      virtual SyncReqType handleRequest() = 0;
      virtual void handleReply(BDBMessage& message) = 0;

      virtual void openSyncDB() = 0;

    protected:
      virtual void closeSyncDB();
      DbEnv* itsEnv;
      Db* itsSyncDB;
      bool itsDBOpened;

    private:
      // Copying is not allowed
      BDBSyncer(const BDBSyncer& that);
      BDBSyncer& operator=(const BDBSyncer& that);
      ALLOC_TRACER_ALIAS(BDBSite);
    };

    class BDBSyncMaster : public BDBSyncer
    {
    public:
      BDBSyncMaster(DbEnv* env, int noSlaves);
      ~BDBSyncMaster();

      virtual SyncReqType handleRequest();
      virtual void handleReply(BDBMessage& message);

      SyncReqType requestSync();
      bool isSyncDone(SyncReqType sr);

      virtual void openSyncDB();
    private:
      // Copying is not allowed
      BDBSyncMaster(const BDBSyncMaster& that);
      BDBSyncMaster& operator=(const BDBSyncMaster& that);

      void writeSyncRequest(SyncReqType sr);
      void removeSyncRequest(SyncReqType sr);
      int itsNoSlaves;
      SyncReqType itsNewSyncReq;
      SyncReqType itsLastCompleteSyncReq;
      map<SyncReqType, int> itsSyncReplies;

      // mutex to protect itsSyncReplies
      boost::mutex itsMutex;
    };

    class BDBSyncSlave : public BDBSyncer
    {
    public:
      BDBSyncSlave(DbEnv* env);
      ~BDBSyncSlave();

      virtual SyncReqType handleRequest();
      virtual void handleReply(BDBMessage& message);

      virtual void openSyncDB();
    private:
      // Copying is not allowed
      BDBSyncSlave(const BDBSyncSlave& that);
      BDBSyncSlave& operator=(const BDBSyncSlave& that);

      SyncReqType getHighestSyncRequest();
      SyncReqType itsExpectedSyncReq;
    };


    // @}

  } // namespace BDBReplication
} // namespace LOFAR

#endif
