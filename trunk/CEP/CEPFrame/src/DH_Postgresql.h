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

  bool StoreInDatabase (int appId, int tag, char * buf, int size);
  bool RetrieveFromDatabase (int appId, int tag, char * buf, int size);

protected:
  class DataPacket:
    public DH_Database::DataPacket
  {
  public:
    DataPacket () {}
  };

private:

  bool Store (unsigned long wrseqno);
  bool Retrieve (unsigned long rdweqno);

  void ConnectDatabase (void);
  void DisconnectDatabase (void);

  bool ExecuteSQLCommand (char * str);
  bool ExecuteSQLCommand (ostringstream & q);

  ulong itsReadSeqNo;
  ulong itsWriteSeqNo;

  bool isConnected;

  static ulong theirInstanceCount;
  static PGconn * theirConnection;

};

}

#endif



