//# tBox.cc: Program to test the Box classes
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
#include <ParmDB/Box.h>
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace std;

void check (const Box& box, double sx, double sy, double ex, double ey)
{
  ASSERT (box.lower() == Point(sx,sy));
  ASSERT (box.lowerX() == sx);
  ASSERT (box.lowerY() == sy);
  ASSERT (box.upper() == Point(ex,ey));
  ASSERT (box.upperX() == ex);
  ASSERT (box.upperY() == ey);
  ASSERT (box.widthX() == ex-sx);
  ASSERT (box.widthY() == ey-sy);
  ASSERT (box.contains (Box(Point(sx,sy), Point(ex,ey))));
  if (! box.empty()) {
    ASSERT (box.intersects (Box(Point(sx,sy), Point(ex,ey))));
  }
  ASSERT (box == Box(Point(sx,sy), Point(ex,ey)));
  ASSERT (! (box != Box(Point(sx,sy), Point(ex,ey))));
  ASSERT (! (box < Box(Point(sx,sy), Point(ex,ey))));
  ASSERT (! (box > Box(Point(sx,sy), Point(ex,ey))));
}

void testBox()
{
  check (Box(), 0, 0, 0, 0);
  check (Box(1,2,3,4,true), 1,3,2,4);
  check (Box(1,2,3,4), 0,1,2,5);
  Box box(Point(1,2), Point(3,4));
  check (box, 1, 2, 3, 4);
  Box box2;
  ASSERT (box2.empty());
  ASSERT (!(box == box2));
  ASSERT (! box.intersects(box2));
  box2 = box;
  ASSERT (!box2.empty());
  check (box2, 1, 2, 3, 4);
  ASSERT (box == box2);
  ASSERT (box.intersects(box2));
  ASSERT (box.intersects (Box(Point(1.5,2.5), Point(4,4))));
  ASSERT (!box.intersects (Box(Point(1.5,1.5), Point(4,1.95))));
  Box box3 (Point(1.5,2.5), Point(4,5));
  ASSERT ((box & box3)  ==  Box(Point(1.5,2.5), Point(3,4)));
  ASSERT ((box | box3)  ==  Box(Point(1,2), Point(4,5)));
  ASSERT (box.contains (Box(Point(1.1, 2.1), Point(2.9, 3.9))));
  ASSERT (!box.contains (Box(Point(1.1, 2.1), Point(2.9, 4.1))));
}

void testVec()
{
  Box box1((vector<double>()));
  ASSERT (box1.lowerX()==-1e30 && box1.lowerY()==-1e30);
  ASSERT (box1.upperX()== 1e30 && box1.upperY()== 1e30);
  Box box2(vector<double>(1,2.5));
  ASSERT (box2.lowerX()== 2.5  && box2.lowerY()==-1e30);
  ASSERT (box2.upperX()== 1e30 && box2.upperY()== 1e30);
  Box box3(vector<double>(4,2.5));
  ASSERT (box3.lowerX()== 2.5  && box3.lowerY()== 2.5);
  ASSERT (box3.upperX()== 2.5  && box3.upperY()== 2.5);
}

void testSort()
{
  vector<Box> boxes;
  boxes.push_back (Box(Point(0,0), Point(5,5)));
  boxes.push_back (Box(Point(0,5), Point(5,10)));
  boxes.push_back (Box(Point(5,0), Point(10,5)));
  boxes.push_back (Box(Point(10,0), Point(15,5)));
  boxes.push_back (Box(Point(10,5), Point(15,10)));
  boxes.push_back (Box(Point(5,5), Point(10,10)));
  sort (boxes.begin(), boxes.end());
  ASSERT (boxes.size() == 6);
  ASSERT (boxes[0].lowerX() == 0  &&  boxes[0].lowerY() == 0);
  ASSERT (boxes[1].lowerX() == 5  &&  boxes[1].lowerY() == 0);
  ASSERT (boxes[2].lowerX() == 10 &&  boxes[2].lowerY() == 0);
  ASSERT (boxes[3].lowerX() == 0  &&  boxes[3].lowerY() == 5);
  ASSERT (boxes[4].lowerX() == 5  &&  boxes[4].lowerY() == 5);
  ASSERT (boxes[5].lowerX() == 10 &&  boxes[5].lowerY() == 5);
}

int main()
{
  try {
    INIT_LOGGER("tBox");
    testBox();
    testVec();
    testSort();
  } catch (exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
