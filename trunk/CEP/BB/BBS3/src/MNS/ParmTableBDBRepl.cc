//# ParmTableBDBRepl.cc: Object to hold parameters in a mysql database table.
//#
//# Copyright (C) 2002
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

#include <lofar_config.h>
#include <MNS/ParmTableBDBRepl.h>
#include <MNS/MeqDomain.h>
#include <Common/BlobIBufChar.h>
#include <Common/BlobOBufChar.h>
#include <Common/BlobIBufString.h>
#include <Common/BlobOBufString.h>
#include <Common/BlobIStream.h>
#include <Common/BlobOStream.h>
#include <Common/BlobString.h>
#include <Common/LofarLogger.h>
#include <casa/BasicMath/Math.h>
#include <stdlib.h>
#include <sstream>
#include <Transport/TH_MPI.h> // for rank
#include <sys/stat.h>         // for mkdir
#include <sys/types.h>        // for mkdir

using namespace casa;

namespace LOFAR {

  BDBReplicator* ParmTableBDBRepl::theirReplicator = 0;
  int ParmTableBDBRepl::theirReplicatorRefCount = 0;

  ParmTableBDBRepl::ParmTableBDBRepl (const string& tableName,
				      const string& masterHostName,
				      const int masterPort,
				      const bool isMaster) 
    : ParmTableBDB(tableName),
      itsIsMaster(isMaster)
  {
    LOG_TRACE_FLOW("creating replicating bdb");
    if (isMaster){
      LOG_TRACE_FLOW(" master");
    } else {
      LOG_TRACE_FLOW(" client");
    }
    string rank = "0";
#ifdef HAVE_MPI    
    int isInitialized = 0;
    MPI_Initialized(&isInitialized); 
    // this class can also be used by parmdb (which can be built with HAVE_MPI)
    // parmdb does not call MPI_INIT, so we can't get our rank
    if (isInitialized) {
      stringstream rankss;
      rankss << TH_MPI::getCurrentRank();
      rankss >> rank;
      LOG_TRACE_FLOW_STR("BDBRepl rank " << TH_MPI::getCurrentRank() << " -> " << rank);
    }
#endif    
    if (isMaster) {
      rank = "master";
    }
    if (theirReplicator==0) {
      itsBDBHomeName = "BBS3.ParmBDBRepl." + rank;

      char hostnameBuffer[128];
      gethostname(hostnameBuffer, 128);
      string myHostName = hostnameBuffer;

      mkdir(itsBDBHomeName.c_str(), S_IRWXU|S_IRGRP|S_IXGRP);
      theirReplicator = new BDBReplicator(itsBDBHomeName, 
					  myHostName,
					  masterPort, //start looking for open ports at <masterPort>
					  masterHostName,
					  masterPort,
					  isMaster);
    }
    theirReplicatorRefCount ++;

    itsBDBTableName = tableName + ".bdbrepl";
    delete itsDb;
    itsDb = 0;
  }

  ParmTableBDBRepl::~ParmTableBDBRepl()
  {
    LOG_TRACE_FLOW("closing database");
    if (itsDb != 0) {
      itsDb->sync(0);
      itsDb->close(0);
      delete itsDb;
      itsDb = 0;
    }

    // We shouldn't destroy the environment because it is owned by the BDBReplicator
    itsDbEnv = 0;

    theirReplicatorRefCount--;
    if (theirReplicatorRefCount == 0) {
      delete theirReplicator;
      theirReplicator = 0;
    }
    LOG_TRACE_FLOW("database closed");
  }

  void ParmTableBDBRepl::connect(){
    LOG_TRACE_FLOW("connecting bdbrepl");
    theirReplicator->startReplication();
    LOG_TRACE_FLOW("replication started");
    itsDbEnv = theirReplicator->getDbEnv();

    u_int32_t oFlags;
    if (itsIsMaster) {
      oFlags = DB_CREATE;
    } else {
      oFlags = DB_RDONLY;
    }

    bool dbOpened = false;
    while(!dbOpened) {
      DbTxn* myTxn = 0;
      int ret = 0;
      ret = itsDbEnv->txn_begin(NULL, &myTxn, 0);
      LOG_TRACE_FLOW_STR("environment: "<<itsDbEnv<<" transaction: "<<myTxn);
      ASSERTSTR(ret == 0, "BDBRepl no transaction while opening database "<<itsBDBTableName<<" in "<<itsBDBHomeName<<": "<< itsDbEnv->strerror(ret));
      delete itsDb;
      itsDb = new Db(itsDbEnv, DB_CXX_NO_EXCEPTIONS);

      itsDb->set_flags(DB_DUPSORT);
      //  if (itsDb->open(transid, filename, database, dbtype, flags, mode) !=0) {
      LOG_TRACE_FLOW_STR("opening replicated database "<<itsBDBTableName);
#if 0
      ret = itsDb->open(myTxn, itsBDBTableName.c_str(), itsBDBTableName.c_str(), DB_BTREE, oFlags, 0);
      cout<<" OPEN FAILED : "<<itsDbEnv->strerror(ret)<<endl;
      ASSERTSTR( ret == 0, "BDBRepl while opening database "<<itsBDBTableName<<" in "<<itsBDBHomeName<<": "<< itsDbEnv->strerror(ret));
#else
      //ret = itsDb->open(NULL, itsBDBTableName.c_str(), itsBDBTableName.c_str(), DB_BTREE, oFlags, 0);
      ret = itsDb->open(myTxn, itsBDBTableName.c_str(), itsBDBTableName.c_str(), DB_BTREE, oFlags, 0);
      if (ret != 0 ) {
	itsDb->close(0);
	myTxn->abort();
	cerr<<"BDBRepl while opening database "<<itsBDBTableName<<" in "<<itsBDBHomeName<<": "<< itsDbEnv->strerror(ret)<<endl;
	sleep(1);
      } else {
	myTxn->commit(0);
	dbOpened = true;
      }
#endif
    }
    LOG_TRACE_STAT("connected to database");
  }

  void ParmTableBDBRepl::createTable(const string& tableName){
    ParmTableBDBRepl PT(tableName,
			"localhost",
			13157,
			true);
    PT.connect();
  }
}
