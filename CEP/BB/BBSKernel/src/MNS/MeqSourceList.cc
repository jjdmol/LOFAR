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
#include <BBS/MNS/MeqParmFunklet.h>
#include <Common/LofarLogger.h>

#include <casa/Arrays/Vector.h>
#include <casa/Utilities/GenSort.h>

using namespace casa;

namespace LOFAR {

MeqSourceList::MeqSourceList (ParmDB::ParmDB& parmTable, MeqParmGroup* group)
{
  // Get the vector of all parms containing a source name.
  vector<string> nams = parmTable.getNames("RA.*");
  if (nams.size() == 0) {
    map<string,ParmDB::ParmValueSet> pset;
    parmTable.getDefValues (pset, "RA.*");
    for (map<string,ParmDB::ParmValueSet>::const_iterator iter = pset.begin();
	 iter != pset.end();
	 iter++) {
      nams.push_back (iter->first);
    }
  }
  vector<int> srcs(nams.size());
  // Extract the sourcenrs from the names.
  for (uint i=0; i<nams.size(); i++) {
    string name = nams[i];
    // Remove first part from the name which looks like RA.CPn..
    string::size_type idx = name.rfind ('.');
    ASSERT (idx != string::npos);
    // Remove first part (RA or so).
    name = name.substr (idx+1);
    nams[i] = name;
    int srcnr = -1;
    // Get sourcenr from name which looks like CPn.
    if (name.substr(0,2) == "CP") {
      istringstream istr(name.substr(2));
      istr >> srcnr;
      srcs[i] = srcnr-1;
    }
  }
  // Sort the srcnrs uniquely.
  Vector<uInt> index;
  int nr = GenSortIndirect<int>::sort (index, &srcs[0], srcs.size(),
				       Sort::Ascending,
				       Sort::QuickSort|Sort::NoDuplicates);
  for (int i=0; i<nr; i++) {
    int inx = index(i);
    string name = nams[inx];
    MeqParmFunklet* mr = new MeqParmFunklet("RA."+name,
					    group, &parmTable);
    MeqParmFunklet* md = new MeqParmFunklet("DEC."+name,
					    group, &parmTable);
    MeqParmFunklet* mi = new MeqParmFunklet("StokesI."+name,
					    group, &parmTable);
    MeqParmFunklet* mq = new MeqParmFunklet("StokesQ."+name,
					    group, &parmTable);
    MeqParmFunklet* mu = new MeqParmFunklet("StokesU."+name,
					    group, &parmTable);
    MeqParmFunklet* mv = new MeqParmFunklet("StokesV."+name,
					    group, &parmTable);
    add (MeqPointSource(name, mi, mq, mu, mv, mr, md));
//    cout << "Found source " << name << " (srcnr=" << srcnr << ')' << endl;
  }
}

void MeqSourceList::add (const MeqPointSource& source)
{
  itsSelected.push_back (itsSources.size());
  itsSources.push_back (source);
}

void MeqSourceList::setSelected (const vector<int>& sel)
{
  if (sel.size() == 0) {
    itsSelected.resize (itsSources.size());
    for (unsigned int i=0; i<itsSources.size(); i++) {
      itsSelected[i] = i;
    }
  } else {
    for (unsigned int i=0; i<sel.size(); i++) {
      ASSERT (sel[i] >= 0);
      ASSERT (sel[i] < int(itsSources.size()));
    }
    itsSelected = sel;
  }
}

}
