//# ParmSet.cc: Set of parameters to be used
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
#include <ParmDB/ParmSet.h>
#include <ParmDB/ParmDB.h>

namespace LOFAR {
namespace BBS {

  ParmSet::ParmSet()
  {}

  ParmId ParmSet::addParm (ParmDB& parmdb, const string& name)
  {
    // See if the parm has already been added.
    map<string,int>::const_iterator pos = itsNames.find(name);
    if (pos != itsNames.end()) {
      return itsParms[pos->second].getParmId();
    }
    // A new parameter name, so add it.
    // First find its name id.
    int nameId = parmdb.getNameId (name);
    // Assign a unique parm id.
    ParmId parmId = itsParms.size();
    itsParms.push_back (ParmKey(&parmdb, name, nameId, parmId));
    itsNames.insert (make_pair(name, parmId));
    // If needed, add its ParmDB to the list of used ParmDBs.
    uint i;
    for (i=0; i<itsDBs.size(); ++i) {
      if (&parmdb == itsDBs[i]) {
        break;
      }
    }
    if (i == itsDBs.size()) {
      itsDBs.push_back (&parmdb);
    }
    return parmId;
  }

  ParmId ParmSet::find (const string& name) const
  {
    map<string,int>::const_iterator pos = itsNames.find(name);
    ASSERTSTR (pos != itsNames.end(), "Parm " << name
               << " not found in ParmSet");
    return itsParms[pos->second].getParmId();
  }

  void ParmSet::getValues (vector<ParmValueSet>& vsets,
                           const Box& workDomain) const
  {
    if (vsets.size() == itsParms.size()) {
      return;      // nothing to do
    }
    ASSERT (vsets.size() < itsParms.size());
    uint todo = itsParms.size() - vsets.size();
    uint start = vsets.size();
    vsets.resize (itsParms.size());
    vector<uint>   nameIds;
    vector<ParmId> parmIds;
    nameIds.reserve (todo);
    parmIds.reserve (todo);
    // Get values for all new parameters.
    // Do it in order of ParmDB to query as little as possible.
    for (uint i=0; i<itsDBs.size(); ++i) {
      ParmDB* pdb = itsDBs[i];
      for (uint j=start; j<itsParms.size(); ++j) {
        if (itsParms[j].getParmDBPtr() == pdb) {
          int nameid = itsParms[j].getNameId();
          ParmId parmid = itsParms[j].getParmId();
          if (nameid < 0) {
            // Use default value.
            vsets[parmid] = pdb->getDefValue (itsParms[parmid].getName(),
                                              ParmValue());
          } else {
            nameIds.push_back (nameid);
            parmIds.push_back (parmid);
          }
        }
      }
      if (nameIds.size() > 0) {
        pdb->getValues (vsets, nameIds, parmIds, workDomain);
        todo -= nameIds.size();
        if (todo == 0) break;
        nameIds.clear();
        parmIds.clear();
      }
    }
  }

  void ParmSet::write (uint parmId, ParmValueSet& pvset)
  {
    ParmDB* pdb = const_cast<ParmDB*>(itsParms[parmId].getParmDBPtr());
    pdb->putValues (itsParms[parmId].getName(),
                    itsParms[parmId].getNameId(),
                    pvset);
  }

  void ParmSet::clear()
  {
    itsDBs.clear();
    itsParms.clear();
    itsNames.clear();
  }

} //# end namespace BBS
} //# end namspace LOFAR
