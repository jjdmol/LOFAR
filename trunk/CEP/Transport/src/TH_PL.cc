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


#include <libTransport/TH_PL.h>
#include <libTransport/Transporter.h>
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
PL::PersistenceBroker TH_PL::theirPersistenceBroker;


TH_PL TH_PL::proto;

TH_PL::TH_PL() :
   itsTableName("defaultTable"),
   itsWriteSeqNo(0),
   itsReadSeqNo(0)
   
{

  // ToDo: test dynamic cast
  // ToDo: getPO->setTableName();

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

bool TH_PL::sendBlocking(void* buf, int nbytes, int destination, int tag)

{
  DbgAssertStr(getTransporter() -> getBaseDataHolder() != 0, 
	       "TH_PL::send():Transportable not found.");
  DH_PL* DHPLptr = (DH_PL*)getTransporter () -> getBaseDataHolder ();
  // ToDo:   DHPLptr->setTag(tag);
  // ToDo:   DHPLptr->setDestination(destination);
  // ToDo: DbgAssertStr(DHPLptr->getType = "", "Wrong DH type;")
  
  PL::PersistentObject& aPO = DHPLptr -> preparePO(itsWriteSeqNo++);
  theirPersistenceBroker.save(aPO);
  return true;
}

bool TH_PL::recvBlocking(void* buf, int nbytes, int source , int tag)

{ 
  DbgAssertStr( (getTransporter() -> getBaseDataHolder () != 0),
		"TH_PL::recv():Transportable not found.");
 
  // get a refernece to the DHPL object
  DH_PL* DHPLptr = (DH_PL*)getTransporter() -> getBaseDataHolder();  
  PL::PersistentObject& aPO = DHPLptr -> getPO();

  // PL is based on query objects to identify records in database
  // tables. A query object is now prepared to retrieve the record with
  // the correct tag and sequence number.
  char q [200]; 
  sprintf (q, "WHERE tag='%d' AND SeqNo='%ld' AND source='%ld'",
	   tag, 
	   itsReadSeqNo++,
	   source);
   PL::QueryObject qGetRecord(q);
    
  // Perform the actual query and obtain the data collection.

   int result;
   // ToDo: result = aPO.retrieve(qGetRecord);
   DbgAssert (result == 1);

  // ToDo: DbgAssertStr(itsReadSeqnNo == ... ,"");

   // ToDo: DbgAssertStr((resSize == size),"TH_PL Warning: Not matching size, continuing anyway.");
  
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

