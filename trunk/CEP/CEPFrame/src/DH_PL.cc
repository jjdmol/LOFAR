//  DH_PL.cc: Database persistent DH_Database
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

#include <DH_PL.h>			// for class definition
#include <Common/lofar_iostream.h>		// for cout, cerr
#include <Common/Debug.h>			// for AssertStr

#include <sstream>				// for ostrstream

#include "DH_PL_MessageRecord.h"
#include "PO_DH_PL_MessageRecord.h"

using namespace std;

namespace LOFAR{


ulong DH_PL::theirInstanceCount = 0L;
PersistenceBroker DH_PL::theirPersistenceBroker;

string DH_PL::theirDSN = "tanaka";
string DH_PL::theirUserName = "postgres";


DH_PL::DH_PL (const string& name, const string& type)
  : DH_Database (name, type) {

  if (DH_PL::theirInstanceCount == 0L) {
    ConnectDatabase ();
  }

  DH_PL::theirInstanceCount ++;

  cerr << DH_PL::theirInstanceCount 
      << ") DH_PL constructed." << endl;
} 


DH_PL::~DH_PL () {
  cerr << DH_PL::theirInstanceCount
       << ") DH_PL destroyed." << endl;

  DH_PL::theirInstanceCount --;

  if (DH_PL::theirInstanceCount == 0L) {
    DisconnectDatabase ();
  }
}

void DH_PL::UseDatabase 
  (char * dbDSN, char * userName) {
  DH_PL::theirDSN   = dbDSN;
  DH_PL::theirUserName = userName;

  cerr << "DH_PL::UseaDatabase () Using " << dbDSN << ", "
       << ", " << userName << endl;
}


void DH_PL::ConnectDatabase (void) {
  if (DH_PL::theirDSN == "tanaka") {
    cerr << "***WARNING***: DH_PL::ConnectDatabase (); DH_PL "
	 << "is trying to connect to a test database residing on "
	 << "dop50. You probably have forgotten to call the DH_PL "
	 << "method UseDatabase (). See the comments in DH_PL.h " 
	 << "for more detaills. Continuing execution... " << endl;
  }

  theirPersistenceBroker.connect (DH_PL::theirDSN, DH_PL::theirUserName);

  cerr << "DH_PL::ConnectDatabase (): Succesfully connected "
       << "to database" << endl;
}


void DH_PL::DisconnectDatabase (void) {
  cerr << "DH_PL::Disconnect(); Disconnected from database." << endl;
}


bool DH_PL::StoreInDatabase (int, int tag, char * buf, int size) {
  cerr << "DH_PL::StoreInDatabase () called." << endl;

  // First create blob:
  int i;
  ostringstream ostr;

  char hexrep [40];
  for (i = 0; i < size; i ++) {  
    sprintf (hexrep, "%02X ", (unsigned char) (buf [i]));
    ostr << hexrep;
  }

  DH_PL_MessageRecord mr;

  mr.AppId = 1234;
  mr.Tag = tag;
  mr.SeqNo = itsWriteSeqNo;
  mr.Status = "Sent";
  mr.Size = size;
  mr.TimeStamp = "00:00";
  mr.Type = getType ();
  mr.Type = getName ();
  mr.Blob = ostr.str ();

  TPersistentObject <DH_PL_MessageRecord> po_mr (mr);

  theirPersistenceBroker.save (po_mr);

  itsWriteSeqNo ++;

  return true; 
}


bool DH_PL::RetrieveFromDatabase (int,int tag, char * buf, int size) { 
  cerr << "DH_PL::RetrieveFromDatabase () called." << endl;

  DH_PL_MessageRecord mr;

  TPersistentObject <DH_PL_MessageRecord> po_mr (mr);

  char q [200]; 
  sprintf (q, "WHERE tag='%d' AND SeqNo='%ld'",
    tag, itsReadSeqNo);

  Query qGetMarcel (q);

  Collection<TPersistentObject<DH_PL_MessageRecord> > Results;

  Results = theirPersistenceBroker.retrieve<DH_PL_MessageRecord> (qGetMarcel);

  if (Results.size () == 0) {
    cerr << "msgrec not found." << endl;
  }
  if (Results.size () == 1) {
    cerr << "Found msgrec." << endl;
  }
  if (Results.size () > 1) {
    cerr << "Found more than one msg rec." << endl;
  }

  int resSize = Results.begin () -> data().Size;

  if (resSize != size) {
    cerr << "Not matching size." << endl;
  }

  unsigned int k;
  char token[40];
  istringstream istr (Results.begin () -> data().Blob);
  for (k = 0; k < (unsigned int) size; k ++) {
    istr >> token;
    buf [k] = (char) strtoul (token, NULL, 16);
  }


  itsReadSeqNo ++;
  /*
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
    res = PQexec (DH_PL::theirConnection, (q.str ()).c_str ());
  
    AssertStr (PQresultStatus (res) == PGRES_TUPLES_OK,
	       "DH_Postgressql::Retrieve (); Select query failed.")
      // TODO: Do a PQclear here?
  } while (PQntuples (res) == 0);

  int nRows;
  nRows = PQntuples (res);
  AssertStr (nRows == 1, "DH_PL::Retrieve ();"
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
  */
  return true;
}

}


