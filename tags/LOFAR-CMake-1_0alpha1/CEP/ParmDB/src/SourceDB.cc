//# SourceDB.cc: Object to hold parameters in a table.
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

#include <lofar_config.h>
#include <ParmDB/SourceDB.h>
#include <ParmDB/SourceDBCasa.h>
#include <ParmDB/ParmDB.h>
#include <Common/LofarLogger.h>

using namespace std;
using namespace casa;

namespace LOFAR {
namespace BBS {

  SourceDBRep::SourceDBRep (const ParmDBMeta& ptm, bool forceNew)
    : itsCount  (0),
      itsParmDB (ptm, forceNew)
  {}

  SourceDBRep::~SourceDBRep()
  {}

  void SourceDBRep::lock (bool)
  {}

  void SourceDBRep::unlock()
  {}


  SourceDB::SourceDB (const ParmDBMeta& ptm, bool forceNew)
  {
    // Open the correct SourceDB.
    if (ptm.getType() == "casa") {
      itsRep = new SourceDBCasa (ptm, forceNew);
    } else {
      ASSERTSTR(false, "unknown sourceTableType: " << ptm.getType());
    }
    itsRep->link();
  }

  SourceDB::SourceDB (SourceDBRep* rep)
  : itsRep (rep)
  {
    itsRep->link();
  }

  SourceDB::SourceDB (const SourceDB& that)
  : itsRep (that.itsRep)
  {
    itsRep->link();
  }

  SourceDB& SourceDB::operator= (const SourceDB& that)
  {
    if (this != &that) {
      decrCount();
      itsRep = that.itsRep;
      itsRep->link();
    }
    return *this;
  }

  void SourceDB::decrCount()
  {
    if (itsRep->unlink() == 0) {
      delete itsRep;
      itsRep = 0;
    }
  }

} // namespace BBS
} // namespace LOFAR
