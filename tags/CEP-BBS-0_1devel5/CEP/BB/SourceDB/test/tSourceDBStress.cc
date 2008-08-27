//# tSourceDBStress.cc: Stress test for adding and searching sources
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

#include <SourceDB/SourceDB.h>
#include <casa/OS/Timer.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;
using namespace casa;
using namespace LOFAR;
using namespace SourceDB;
using namespace ParmDB;

int main(int argc, const char* argv[])
{
  try {
    int nrsrc = 10000;
    int nrsearch = 1;
    if (argc > 1) {
      istringstream istr(argv[1]);
      istr >> nrsrc;
    }
    if (argc > 2) {
      istringstream istr(argv[2]);
      istr >> nrsearch;
    }
    SourceDBMeta meta ("aips", "tSourceDBStress_tmp.sdb");
    LOFAR::SourceDB::SourceDB sourcetab (meta, true);
    sourcetab.lock();
    // Add all the sources.
    Timer timer;
    for (int i=0; i<nrsrc; ++i) {
      SourceValue pvalue;
      double ra  = 1.2;   // radians
      double dec = 2.1;
      double spindex = 1;
      double flux[4] = {1,0,0,0};
      pvalue.setPointSource ("src", ra, dec, flux, spindex);
      sourcetab.addSource (pvalue);
    }
    cout << "added " << nrsrc << " sources";
    timer.show();

    // Now do a search for some sources.
    timer.mark();
    for (int i=0; i<nrsearch; ++i) {
      sourcetab.getSources (0.1, 0.1, 0.00001, ParmDomain());
    }
    cout << "searched " << nrsearch << " sources";
    timer.show();
    
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
  }
}
