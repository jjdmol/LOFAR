//# TH_PL.cc: TransportHolder using the Persistency Layer
//#
//# Copyright (C) 2000, 2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$


#include <Transport/TH_PL.h>
#include <Transport/Transporter.h>
#include <PL/PersistenceBroker.h>
#include <PL/TPersistentObject.h>
#include <PL/DBRepHolder.h>    // needed to see ObjectId specialization
#include <PL/Collection.h>
#include <PL/QueryObject.h>

namespace LOFAR
{

// Default data source and account names for PL. You should override them
// with a call to UseDatabase () in main ().
string TH_PL::theirDSN = "_XXX_";
string TH_PL::theirUserName = "postgres";

int TH_PL::theirInstanceCount=0;   
   
// The PersistenceBroker is a management object from PL. One static
// PersistenceBroker is used to optimally use resources.
PL::PersistenceBroker TH_PL::theirPersistenceBroker;

TH_PL::TH_PL (const string& tableName)
  : itsTableName  (tableName),
    itsWriteSeqNo (0),
    itsReadSeqNo  (0),
    itsInitCalled (false)
{
  LOG_TRACE_FLOW( "TH_PL constructor" );
}

TH_PL::~TH_PL()
{
  LOG_TRACE_FLOW( "TH_PL destructor" );  
  if (itsInitCalled)           // Count only initialized instances
  {
    TH_PL::theirInstanceCount--;
    // If this was the last instance of DH_PL, disconnect from the database. 
    if (TH_PL::theirInstanceCount == 0L) {
      disconnectDatabase ();
    }
  }
}

TH_PL* TH_PL::make() const
{
  return new TH_PL(itsTableName);
}

bool TH_PL::init()
{
  // Do a dynamic cast of the DataHolder to a DH_PL.
  // and check if it is correct.
  itsDHPL = dynamic_cast<DH_PL*>(getTransporter()->getDataHolder());
  if (itsDHPL == 0) {
    throw LOFAR::Exception("TH_PL: DataHolder used is not derived from DH_PL");
  }
  // Initialize the TPO object in the DH_PL.
  itsDHPL->initPO (itsTableName);

  // If this is the first instance of TH_PL in the (current) process,
  // connect to the database.
  if (TH_PL::theirInstanceCount == 0L) {
    connectDatabase ();
  }
  TH_PL::theirInstanceCount++;
  itsInitCalled = true;
  return true;
}

void TH_PL::useDatabase (const string& dbDSN, const string& userName)
{
  TH_PL::theirDSN      = dbDSN;
  TH_PL::theirUserName = userName;
}

void TH_PL::connectDatabase()
{
  if (TH_PL::theirDSN == "_XXX_") {
    LOG_WARN_STR("***WARNING***: TH_PL::ConnectDatabase (); TH_PL "
		 << "is trying to connect to a test database residing on "
		 << "dop50. You probably have forgotten to call the TH_PL "
		 << "method UseDatabase (). See the comments in TH_PL.h " 
		 << "for more details. Continuing execution... " );
  }
  theirPersistenceBroker.connect (TH_PL::theirDSN, TH_PL::theirUserName);
  LOG_INFO( "Connected to database" );
}


void TH_PL::disconnectDatabase()
{}

string TH_PL::getType() const
{
  return "TH_PL";
}

bool TH_PL::connectionPossible (int srcRank, int dstRank) const
{
  return srcRank == dstRank;
}

void TH_PL::insertDB (int tag)  
{  
  PL::PersistentObject& aPO = itsDHPL->preparePO (tag, itsWriteSeqNo++);  
  theirPersistenceBroker.save(aPO, PL::PersistenceBroker::INSERT);  
}  

void TH_PL::updateDB (int tag)  
{  
  PL::PersistentObject& aPO = itsDHPL->preparePO (tag, itsWriteSeqNo++);  
  theirPersistenceBroker.save(aPO, PL::PersistenceBroker::UPDATE);  
}  
     
int TH_PL::queryDB (const string& queryString, int tag)  
{  
  // Get a reference to the DHPL's TPO object.  
  PL::PersistentObject& aPO = itsDHPL->getPO();  
  int result = aPO.retrieveInPlace(PL::QueryObject(queryString));  
  ASSERT (result >= 0);  
  itsReadSeqNo++;  
  return result;  
}  

bool TH_PL::sendBlocking(void*, int, int tag)
{
  LOG_TRACE_RTTI( "TH_PL sendBlocking()" );
  PL::PersistentObject& aPO = itsDHPL->preparePO (tag, itsWriteSeqNo++);
  theirPersistenceBroker.save(aPO, PL::PersistenceBroker::INSERT);
  return true;
}

bool TH_PL::sendNonBlocking(void* buf, int nbytes, int tag)
{
  LOG_WARN( "TH_PL::sendNonBlocking() is not implemented. The sendBlocking() method is used instead.");
  return sendBlocking(buf, nbytes, tag);
}

bool TH_PL::waitForSent(void*, int, int)
{
  LOG_TRACE_RTTI( "TH_PL waitForSent()" );
  return true;
}

bool TH_PL::recvBlocking(void*, int nbytes, int tag)
{
  LOG_TRACE_RTTI( "TH_PL recvBlocking()" );
  bool result = recvVarBlocking (tag);
  if (result) {
    DBGASSERTSTR(int(itsDHPL->getDataBlock().size()) == nbytes,
		 "TH_PL::recv - non matching size; found "
		 << itsDHPL->getDataBlock().size() << ", expected " << nbytes);
  }
  return result;
}

bool TH_PL::recvVarBlocking(int tag)
{
  LOG_TRACE_RTTI( "TH_PL recvVarBlocking()" );
  // Get a reference to the DHPL's TPO object.
  PL::PersistentObject& aPO = itsDHPL->getPO();
  // PL is based on query objects to identify records in database
  // tables. A query object is now prepared to retrieve the record with
  // the correct tag and sequence number.
  std::ostringstream q;
  q << "tag=" << tag << " AND seqnr=" << itsReadSeqNo;
  itsReadSeqNo++;
  int result = aPO.retrieveInPlace(PL::QueryObject(q.str()));
  ASSERT (result == 1);
  // ToDo: DBGASSERTSTR(itsReadSeqnNo == ... ,"");
  return true;
}

bool TH_PL::recvNonBlocking(void* buf, int nbytes, int tag)
{ 
  LOG_WARN( "TH_PL::recvNonBlocking() is not implemented. recvBlocking() is used instead." );    
  return recvBlocking (buf, nbytes, tag);
}

bool TH_PL::recvVarNonBlocking(int tag)
{ 
  LOG_WARN( "recvVarNonBlocking() is not implemented. recvBlocking() is used instead.");
  return recvVarBlocking (tag);
}

bool TH_PL::waitForReceived(void*, int, int)
{
  LOG_TRACE_RTTI( "TH_PL waitForReceived()" );
  return true;
}

void TH_PL::waitForBroadCast () {}
void TH_PL::waitForBroadCast (unsigned long&) {}
void TH_PL::sendBroadCast (unsigned long) {}
int  TH_PL::getCurrentRank () { return -1; }
int  TH_PL::getNumberOfNodes () { return 1; }
void TH_PL::finalize () {}
void TH_PL::synchroniseAllProcesses () {}


BlobStringType TH_PL::blobStringType() const
{
  // Use a string buffer (for the blob in PL).
  return BlobStringType(true);
}


} // end namespace
