//# TH_Postgresql.h: TransportHolder to/from a Postgres database
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

#ifndef TRANSPORTPOSTGRES_TH_POSTGRES_H
#define TRANSPORTPOSTGRES_TH_POSTGRES_H

#include <TransportPostgres/TH_DB.h>
#include <pqxx/pqxx>

// This is a simple transportholder to store data in a Postgresql database.
// It also allows you to execute your own SQL statements, so it is completely flexible 
// regard to the way you want to store/retrieve your data.
// To make DataHolders persistent, use the Persistence Layer (located in 
// LOFAR/LCS/databases/TransportPL)

namespace LOFAR
{

class TH_Postgresql: public TH_DB
{
public:
  // Constructor. The table name specified here is used in the send/recv methods. This table
  // should have columns: SEQNR of type BIGINT, TAG of type INTEGER and a DATA of type BYTEA.
  // If only the queryDB/executeSQL methods are used, the table name does not have to be
  // specified in the constructor.
  TH_Postgresql (const string& tableName = "");
  virtual ~TH_Postgresql ();

  virtual TH_Postgresql* clone () const;

  virtual bool isClonable() const;

  virtual bool init();

  virtual bool recvBlocking (void* buf, int nbytes, int tag, 
			     int nBytesRead=0, DataHolder* dh=0);

  virtual int32 recvNonBlocking (void* buf, int nbytes, int tag, 
				int nBytesRead=0, DataHolder* dh=0);

  virtual void waitForReceived (void* buf, int nbytes, int tag);

  virtual bool sendBlocking (void* buf, int nbytes, int tag, DataHolder* dh=0);
  virtual bool sendNonBlocking (void* buf, int nbytes, int tag, DataHolder* dh=0);

  virtual void waitForSent (void* buf, int nbytes, int tag);

  virtual string getType () const;

  virtual void reset();

  // Specify the host, database and username to be used.
  static void useDatabase (const string& dbHost, const string& dbName,
                           const string& userName="postgres");

  // Special functions to deal with database records in a special way.  
  int queryDB (const string& queryString, DataHolder* dh);  

  // Execute SQL statement and store first field of first record in pResult. 
  // Return 0 if no result is found.
  virtual int queryDB (const string& queryString, 
		       char* pResult, int maxResultSize); 

  void executeSQL (const string& sqlStatement);  

  // <group>  
  // Convert DataHolder blob to string to be used in SQL statement.
  void addDBBlob(DataHolder* dh, ostringstream& str);

protected:
  static int theirInstanceCount;  //??protected

  /// Statically shared connection object containing connection
  /// information to database.
  static pqxx::connection* theirConnection;
  
private:
  // Methods to manage the connection to the dbms
   // <group> 
   void connectDatabase();
   void disconnectDatabase();
  // </group> 

 /// Strings containing the name specs describing the ODBC connection.
  static string theirHost;
  static string theirDBName;
  static string theirUserName;
  string        itsTableName;
 
  int64  itsWriteSeqNo;
  int64  itsReadSeqNo;

  bool itsInitCalled; // Flag to indicate if the init method has been called
                      // This is used in the counting of initialized instances.
};

inline bool TH_Postgresql::isClonable() const
{ return true; }

inline void TH_Postgresql::reset()
{}
 
} // end namespace

#endif
