 //  DH_Postgresql.cc: Database persistent DH_Database
//
//  Copyright (C) 2000, 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//////////////////////////////////////////////////////////////////////////

#include <DH_Postgresql.h>			// for class definition
// TODO: lofar_strstream.h results in warning on lofar17:
//#include <Common/lofar_strstream.h>		// for ostringstream
#include <Common/lofar_iostream.h>		// for cout, cerr
#include <Common/Stopwatch.h>			// for Stopwatch class
#include <Common/Debug.h>			// for AssertStr


#include <sstream>				// for ostrstream
#include <sys/time.h>				// for ctime (), time ()
#include <unistd.h>				// for gethostname ()
#include <stdlib.h>				// for strtoul


using namespace std;

namespace LOFAR
{


// TODO: Convert all couts to debug trace statements


ulong    DH_Postgresql::theirInstanceCount = 0L;
PGconn * DH_Postgresql::theirConnection = NULL;

DH_Postgresql::DH_Postgresql (const string& name, const string& type)
  : DH_Database (name, type) {

  if (DH_Postgresql::theirInstanceCount == 0L) {
    ConnectDatabase ();
  }

  DH_Postgresql::theirInstanceCount ++;

  cerr << DH_Postgresql::theirInstanceCount 
    << ") DH_Postgresql constructed." << endl;
} 


DH_Postgresql::~DH_Postgresql () {
  cerr << DH_Postgresql::theirInstanceCount
    << ") DH_Postgresql destroyed." << endl;

  DH_Postgresql::theirInstanceCount --;

  if (DH_Postgresql::theirInstanceCount == 0L) {
    DisconnectDatabase ();
  }
}


void DH_Postgresql::ConnectDatabase (void) {
  // TODO: Make configurable database host and name.

  cerr << "DH_Postgresql::ConnectDatabase (); Connecting to databse."
       << endl;

  ostringstream ConnInfo;

// NOTE: lofar3 cannot see dop49. Must use ip address.

  ConnInfo << "host=" << "10.87.2.49" << " dbname=TransportHolder";
  cerr << "  using: " << ConnInfo.str () << endl;

  /*
  nStore = 0;
  nStoredBytes = 0;
  dtStorage = 0;
  dtInsert = 0;
 
  nRetrieve = 0;
  nRetrievedBytes = 0;
  dtQuery = 0;
  dtUpdate = 0;
  */

  DH_Postgresql::theirConnection = PQconnectdb (ConnInfo.str (). c_str ());

  AssertStr (PQstatus (DH_Postgresql::theirConnection) == CONNECTION_OK, 
    "ConnectDatabase(); Could not connect to database.") 

  cerr << "DH_Postgresql::ConnectDatabase (): Succesfully connected "
       << "to database" << endl;

//  // Delete all previous message from message table
//  ExecuteCommand ("DROP TABLE message;");
//  // TODO: error recovery
//
//  ExecuteCommand ("CREATE TABLE message (AppId int, Tag int, SeqNo int, Status text, Size int, TimeStamp text, Type text, Name text, Blob text, ByteString bytea);");
//
//  // log table
//  ExecuteCommand ("DROP TABLE log;");
//  // foutafhandeling
//
//  ExecuteCommand ("CREATE TABLE log (Time text, Entry text);");
//  // foutafhandeling

/*
  ostringstream ostr;
  char curhostname [80];
  gethostname (curhostname, 80);
  ostr << "Connected from " << curhostname << " to " << hostname;
  LogEntry (ostr);

  cerr << curhostname << " connecting to database." << endl;

  LogEntry ("  (Dropped message table).");
  LogEntry ("  (Re-created message table.)");
  LogEntry ("  (Dropped log table.)");
  LogEntry ("  (Re-created log table.)");
*/
#if defined (__GNUC__) && (__GNUC__) < 3
    /*  
  LogEntry ("WARNING: The client library (libpq) does not support writing");
  LogEntry ("  and reading binary strings. This function will be disabled");
  LogEntry ("  for this session.");
    */
#endif
}


void DH_Postgresql::DisconnectDatabase (void) {
  /*
  double avgStore, avgInsert, avgStorageThP,
    avgRetrieve, avgQuery, avgUpdate, avgRetrievalThP;

  // Calculate metrics
  avgStore = dtStorage / nStore;
  avgInsert = dtInsert / nStore;

  avgStorageThP = nStoredBytes / dtStorage;

  avgRetrieve = dtRetrieve / nRetrieve;
  avgQuery = dtQuery / nRetrieve;
  avgUpdate = dtUpdate / nRetrieve;

  avgRetrievalThP = nRetrievedBytes / dtRetrieve;

  // Display the metrics
  // TODO: Perhaps move the metrics to log database.
  cerr << "Average store time: " << avgStore 
       << " sec." << endl;
  cerr << "Average insert time: " << avgInsert
       << " sec. (" << (avgInsert / avgStore) * 100 
       << " %)" << endl;

  cerr << "Average storage throughput: "
       << avgStorageThP << " bytes/sec" << endl;;

  cerr << "Average retrieve time: " << avgRetrieve
       << " sec." << endl;
  cerr << "Average query time: " << avgQuery
       << " sec. (" << (avgQuery / avgRetrieve) * 100
       << " %)" << endl;
  cerr << "Average update time: " << avgUpdate 
       << " sec. (" << (avgUpdate / avgRetrieve) * 100 
       << " %)" << endl;

  cerr << "Average retrieval throughput: "
       << avgRetrievalThP << " bytes/sec" << endl;

  LogEntry ("Disconnecting.");
  */

  cerr << "DH_Postgresql::Disconnect(); Disconnecting from database." << endl;
  PQfinish (theirConnection);

  // TODO: Check if disconnect has succeeded.
}

#define MAX_BYTEA_SIZE (5*4096)
// TODO: Do a check on max-bytea-size

bool DH_Postgresql::StoreInDatabase (int, int tag, char * buf, int size) {
  //  cerr << "DH_Postgresql::StoreInDatabase () called." << endl;

  //  Stopwatch StorageTime;

  // First create blob:
  int i;
  ostringstream ostr;

  char hexrep [40];
  for (i = 0; i < size; i ++) {  
    sprintf (hexrep, "%02X ", (unsigned char) (buf [i]));
    ostr << hexrep;
  }

  // Create a postgres byte string
  ostringstream ByteString;

#if defined (__GNUC__) && (__GNUC__) > 2
    size_t len;
    ByteString << "'" 
  	       << PQescapeBytea ((unsigned char *)
  		    buf, size, &len)
  	       << "'::bytea";
    // TODO check len with MAX_BYTEA_SIZE
    // TODO Maybe must free buffer returned from fuction.
#else
    ByteString << "'byteaNotSupported'::bytea";
#endif

  // Create inseertion command
  ostringstream q;
  /* ORIGINAL QUERY:
  q << "INSERT INTO message VALUES ("
    << 123 << ", "
    << getMessageTag () << ", "
    << wrseqno << ", "
    << "'Pending', "  
    << getByteStringLength () << ", "
    << "'" << getTimeStamp ()<< "', "
    << "'" << getType () << "', "
    << "'" << getName () << "', "
    << "'" << ostr.str() << "', "
    << ByteString.str () << ");";
  */
  q << "INSERT INTO message VALUES ("
    << 123 << ", "
    << tag << ", "
    << itsWriteSeqNo << ", "
    << "'noStatus', "  
    << size << ", "
    << "'" << "someTime" << "', "
    << "'" << getType () << "', "
    << "'" << getName () << "', "
    << "'" << ostr.str() << "', "
    << ByteString.str () << ");";

  //  Stopwatch InsertTime;
  ExecuteSQLCommand (q);
  //  dtInsert += InsertTime.delta ().real ();

  //  dtStorage += StorageTime.delta ().real ();

  //  nStore ++;
  //  nStoredBytes += getByteStringLength ();

  itsWriteSeqNo ++;

  return true; 
}


bool DH_Postgresql::RetrieveFromDatabase (int,int tag, char * buf, int size) { 
  //  cerr << "DH_Postgresql::RetrieveFromDatabase () called." << endl;

  int i;
  i = 0;

  //  Stopwatch RetrieveTime;

  // Construct query
  ostringstream q;

  q << "SELECT * FROM message WHERE "
    << "appid = " << 123 << " AND "
    << "status = 'noStatus' AND "
    << "tag = " << tag << " AND "
    << "seqno = " << itsReadSeqNo << " AND "
    << "appid = " << 123 << ";";

  //  cout << "<<<QUERY: " << q.str () << endl;

  PGresult * res;

  // Block until a packet appears in the table
  do {
    cerr << '.';
    //    Stopwatch QueryTime;
    res = PQexec (DH_Postgresql::theirConnection, (q.str ()).c_str ());
    //    dtQuery += QueryTime.delta ().real ();
  
    AssertStr (PQresultStatus (res) == PGRES_TUPLES_OK,
	       "DH_Postgressql::Retrieve (); Select query failed.")
      // TODO: Do a PQclear here?
  } while (PQntuples (res) == 0);

  int nRows;
  nRows = PQntuples (res);
  AssertStr (nRows == 1, "DH_Postgresql::Retrieve ();"
    << "ERROR: Found less or more than 1 message in database.")
  
    // TODO: What to do with these?:
  // Read the found tuple
    //  setMessageTag       (atoi (PQgetvalue (res, 0, 1)));
    //  setByteStringLength (atoi (PQgetvalue (res, 0, 4)));
    //  setTimeStamp        (atoi (PQgetvalue (res, 0, 5)));
    //  setType             (PQgetvalue (res, 0, 6));
    //  setName             (PQgetvalue (res, 0, 7));

  // Copy the data packet

#if defined (__GNUC__) && (__GNUC__) > 2
  unsigned char * ByteString;
  size_t size;
  ByteString = PQunescapeBytea 
    ((unsigned char*)PQgetvalue (res, 0, 9), & size);
  
  memcpy (buf, ByteString, size);
  // TODO: Must free ByteString here?
#else
  unsigned int k;
  char token[40];
  istringstream istr (PQgetvalue (res, 0, 8));
  for (k = 0; k < (unsigned int) size; k ++) {
    istr >> token;
    buf [k] = (char) strtoul (token, NULL, 16);
  }
#endif
    
  PQclear (res);

//  dtRetrieve += RetrieveTime.delta ().real ();

//  nRetrieve ++;
//  nRetrievedBytes += getByteStringLength ();

  itsReadSeqNo ++;
  return true;
}

bool DH_Postgresql::ExecuteSQLCommand (char * str) {
  ostringstream ostr;
  ostr << str;
  return (ExecuteSQLCommand (ostr));
}


bool DH_Postgresql::ExecuteSQLCommand (ostringstream & q) {
  PGresult * res;

  //  cerr << ">>>QUERY: " << q.str () << endl;

  res = PQexec (DH_Postgresql::theirConnection, ((q.str ()).c_str ()));

  AssertStr (PQresultStatus (res) == PGRES_COMMAND_OK,
    "ERROR: ExecuteCommand () Failed (" 
    << PQresStatus (PQresultStatus (res)) 
    << "): " << q.str ())

  PQclear (res);

  return true;
}

// Internal Functions
/*
bool LogEntry (char *);
bool LogEntry (ostringstream & q);

bool ExecuteCommand (char * str);
bool ExecuteCommand (ostringstream & q);


PGconn * pgconn;

long nStore;
long nStoredBytes;
double dtStorage;
double dtInsert;

long nRetrieve;
long nRetrievedBytes;
double dtRetrieve;
double dtQuery;
double dtUpdate;
*/


/*
bool LogEntry (char * str) {
  ostringstream ostr;
  time_t t = time (NULL);

  ostr << "INSERT INTO log VALUES("
       << "'" << ctime (& t) << "', "
       << "'" << str << "');";

  return ExecuteCommand (ostr);
}

bool LogEntry (ostringstream & q) {
  ostringstream ostr;
  time_t t = time (NULL);

  ostr << "INSERT INTO log VALUES("
       << "'" << ctime (& t) << "', "
       << "'" << q.str () << "');";

  return ExecuteCommand (ostr);
}



*/

}
