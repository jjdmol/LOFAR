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

#include <MNS/ParmTable.h>
#include <MNS/ParmTableAIPS.h>
#include <MNS/ParmTableDB.h>
#include <MNS/MeqStoredParmPolc.h>
#include <Common/Debug.h>
#include <aips/Arrays/Vector.h>
#include <aips/Utilities/GenSort.h>


ParmTable::ParmTable (const string& dbType, const string& tableName,
		      const string& userName, const string& passwd)
: itsRep (0)
{
  if (dbType == "aips") {
    itsRep = new ParmTableAIPS (tableName);
  } else {
    itsRep = new ParmTableDB (dbType, tableName, userName, passwd);
  }
}

MeqSourceList ParmTable::getPointSources (const Vector<int>& srcnrs)
{
  vector<MeqExpr*> exprDel;
  return getPointSources (srcnrs, exprDel);
}

MeqSourceList ParmTable::getPointSources (const Vector<int>& srcnrs,
					  vector<MeqExpr*>& exprDel)
{
  // Get the vector of all parms containing a source name.
  vector<string> nams;
  vector<int> srcs;
  itsRep->getSources (nams, srcs);
  // Sort the srcnrs uniquely.
  Vector<uInt> index;
  int nr = GenSortIndirect<Int>::sort (index, &srcs[0], srcs.size(),
				       Sort::Ascending,
				       Sort::QuickSort | Sort::NoDuplicates);
  MeqSourceList sources;
  for (int i=0; i<nr; i++) {
    int inx = index(i);
    int srcnr = srcs[inx];
    // Check if in srcnrs.
    bool fnd = true;
    if (srcnrs.nelements() > 0) {
      fnd = false;
      for (unsigned int i=0; i<srcnrs.nelements(); i++) {
	if (srcnr == srcnrs[i]) {
	  fnd = true;
	  break;
	}
      }
    }
    if (fnd) {
      string name = nams[inx];
      string::size_type idx = name.rfind ('.');
      Assert (idx != string::npos);
      // Remove first part (RA).
      name = name.substr (idx+1);
      MeqStoredParmPolc* mr = new MeqStoredParmPolc("RA."+name,
						    srcnr, -1, this);
      exprDel.push_back (mr);
      MeqStoredParmPolc* md = new MeqStoredParmPolc("DEC."+name,
						    srcnr, -1, this);
      exprDel.push_back (md);
      MeqStoredParmPolc* mi = new MeqStoredParmPolc("StokesI."+name,
						    srcnr, -1, this);
      exprDel.push_back (mi);
      MeqStoredParmPolc* mq = new MeqStoredParmPolc("StokesQ."+name,
						    srcnr, -1, this);
      exprDel.push_back (mq);
      MeqStoredParmPolc* mu = new MeqStoredParmPolc("StokesU."+name,
						    srcnr, -1, this);
      exprDel.push_back (mu);
      MeqStoredParmPolc* mv = new MeqStoredParmPolc("StokesV."+name,
						    srcnr, -1, this);
      exprDel.push_back (mv);
      sources.add (MeqPointSource(name, mi, mq, mu, mv, mr, md));
      cout << "Found source " << name << " (srcnr=" << srcnr << ')' << endl;
    }
  }
  return sources;
}
