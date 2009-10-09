//# ParmDBLocker.cc: Class to hold a read or write lock on ParmDBs
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <ParmDB/ParmDBLocker.h>
#include <ParmDB/ParmDB.h>
#include <ParmDB/ParmSet.h>

namespace LOFAR {
namespace BBS {

  ParmDBLocker::ParmDBLocker (const ParmSet& parmSet, bool lockForWrite)
    : itsDBs (parmSet.getDBs())
  {
    for (uint i=0; i<itsDBs.size(); ++i) {
      itsDBs[i]->lock (lockForWrite);
    }
  }

  ParmDBLocker::ParmDBLocker (ParmDB& parmdb, bool lockForWrite)
    : itsDBs (1, &parmdb)
  {
    parmdb.lock (lockForWrite);
  }

  ParmDBLocker::~ParmDBLocker()
  {
    for (uint i=0; i<itsDBs.size(); ++i) {
      itsDBs[i]->unlock();
    }
  }

} //# end namespace BBS
} //# end namspace LOFAR
