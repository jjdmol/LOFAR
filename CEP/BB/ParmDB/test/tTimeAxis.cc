//# tTimeAxis.cc: Program to read a time axis from an MS
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
#include <ParmDB/Axis.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_sstream.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace casa;

void doIt (const string& msname, int timestep)
{
  Table tab(msname);
  Table tabs (tab.sort ("TIME", Sort::Ascending, Sort::NoDuplicates));
  cout<<tab.nrow() <<' '<<tabs.nrow()<<endl;
  Vector<double> times = ROScalarColumn<double>(tabs, "TIME").getColumn();
  Vector<double> intvs = ROScalarColumn<double>(tabs, "INTERVAL").getColumn();
  OrderedAxis axis(vector<double>(times.begin(), times.end()),
                   vector<double>(intvs.begin(), intvs.end()));
  //Axis::ShPtr axis1 (axis.compress (1));
  Axis::ShPtr axis1 (new OrderedAxis(axis));
  cout.precision (20);
  double last = axis1->lower(0);
  for (uint i=0; i<times.size(); i+=timestep) {
    uint n = timestep;
    if (n > times.size() - i) {
      n = times.size() - i;
    }
    Axis::ShPtr axis2 (axis1->subset (i, i+n-1));
    cout << "step " << i << ": " << n << " times  " << axis2->lower(0)
         << ' ' <<  axis2->upper(n-1) << "  " 
         << casa::near(axis2->lower(0), last) << endl;
    last = axis2->upper(n-1);
  }
}

int main (int argc, char* argv[])
{
  try {
    int timestep = 50;
    if (argc > 2) {
      istringstream istr(argv[2]);
      istr >> timestep;
    }
    doIt (argv[1], timestep);
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
