//# MeqSourceList.cc: List of sources
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
#include <BBS/MNS/MeqSourceList.h>
#include <BBS/MNS/MeqPointSource.h>
#include <BBS/MNS/MeqGaussSource.h>
#include <BBS/MNS/MeqParmFunklet.h>
#include <Common/LofarLogger.h>

#include <casa/Arrays/Vector.h>
#include <casa/Utilities/GenSort.cc>     // for automatic template
#include <algorithm>

using namespace casa;

namespace LOFAR {

MeqSourceList::MeqSourceList (ParmDB::ParmDB& parmTable, MeqParmGroup* group)
{
  // Get the vector of all parms containing a source name.
  // Also get all parms representing a gaussian source.
  vector<string> nams = parmTable.getNames("RA:*");
  vector<string> gnams = parmTable.getNames("Phi:*");
  if (nams.size() == 0) {
    map<string,ParmDB::ParmValueSet> pset;
    parmTable.getDefValues (pset, "RA:*");
    for (map<string,ParmDB::ParmValueSet>::const_iterator iter = pset.begin();
	 iter != pset.end();
	 iter++) {
      nams.push_back (iter->first);
    }
    pset.clear();
    parmTable.getDefValues (pset, "Phi:*");
    for (map<string,ParmDB::ParmValueSet>::const_iterator iter = pset.begin();
	 iter != pset.end();
	 iter++) {
      gnams.push_back (iter->first);
    }
  }
  // Extract the sourcenames from the parmnames.
  for (uint i=0; i<nams.size(); i++) {
    string name = nams[i];
    // Remove first part from the name which looks like RA:name.
    string::size_type idx = name.rfind (':');
    ASSERT (idx != string::npos);
    name = name.substr (idx+1);
    nams[i] = name;
  }
  // Extract the sourcenames from the parmnames.
  for (uint i=0; i<gnams.size(); i++) {
    string name = gnams[i];
    // Remove first part from the name which looks like RA:name.
    string::size_type idx = name.rfind (':');
    ASSERT (idx != string::npos);
    name = name.substr (idx+1);
    gnams[i] = name;
  }
  // Sort the names uniquely.
  Vector<uInt> index;
  int nr = GenSortIndirect<string>::sort (index, &nams[0], nams.size(),
					  Sort::Ascending,
					  Sort::QuickSort|Sort::NoDuplicates);
  for (int i=0; i<nr; i++) {
    int inx = index(i);
    const string& name = nams[inx];
    MeqParmFunklet* mr = new MeqParmFunklet("RA:"+name,
					    group, &parmTable);
    MeqParmFunklet* md = new MeqParmFunklet("DEC:"+name,
					    group, &parmTable);
    MeqParmFunklet* mi = new MeqParmFunklet("StokesI:"+name,
					    group, &parmTable);
    MeqParmFunklet* mq = new MeqParmFunklet("StokesQ:"+name,
					    group, &parmTable);
    MeqParmFunklet* mu = new MeqParmFunklet("StokesU:"+name,
					    group, &parmTable);
    MeqParmFunklet* mv = new MeqParmFunklet("StokesV:"+name,
					    group, &parmTable);
    if (std::find(gnams.begin(), gnams.end(), name) == gnams.end()) {
      add (new MeqPointSource(name, mi, mq, mu, mv, mr, md));
    } else {
      MeqParmFunklet* mmin = new MeqParmFunklet("Minor:"+name,
						group, &parmTable);
      MeqParmFunklet* mmaj = new MeqParmFunklet("Major:"+name,
						group, &parmTable);
      MeqParmFunklet* mphi = new MeqParmFunklet("Phi:"+name,
						group, &parmTable);
      add (new MeqGaussSource(name, mi, mq, mu, mv, mr, md, mmin, mmaj, mphi));
    }
//    cout << "Found source " << name << " (srcnr=" << srcnr << ')' << endl;
  }
}

MeqSourceList::~MeqSourceList()
{
  for (vector<MeqSource*>::iterator iter = itsSources.begin();
       iter != itsSources.end();
       iter++) {
    delete *iter;
  }
}

const vector<int>& MeqSourceList::getGroup (const string& groupName) const
{
  map<string,vector<int> >::const_iterator idx = itsGroupMap.find (groupName);
  ASSERTSTR (idx != itsGroupMap.end(),
	     "Source group " << groupName << " is unknown");
  return idx->second;
}

void MeqSourceList::add (MeqSource* source)
{
  int idx = itsSources.size();
  itsSources.push_back (source);
  itsNameMap[source->getName()] = idx;
  vector<int>& group = itsGroupMap[source->getGroupName()];
  group.push_back (idx);
}

}

