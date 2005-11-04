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

using namespace LOFAR;
using namespace LOFAR::BDBReplication;


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
  Dbc* cursorp = 0;
  int ret = 0;
  try {
    ret = myDb->cursor(0, &cursorp, 0);
  } catch (DbException& e) {
    cerr<<"exception while getting cursor: "<<e.what()<<endl;
  } catch (...) {
    cerr<<"general exception while getting cursor"<<endl;
  }
  if(ret !=0) {
    LOG_TRACE_FLOW_STR("could not get cursor from database on client:"<<myDbEnv->strerror(ret));
    return 0;
  }

  ret = cursorp->get(&keyT, &valueT, DB_SET);
//   DbTxn* myTxn = 0;
//   myDbEnv->txn_begin(NULL, &myTxn, 0);
//   myDb->get(NULL, &keyT, &valueT, DB_AUTO_COMMIT);
//   myTxn->commit(0);
  cursorp->close();

//   myDbEnv->set_msgfile(stdout);
//   myDb->stat_print(0);
//   ret = myDb->get(0, &keyT, &valueT, DB_AUTO_COMMIT);
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
  const char* home;
  myDbEnv->get_home(&home);
  LOG_TRACE_FLOW_STR("opening database on master in directory "<<home);  
  if (myDb.open(myTxn, "test", "test", DB_BTREE, DB_CREATE, 0)!=0)
    LOG_TRACE_FLOW("could not open database on master");

  if (myTxn->commit(0) != 0)
    LOG_TRACE_FLOW("could not commit open transaction");

  Dbt key, value;

  // set 4 values in the db:
  // first a = 2, b = 3, a = 5, c = 7
  // the client knows now that a*b*c should be 105

  write2db(myDbEnv, &myDb, "keya", "2");
  write2db(myDbEnv, &myDb, "keyb", "3");
  write2db(myDbEnv, &myDb, "keya", "5");
  write2db(myDbEnv, &myDb, "keyc", "7");
  
  return 0;
}

bool
openClientDB (DbEnv* dbenv, Db** myDb)
{
  DbTxn* myTxn;
  dbenv->txn_begin(NULL, &myTxn, 0);
  
  *myDb = new Db(dbenv, DB_CXX_NO_EXCEPTIONS);

  LOG_TRACE_FLOW_STR ("opening database in environment "<<dbenv);
  int minor, major, patch;
  dbenv->version(&major, &minor, &patch);
  LOG_TRACE_FLOW_STR("db version: "<<major<<"."<<minor<<"-"<<patch);

  int ret = (*myDb)->open(myTxn, "test", "test", DB_BTREE, DB_RDONLY, 0);
  if(ret !=0) {
    cerr<<"Could not open database"<<dbenv->strerror(ret)<<endl;
    LOG_TRACE_FLOW_STR("Could not open database"<<dbenv->strerror(ret));
    myTxn->abort();
    return false;
  } else {
    cout<<"db opened"<<endl;	
    myTxn->commit(0);
    return true;
  }
}
bool doClientTest(DbEnv* dbenv, Db* myDb)
{
  LOG_TRACE_FLOW("Reading");
    
  int a = readFromDb(dbenv, myDb, "keya");
  int b = readFromDb(dbenv, myDb, "keyb");
  int c = readFromDb(dbenv, myDb, "keyc");
  
  if (a*b*c == 105) {
    cerr<<"The answer is 105"<<endl;
    return true;
  } else { 
    cerr<<"The answer is not 105, but "<<a*b*c<<endl;
    return false;
  }
}


int
main (int argc, char *argv[])
{
  try {
  INIT_LOGGER("tBDBReplication");
  unsigned short myPort = 8020;
  unsigned short masterPort = 8020;
  
  extern char *optarg;
  bool amMaster = true;
  char *home = "masterDir";
  char ch;

  while ((ch = getopt (argc, argv, "h:m:o:")) != EOF) {
    switch (ch)
      {
      case 'f':
	break;
      case 'h':
	home = optarg;
	break;
      case 'o':
	myPort = atoi (optarg);
	break;
      case 'm':
	amMaster = false;
	masterPort = atoi (optarg);
	break;
      default:
	break;
      }
  }

  BDBReplicator* BDBR;
  if (amMaster){
    BDBR = new BDBReplicator(home,"localhost", myPort, "localhost", myPort, 1);
  } else {
    BDBR = new BDBReplicator(home,"localhost", myPort, "localhost", masterPort, 0);
  };


  BDBR->startReplication();
  DbEnv* myDbEnv = BDBR->getDbEnv();

  if (amMaster)
    {
      LOG_TRACE_FLOW_STR("starting master");
      master (myDbEnv);
      while(1) BDBR->handleMessages();
    }
  else
    {
      LOG_TRACE_FLOW_STR("starting client");
      sleep(10);
      Db* myDb = 0;
      while (!openClientDB(myDbEnv, &myDb))
	BDBR->handleMessages();
	     
      bool testSucceeded = false;
      BDBR->handleMessages();
      while(!doClientTest(myDbEnv, myDb)) {
	BDBR->handleMessages();
      };
      myDb->close(0);
    }

  delete BDBR;

  return 0;
  } catch ( Exception& e ) {
    cout<<"EXCEPTION: "<<e.what()<<endl;
    return 1;
  } catch ( ... ) {
    return 1;
  }
}
