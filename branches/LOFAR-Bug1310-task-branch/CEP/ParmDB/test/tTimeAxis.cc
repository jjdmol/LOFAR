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

void doIt (const string& msname, int timestep, int timestart)
{
  Table tab(msname);
  Table tabs (tab.sort ("TIME", Sort::Ascending, Sort::NoDuplicates));
  cout<<tab.nrow() <<' '<<tabs.nrow()<<endl;
  Vector<double> times1 = ROScalarColumn<double>(tabs, "TIME").getColumn();
  Vector<double> intvs1 = ROScalarColumn<double>(tabs, "INTERVAL").getColumn();
  Vector<double> times = times1(Slicer(IPosition(1,timestart),
                                       IPosition(1,Slicer::MimicSource)));
  Vector<double> intvs = intvs1(Slicer(IPosition(1,timestart),
                                       IPosition(1,Slicer::MimicSource)));
  OrderedAxis axis(vector<double>(times.begin(), times.end()),
                   vector<double>(intvs.begin(), intvs.end()));
  //Axis::ShPtr axis1 (axis.compress (1));
  Axis::ShPtr axis1 (new OrderedAxis(axis));
  cout.precision (20);
  double last2 = axis1->lower(0);
  double last3 = axis1->lower(0);
  for (uint i=0; i<times.size(); i+=timestep) {
    uint n = timestep;
    if (n > times.size() - i) {
      n = times.size() - i;
    }
    Axis::ShPtr axis2 (axis1->subset (size_t(i), size_t(i+n-1)));
    cout << "step " << i << ": " << n << " times  " << axis2->lower(0)
         << ' ' <<  axis2->upper(n-1) << "  " 
         << casa::near(axis2->lower(0), last2);
    last2 = axis2->upper(n-1);
    // Create a few times the same axis.
    // Each time it converts from start/end to center/width and back.
    // So let's see what the accumulated rounding effect is.
    Axis::ShPtr axis3(axis2);
    for (uint i=0; i<10; ++i) {
      vector<double> start, end;
      for (uint j=0; j<n; ++j) {
        start.push_back (axis3->lower(j));
        end.push_back (axis3->upper(j));
      }
      axis3 = Axis::ShPtr (new OrderedAxis(start, end, true));
    }
    cout << "  diff: " << axis3->lower(0) - axis2->lower(0)
         << ' ' <<  axis3->upper(n-1) - axis2->upper(n-1)<< "  " 
         << casa::near(axis3->lower(0), last3) << endl;
    last3 = axis3->upper(n-1);
  }
}

int main (int argc, char* argv[])
{
  try {
    int timestep = 50;
    int timestart = 0;
    if (argc > 2) {
      istringstream istr(argv[2]);
      istr >> timestep;
    }
    if (argc > 3) {
      istringstream istr(argv[3]);
      istr >> timestart;
    }
    doIt (argv[1], timestep, timestart);
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
