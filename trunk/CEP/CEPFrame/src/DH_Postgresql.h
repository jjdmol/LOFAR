//  PO_DH_Database.h: Standard database persistent DH_Database
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


/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 
   WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 

   Using DH_Database requires that the package postgresql-devel be
   installed on the machine. For lofar17 and lofar3, the package has
   been installed correctly. However, some dop* hosts may not have
   this package installed. In order to prevent that CEPFrame will not
   build on such machines, the DH_Database.cc and DH_Postgresql.cc HAVE
   NOT BEEN INCLUDED IN Makefile.am. YOU HAVE TO ADD THEM YOURSELF
   FOR THE TIME BEING.

   WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 
   WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */

#ifndef CEPFRAME_DH_POSTGRESQL_H
#define CEPFRAME_DH_POSTGRESQL_H

#include <CEPFrame/DH_Database.h>		// for class definition
#include <Common/LofarTypes.h>			// for ulong
#include <libpq-fe.h>				// for PGconn et al
#include <sstream>

using namespace std;

namespace LOFAR {

class DH_Postgresql : public DH_Database {

public:

  explicit DH_Postgresql (const string& name, const string& type);
  virtual ~DH_Postgresql ();

  virtual bool StoreInDatabase (int appId, int tag, char * buf, int size);
  virtual bool RetrieveFromDatabase (int appId, int tag, char * buf, int size);

  // Connect to the database named DBName, residing at host DBHost, using
  // database account UserName. The following arguments will work for
  // testing purposes: DBHost="10.87.2.50", DBName=<YourLogInName>,
  // UserName="postgres". This method must be called before the first 
  // send or receive event occurs in the TransportHolder. Calling this
  // method right after connect (TH_Database::proto) is the best place
  // to guarantiee this. Note: 10.87.2.50 is dop50 the database server.
  // If you use the hostname dop50, you have to make sure that the
  // client host is able to resolve dop50; this may not always be the
  // case. lofar3 is an example of such a case. If this method is not
  // called, the test database residing on dop49 is used as a default.
  static void UseDatabase (char * dbHost, char * dbName, char * userName);

protected:
  bool ExecuteSQLCommand (char * str);
  bool ExecuteSQLCommand (ostringstream & q);

  class DataPacket:
    public DH_Database::DataPacket
  {
  public:
    DataPacket () {}
  };

private:

  void ConnectDatabase (void);
  void DisconnectDatabase (void);

  ulong itsReadSeqNo;
  ulong itsWriteSeqNo;

  bool isConnected;

  static ulong theirInstanceCount;
  static PGconn * theirConnection;

  static string theirDBHost;
  static string theirDBName;
  static string theirUserName;

};

}

#endif



