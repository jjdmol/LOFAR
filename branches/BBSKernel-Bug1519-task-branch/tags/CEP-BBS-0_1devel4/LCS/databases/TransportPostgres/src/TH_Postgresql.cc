//# TH_Postgresql.cc: TransportHolder to/from a Postgres database
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Common/LofarLogger.h> 
#include <TransportPostgres/TH_Postgresql.h>
#include <TransportPostgres/DH_DB.h>
#include <pqxx/binarystring>

using namespace pqxx;

namespace LOFAR
{

// Default data source and account names for Postgresql. You should override 
// them with a call to UseDatabase () in main ().
string TH_Postgresql::theirHost = "dop50.astron.nl";
string TH_Postgresql::theirDBName = "_XXX_";
string TH_Postgresql::theirUserName = "postgres";

int TH_Postgresql::theirInstanceCount=0;
pqxx::connection* TH_Postgresql::theirConnection=0;

TH_Postgresql::TH_Postgresql (const string& tableName)
  : itsTableName  (tableName), 
    itsWriteSeqNo (0),
    itsReadSeqNo  (0),
    itsInitCalled (false)
{
  LOG_TRACE_FLOW( "TH_Postgresql constructor" );
}

TH_Postgresql::~TH_Postgresql()
{
LOG_TRACE_FLOW( "TH_Postgresql destructor" );
  if (itsInitCalled)           // Count only initialized instances
  {
    TH_Postgresql::theirInstanceCount--;
    // If this was the last instance of TH_Postgresql, disconnect from 
    // the database.
    if (TH_Postgresql::theirInstanceCount == 0L) {
      disconnectDatabase ();
    }
  }

}

TH_Postgresql* TH_Postgresql::clone() const
{
  return new TH_Postgresql(itsTableName);
}


bool TH_Postgresql::init()
{
  if (TH_Postgresql::theirInstanceCount == 0L) {
    connectDatabase ();
  }
  TH_Postgresql::theirInstanceCount++;
  itsInitCalled = true;
  return true;
}

void TH_Postgresql::useDatabase (const string& dbHost, const string& dbName,
				 const string& userName)
{
  TH_Postgresql::theirHost     = dbHost;
  TH_Postgresql::theirDBName   = dbName;
  TH_Postgresql::theirUserName = userName;

}

void TH_Postgresql::connectDatabase()
{
  ASSERTSTR(theirDBName != "_XXX_", "The TH_Postgresql::useDatabase() method has not yet been called. Do this before connecting!");
  ostringstream connInfo;
  
  connInfo << "host=" << theirHost 
	   << " dbname=" << theirDBName
	   << " user="<< theirUserName;
  
  theirConnection = new pqxx::connection(connInfo.str());
    
  LOG_INFO( "Connected to database" );
}


void TH_Postgresql::disconnectDatabase()
{
  delete theirConnection;
  theirConnection = 0;
}

string TH_Postgresql::getType() const
{
  return "TH_Postgresql";
}

void TH_Postgresql::executeSQL (const string& sqlStatement)  
{
  DBGASSERT(theirConnection != 0);
  work wrk(*theirConnection);
  wrk.exec(sqlStatement);
  wrk.commit();
}  
   
int TH_Postgresql::queryDB (const string& queryString, DataHolder* dh)  
{ 
  DBGASSERT(theirConnection != 0);
  work wrk(*theirConnection);
  result res = wrk.exec(queryString);

  int nRows;
  nRows = res.size();
  if (nRows == 0)
  {
    return nRows;
  }

  binarystring bs(res[0]["DATA"]);

  // Resize buffer if necessary
  if (bs.size() != dh->getDataSize())
  {
    dh->resizeBuffer(bs.size());
  }
  // Copy the data
  memcpy (dh->getDataPtr(), bs.data(), bs.size());

  return nRows;
}  

void TH_Postgresql::addDBBlob(DataHolder* dh, ostringstream& str)
{
  // Add binary string to stream.
  const string esc = escape_binary ((char *)(dh->getDataPtr()), dh->getDataSize());
  str << esc;
}

bool TH_Postgresql::sendBlocking(void*, int, int tag, DataHolder* dh)
{
   LOG_TRACE_RTTI( "TH_Postgresql sendBlocking()" );
   ASSERTSTR(itsTableName!="", "No table name has been specified in the constructor");

   ostringstream q;
   q << "INSERT INTO " << itsTableName << " VALUES ("
     << itsWriteSeqNo++ << ", "
     << tag << ", '";
   addDBBlob(dh, q);
   q << "')";
   executeSQL(q.str());   
   return true;
}

bool TH_Postgresql::sendNonBlocking(void* buf, int nbytes, int tag, DataHolder* dh)
{
  LOG_WARN( "TH_Postgresql::sendNonBlocking() is not implemented. The sendBlocking() method is used instead.");
  return sendBlocking(buf, nbytes, tag, dh);
}

void TH_Postgresql::waitForSent(void*, int, int)
{
  LOG_TRACE_RTTI( "TH_Postgresql waitForSent()" );
}

bool TH_Postgresql::recvBlocking(void*, int, int tag, int nBytesRead, DataHolder* dh)
{
  LOG_TRACE_RTTI( "TH_Postgresql recvBlocking()" );
   ASSERTSTR(itsTableName!="", "No table name has been specified in the constructor");

   ostringstream q;
   q << "SELECT DATA FROM " << itsTableName << " WHERE TAG=" 
     << tag << " AND SEQNR="
     << itsReadSeqNo;

   int result = 1;
   if (nBytesRead <= 0)
   {
     result = queryDB (q.str(), dh);
     ASSERTSTR (result == 1, "Query " << q.str() << " returned " << result << " results.");
     itsReadSeqNo++;
   }

   return result;
}

int32 TH_Postgresql::recvNonBlocking(void* buf, int nbytes, int tag, int nBytesRead, DataHolder* dh)
{ 
  LOG_WARN( "TH_Postgresql::recvNonBlocking() is not implemented. recvBlocking() is used instead." );    
  return (recvBlocking (buf, nbytes, tag, nBytesRead, dh)?nbytes:0);
}

void TH_Postgresql::waitForReceived(void*, int, int)
{
  LOG_TRACE_RTTI( "TH_Postgresql waitForReceived()" );
}

int TH_Postgresql::queryDB (const string& queryString, 
			    char* pResult, int maxResultSize) 
{
  DBGASSERT(theirConnection != 0);
  work wrk(*theirConnection);
  result res = wrk.exec(queryString);

  int nRows = res.size();
  if (nRows == 0)
  {
    return 0; 
  }
  int length = res[0][0].size();
  ASSERTSTR(length <= maxResultSize, "Result buffer is too small for actual result");
  memcpy(pResult, res[0][0].c_str(), length);

  return nRows;
}

} // end namespace
