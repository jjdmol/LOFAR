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

#include <BDBReplicator.h>

int
print_data (Dbc * dbc)
{
  Dbt key, value;

  cout<<"\tkey\tvalue"<<endl;
  cout<<"\t======\t====="<<endl;

  int flags = DB_FIRST;
  while(dbc->get(&key, &value, flags) == 0) {
    flags = DB_NEXT;
    char buffer1[40];
    char buffer2[40];
    snprintf (buffer1, key.get_size()+1, "%s\0", (char*)key.get_data());
    snprintf (buffer2, value.get_size()+1, "%s\0", (char*)value.get_data());
    cout<<"\t"<<buffer1<<"\t"<<buffer2<<endl;
  }
  return 0;
}

int
client (DbEnv* dbenv)
{
  DbTxn* myTxn;
  dbenv->txn_begin(NULL, &myTxn, 0);
  
  Db myDb(dbenv, DB_CXX_NO_EXCEPTIONS);
  Db mydDb(dbenv, DB_CXX_NO_EXCEPTIONS);

  LOG_TRACE_FLOW_STR ("opening database in environment "<<dbenv);
  int minor, major, patch;
  dbenv->version(&major, &minor, &patch);
  LOG_TRACE_FLOW_STR("db version: "<<major<<"."<<minor<<"-"<<patch);

  try{
//     int ret = mydDb.open(myTxn, "quotes", "quotes", DB_BTREE, DB_CREATE, 0);
//     if(ret !=0) {
//       LOG_TRACE_FLOW_STR("Could not open database"<<dbenv->strerror(ret));
//       cerr<<"Could not open database"<<dbenv->strerror(ret)<<endl;
//     }
//     mydDb.close(0);
    int ret = myDb.open(myTxn, "quotes", "quotes", DB_BTREE, DB_RDONLY, 0);
    if(ret !=0) {
      cerr<<"Could not open database"<<dbenv->strerror(ret)<<endl;
      LOG_TRACE_FLOW_STR("Could not open database"<<dbenv->strerror(ret));
    }
  } catch (Exception &e) {
    cout<<"Exception while opening database: "<<e.what()<<endl;
    exit(1);
  } catch (...) {
    cout<<"caught unknown exception"<<endl;
    exit(1);
  }
  cout<<"db opened"<<endl;
  myTxn->commit(0);

  for (;;) {    
    cout<<"Reading from database .."<<endl;
    LOG_TRACE_FLOW("Reading");
    
    Dbc* cursor;
    int ret = myDb.cursor(NULL, &cursor, 0);
    if (ret != 0) {
      cerr<<"Error while getting cursor: "<<dbenv->strerror(ret)<<endl;
      exit(1);
    }
    
    print_data (cursor);
    cursor->close ();
    
    sleep (3);
  }
  
  myDb.close(0);

  while(1);
  return 0;
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
  if (myDb.open(myTxn, "quotes", "quotes", DB_BTREE, DB_CREATE, 0)!=0)
    LOG_TRACE_FLOW("could not open database on master");

  if (myTxn->commit(0) != 0)
    LOG_TRACE_FLOW("could not commit open transaction");

  Dbt key, value;

  char buf[128], *rbuf;

  while(1) {
    cout << "QUOTESERVER> ";
    fflush (stdout);
    
    if (fgets (buf, sizeof (buf), stdin) == NULL)
      break;
    (void) strtok (&buf[0], " \t\n");
    rbuf = strtok (NULL, " \t\n");
    if (rbuf == NULL || rbuf[0] == '\0') {
      if (strncmp (buf, "exit", 4) == 0 || strncmp (buf, "quit", 4) == 0)
	break;
      continue;
    }
    
    key.set_data(buf);
    key.set_size(strlen (buf));

    value.set_data(rbuf);
    value.set_size(strlen (rbuf));

    LOG_TRACE_FLOW_STR ("writing to master database: <"<<(char*)buf<<"="<<rbuf<<">");
    int ret = myDb.put(0, &key, &value, DB_AUTO_COMMIT);
    if(ret !=0) {
      LOG_TRACE_FLOW_STR("could not write to database on master:"<<myDbEnv->strerror(ret));
    }
  }
  
  return 0;
}

int mainNoRep(int argc, char** argv)
{
  string itsDbEnvName = "dir0";
  LOG_TRACE_FLOW("Starting up");
  DbEnv* itsDbEnv = new DbEnv(DB_CXX_NO_EXCEPTIONS);
  u_int32_t flags = DB_CREATE | DB_INIT_MPOOL | DB_INIT_TXN;
  LOG_TRACE_FLOW("Opening environment");
  int ret = itsDbEnv->open(itsDbEnvName.c_str(), flags, 0);
  if (ret != 0) {
    LOG_TRACE_FLOW("Cannot open db environment");
    LOG_TRACE_FLOW_STR("Error: "<<itsDbEnv->strerror(ret));
  }
  LOG_TRACE_FLOW("Starting master");
  try {
    master (itsDbEnv);
  } catch (Exception &e) {
    cout<<"Exception: "<<e.what()<<endl;
  } catch (DbDeadlockException &e) {
    cout<<"DbException: "<<e.what()<<endl;
  } catch (DbException &e) {
    cout<<"DbException: "<<e.what()<<endl;
  } catch (DbLockNotGrantedException &e) {
    cout<<"DbException: "<<e.what()<<endl;
  } catch (DbMemoryException &e) {
    cout<<"DbException: "<<e.what()<<endl;
  } catch (DbRunRecoveryException &e) {
    cout<<"DbException: "<<e.what()<<endl;
  } 
  return 0;
} 

int
main (int argc, char *argv[])
{
  INIT_LOGGER("BDBrepQuote");
#if 0
  return mainNoRep(argc, argv);
#else
  int master_eid;
  int my_eid;
  char *myHostName;
  unsigned short myPort;
  char *masterHostName;
  unsigned short masterPort;
  

  extern char *optarg;
  enum
  { MASTER, CLIENT, UNKNOWN } whoami;
  int maxsites, nsites, ret, priority, verbose;
  char *c, ch;
  const char *home, *progname;

  master_eid = DB_EID_INVALID;

  whoami = UNKNOWN;
  maxsites = nsites = ret = verbose = 0;
  priority = 100;
  home = "TESTDIR";
  progname = "ex_repquote";

  while ((ch = getopt (argc, argv, "i:Ch:Mm:n:o:p:v")) != EOF)
    switch (ch)
      {
      case 'i':
	my_eid = atoi (optarg);
	if (my_eid == 0)
	  {
	    whoami = MASTER;
	    master_eid = 1;
	  }
	else
	  {
	    whoami = CLIENT;
	  }
	break;
      case 'h':
	home = optarg;
	break;
      case 'm':
	myHostName = strtok(optarg, ":");
	if ((c = strtok (NULL, ":")) == NULL)
	  {
	    fprintf (stderr, "Bad host specification.\n");
	  }
	myPort = (unsigned short) atoi (c);
	break;
      case 'o':
	masterHostName = strtok(optarg, ":");
	if ((c = strtok (NULL, ":")) == NULL)
	  {
	    fprintf (stderr, "Bad host specification.\n");
	  }
	masterPort = atoi (c);
	break;
      case 'p':
	priority = atoi (optarg);
	break;
      case 'v':
	verbose = 1;
	break;
      case '?':
      default:
	break;
      }

  BDBReplicator* BDBR;
  if (whoami == MASTER){
    BDBR = new BDBReplicator(home,"localhost", myPort, "localhost", masterPort, true);
  } else {
    BDBR = new BDBReplicator(home,"localhost", myPort, "localhost", masterPort, false);
  };


  BDBR->startReplication();
  DbEnv* myDbEnv = BDBR->getDbEnv();

  if (whoami == MASTER)
    {
      LOG_TRACE_FLOW_STR("starting master");
      master (myDbEnv);
    }
  else
    {
      //      sleep (5);
      client (myDbEnv);
    }

  return 0;
#endif
}

