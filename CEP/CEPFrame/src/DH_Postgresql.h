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


#ifndef CEPFRAME_PO_DH_DATABASE_H
#define CEPFRAME_PO_DH_DATABASE_H

#include <CEPFrame/DH_Database.h>		// for class definition
#include <Common/LofarTypes.h>			// for ulong
#include <pgsql/libpq-fe.h>				// for PGconn et al
#include <sstream>

using namespace std;


class DH_Postgresql : public DH_Database {

public:

  explicit DH_Postgresql (const string& name, const string& type);
  virtual ~DH_Postgresql ();

  bool StoreInDatabase (int appId, int tag, char * buf, int size);
  bool RetrieveFromDatabase (int appId, int tag, char * buf, int size);

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



#endif



