//# ParmCache.cc: A class to cache ParmDB entries for a given work domain
//#
//# Copyright (C) 2008
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
#include <ParmDB/ParmCache.h>
#include <ParmDB/ParmValue.h>
#include <ParmDB/ParmDBLocker.h>
#include <ParmDB/ParmDB.h>

namespace LOFAR {
namespace BBS {

  ParmCache::ParmCache (ParmSet& parmSet)
    : itsParmSet (&parmSet)
  {}

  ParmCache::ParmCache (ParmSet& parmSet, const Box& workDomain)
    : itsParmSet    (&parmSet),
      itsWorkDomain (workDomain)
  {
    cacheValues();
  }

  void ParmCache::clear()
  {
    itsValueSets.clear();
    itsAxisCache.clear();
  }

  void ParmCache::reset (const Box& workDomain)
  {
    clear();
    itsWorkDomain = workDomain;
    cacheValues();
  }

  void ParmCache::cacheValues()
  {
    if (itsParmSet->size() > itsValueSets.size()) {
      itsParmSet->getValues (itsValueSets, itsWorkDomain);
    }
  }

  void ParmCache::setSolveGrid (ParmId parmId, const Grid& solveGrid)
  {
    ASSERT (parmId < itsValueSets.size());
    itsValueSets[parmId].setSolveGrid (solveGrid);
  }

  void ParmCache::flush()
  {
    ParmDBLocker (*itsParmSet, true);
    for (uint i=0; i<itsValueSets.size(); ++i) {
      ParmValueSet& pvset = itsValueSets[i];
      if (pvset.isDirty()) {
        itsParmSet->write (i, pvset);
        pvset.setDirty (false);
      }
    }
  }

} //# end namespace BBS
} //# end namspace LOFAR
