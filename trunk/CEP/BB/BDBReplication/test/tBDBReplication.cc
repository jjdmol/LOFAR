//#  test_client.cc: The client for the BDBReplicator test program
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <db_cxx.h>

#include <BDBReplication/BDBReplicator.h>

void write2db(DbEnv* myDbEnv, Db* myDb, char* key, char* value) {
  Dbt keyT, valueT;

  keyT.set_data(key);
  keyT.set_size(strlen(key));

  valueT.set_data(value);
  valueT.set_size(strlen(value));
  
  LOG_TRACE_FLOW_STR ("writing to master database: <"<<(char*)key<<"="<<value<<">");
  int ret = myDb->put(0, &keyT, &valueT, DB_AUTO_COMMIT);
  if(ret !=0) {
    LOG_TRACE_FLOW_STR("could not write to database on master:"<<myDbEnv->strerror(ret));
  }
}  

int readFromDb(DbEnv* myDbEnv, Db* myDb, char* key) {
  Dbt keyT, valueT;

  keyT.set_data(key);
  keyT.set_size(strlen(key));
  
  valueT.set_data(key);
  valueT.set_size(strlen(key));

  LOG_TRACE_FLOW_STR ("reading from client database: key = "<<key);
  Dbc* cursorp;
  myDb->cursor(NULL, &cursorp, 0);

  int ret = cursorp->get(&keyT, &valueT, DB_SET);
//   DbTxn* myTxn = 0;
//   myDbEnv->txn_begin(NULL, &myTxn, 0);
//myDb->get(NULL, &keyT, &valueT, DB_AUTO_COMMIT);
//   myTxn->commit(0);
  cursorp->close();

  if(ret !=0) {
    LOG_TRACE_FLOW_STR("could not read from database on client:"<<myDbEnv->strerror(ret));
    cerr<<"could not read from database on client:"<<myDbEnv->strerror(ret)<<endl;;
    return 0;
  }
  return atoi((const char*)valueT.get_data());
}  

int
master (DbEnv* myDbEnv)
{  
  //open database

  DbTxn* myTxn = 0;
  myDbEnv->txn_begin(NULL, &myTxn, 0);
  LOG_TRACE_FLOW_STR("creating db");  
  Db myDb(myDbEnv, 0);
  LOG_TRACE_FLOW_STR("opening database on master");  
  if (myDb.open(myTxn, "test", "test", DB_BTREE, DB_CREATE, 0)!=0)
    LOG_TRACE_FLOW("could not open database on master");

  if (myTxn->commit(0) != 0)
    LOG_TRACE_FLOW("could not commit open transaction");

  Dbt key, value;

  // set 4 values in the db:
  // first a = 2, b = 3, a = 5, c = 7
  // the client knows now that a*b*c should be 105

  write2db(myDbEnv, &myDb, "a", "2");
  write2db(myDbEnv, &myDb, "b", "3");
  write2db(myDbEnv, &myDb, "a", "5");
  write2db(myDbEnv, &myDb, "c", "7");
  
  while(1);

  return 0;
}

bool
client (DbEnv* dbenv)
{
  DbTxn* myTxn;
  dbenv->txn_begin(NULL, &myTxn, 0);
  
  Db myDb(dbenv, DB_CXX_NO_EXCEPTIONS);

  LOG_TRACE_FLOW_STR ("opening database in environment "<<dbenv);
  int minor, major, patch;
  dbenv->version(&major, &minor, &patch);
  LOG_TRACE_FLOW_STR("db version: "<<major<<"."<<minor<<"-"<<patch);

  int retries = 3;
  while(1) {
    try{
      int ret = myDb.open(myTxn, "test", "test", DB_BTREE, DB_RDONLY, 0);
      if(ret !=0) {
	cerr<<"Could not open database"<<dbenv->strerror(ret)<<endl;
	LOG_TRACE_FLOW_STR("Could not open database"<<dbenv->strerror(ret));
      } else {
	cout<<"db opened"<<endl;	
	break;
      }
    } catch (Exception &e) {
      cout<<"Exception while opening database: "<<e.what()<<endl;
      exit(1);
    } catch (...) {
      cout<<"caught unknown exception"<<endl;
      exit(1);
    }
    retries--;
    if (retries <= 0) exit(1);
    sleep(3);
  }
  myTxn->commit(0);

  bool testOK = false;
  for (int i=0; i<3; i++) {    
    LOG_TRACE_FLOW("Reading");
    
    int a = readFromDb(dbenv, &myDb, "a");
    int b = readFromDb(dbenv, &myDb, "b");
    int c = readFromDb(dbenv, &myDb, "c");

    if (a*b*c == 105) {
      cerr<<"The answer is 105"<<endl;
      testOK = true;
      break;
    } else { 
      cerr<<"The answer is not 105, but "<<a*b*c<<endl;
      sleep(3); // wait a few seconds before a retry
    }
  }  
  myDb.close(0);

  return testOK;
}

int
main (int argc, char *argv[])
{
  try {
  INIT_LOGGER("tBDBReplication");
  unsigned short myPort = 0;
  unsigned short masterPort = 0;
  
  extern char *optarg;
  bool amMaster = false;
  char *home = "";
  char ch;

  while ((ch = getopt (argc, argv, "fh:m:o:")) != EOF) {
    switch (ch)
      {
      case 'f':
	amMaster = true;
	break;
      case 'h':
	home = optarg;
	break;
      case 'o':
	myPort = atoi (optarg);
	break;
      case 'm':
	masterPort = atoi (optarg);
	break;
      default:
	break;
      }
  }

  BDBReplicator* BDBR;
  if (amMaster){
    BDBR = new BDBReplicator(home,"localhost", myPort, "localhost", myPort, true);
  } else {
    BDBR = new BDBReplicator(home,"localhost", myPort, "localhost", masterPort, false);
  };


  BDBR->startReplication();
  DbEnv* myDbEnv = BDBR->getDbEnv();

  sleep(1);

  if (amMaster)
    {
      LOG_TRACE_FLOW_STR("starting master");
      master (myDbEnv);
    }
  else
    {
      LOG_TRACE_FLOW_STR("starting client");
      sleep(5);
      if(!client (myDbEnv)) return 1;
    }

  return 0;
  } catch ( ... ) {
    return 1;
  }
}

