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

#include <PO_DH_Database.h>
#include <Common/lofar_iostream.h>

#include <libpq-fe.h>

#include <sstream>
using namespace std;

PGconn * pgconn;
int pgSeqNo;

bool ConnectDatabase (void) {
  pgSeqNo = 0;
  pgconn = PQconnectdb ("dbname=testdb01");
  if (PQstatus (pgconn) != CONNECTION_OK) {
    cout << "CEPFrame::PO_DH_Database.cc::ConnectDatabase ():" << endl;
    cout << "Failed to connect to postgress database." << endl;
    cout << PQerrorMessage (pgconn);
    return false;
  }
  return true;
}

// ==========================================
// [>] Database implementation
/*
#define TH_ARRAY_SIZE       (512)

typedef enum { msgPending, msgDiscarded } msgStatus;

int       NrOfMessages;

msgStatus     MessageStatus     [TH_ARRAY_SIZE];
int           MessageTag        [TH_ARRAY_SIZE];
int           ReservedData1     [TH_ARRAY_SIZE];
int	      ReservedData2     [TH_ARRAY_SIZE];
int	      ReservedData3     [TH_ARRAY_SIZE];
int	      ReservedData4     [TH_ARRAY_SIZE];
int           MessageSize       [TH_ARRAY_SIZE];
//void *        MessageContent    [TH_ARRAY_SIZE];
unsigned long TimeStamp         [TH_ARRAY_SIZE];
string        MessageType       [TH_ARRAY_SIZE];
string        MessageName       [TH_ARRAY_SIZE];
string        HumanReadableForm [TH_ARRAY_SIZE];
string        PseudoBlob        [TH_ARRAY_SIZE];
*/
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

bool PO_DH_Database::Store () {
  // First create blob:
  int i;
  ostringstream ostr;

  cout << "{S}";

  for (i = 0; i < getByteStringLength (); i ++) {  
    ostr << (int) (getByteString ()) [i] << ' ';
  }
  ostr << "-1";

  // Create inseertion command
  ostringstream q;

  q << "INSERT INTO message VALUES ("
    << 123 << ", "
    << getMessageTag () << ", "
    << pgSeqNo << ", "
    << "'Pending', "  
    << getByteStringLength () << ", "
    << "'" << getTimeStamp ()<< "', "
    << "'" << getType () << "', "
    << "'" << getName () << "', "
    << "'" << ostr.str() << "');";

  pgSeqNo ++;

  PGresult * res;
  string query = q.str ();
  res = PQexec (pgconn, (q.str ()).c_str());

  return true;
  /*
  if (NrOfMessages == TH_ARRAY_SIZE - 1) {
    cout << "Fatal error in TH_Database::send (). Maximum number of message transfers reached." << endl;
    return false;
  }

  MessageStatus  [NrOfMessages] = msgPending;
  MessageTag     [NrOfMessages] = getMessageTag ();
  ReservedData1  [NrOfMessages] = getReservedData1 ();
  ReservedData2  [NrOfMessages] = getReservedData2 ();
  ReservedData3  [NrOfMessages] = getReservedData3 ();
  ReservedData4  [NrOfMessages] = getReservedData4 ();
  MessageSize    [NrOfMessages] = getByteStringLength ();

  //  MessageContent [NrOfMessages] = malloc (getByteStringLength ());

  // if (MessageContent [NrOfMessages] == 0) {
  //   cout << "Fatal error in TH_Database::send (). Could not allocate " 
  //   << getByteStringLength () << " memory." << endl;
  //   return false;
  // }

  TimeStamp   [NrOfMessages] = getTimeStamp ();
  MessageType [NrOfMessages] = getType ();
  MessageName [NrOfMessages] = getName ();

  PseudoBlob [NrOfMessages] = ostr.str ();

  NrOfMessages ++;

  return true;
  */
}


bool PO_DH_Database::Retrieve () {
  int i;
  i = 0;

  cout << "{R}" << endl;;

  // Construct query
  ostringstream q;

  q << "SELECT * FROM message WHERE "
    << "appid = " << 123 << " AND "
    << "tag = " << getMessageTag () << ";";

  //  cout << q.str() << endl;

  // Do the query
  PGresult * res;

  res = PQexec (pgconn, (q.str ()).c_str ());
  if (PQresultStatus (res) != PGRES_TUPLES_OK) {
    cout << "Query failed." << endl;
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

  int j, idx=0;
  istringstream istr (PQgetvalue (res, 0, 8));

  istr >> j;
  while (j != -1) {
    * (getByteString () + idx) = (char) j;
 idx ++;
    istr >> j;
  }

  // Construct the deletion query
  ostringstream qq;
  qq << "DELETE FROM message WHERE "
     << "appid = " << 123 << " AND "
     << "tag = " << getMessageTag () << ";";

  //  cout << qq.str() << endl;

  // What happens to the old PQres? Do I have to free it?

  // Do the query
  res = PQexec (pgconn, (qq.str ()).c_str ());
  //if (PQresultStatus (res) != PGRES_TUPLES_OK) {
  /*
  if (PQcmdTuples (res) != 1) {
    cout << "Deletion query failed." << endl;
    return false;
  }
  */
  //  cout << "Deletion result: " << PQcmdStatus (res) << endl;
  
  return true;
  /*
  // Search for the record with the correct tag.
  while (! (MessageTag [i] == getMessageTag () 
    && MessageStatus [i] == msgPending)) {
    i ++;
    if (i == NrOfMessages) {
      cout << "Fatal error in TH_Database::recv (). "
	   << "No message has been sent which can be received." 
	   << endl;
      return false;
    }
  }

  setMessageTag       (MessageTag     [i]);
  setReservedData1    (ReservedData1  [i]);
  setReservedData2    (ReservedData2  [i]);
  setReservedData3    (ReservedData3  [i]);
  setReservedData4    (ReservedData4  [i]);
  setByteStringLength (MessageSize    [i]);

  setTimeStamp (TimeStamp   [i]);
  setType      (MessageType [i]);
  setName      (MessageName [i]);

  MessageStatus [i] = msgDiscarded;

  cout << "PseudoBlob(Receive): " << PseudoBlob [i] << endl;

//  int j, idx=0;
//  istringstream istr (PseudoBlob[i]);
//
//  istr >> j;
//  while (j != -1) {
//    * (getByteString () + idx) = (char) j;
//    idx ++;
//    istr >> j;
//  }
//
//  NrOfMessages ++;

  return true;
  */
}



