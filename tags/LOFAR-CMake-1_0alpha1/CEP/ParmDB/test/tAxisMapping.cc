//# tAxisMapping.cc: Program to test the AxisMapping class
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
#include <ParmDB/AxisMapping.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <casa/BasicMath/Math.h>

using namespace LOFAR;
using namespace LOFAR::BBS;

void checkMapping (const Axis& predictAxis, const AxisMapping& mapping)
{
  ASSERT (mapping.size() == predictAxis.size());
  int parmCell = 1;
  for (size_t i=0; i<predictAxis.size(); ++i) {
    if (i%5 == 3) ++parmCell;
    ASSERT (mapping[i] == parmCell);
  }
  parmCell = 1;
  int nr=0;
  double cen = (10.5 - 8) / 5.;
  const double* centers = mapping.getScaledCenters();
  for (AxisMapping::const_iterator iter=mapping.begin();
       iter!=mapping.end(); ++iter) {
    if (nr%5 == 3) {
      ++parmCell;
      cen = 0.5/5.;
    }
    //cout << nr << ' '<<parmCell<<' '<<cen <<' ' <<*centers<<endl;
    ASSERT (*iter == parmCell);
    ASSERT (casa::near (*centers, cen));
    ++centers;
    ++nr;
    cen += 1/5.;
  }
  const vector<int>& borders = mapping.getBorders();
  ASSERT (borders.size() == 21);
  int st=3;
  for (uint i=0; i<20; ++i) {
    ASSERT (borders[i] == st);
    st += 5;
  }
  ASSERT (borders[20] == int(predictAxis.size()));
}

void testMapping()
{
  RegularAxis predictAxis (10, 1, 100);
  RegularAxis parmAxis (3, 5, 30);
  AxisMapping mapping (predictAxis, parmAxis);
  checkMapping (predictAxis, mapping);
}

void testCache()
{
  AxisMappingCache cache;
  RegularAxis predictAxis (10, 1, 100);
  RegularAxis parmAxis (3, 5, 30);
  const AxisMapping& mapping = cache.get (predictAxis, parmAxis);
  ASSERT (cache.size() == 1);
  checkMapping (predictAxis, mapping);
  const AxisMapping& mapping2 = cache.get (predictAxis, parmAxis);
  ASSERT (cache.size() == 1);
  ASSERT (&mapping2 == &mapping);
  // Clone copies the id, so mapping should still be the same.
  Axis::ShPtr parmAxis2 = Axis::ShPtr(parmAxis.clone());
  const AxisMapping& mapping3 = cache.get (predictAxis, *parmAxis2);
  ASSERT (cache.size() == 1);
  ASSERT (&mapping3 == &mapping);
  // Make a new axis, so a new mapping should be created.
  RegularAxis parmAxis3 (3, 5, 30);
  const AxisMapping& mapping4 = cache.get (predictAxis, parmAxis3);
  ASSERT (cache.size() == 2);
  ASSERT (&mapping4 != &mapping);
  checkMapping (predictAxis, mapping4);
}

int main()
{
  try {
    INIT_LOGGER("tAxisMapping");
    cout << "testing AxisMapping ..." << endl;
    testMapping();
    cout << "testing AxisMappingCache ..." << endl;
    testCache();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
