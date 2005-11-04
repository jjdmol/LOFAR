//#  BDBSyncer.cc: one line description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <BDBReplication/BDBSyncer.h>
#include <BDBReplication/BDBMessage.h>

namespace LOFAR {
  namespace BDBReplication {

    void BDBSyncer::closeSyncDB(){
      LOG_TRACE_FLOW("closing database");
      if (itsSyncDB != 0) {
	// itsSyncDB->sync(0);
	itsSyncDB->close(0);
	delete itsSyncDB;
	itsSyncDB = 0;
      }
      itsEnv = 0;
    }

    BDBSyncMaster::BDBSyncMaster(DbEnv* env, int noSlaves):
      BDBSyncer(env),
      itsNoSlaves(noSlaves),
      itsNewSyncReq(0)
    {
    }

    BDBSyncMaster::~BDBSyncMaster()
    {
      closeSyncDB();
    }

    SyncReqType BDBSyncMaster::requestSync()
    {
      while (!itsDBOpened) {
	openSyncDB();
      }
	
      itsNewSyncReq++;
      {
	boost::mutex::scoped_lock sml(itsMutex);
	itsSyncReplies[itsNewSyncReq] = 0;
      }      
      writeSyncRequest(itsNewSyncReq);
      return itsNewSyncReq;
    }

    bool BDBSyncMaster::isSyncDone(SyncReqType sr)
    {
      bool ret = false;
      boost::mutex::scoped_lock sml(itsMutex);
      DBGASSERTSTR(itsSyncReplies.find(sr) != itsSyncReplies.end(), "Unknown SyncRequest (maybe sync was already completed)");
      ret = (itsSyncReplies[sr] == itsNoSlaves);
      if (ret) {
	removeSyncRequest(sr);
      }
      return ret;
    }

    SyncReqType BDBSyncMaster::handleRequest()
    {
      // handleRequest should not do anything on the master
      return 0;
    }

    void BDBSyncMaster::handleReply(BDBMessage& message)
    {
      int reply = message.getSyncRequestNumber();
      LOG_TRACE_FLOW_STR("SyncRequest " << reply << " received on master");
      boost::mutex::scoped_lock sml(itsMutex);
      itsSyncReplies[reply]++;
    }

    void BDBSyncMaster::openSyncDB()
    {
      DbTxn* myTxn = 0;
      itsEnv->txn_begin(NULL, &myTxn, 0);
      LOG_TRACE_FLOW_STR("creating db");  
      delete itsSyncDB;
      itsSyncDB = new Db(itsEnv, 0);
      const char* home;
      itsEnv->get_home(&home);
      LOG_TRACE_FLOW_STR("opening sync database on master in directory "<<home);  

      int ret = itsSyncDB->open(myTxn, "BDB_SYNC", "BDB_SYNC", DB_BTREE, DB_CREATE, 0);
      ASSERTSTR( ret == 0, "BDBRepl while opening sync database " << itsEnv->strerror(ret));
      itsDBOpened = true;

      if (myTxn->commit(0) != 0)
	LOG_TRACE_FLOW("could not commit open transaction");
      //      itsSyncDB = new Db(itsEnv, DB_CXX_NO_EXCEPTIONS);
      //      itsSyncDB->set_flags(DB_DUPSORT);
    }
    
    void BDBSyncMaster::writeSyncRequest(SyncReqType sr)
    {
      Dbt key, value;
      char *thekey = "lastSyncRequest";
      key.set_data(thekey);
      key.set_size(strlen(thekey));
      value.set_data(&sr);
      value.set_size(sizeof(SyncReqType));
       
      int ret = itsSyncDB->put(NULL, &key, &value, DB_AUTO_COMMIT);
      ASSERTSTR(ret == 0, "Could not insert syncrequest "<<sr<<endl<<"Error: "<<itsEnv->strerror(ret));
      LOG_TRACE_FLOW_STR("Sync request "<<sr<<" inserted into sync database");
    }

    void BDBSyncMaster::removeSyncRequest(SyncReqType sr)
    {
      Dbt key, value;
      char *thekey = "lastSyncRequest";
      key.set_data(thekey);
      key.set_size(strlen(thekey));
      value.set_data(&sr);
      value.set_size(sizeof(SyncReqType));
      DbTxn* transaction = 0;
      itsEnv->txn_begin(NULL, &transaction, 0);
      Dbc* cursorp;
      itsSyncDB->cursor(transaction, &cursorp, 0);

      int flags = DB_SET;
      LOG_TRACE_FLOW("Looking for SyncRequest in sync database");
      int ret = 0;
      while ((ret = cursorp->get(&key, &value, flags)) == 0) {
	flags = DB_NEXT_DUP;
	SyncReqType srInDB = *(static_cast<SyncReqType*>(value.get_data()));
	if (srInDB == sr) {
	  ASSERTSTR(cursorp->del(0) == 0, "Could not remove SyncRequest");
	  LOG_TRACE_FLOW("SyncRequest removed from sync database");
	  break;
	} else {
	  LOG_TRACE_FLOW("SyncRequest was no longer in the database");
	}
      }
      LOG_TRACE_FLOW_STR("Stopped looking for SyncReq because: "<<itsEnv->strerror(ret));
      cursorp->close();
      transaction->commit(0);
    }

    BDBSyncSlave::BDBSyncSlave(DbEnv* env):
      BDBSyncer(env),
      itsExpectedSyncReq(1)
    {
    }

    BDBSyncSlave::~BDBSyncSlave()
    {
      closeSyncDB();
    }

    SyncReqType BDBSyncSlave::handleRequest()
    {      
      if (itsDBOpened) {
	if (getHighestSyncRequest() >= itsExpectedSyncReq) {
	  itsExpectedSyncReq++;
	  return itsExpectedSyncReq-1;
	} else {
	  return 0;
	}
      } else {
	return 0;
      }
    }

    void BDBSyncSlave::handleReply(BDBMessage& message)
    {
      // right now syncreplies are broadcasted, so slaves can receive a sync reply
      // ASSERTSTR(false, "BDBSyncSlave should not get a SYNC_REPLY");
    }

    void BDBSyncSlave::openSyncDB()
    {
      while(!itsDBOpened) {
	DbTxn* myTxn = 0;
	itsEnv->txn_begin(NULL, &myTxn, 0);
	int ret = 0;
	delete itsSyncDB;
	itsSyncDB = new Db(itsEnv, 0);
	
	//itsSyncDB->set_flags(DB_DUPSORT);
	const char* home;
	itsEnv->get_home(&home);
	LOG_TRACE_FLOW_STR("opening sync database on slave in directory "<<home);  
	ret = itsSyncDB->open(NULL, "BDB_SYNC", "BDB_SYNC", DB_BTREE, DB_RDONLY, 0);
	if (ret == 0) {
	  myTxn->commit(0);
	  itsDBOpened = true;
	} else {
	  myTxn->abort();
	  itsSyncDB->close(0);
	  sleep(1);
	}
      }
    }
    
    SyncReqType BDBSyncSlave::getHighestSyncRequest()
    {
      Dbt key, value;
      Dbc* cursorp;
      int ret = 0;
      char *thekey = "lastSyncRequest";
      key.set_data(thekey);
      key.set_size(strlen(thekey));
       
      itsSyncDB->cursor(NULL, &cursorp, 0);

      ret = cursorp->get(&key, &value, DB_SET);
      SyncReqType sr;
      if (ret == 0) {
	// found a syncrequest
	sr = *(static_cast<SyncReqType*>(value.get_data()));
	LOG_TRACE_FLOW_STR("SyncRequest " << sr << " found in db on slave");
      } else {
	LOG_TRACE_FLOW_STR(" No SyncRequest found in db on slave: "<<itsEnv->strerror(ret));
	sr = -1;
      }
      ret = cursorp->close();
      return sr;      
    }

    BDBSyncer::BDBSyncer(DbEnv* env) :
      itsEnv(env),
      itsSyncDB(0),
      itsDBOpened(false)
    {}

    BDBSyncer::~BDBSyncer()
    {}

  } // namespace BDBReplication
} // namespace LOFAR
