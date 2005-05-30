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

  ParmTableBDBRepl::ParmTableBDBRepl (const string& userName, 
				      const string& tableName,
				      const string& hostName,
				      const int port,
				      const string& masterHostName,
				      const int masterPort,
				      const bool isMaster) 
    : ParmTableBDB(userName, tableName),
      itsIsMaster(isMaster)
  {
    cout<<"creating replicating bdb";
    if (isMaster){
      cout<<" master"<<endl;
    } else {
      cout<<" client"<<endl;
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
      cout << "BDBRepl rank " << TH_MPI::getCurrentRank() << " -> " << rank << endl;
    }
#endif    
    if (isMaster) {
      rank = "master";
    }
    if (theirReplicator==0) {
      string envName = "/tmp/" + userName + "." + rank + ".BBS3.ParmDB";
      cout<<"Creating environment in "<<envName<<endl;
      
      mkdir(envName.c_str(), S_IRWXU|S_IRGRP|S_IXGRP);
      theirReplicator = new BDBReplicator(envName, 
					  hostName,
					  port,
					  masterHostName,
					  masterPort,
					  isMaster);
    }
    theirReplicatorRefCount ++;

    itsTableName = tableName + ".bdb";
    delete itsDb;
    itsDb = 0;
  }

  ParmTableBDBRepl::~ParmTableBDBRepl()
  {
    cout<<"closing database"<<endl;
    if (itsDb != 0) {
      itsDb->sync(0);
      itsDb->close(0);
      delete itsDb;
      itsDb = 0;
    }

    theirReplicatorRefCount--;
    if (theirReplicatorRefCount == 0) {
      delete theirReplicator;
      theirReplicator = 0;
    }
    cout<<"database closed"<<endl;
  }

  void ParmTableBDBRepl::connect(){
    cout<<"connecting bdbrepl"<<endl;
    theirReplicator->startReplication();
    cout<<"replication started"<<endl;
    DbEnv* myDbEnv = theirReplicator->getDbEnv();
    cout<<"environment: "<<myDbEnv<<endl;
    itsDb = new Db(myDbEnv, DB_CXX_NO_EXCEPTIONS);

    u_int32_t oFlags;
    if (itsIsMaster) {
      oFlags = DB_CREATE;
    } else {
      oFlags = DB_RDONLY;
    }
    itsDb->set_flags(DB_DUPSORT);
    //  if (itsDb->open(transid, filename, database, dbtype, flags, mode) !=0) {
    cout<<"opening replicated database "<<itsTableName<<endl;
    int ret = 1;
    while (ret != 0){
      //ret = itsDb->open(NULL, itsTableName.c_str(), itsTableName.c_str(), DB_BTREE, oFlags, 0);
      DbTxn* myTxn = 0;
      myDbEnv->txn_begin(NULL, &myTxn, 0);
      ret = itsDb->open(myTxn, itsTableName.c_str(), itsTableName.c_str(), DB_BTREE, oFlags, 0);
      if (ret != 0 ) {
	itsDb->close(0);
	char *homeName;
	myDbEnv->get_home((const char**)&homeName);
	cerr<<"BDBRepl while opening database "<<itsTableName<<" in "<<homeName<<": "<< myDbEnv->strerror(ret)<<endl;
      }
      myTxn->commit(0);
    }

    LOG_TRACE_STAT("connected to database");
  }

  void ParmTableBDBRepl::createTable(const string& userName, const string& tableName){
#if 1

    ParmTableBDBRepl PT(userName, 
			tableName,
			"localhost",
			13157,
			"localhost",
			13157,
			true);
    PT.connect();
#else



    string envName = "/tmp/" + userName + ".0.BBS3.ParmDB";
    mkdir(envName.c_str(), S_IRWXU|S_IRGRP|S_IXGRP);

    BDBReplicator BDBR(envName, 
		       "localhost",
		       13157,
		       "localhost",
		       13157,
		       true);
    BDBR.startReplication();
    DbEnv* myDbEnv = BDBR.getDbEnv();
    Db tmpDb(myDbEnv, DB_CXX_NO_EXCEPTIONS);

    string fullTableName = envName + "/" + tableName + ".bdb";
    // Do the same as the connect but now with the flag DB_CREATE
    cout<<"Creating database: " << fullTableName<<endl;
    u_int32_t oFlags = DB_CREATE;
    Db tmpDb(NULL, 0);
    tmpDb.set_flags(DB_DUPSORT);
    cout<<"ready to open database"<<endl;
    if (tmpDb.open(NULL, fullTableName.c_str(), fullTableName.c_str(), DB_BTREE, oFlags, 0) != 0 ) {
      tmpDb.close(0);
      ASSERTSTR(false, "could not create BDBRepl database");    
    }
    LOG_TRACE_STAT("created database");
    tmpDb.sync(0);
    tmpDb.close(0);
#endif
  }
}
