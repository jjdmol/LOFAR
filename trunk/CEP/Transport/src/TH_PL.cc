//  TH_PL.cc: Database TransportHolder Implementation
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


#include <TH_PL.h>
#include <BaseTransport.h>
#include <Transportable.h>
#include <Common/lofar_iostream.h>
#include <PL/Query.h>

namespace LOFAR
{

// Default data source and account names for PL. You should override them
// with a call to UseDatabase () in main ().
string TH_PL::theirDSN = "tanaka";
string TH_PL::theirUserName = "postgres";

int TH_PL::theirInstanceCount=0;   
   
// The PersistenceBroker is a management object from PL. One static
// PersistenceBroker is used to optimally use resources.
PersistenceBroker TH_PL::theirPersistenceBroker;


TH_PL TH_PL::proto;

TH_PL::TH_PL() :
   itsWriteSeqNo(0),
   itsReadSeqNo(0)
{
  // If this is the first instance of TH_PL in the (current) process,
  // connect to the database.
  if (TH_PL::theirInstanceCount == 0L) {
    ConnectDatabase ();
  }
  TH_PL::theirInstanceCount ++;
}

TH_PL::~TH_PL() {
   TH_PL::theirInstanceCount --;

  // If this was the last instance of DH_PL, disconnect from the database. 

  if (TH_PL::theirInstanceCount == 0L) {
    DisconnectDatabase ();
  }
}

TH_PL* TH_PL::make() const
  { return new TH_PL(); }

void TH_PL::UseDatabase (char * dbDSN, char * userName) {
  TH_PL::theirDSN   = dbDSN;
  TH_PL::theirUserName = userName;
}

void TH_PL::ConnectDatabase (void) {
  if (TH_PL::theirDSN == "tanaka") {
    cerr << "***WARNING***: TH_PL::ConnectDatabase (); TH_PL "
	 << "is trying to connect to a test database residing on "
	 << "dop50. You probably have forgotten to call the TH_PL "
	 << "method UseDatabase (). See the comments in TH_PL.h " 
	 << "for more detaills. Continuing execution... " << endl;
  }

  theirPersistenceBroker.connect (TH_PL::theirDSN, TH_PL::theirUserName);
}


void TH_PL::DisconnectDatabase (void) {
}

string TH_PL::getType() const
  { return "TH_PL"; }

bool TH_PL::connectionPossible(int srcRank, int dstRank) const
  { return srcRank == dstRank; }

bool TH_PL::send(void* buf, int nbytes, int, int tag)
{
  if (getTransporter() -> getBaseDataHolder() != 0) {
        
    DH_PL* DHPLptr = (DH_PL*)getTransporter () -> getBaseDataHolder ();
    // ToDo: DbgAssertStr(DHPLptr->getType = "", "Wrong DH type;")

    //ToDo:  DHPLptr->setSeqNo(itsWriteSeqNo ++);
    theirPersistenceBroker.save(DHPLptr -> getPO());

  } else {
    cerr << "TH_PL::send():Transportable not found." << endl;
    return false;
  }

  return true;
}

bool TH_PL::recv(void* buf, int nbytes, int, int tag)
{ 
  DbgAssertStr( (getTransporter() -> getBaseDataHolder () != 0),
		"TH_PL::recv():Transportable not found.");
  
  // get a refernece to the DHPL object
  DH_PL* DHPLptr = (DH_PL*)getTransporter() -> getBaseDataHolder();
  
 
  // PL is based on query objects to identify records in database
  // tables. A query object is now prepared to retrieve the record with
  // the correct tag and sequence number.
  char q [200]; 
  sprintf (q, "WHERE tag='%d' AND SeqNo='%ld'",
	   tag, 
	   itsReadSeqNo);
   PL::QueryObject qGetRecord(q);
    
  // Perform the actual query and obtain the data collection.
   Results = theirPersistenceBroker.retrieve<DH_PL::DataPacket> (qGetRecord);
  
  // ToDo: DbgAssertStr(itsReadSeqnNo == ... ,"");
  itsReadSeqNo ++;

  // Take the size of the first TH_PL_MessageRecord in the collection.
  int resSize = Results.begin () -> data().Size;
  
  // Check its size.
  DbgAssertStr((resSize == size),
	       "TH_PL Warning: Not matching size, continuing anyway.");
  
  // Unpack the blob
  unsigned int k;
  char token[40];
  istringstream istr (Results.begin () -> data().Blob);
  for (k = 0; k < (unsigned int) size; k ++) {
    istr >> token;
    buf [k] = (char) strtoul (token, NULL, 16);
  }
  return true;
}

void TH_PL::waitForBroadCast () {}
void TH_PL::waitForBroadCast (unsigned long&) {}
void TH_PL::sendBroadCast (unsigned long) {}
int  TH_PL::getCurrentRank () { return -1; }
int  TH_PL::getNumberOfNodes () { return 1; }
void TH_PL::init (int, const char * []) {}
void TH_PL::finalize () {}
void TH_PL::synchroniseAllProcesses () {}

}

