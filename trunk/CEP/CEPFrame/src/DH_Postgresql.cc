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
#include <Common/lofar_iostream.h>		// for cout, cerr
#include <Common/Stopwatch.h>			// for Stopwatch class
#include <Common/Debug.h>			// for AssertStr


#include <sstream>				// for ostrstream
#include <sys/time.h>				// for ctime (), time ()
#include <unistd.h>				// for gethostname ()
#include <stdlib.h>				// for strtoul

using namespace std;

namespace LOFAR{


ulong    DH_Postgresql::theirInstanceCount = 0L;
PGconn * DH_Postgresql::theirConnection = NULL;

string DH_Postgresql::theirDBHost = "10.87.2.49";
string DH_Postgresql::theirDBName = "TransportHolder";
string DH_Postgresql::theirUserName = "tanaka";



DH_Postgresql::DH_Postgresql (const string& name, const string& type)
  : DH_Database (name, type) {

  if (DH_Postgresql::theirInstanceCount == 0L) {
    ConnectDatabase ();
  }

  DH_Postgresql::theirInstanceCount ++;

  //  cerr << DH_Postgresql::theirInstanceCount 
  //    << ") DH_Postgresql constructed." << endl;
} 


DH_Postgresql::~DH_Postgresql () {
  //  cerr << DH_Postgresql::theirInstanceCount
  //    << ") DH_Postgresql destroyed." << endl;

  DH_Postgresql::theirInstanceCount --;

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
  cerr << "  using: " << ConnInfo.str () << endl;

  DH_Postgresql::theirConnection = PQconnectdb (ConnInfo.str (). c_str ());

  AssertStr (PQstatus (DH_Postgresql::theirConnection) == CONNECTION_OK, 
    "ConnectDatabase(); Could not connect to database.") 

  cerr << "DH_Postgresql::ConnectDatabase (): Succesfully connected "
       << "to database" << endl;
}


void DH_Postgresql::DisconnectDatabase (void) {
  PQfinish (theirConnection);
  cerr << "DH_Postgresql::Disconnect(); Disconnected from database." << endl;
}

#define MAX_BYTEA_SIZE (5*4096)

bool DH_Postgresql::StoreInDatabase (int, int tag, char * buf, int size) {
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

  itsWriteSeqNo ++;

  return true; 
}


bool DH_Postgresql::RetrieveFromDatabase (int,int tag, char * buf, int size) { 

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

  // Block until a packet appears in the table
  do {
    res = PQexec (DH_Postgresql::theirConnection, (q.str ()).c_str ());
  
    AssertStr (PQresultStatus (res) == PGRES_TUPLES_OK,
	       "DH_Postgressql::Retrieve (); Select query failed.")
      // TODO: Do a PQclear here?
  } while (PQntuples (res) == 0);

  int nRows;
  nRows = PQntuples (res);
  AssertStr (nRows == 1, "DH_Postgresql::Retrieve ();"
    << "ERROR: Message table may not have been cleaned up before starting program.")
  
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
  size_t sizeRes;
  ByteString = PQunescapeBytea 
    ((unsigned char*)PQgetvalue (res, 0, 9), & sizeRes);
  
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

  res = PQexec (DH_Postgresql::theirConnection, ((q.str ()).c_str ()));

  AssertStr (PQresultStatus (res) == PGRES_COMMAND_OK,
    "ERROR: ExecuteCommand () Failed (" 
    << PQresStatus (PQresultStatus (res)) 
    << "): " << q.str ())

  PQclear (res);

  return true;
}

}


