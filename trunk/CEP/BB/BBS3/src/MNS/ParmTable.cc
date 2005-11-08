//# ParmTable.cc: Object to hold parameters in a table.
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
#include <unistd.h>
#include <BBS3/MNS/ParmTable.h>
#include <BBS3/MNS/ParmTableAIPS.h>
#include <BBS3/MNS/ParmTableBDB.h>
#include <BBS3/MNS/ParmTableBDBRepl.h>
#include <BBS3/MNS/MeqStoredParmPolc.h>
#include <Common/LofarLogger.h>
#include <casa/Arrays/Vector.h>
#include <casa/Utilities/GenSort.h>

using namespace casa;

namespace LOFAR {

ParmTable::ParmTable (const ParmTableData& ptd)
: itsRep (0)
{
  if (ptd.itsType == "aips") {
    itsRep = new ParmTableAIPS (ptd.itsTableName);
  } else if (ptd.itsType == "bdb") {
    itsRep = new ParmTableBDB (ptd.itsTableName);
  } else if (ptd.itsType == "bdbrepl") {
    if (ptd.itsIsMaster) {
      itsRep = new ParmTableBDBRepl (ptd.itsTableName, ptd.itsHostName, ptd.itsMasterPort, ptd.itsNoSlaves);
    } else {
      itsRep = new ParmTableBDBRepl (ptd.itsTableName, ptd.itsHostName, ptd.itsMasterPort, -1);
    }
  } else {
    ASSERTSTR(false, "unknown parmTableType: "<<ptd.itsType);
  }
  itsRep->setParmTableData(ptd);
  itsRep->connect();
}

MeqSourceList ParmTable::getPointSources (MeqParmGroup* group,
					  const Vector<int>& srcnrs)
{
  // Get the vector of all parms containing a source name.
  vector<string> nams = itsRep->getSources();
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
  MeqSourceList sources;
  for (int i=0; i<nr; i++) {
    int inx = index(i);
    int srcnr = srcs[inx];
    // Check if in srcnrs.
    bool fnd = true;
    if (srcnrs.nelements() > 0) {
      fnd = false;
      for (unsigned int j=0; j<srcnrs.nelements(); j++) {
	if (srcnr == srcnrs[j]) {
	  fnd = true;
	  break;
	}
      }
    }
    if (fnd) {
      string name = nams[inx];
      MeqStoredParmPolc* mr = new MeqStoredParmPolc("RA."+name,
						    group, this);
      MeqStoredParmPolc* md = new MeqStoredParmPolc("DEC."+name,
						    group, this);
      MeqStoredParmPolc* mi = new MeqStoredParmPolc("StokesI."+name,
						    group, this);
      MeqStoredParmPolc* mq = new MeqStoredParmPolc("StokesQ."+name,
						    group, this);
      MeqStoredParmPolc* mu = new MeqStoredParmPolc("StokesU."+name,
						    group, this);
      MeqStoredParmPolc* mv = new MeqStoredParmPolc("StokesV."+name,
						    group, this);
      sources.add (MeqPointSource(name, mi, mq, mu, mv, mr, md));
//    cout << "Found source " << name << " (srcnr=" << srcnr << ')' << endl;
    }
  }

  return sources;
}

}
