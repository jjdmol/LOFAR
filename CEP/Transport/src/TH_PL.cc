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

namespace LOFAR
{

// The PersistenceBroker is a management object from PL. One static
// PersistenceBroker is used to optimally use resources.

PersistenceBroker TH_PL::theirPersistenceBroker;


TH_PL TH_PL::proto;

TH_PL::TH_Database() {}

TH_PL::~TH_Database() {}

TH_PL* TH_PL::make() const
  { return new TH_Database(); }

void TH_PL::UseDatabase (char * dbDSN, char * userName) {
  DH_PL::theirDSN   = dbDSN;
  DH_PL::theirUserName = userName;
}

void TH_PL::ConnectDatabase (void) {
  if (DH_PL::theirDSN == "tanaka") {
    cerr << "***WARNING***: DH_PL::ConnectDatabase (); DH_PL "
	 << "is trying to connect to a test database residing on "
	 << "dop50. You probably have forgotten to call the DH_PL "
	 << "method UseDatabase (). See the comments in DH_PL.h " 
	 << "for more detaills. Continuing execution... " << endl;
  }

  theirPersistenceBroker.connect (DH_PL::theirDSN, DH_PL::theirUserName);
}


void TH_PL::DisconnectDatabase (void) {
}

string TH_Database::getType() const
  { return "TH_Database"; }

bool TH_Database::connectionPossible(int srcRank, int dstRank) const
  { return srcRank == dstRank; }

bool TH_Database::recv(void* buf, int nbytes, int, int tag)
{ 
  DbgAssertStr( (getBaseTransport () -> getTransportable () != 0),
		"TH_Database::recv():Transportable not found.");
  
  // get a refernece to the DHPL object
  DH_PL* DHPLptr = getBaseTransport () -> getTransportable ();
  
  // get the massage record form the DH_PL
  DH_PL_MessageRecord *mr = DHPLptr->itsMR;
  
  // PL is based on query objects to identify records in database
  // tables. A query object is now prepared to retrieve the record with
  // the correct tag and sequence number.
  char q [200]; 
  sprintf (q, "WHERE tag='%d' AND SeqNo='%ld'",
	   tag, itsReadSeqNo);
  Query qGetRecord (q);
    
  // Perform the actual query and obtain the data collection.
  Results = theirPersistenceBroker.retrieve<DH_PL_MessageRecord> (qGetRecord);
  
  // ToDo: DbgAssertStr(itsReadSeqnNo == ... ,"");
  itsReadSeqNo ++;

  // Take the size of the first DH_PL_MessageRecord in the collection.
  int resSize = Results.begin () -> data().Size;
  
  // Check its size.
  DbgAssertStr((resSize == size),
	       "DH_PL Warning: Not matching size, continuing anyway.");
  
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
 

bool TH_Database::send(void* buf, int nbytes, int, int tag)
{
  if (getBaseTransport () -> getTransportable () != 0) {
        
    DH_PL* DHPLptr = getBaseTransport () -> getTransportable ();
    // ToDo: DbgAssertStr(DHPLptr->getType = "", "Wrong DH type;")

    DHPLptr->setSeqNo(itsWriteSeqNo ++);
    theirPersistenceBroker.save(DHPLptr -> getPO());

  } else {
    cerr << "TH_Database::send():Transportable not found." << endl;
    return false;
  }

  return true;
}


void TH_Database::waitForBroadCast () {}
void TH_Database::waitForBroadCast (unsigned long&) {}
void TH_Database::sendBroadCast (unsigned long) {}
int  TH_Database::getCurrentRank () { return -1; }
int  TH_Database::getNumberOfNodes () { return 1; }
void TH_Database::init (int, const char * []) {}
void TH_Database::finalize () {}
void TH_Database::synchroniseAllProcesses () {}

}

