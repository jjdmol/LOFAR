//# tAxis.cc: Program to test the Axis classes
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <ParmDB/Axis.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>

using namespace LOFAR;
using namespace LOFAR::BBS;

// Test the default constructor.
template<typename AxisType>
void check1 (const AxisType& axis)
{
  ASSERT (axis.size() == 1);
  ASSERT (axis.center(0) == 0);
  ASSERT (axis.width(0) == 2e30);
  ASSERT (axis.lower(0) == -1e30);
  ASSERT (axis.upper(0) == 1e30);
  ASSERT (axis.range().first == -1e30);
  ASSERT (axis.range().second == 1e30);
  ASSERT (axis.locate(0.5e30) == 0);
  ASSERT (axis == axis);
  ASSERT (axis.checkIntervals (axis));
}

// Test an axis of 10 intervals of width 2 starting at 1.
template<typename AxisType>
void check10 (const AxisType& axis, bool isRegular)
{
  ASSERT (axis.isRegular() == isRegular);
  ASSERT (axis == axis);
  ASSERT (! (axis != axis));
  ASSERT (axis.checkIntervals (axis));
  ASSERT (axis.size() == 10);
  ASSERT (axis.center(0) == 2);
  ASSERT (axis.width(0) == 2);
  ASSERT (axis.lower(0) == 1);
  ASSERT (axis.upper(0) == 3);
  ASSERT (axis.center(9) == 20);
  ASSERT (axis.width(9) == 2);
  ASSERT (axis.lower(9) == 19);
  ASSERT (axis.upper(9) == 21);
  ASSERT (axis.range().first == 1);
  ASSERT (axis.range().second == 21);
  pair<double,bool> res = axis.find(0.5);
  ASSERT (res.first == 0  &&  !res.second);
  ASSERT (axis.locate(1.5) == 0);
  ASSERT (axis.locate(20) == 9);
  pair<double,bool> res2 = axis.find(200);
  ASSERT (res2.first == 10  &&  !res2.second);
  ASSERT (axis.locate(19, true) == 9);
  ASSERT (axis.locate(19, false) == 8);
  {
    // Check subset.
    size_t sp;
    Axis::ShPtr sub = axis.subset (3.5, 9.5, sp);
    ASSERT (sp == 1);
    ASSERT (sub->size() == 4);
    ASSERT (sub->lower(0) == 3);
    ASSERT (sub->upper(3) == 11);
    ASSERT (sub->width(2) == 2);
  }
}

// Test an axis of 5 intervals with gaps.
template<typename AxisType>
void checkGap5 (const AxisType& axis)
{
  ASSERT (axis == axis);
  ASSERT (axis.checkIntervals (axis));
  ASSERT (axis.size() == 5);
  ASSERT (axis.center(0) == 0.5);
  ASSERT (axis.width(0) == 1);
  ASSERT (axis.lower(0) == 0);
  ASSERT (axis.upper(0) == 1);
  ASSERT (axis.center(4) == 22.5);
  ASSERT (axis.width(4) == 5);
  ASSERT (axis.lower(4) ==20);
  ASSERT (axis.upper(4) ==25);
  ASSERT (axis.range().first == 0);
  ASSERT (axis.range().second == 25);
  ASSERT (axis.locate(0.5) == 0);
  pair<double,bool> res1 = axis.find(-1.5);
  ASSERT (res1.first == 0  &&  !res1.second);
  ASSERT (axis.locate(20) == 4);
  pair<double,bool> res2 = axis.find(200);
  ASSERT (res2.first == 5  &&  !res2.second);
  pair<double,bool> res3 = axis.find(12, false);
  ASSERT (res3.first == 3  &&  !res3.second);
  ASSERT (axis.locate(12, true) == 3);
  ASSERT (axis.locate(16, false) == 3);
  pair<double,bool> res4 = axis.find(16, true);
  ASSERT (res4.first == 4  &&  !res4.second);
}

void checkCombAxis (const Axis& axis)
{
  ASSERT (!axis.isRegular());
  ASSERT (axis.size() == 9);
  double val=1;
  for (int i=0; i<9; ++i) {
    ASSERT (axis.lower(i) == val);
    val += 2;
    if (i == 5) val += 1;
    ASSERT (axis.upper(i) == val);
  }
}

void testRegular()
{
  RegularAxis axis1;
  ASSERT (axis1.getId() == 0);
  check1  (axis1);
  RegularAxis axis2(1,2,10);
  ASSERT (axis2.getId() == 3);      // check1 creates 2 Axis objects
  check10 (axis2, true);
  RegularAxis axis(1,21,10,true);
  ASSERT (axis.getId() == 7);
  check10 (axis, true);             // check10 creates 3 Axis objects
  Axis::ShPtr clonedAxis(axis.clone());
  ASSERT (clonedAxis->getId() == 7);
  check10 (*clonedAxis, true);
  int s1,s2,e1,e2;
  Axis::ShPtr combAxis = axis.combine(axis, s1, e1, s2, e2);
  ASSERT (s1==0 && s2==0 && e1==10 && e2==10);
  check10 (*combAxis, true);
}

void testOrdered()
{
  check1 (OrderedAxis());
  // 10 intervals of width 2 starting at 1.
  vector<double> cen(10);
  for (uint i=0; i<cen.size(); ++i) {
    cen[i] = 2*(i+1);
  }
  OrderedAxis axis(cen, vector<double>(10,2));
  check10 (axis, false);
  Axis::ShPtr clonedAxis(axis.clone());
  check10 (*clonedAxis, false);
  int s1,s2,e1,e2;
  Axis::ShPtr combAxis = axis.combine(axis, s1, e1, s2, e2);
  ASSERT (s1==0 && s2==0 && e1==10 && e2==10);
  check10 (*combAxis, false);
  // 5 intervals of varying size with gaps between intervals.
  vector<double> s(5);
  vector<double> e(5);
  double v = 0;
  for (uint i=0; i<s.size(); ++i) {
    s[i] = v;
    e[i] = v+i+1;
    v += 2*(i+1);
  }
  OrderedAxis axis2(s, e, true);
  checkGap5 (axis2);
  combAxis = axis2.combine(axis2, s1, e1, s2, e2);
  ASSERT (s1==0 && s2==0 && e1==5 && e2==5);
  checkGap5 (*combAxis);
  // Combining axis ands axis2 should fail.
  bool fail = false;
  try {
    combAxis = axis2.combine(axis, s1, e1, s2, e2);
  } catch (std::exception& x) {
    cout << "Expected exception: " << x.what() << endl;
    fail = true;
  }
  ASSERT (fail);
  fail = false;
  try {
    combAxis = axis.combine(axis2, s1, e1, s2, e2);
  } catch (std::exception& x) {
    cout << "Expected exception: " << x.what() << endl;
    fail = true;
  }
  ASSERT (fail);
}

void testCombine()
{
  // Create various axes which overlap or not in various ways.
  // They all have width 2.
  RegularAxis axis1_21 (1, 21,10,true);
  RegularAxis axis1_11 (1, 11,5, true);
  RegularAxis axis11_21(11,21,5, true);
  RegularAxis axis7_21 (7, 21,7, true);
  RegularAxis axis9_19 (9, 19,5, true);
  RegularAxis axis13_21(13,21,4, true);
  RegularAxis axis14_20(14,20,3, true);
  // Combine in all kind of ways.
  int s1, s2, e1, e2;
  Axis::ShPtr combAxis;
  // Two overlapping intervals with same start.
  combAxis = axis1_21.combine (axis1_11, s1, e1, s2, e2);
  ASSERT (*combAxis == axis1_21);
  ASSERT(s1==0 && e1==10 && s2==0 && e2==5);
  combAxis = axis1_11.combine (axis1_21, s1, e1, s2, e2);
  ASSERT (*combAxis == axis1_21);
  ASSERT(s1==0 && e1==5 && s2==0 && e2==10);
  // Two overlapping intervals with same end.
  combAxis = axis1_21.combine (axis11_21, s1, e1, s2, e2);
  ASSERT (*combAxis == axis1_21);
  ASSERT(s1==0 && e1==10 && s2==5 && e2==10);
  combAxis = axis11_21.combine (axis1_21, s1, e1, s2, e2);
  ASSERT (*combAxis == axis1_21);
  ASSERT(s1==5 && e1==10 && s2==0 && e2==10);
  // An interval fully contained in the other.
  combAxis = axis1_21.combine (axis9_19, s1, e1, s2, e2);
  ASSERT (*combAxis == axis1_21);
  ASSERT(s1==0 && e1==10 && s2==4 && e2==9);
  combAxis = axis9_19.combine (axis1_21, s1, e1, s2, e2);
  ASSERT (*combAxis == axis1_21);
  ASSERT(s1==4 && e1==9 && s2==0 && e2==10);
  // Two adjacent intervals.
  combAxis = axis1_11.combine (axis11_21, s1, e1, s2, e2);
  ASSERT (*combAxis == axis1_21);
  ASSERT(s1==0 && e1==5 && s2==5 && e2==10);
  combAxis = axis11_21.combine (axis1_11, s1, e1, s2, e2);
  ASSERT (*combAxis == axis1_21);
  ASSERT(s1==5 && e1==10 && s2==0 && e2==5);
  // Two disjoint intervals at nice distance of 2.
  combAxis = axis1_11.combine (axis13_21, s1, e1, s2, e2);
  ASSERT (*combAxis == axis1_21);
  ASSERT(s1==0 && e1==5 && s2==6 && e2==10);
  combAxis = axis13_21.combine (axis1_11, s1, e1, s2, e2);
  ASSERT (*combAxis == axis1_21);
  ASSERT(s1==6 && e1==10 && s2==0 && e2==5);
  // Two disjoint intervals at not so nice distance of 3.
  combAxis = axis1_11.combine (axis14_20, s1, e1, s2, e2);
  checkCombAxis (*combAxis);
  ASSERT(s1==0 && e1==5 && s2==6 && e2==9);
  combAxis = axis14_20.combine (axis1_11, s1, e1, s2, e2);
  checkCombAxis (*combAxis);
  ASSERT(s1==6 && e1==9 && s2==0 && e2==5);
  // Two partly overlapping intervals.
  combAxis = axis1_11.combine (axis7_21, s1, e1, s2, e2);
  ASSERT (*combAxis == axis1_21);
  ASSERT(s1==0 && e1==5 && s2==3 && e2==10);
  combAxis = axis7_21.combine (axis1_11, s1, e1, s2, e2);
  ASSERT (*combAxis == axis1_21);
  ASSERT(s1==3 && e1==10 && s2==0 && e2==5);
  // Check interval matching.
  ASSERT (axis1_11.checkIntervals (axis1_21));
  ASSERT (axis1_21.checkIntervals (axis1_11));
  ASSERT (axis1_11.checkIntervals (axis7_21));
  ASSERT (axis7_21.checkIntervals (axis1_11));
  ASSERT (! axis14_20.checkIntervals (axis1_21));
  ASSERT (! axis1_21.checkIntervals (axis14_20));
  ASSERT (axis1_11.checkIntervals (axis13_21));
  ASSERT (axis13_21.checkIntervals (axis1_11));
}

int main()
{
  try {
    INIT_LOGGER("tAxis");
    cout << "testing RegularAxis ..." << endl;
    testRegular();
    cout << "testing OrderedAxis ..." << endl;
    testOrdered();
    cout << "testing Axis::combine ..." << endl;
    testCombine();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
