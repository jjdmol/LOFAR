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

#include <sstream>				// for ostringstream
#include <sys/time.h>				// for ctime (), time ()
#include <unistd.h>				// for gethostname ()
#include <stdlib.h>				// for strtoul
#include <Common/lofar_iostream.h>		// for cout, cerr
#include <Common/Stopwatch.h>			// for Stopwatch class
#include <Common/Debug.h>			// for AssertStr
#include <DH_Postgresql.h>			// for class definition

using namespace std;

namespace LOFAR{

ulong    DH_Postgresql::theirInstanceCount = 0L;

#ifdef HAVE_PSQL
PGconn * DH_Postgresql::theirConnection = NULL;
#endif

string DH_Postgresql::theirDBHost = "10.87.2.49";
string DH_Postgresql::theirDBName = "TransportHolder";
string DH_Postgresql::theirUserName = "tanaka";


DH_Postgresql::DH_Postgresql (const string& name, const string& type)
  : DH_Database (name, type) {

  // If this is the first DH_Postgresql object being created in the current
  // process, establish a connection with the Postgresql database.
  if (DH_Postgresql::theirInstanceCount == 0L) {
    ConnectDatabase ();
  }

  DH_Postgresql::theirInstanceCount ++;
} 


DH_Postgresql::~DH_Postgresql () {
  DH_Postgresql::theirInstanceCount --;

  // If this is the last DH_Postgresql object being destroyed in the current
  // process, destroy the connection with the Postgresql database.
  if (DH_Postgresql::theirInstanceCount == 0L) {
    DisconnectDatabase ();
  }
}


void DH_Postgresql::UseDatabase 
  (char * dbHost, char * dbName, char * userName) {
  DH_Postgresql::theirDBHost   = dbHost;
  DH_Postgresql::theirDBName   = dbName;
  DH_Postgresql::theirUserName = userName;
}


void DH_Postgresql::ConnectDatabase (void) {
#ifdef HAVE_PSQL
  ostringstream ConnInfo;

  if (DH_Postgresql::theirDBHost == "10.87.2.49") {
    cerr << "***WARNING***: DH_Postgresql::ConnectDatabase (); DH_Postgresql "
	 << "is trying to connect to a test database residing on "
	 << "dop49. You probably have forgotten to call the DH_Postgresql "
	 << "method UseDatabase (). See the comments in DH_Postgresql.h " 
	 << "for more detaills. Continuing execution... " << endl;
  }

  ConnInfo << "host=" << DH_Postgresql::theirDBHost 
	   << " dbname=" << DH_Postgresql::theirDBName
  	   << " user="<< DH_Postgresql::theirUserName;

  DH_Postgresql::theirConnection = PQconnectdb (ConnInfo.str (). c_str ());

  AssertStr (PQstatus (DH_Postgresql::theirConnection) == CONNECTION_OK, 
    "ConnectDatabase(); Could not connect to database.") 
#else
  AssertStr (false, "PSQL is not configured in");
#endif
}


void DH_Postgresql::DisconnectDatabase (void) {
#ifdef HAVE_PSQL
  PQfinish (theirConnection);
#else
  AssertStr (false, "PSQL is not configured in");
#endif
}

// Maximum size of Postgresql blob, which is used for communication.
#define MAX_BYTEA_SIZE (5*4096)

bool DH_Postgresql::StoreInDatabase (int, int tag, char * buf, int size) {
#ifdef HAVE_PSQL
  // First create a string based blob and fill it with the content of buf:
  int i;
  ostringstream ostr;

  char hexrep [40];
  for (i = 0; i < size; i ++) {  
    sprintf (hexrep, "%02X ", (unsigned char) (buf [i]));
    ostr << hexrep;
  }

  // Now create a postgres blob and fill it with the content of the buf.
  ostringstream ByteString;

#if defined (__GNUC__) && (__GNUC__) > 2
    size_t len;
    ByteString << "'" 
  	       << PQescapeBytea ((unsigned char *)
  		    buf, size, &len)
  	       << "'::bytea";
#else
    ByteString << "'byteaNotSupported'::bytea";
#endif

  // Create insertion command
  ostringstream q;

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

  ExecuteSQLCommand (q);

  // Both the string based blob as the postgresql blob are stored in the
  // database due to incompatibility issues on lofar3. If the receiving
  // end is lofar3, the string based blob is used to read out the message
  // content. If the receiving end is not lofar3, the postgresql blob is
  // used to read out the message content.

  itsWriteSeqNo ++;

#else
  AssertStr (false, "PSQL is not configured in");
#endif

  return true; 
}


bool DH_Postgresql::RetrieveFromDatabase (int,int tag, char * buf, int size) { 

#ifdef HAVE_PSQL
  int i;
  i = 0;

  // Construct query
  ostringstream q;

  q << "SELECT * FROM message WHERE "
    << "appid = " << 123 << " AND "
    << "status = 'noStatus' AND "
    << "tag = " << tag << " AND "
    << "seqno = " << itsReadSeqNo << " AND "
    << "appid = " << 123 << ";";

  PGresult * res;

  // The following do-while loop implements the blocking behaviour of
  // DH_Postgresql. It waits until a records appears in the database with
  // the correct readSeqNo.
  do {
    res = PQexec (DH_Postgresql::theirConnection, (q.str ()).c_str ());
  
    AssertStr (PQresultStatus (res) == PGRES_TUPLES_OK,
	       "DH_Postgressql::Retrieve (); Select query failed.")
  } while (PQntuples (res) == 0);

  int nRows;
  nRows = PQntuples (res);
  AssertStr (nRows == 1, "DH_Postgresql::Retrieve ();"
    << "ERROR: Message table may not have been cleaned up before starting program.")
  
  // Communication of the following fields are (yet) optional:
  // setMessageTag       (atoi (PQgetvalue (res, 0, 1)));
  // setByteStringLength (atoi (PQgetvalue (res, 0, 4)));
  // setTimeStamp        (atoi (PQgetvalue (res, 0, 5)));
  // setType             (PQgetvalue (res, 0, 6));
  // setName             (PQgetvalue (res, 0, 7));

  // Copy the data packet

#if defined (__GNUC__) && (__GNUC__) > 2
  // If the receiving end is not lofar3, the postgresql blob is used to
  // read out the message content.

  unsigned char * ByteString;
  size_t sizeRes;
  ByteString = PQunescapeBytea 
    ((unsigned char*)PQgetvalue (res, 0, 9), & sizeRes);
  
  memcpy (buf, ByteString, size);
#else
  // If the receiving end is lofar3, the string based blob is used to read
  // out the message content. 

  unsigned int k;
  char token[40];
  istringstream istr (PQgetvalue (res, 0, 8));
  for (k = 0; k < (unsigned int) size; k ++) {
    istr >> token;
    buf [k] = (char) strtoul (token, NULL, 16);
  }
#endif
    
  PQclear (res);

  itsReadSeqNo ++;
#else
  AssertStr (false, "PSQL is not configured in");
#endif
  return true;
}

bool DH_Postgresql::ExecuteSQLCommand (char * str) {
  ostringstream ostr;
  ostr << str;
  return (ExecuteSQLCommand (ostr));
}


bool DH_Postgresql::ExecuteSQLCommand (ostringstream & q) {
#ifdef HAVE_PSQL
  PGresult * res;

  res = PQexec (DH_Postgresql::theirConnection, ((q.str ()).c_str ()));

  AssertStr (PQresultStatus (res) == PGRES_COMMAND_OK,
    "ERROR: ExecuteCommand () Failed (" 
    << PQresStatus (PQresultStatus (res)) 
    << "): " << q.str ())

  PQclear (res);

#else
  AssertStr (false, "PSQL is not configured in");
#endif

  return true;
}

}
