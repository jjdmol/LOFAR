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
#include <BBSKernel/MNS/MeqSourceList.h>
#include <BBSKernel/MNS/MeqPointSource.h>
#include <BBSKernel/MNS/MeqGaussianSource.h>
#include <BBSKernel/MNS/MeqParmFunklet.h>
#include <Common/LofarLogger.h>

#include <casa/Arrays/Vector.h>
#include <casa/Utilities/GenSort.h>
#ifdef AIPS_NO_TEMPLATE_SRC
#include <casa/Utilities/GenSort.cc>     // for automatic template
#endif
#include <algorithm>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

MeqSourceList::MeqSourceList (LOFAR::ParmDB::ParmDB& parmTable, MeqParmGroup& group)
{
  // Get the vector of all parms containing a source name.
  // Also get all parms representing a gaussian source.
  vector<string> nams = parmTable.getNames("RA:*");
  vector<string> gnams = parmTable.getNames("Phi:*");
  if (nams.size() == 0) {
    map<string,LOFAR::ParmDB::ParmValueSet> pset;
    parmTable.getDefValues (pset, "RA:*");
    for (map<string,LOFAR::ParmDB::ParmValueSet>::const_iterator iter = pset.begin();
	 iter != pset.end();
	 iter++) {
      nams.push_back (iter->first);
    }
    pset.clear();
    parmTable.getDefValues (pset, "Phi:*");
    for (map<string,LOFAR::ParmDB::ParmValueSet>::const_iterator iter = pset.begin();
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
    MeqExpr mr = MeqParmFunklet::create ("RA:"+name,
					 group, &parmTable);
    MeqExpr md = MeqParmFunklet::create ("DEC:"+name,
					 group, &parmTable);
    MeqExpr mi = MeqParmFunklet::create ("StokesI:"+name,
					 group, &parmTable);
    MeqExpr mq = MeqParmFunklet::create ("StokesQ:"+name,
					 group, &parmTable);
    MeqExpr mu = MeqParmFunklet::create ("StokesU:"+name,
					 group, &parmTable);
    MeqExpr mv = MeqParmFunklet::create ("StokesV:"+name,
					 group, &parmTable);
    /// For the time being the group name (patch name) is equal
    /// to the source name.
    if (std::find(gnams.begin(), gnams.end(), name) == gnams.end()) {
      add (new MeqPointSource(name, name, mi, mq, mu, mv, mr, md));
    } else {
      LOG_DEBUG_STR("Found gaussian source: " << name);

      MeqExpr mmin = MeqParmFunklet::create ("Minor:"+name,
					     group, &parmTable);
      MeqExpr mmaj = MeqParmFunklet::create ("Major:"+name,
					     group, &parmTable);
      MeqExpr mphi = MeqParmFunklet::create ("Phi:"+name,
					     group, &parmTable);
      add (new MeqGaussianSource(name, name, mi, mq, mu, mv, mr, md,
			      mmaj, mmin, mphi));
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

MeqSource* MeqSourceList::getSource (const string& name) const
{
  map<string,int>::const_iterator pos = itsNameMap.find (name);
  ASSERTSTR (pos != itsNameMap.end(),
	     "Source " << name << " is unknown");
  return itsSources[pos->second];
}

const vector<int>& MeqSourceList::getGroup (const string& groupName) const
{
  map<string,vector<int> >::const_iterator pos = itsGroupMap.find (groupName);
  ASSERTSTR (pos != itsGroupMap.end(),
	     "Source group " << groupName << " is unknown");
  return pos->second;
}

vector<string> MeqSourceList::getSourceNames() const
{
  vector<string> result;
  for (map<string,int>::const_iterator iter=itsNameMap.begin();
       iter!=itsNameMap.end();
       iter++) {
    result.push_back (iter->first);
  }
  return result;
}

void MeqSourceList::add (MeqSource* source)
{
  int idx = itsSources.size();
  itsSources.push_back (source);
  itsNameMap[source->getName()] = idx;
  vector<int>& group = itsGroupMap[source->getGroupName()];
  group.push_back (idx);
}

} // namespace BBS
} // namespace LOFAR
