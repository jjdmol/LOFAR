//# DH_Postgresql.h: Standard database persistent DH_Database
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

#ifndef CEPFRAME_DH_POSTGRESQL_H
#define CEPFRAME_DH_POSTGRESQL_H

#include <sstream>
#include <lofar_config.h>
#include <CEPFrame/DH_Database.h>		// for class definition
#include <Common/LofarTypes.h>			// for ulong

// DH_Postgresql requires the package postgres-devel to be installed. This
// package, along with development libraries for Postgresql, contains the
// include file libpq-fe.h. If the current installation supports the
// postgresql-devel package (by HAVE_PSQL), then the include file is
// included. If not, CEPFrame will still compile, but with unpredictable
// behaviour as soon as DH_Postgresql is used.

#ifdef HAVE_PSQL
# include <libpq-fe.h>				// for PGconn et al
#endif

namespace LOFAR {

/// The DH_Postgresql class implements a DataHolder communicating over a
/// Postgresql database engine. 

class DH_Postgresql : public DH_Database {

public:

  explicit DH_Postgresql (const string& name, const string& type);
  virtual ~DH_Postgresql ();

  /// Store message content into Postgresql database table.
  virtual bool StoreInDatabase (int appId, int tag, char * buf, int size);

  /// Retrieve message content from Postgresql database table.
  virtual bool RetrieveFromDatabase (int appId, int tag, char * buf, int size);

  /// Specify which name, host and account name should be used for the
  /// connection. The following arguments will work for testing purposes:
  /// DBHost="10.87.2.50", DBName=<YourLogInName>,
  /// UserName="postgres". This method must be called before the
  /// DH_Postgresql constructor. Calling this method at the beginning of
  /// main () is the best place to guarantee this. Note: 10.87.2.50 is
  /// dop50 the database server. If you use the hostname dop50, you have
  /// to make sure that the client host is able to resolve dop50; this may
  /// not always be the case. lofar3 is an example of such a case. If this
  /// method is not called, the test database residing on dop49 is used as
  /// a default.
  static void UseDatabase (char * dbHost, char * dbName, char * userName);

protected:
  /// Execution methods for Postgresql client initiated SQL queriess:
  bool ExecuteSQLCommand (char * str);
  bool ExecuteSQLCommand (std::ostringstream & q);

  class DataPacket:
    public DH_Database::DataPacket
  {
  public:
    DataPacket () {}
  };

protected:

#ifdef HAVE_PSQL
  /// Statically shared connection object containing connection
  /// information to database.
  static PGconn * theirConnection;
#endif

private:

  /// Connect/disconnect to/from PostgresQL database.
  void ConnectDatabase (void);
  void DisconnectDatabase (void);

  /// Internal counters for synchronizing the read and write queries
  ulong itsReadSeqNo;
  ulong itsWriteSeqNo;

  /// Boolean indicating whether a Postgresql connection is in place or not.
  bool isConnected;

  /// Counter for the number of DH_Postgresql instances. Used to coordinate
  /// the connection to the database.
  static ulong theirInstanceCount;

  /// Strings containing the name specs describing the Postgresql connection.
  static string theirDBHost;
  static string theirDBName;
  static string theirUserName;

};

}

#endif
