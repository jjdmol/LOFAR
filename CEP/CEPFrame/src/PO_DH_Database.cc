
//  PO_DH_Database.cc: Database persistent DH_Database
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

#include <PO_DH_Database.h>			// for class definition
#include <Common/lofar_iostream.h>		// for cout
#include <Common/Stopwatch.h>			// for Stopwatch class

#include <libpq-fe.h>				// for direct postgres xs

#include <sstream>				// for ostrstream
#include <sys/time.h>				// for ctime (), time ()
#include <unistd.h>				// for gethostname ()

using namespace std;

// Internal Functions

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


bool ConnectDatabase (char * hostname) {

  cout << "ConnectDatabase (); Connecting to databse."
       << endl;

  ostringstream ConnInfo;

  ConnInfo << "host=" << hostname << " dbname=TransportHolder";

  nStore = 0;
  nStoredBytes = 0;
  dtStorage = 0;
  dtInsert = 0;
 
  nRetrieve = 0;
  nRetrievedBytes = 0;
  dtQuery = 0;
  dtUpdate = 0;

  pgconn = PQconnectdb (ConnInfo.str (). c_str ());
  if (PQstatus (pgconn) != CONNECTION_OK) {
    cout << "CEPFrame::PO_DH_Database.cc::ConnectDatabase ():" << endl;
    cout << "Failed to connect to postgress database." << endl;
    cout << PQerrorMessage (pgconn);
    return false;
  }

  // Delete all previous message from message table
  ExecuteCommand ("DROP TABLE message;");
  // TODO: error recovery

  ExecuteCommand ("CREATE TABLE message (AppId int, Tag int, SeqNo int, Status text, Size int, TimeStamp text, Type text, Name text, Blob text, ByteString bytea);");

  // log table
  ExecuteCommand ("DROP TABLE log;");
  // foutafhandeling

  ExecuteCommand ("CREATE TABLE log (Time text, Entry text);");
  // foutafhandeling

  ostringstream ostr;
  char curhostname [80];
  gethostname (curhostname, 80);
  ostr << "Connected from " << curhostname << " to " << hostname;
  LogEntry (ostr);

  LogEntry ("  (Dropped message table).");
  LogEntry ("  (Re-created message table.)");
  LogEntry ("  (Dropped log table.)");
  LogEntry ("  (Re-created log table.)");

  return true;
}


bool DisconnectDatabase (void) {
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
  cout << "Average store time: " << avgStore 
       << " sec." << endl;
  cout << "Average insert time: " << avgInsert
       << " sec. (" << (avgInsert / avgStore) * 100 
       << " %)" << endl;

  cout << "Average storage throughput: "
       << avgStorageThP << " bytes/sec" << endl;;

  cout << "Average retrieve time: " << avgRetrieve
       << " sec." << endl;
  cout << "Average query time: " << avgQuery
       << " sec. (" << (avgQuery / avgRetrieve) * 100
       << " %)" << endl;
  cout << "Average update time: " << avgUpdate 
       << " sec. (" << (avgUpdate / avgRetrieve) * 100 
       << " %)" << endl;

  cout << "Average retrieval throughput: "
       << avgRetrievalThP << " bytes/sec" << endl;

  LogEntry ("Disconnecting.");

  // Disconnect from database.
  cout << "Disconnecting from database." << endl;
  PQfinish (pgconn);

  return true;
}


// ==========================================
// [>] DatabaseRecord

void DatabaseRecord::CopyFromByteString (char * dest, int size) {
  memcpy (dest, itsByteString, size);
}


void DatabaseRecord::CopyToByteString (char * src, int size) {
  memcpy (itsByteString, src, size);
}


// ==========================================
// [>] PO_DH_Database

#define MAX_BYTEA_SIZE (5*4096)

bool PO_DH_Database::Store (unsigned long wrseqno) {
  Stopwatch StorageTime;

  // First create blob:
  int i;
  ostringstream ostr;

  cout << "{S}";

  char hexrep [4];
  for (i = 0; i < getByteStringLength (); i ++) {  
    sprintf (hexrep, "%02X ", (int) (getByteString ()) [i]);
    ostr << hexrep;
  }

  // Create a postgres byte string
  ostringstream ByteString;
  size_t len;

  ByteString << "'" 
	     << PQescapeBytea ((unsigned char *)
		  getByteString (), getByteStringLength (), &len)
	     << "'::bytea";
  // TODO check len with MAX_BYTEA_SIZE
  // TODO Maybe must free bugfer returned from fuction.

  // Create inseertion command
  ostringstream q;

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

  Stopwatch InsertTime;
  ExecuteCommand (q);
  dtInsert += InsertTime.delta ().real ();

  dtStorage += StorageTime.delta ().real ();

  nStore ++;
  nStoredBytes += getByteStringLength ();

  return true;
}


bool PO_DH_Database::Retrieve (unsigned long rdseqno) {

  int i;
  i = 0;
  Stopwatch RetrieveTime;

  cout << "{R}";

  cout << "Read seq no: " << rdseqno << endl;

  // Construct query
  ostringstream q;

  /*
  q << "SELECT * FROM message WHERE "
    << "appid = " << 123 << " AND "
    << "status = 'Pending' AND "
    << "tag = " << getMessageTag () << " AND "
    << "seqno = (SELECT min (seqno) FROM message WHERE "
    << "appid = " << 123 << " AND "
      << "status = 'Pending' AND "
      << "tag = " << getMessageTag () << ")"
    << ";";
  */
  q << "SELECT * FROM message WHERE "
    << "appid = " << 123 << " AND "
    << "status = 'Pending' AND "
    << "tag = " << getMessageTag () << " AND "
    << "seqno = " << rdseqno << " AND "
    << "appid = " << 123 << ";";

  PGresult * res;

  Stopwatch QueryTime;
  res = PQexec (pgconn, (q.str ()).c_str ());
  dtQuery += QueryTime.delta ().real ();

  if (PQresultStatus (res) != PGRES_TUPLES_OK) {
    cout << "ERROR: Select query failed." << endl;
    return false;
  }

  int nRows;
  nRows = PQntuples (res);
  if (nRows != 1) {
    cout << "ERROR: Found less or more than 1 message in database (" 
	 << nRows << ")." << endl;
    return false;
  }
  
  // Read the found tuple
  setMessageTag       (atoi (PQgetvalue (res, 0, 1)));
  setByteStringLength (atoi (PQgetvalue (res, 0, 4)));
  setTimeStamp        (atoi (PQgetvalue (res, 0, 5)));
  setType             (PQgetvalue (res, 0, 6));
  setName             (PQgetvalue (res, 0, 7));

  unsigned char * ByteString;
  size_t size;
  ByteString = PQunescapeBytea 
    ((unsigned char*)PQgetvalue (res, 0, 9), & size);
  
  memcpy (getByteString (), ByteString, size);

  PQclear (res);

  // Construct the deletion query
  /*
  ostringstream qq;
  qq << "UPDATE message SET status = 'Received' WHERE "
     << "appid = " << 123 << " AND "
     << "status = 'Pending' AND "
     << "tag = " << getMessageTag () << " AND "
     << "seqno = (SELECT min (seqno) FROM message WHERE "
     << "appid = " << 123 << " AND "
       << "status = 'Pending' AND "
       << "tag = " << getMessageTag () << ")"
     << ";";
  */
  // Do the query
  Stopwatch UpdateTime;
  //ExecuteCommand (qq);
  dtUpdate += UpdateTime.delta ().real ();

  dtRetrieve += RetrieveTime.delta ().real ();

  nRetrieve ++;
  nRetrievedBytes += getByteStringLength ();

  return true;
}


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



bool ExecuteCommand (char * str) {
  ostringstream ostr;
  ostr << str;
  return (ExecuteCommand (ostr));
}


bool ExecuteCommand (ostringstream & q) {
  PGresult * res;

  res = PQexec (pgconn, ((q.str ()).c_str ()));

  // TODO: Free res;

  if (PQresultStatus (res) != PGRES_COMMAND_OK) {
    cout << "ERROR: ExecuteCommand () Failed (" 
	 << PQresStatus (PQresultStatus (res)) 
	 << "): " << q.str () << endl;
    return false;
  }

  PQclear (res);

  return true;
}


