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

#include <DH_PL.h>				// for class definition
#include <Common/lofar_iostream.h>		// for cout, cerr
#include <Common/Debug.h>			// for AssertStr
#include <sstream>				// for ostrstream

#include "DH_PL_MessageRecord.h"		// class MessageRecord
#include "PO_DH_PL_MessageRecord.h"		// TPO of MessageRecord

using namespace std;

namespace LOFAR {

// Number of DH_PL objects currently instantiated. Used internally for
// managing the database connection.

ulong DH_PL::theirInstanceCount = 0L;


// Default data source and account names for PL. You should override them
// with a call to UseDatabase () in main ().

string DH_PL::theirDSN = "tanaka";
string DH_PL::theirUserName = "postgres";


DH_PL::DH_PL (const string& name, const string& type)
  : DH_Database (name, type) {

  // If this is the first instance of DH_PL in the (current) process,
  // connect to the database.

  if (DH_PL::theirInstanceCount == 0L) {
    ConnectDatabase ();
  }

  DH_PL::theirInstanceCount ++;



  // Fill its members with correct values.
  (DH_PL::DataPacket*)itsDataPacketPtr->AppId = 1234;
  (DH_PL::DataPacket*)itsDataPacketPtr->Tag = tag;
  (DH_PL::DataPacket*)itsDataPacketPtr->SeqNo = itsWriteSeqNo;
  (DH_PL::DataPacket*)itsDataPacketPtr->Status = "Sent";
  (DH_PL::DataPacket*)itsDataPacketPtr->Size = size;
  (DH_PL::DataPacket*)itsDataPacketPtr->TimeStamp = "00:00";
  (DH_PL::DataPacket*)itsDataPacketPtr->Type = getType ();
  (DH_PL::DataPacket*)itsDataPacketPtr->Type = getName ();
  ostringstream ostr;
  (DH_PL::DataPacket*)itsDataPacketPtr->Blob = ostr.str ();

  itsPO = new TPersistentObject <DH_PL::DataPacket> (itsMR);

} 


DH_PL::~DH_PL () {
  DH_PL::theirInstanceCount --;

  // If this was the last instance of DH_PL, disconnect from the database. 

  if (DH_PL::theirInstanceCount == 0L) {
    DisconnectDatabase ();
  }
}


// Static function, to be called at least once from main () to specifity
// the connection string and database account. If not called, default
// values are used.

void DH_PL::UseDatabase 
  (char * dbDSN, char * userName) {
  DH_PL::theirDSN   = dbDSN;
  DH_PL::theirUserName = userName;
}
 


// Store message content in database table.

bool DH_PL::StoreInDatabase (int, int tag, char * buf, int size) {
  // First create blob:
  int i;
  ostringstream ostr;

  char hexrep [40];
  for (i = 0; i < size; i ++) {  
    sprintf (hexrep, "%02X ", (unsigned char) (buf [i]));
    ostr << hexrep;
  }

  // Store the DH_PL_MessageRecord content.
  theirPersistenceBroker.save (itsPO);

  itsWriteSeqNo ++;

  return true; 
}


// Retrieve message content from table.

bool DH_PL::RetrieveFromDatabase (int,int tag, char * buf, int size) { 
  // Now create a DH_PL_MessageRecord object. This object is a simple
  // structure with members for storing all of the information needed for
  // a message.
  DH_PL_MessageRecord mr;

  // Create the PersistentObject class 'around' DH_PL_MessageRecord. The
  // PersistentObject is a wrapper which knows how to retrieve a
  // previously stored DH_PL_MessageRecord from a (PL supported) database.
  TPersistentObject <DH_PL_MessageRecord> po_mr (mr);

  // PL is based on query objects to identify records in database
  // tables. A query object is now prepared to retrieve the record with
  // the correct tag and sequence number.
  char q [200]; 
  sprintf (q, "WHERE tag='%d' AND SeqNo='%ld'",
    tag, itsReadSeqNo);
  Query qGetRecord (q);

  // The results from a query is returned as a collection of
  // DH_PL_MessageRecord. So we need an object for that purpose:
  Collection<TPersistentObject<DH_PL_MessageRecord> > Results;

  // Perform the actual query.
  Results = theirPersistenceBroker.retrieve<DH_PL_MessageRecord> (qGetRecord);

  // Take the size of the first DH_PL_MessageRecord in the collection.
  int resSize = Results.begin () -> data().Size;

  // Check its size.
  if (resSize != size) {
    cerr << "DH_PL Warning: Not matching size, continuing anyway." << endl;
  }

  // Unpack the blob
  unsigned int k;
  char token[40];
  istringstream istr (Results.begin () -> data().Blob);
  for (k = 0; k < (unsigned int) size; k ++) {
    istr >> token;
    buf [k] = (char) strtoul (token, NULL, 16);
  }

  itsReadSeqNo ++;

  return true;
}

}


