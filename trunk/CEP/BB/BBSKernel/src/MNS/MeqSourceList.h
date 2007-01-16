//# MeqSourceList.h: List of sources
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

#if !defined(MNS_MEQSOURCELIST_H)
#define MNS_MEQSOURCELIST_H

// \file
// List of sources

//# Includes
#include <BBSKernel/MNS/MeqPointSource.h>
#include <ParmDB/ParmDB.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>

namespace LOFAR
{
namespace BBS
{

//# Forward Declarations
class MeqParmGroup;

// \ingroup BBSKernel
// \addtogroup MNS
// @{

class MeqSourceList
{
public:
  // The default constructor.
  MeqSourceList()
    {}

  // Fill the source list from the sources in the parm table.
  MeqSourceList (LOFAR::ParmDB::ParmDB& parmTable, MeqParmGroup& group);

  ~MeqSourceList();

  // Get the total number of sources.
  uint size() const
    { return itsSources.size(); }

  // Get the i-th source.
  MeqSource& operator[] (int i)
    { return *itsSources[i]; }

  // Get the source with the given name.
  // An exception is thrown if unknown.
  MeqSource* getSource (const string& name) const;

  // Get the source indices in the given group.
  const vector<int>& getGroup (const string& groupName) const;

  // Get all source names.
  vector<string> getSourceNames() const;

private:
  // Forbid copies.
  // <group>
  MeqSourceList (const MeqSourceList&);
  MeqSourceList& operator= (const MeqSourceList&);
  // </group>

  // Add a source.
  void add (MeqSource*);

  vector<MeqSource*>       itsSources;    //# all sources
  map<string,int>          itsNameMap;    //# map source name to sourcenr
  map<string,vector<int> > itsGroupMap;   //# map group name to sourcenrs
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
