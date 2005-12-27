//# ParmTableBDBRepl.h: Replicated Berkeley database
//#
//# Copyright (C) 2002
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

#if !defined(MNS_PARMTABLEBDBREPL_H)
#define MNS_PARMTABLEBDBREPL_H

// \file
// Replicated Berkeley database

//# Includes
#include <BBS3/MNS/ParmTableBDB.h>
#include <BBS3/MNS/MeqParmHolder.h>
#include <BBS3/MNS/MeqPolc.h>
#include <BDBReplication/BDBReplicator.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <db_cxx.h>

//# Forward Declarations
namespace casa {
  template<class T> class Vector;
}
namespace LOFAR {
  class MeqDomain;
}

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

  class ParmTableBDBRepl : public ParmTableBDB{
  public:
    // Create the ParmTable object.
    // The dbType argument gives the database type.

    // If this object is not a master, an exception will occur when the user
    // tries to write to it.
    ParmTableBDBRepl (const string& tableName,
		      const string& masterHostName,
		      const int masterPort,
		      const int noSlaves);

    virtual ~ParmTableBDBRepl();

    // Sync the table
    void sync() {theirReplicator->waitForSync();};

    // Connect to the database
    virtual void connect();
    // Create the database or table
    static void createTable(const string& tableName);

  private:
    static BDBReplication::BDBReplicator* theirReplicator;
    static int theirReplicatorRefCount;
    bool itsIsMaster;
    int itsNoSlaves;
  };

  // @}

}

#endif
