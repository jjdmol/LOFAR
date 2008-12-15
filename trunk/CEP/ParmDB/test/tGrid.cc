//# tGrid.cc: Program to test the Grid classes
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
#include <ParmDB/Grid.h>
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace std;

void check (const Grid& grid)
{
  ASSERT (grid.nx() == 5);
  ASSERT (grid.ny() == 8);
  ASSERT (grid.size() == 40);
  ASSERT (grid.shape() == make_pair(size_t(5), size_t(8)));
  ASSERT (grid.getCellId(Location(0,0)) == 0);
  ASSERT (grid.getCellId(Location(0,7)) == 35);
  ASSERT (grid.getCellId(Location(1,0)) == 1);
  ASSERT (grid.getCellId(Location(4,7)) == 39);
  ASSERT (grid.getCellId(Location(2,6)) == 32);
  ASSERT (grid.getCellLocation(32) == Location(2,6));
  ASSERT (grid.getCellCenter(Location(2,6)) == Point(5,85));
  ASSERT (grid.getCell(Location(2,6)) == Box(Point(4,80),Point(6,90)));
  ASSERT (grid.getCell(32) == Box(Point(4,80),Point(6,90)));
  ASSERT (grid.getBoundingBox() == Box(Point(0,20),Point(10,100)));
  ASSERT (grid.getBoundingBox(Location(1,2), Location(4,3)) ==
          Box(Point(2,40),Point(10,60)));
  ASSERT (grid.locate (Point(3,45)) == Location(1,2));
  ASSERT (grid.locate (Point(2,40), true) == Location(1,2));
  ASSERT (grid.locate (Point(2,40), false) == Location(0,1));
  ASSERT (grid.checkIntervals (grid));
}

void checkDomains (const Grid& grid, const vector<Box>& domains)
{
  // Check the toDomains function.
  vector<Box> cdomains;
  grid.toDomains (cdomains);
  ASSERT (domains.size() == cdomains.size());
  for (uint i=0; i<domains.size(); ++i) {
    ASSERT (domains[i] == cdomains[i]);
  }
}

void testGrid()
{
  Axis::ShPtr ax0 (new RegularAxis(0,2,5));
  Axis::ShPtr ax1 (new RegularAxis(20,10,8));
  Grid grid(ax0, ax1);
  check (grid);
  // Check assignment.
  Grid grid2;
  grid2 = grid;
  check (grid2);
  // Check copy constructor.
  Grid grid3(grid);
  check (grid3);
}

void testOneSet()
{
  vector<Box> domains;
  domains.push_back (Box(Point(1,2), Point(3,5)));
  Grid grid(domains, false);
  ASSERT (grid.size() == 1);
  Box box = grid.getCell(0);
  ASSERT (grid.getCell(0) == Box(Point(1,2), Point(3,5)));
  ASSERT (grid.getAxis(0)->classType() == "RegularAxis");
  ASSERT (grid.getAxis(1)->classType() == "RegularAxis");
  ASSERT (grid.hash() == 11);
  ASSERT (grid.hash() == Grid::hash(domains));
  checkDomains (grid, domains);
}

void testRegularSet()
{
  vector<Box> domains;
  for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j) {
      domains.push_back (Box(Point(j*2,i*3), Point(j*2+2,i*3+3)));
    }
  }
  Grid grid(domains, false);
  ASSERT (grid.size() == 12);
  int k=0;
  for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j) {
      Box box = grid.getCell(k++);
      ASSERT (box == Box(Point(j*2,i*3), Point(j*2+2,i*3+3)));
    }
  }
  ASSERT (grid.getAxis(0)->classType() == "RegularAxis");
  ASSERT (grid.getAxis(1)->classType() == "RegularAxis");
  checkDomains (grid, domains);
}

void testIrregularXSet()
{
  vector<Box> domains;
  for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j) {
      domains.push_back (Box(Point(j*2+1,i*3), Point(j*2+2,i*3+3)));
    }
  }
  Grid grid(domains, true);
  ASSERT (grid.size() == 12);
  int k=0;
  for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j) {
      Box box = grid.getCell(k++);
      ASSERT (box == Box(Point(j*2+1,i*3), Point(j*2+2,i*3+3)));
    }
  }
  ASSERT (grid.getAxis(0)->classType() == "OrderedAxis");
  ASSERT (grid.getAxis(1)->classType() == "RegularAxis");
  ASSERT (grid.hash() == Grid::hash(domains));
  checkDomains (grid, domains);
}

void testIrregularYSet()
{
  vector<Box> domains;
  for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j) {
      domains.push_back (Box(Point(j*2,i*3+1), Point(j*2+2,i*3+3)));
    }
  }
  Grid grid(domains);
  ASSERT (grid.size() == 12);
  int k=0;
  for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j) {
      Box box = grid.getCell(k++);
      ASSERT (box == Box(Point(j*2,i*3+1), Point(j*2+2,i*3+3)));
    }
  }
  ASSERT (grid.getAxis(0)->classType() == "RegularAxis");
  ASSERT (grid.getAxis(1)->classType() == "OrderedAxis");
  ASSERT (grid.hash() == Grid::hash(domains));
  checkDomains (grid, domains);
}

void testIrregularXYSet()
{
  vector<Box> domains;
  for (int j=0; j<4; ++j) {
    for (int i=0; i<3; ++i) {
      domains.push_back (Box(Point(j*20,i*30), Point(j*21+2,i*31+3)));
    }
  }
  Grid grid(domains, true);
  ASSERT (grid.size() == 12);
  int k=0;
  for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j) {
      Box box = grid.getCell(k++);
      ASSERT (box == Box(Point(j*20,i*30), Point(j*21+2,i*31+3)));
    }
  }
  ASSERT (grid.getAxis(0)->classType() == "OrderedAxis");
  ASSERT (grid.getAxis(1)->classType() == "OrderedAxis");
  ASSERT (grid.hash() == Grid::hash(domains));
  sort (domains.begin(), domains.end());
  checkDomains (grid, domains);
  // Check hash for vector of grids.
  vector<Grid> grids;
  grids.push_back (grid);
  grids.push_back (grid);
  grids.push_back (grid);
  ASSERT (Grid::hash(grids) == 3*grid.hash());
}

void testCombineGrids()
{
  {
    vector<Grid> grids;
    grids.push_back (Grid (Axis::ShPtr (new RegularAxis(1,2,4)),
                           Axis::ShPtr (new RegularAxis(1,20,40))));
    grids.push_back (Grid (Axis::ShPtr (new RegularAxis(9,2,5)),
                           Axis::ShPtr (new RegularAxis(1,20,40))));
    grids.push_back (Grid (Axis::ShPtr (new RegularAxis(1,2,4)),
                           Axis::ShPtr (new RegularAxis(900,20,10))));
    grids.push_back (Grid (Axis::ShPtr (new RegularAxis(9,2,5)),
                           Axis::ShPtr (new RegularAxis(900,20,10))));
    Grid grid(grids, true);
    ASSERT (grid[0]->isRegular());
    ASSERT (grid[0]->size() == 9);
    ASSERT (grid[0]->width(0) == 2);
    ASSERT (grid[0]->lower(0) == 1);
    ASSERT (grid[0]->upper(8) == 19);
    ASSERT (!grid[1]->isRegular());
    ASSERT (grid[1]->size() == 50);
    for (uint i=0; i<40; ++i) {
      ASSERT (grid[1]->width(i) == 20);
      ASSERT (grid[1]->lower(i) == 1 + i*20);
      ASSERT (grid[1]->upper(i) == 21+ i*20);
    }
    for (uint i=40; i<50; ++i) {
      ASSERT (grid[1]->width(i) == 20);
      ASSERT (grid[1]->lower(i) == 100 + i*20);
      ASSERT (grid[1]->upper(i) == 120 + i*20);
    }
  }
}

int main()
{
  try {
    INIT_LOGGER("tGrid");
    cout << "Testing Grid basics ..." << endl;
    testGrid();
    cout << "Testing OneSet ..." << endl;
    testOneSet();
    cout << "Testing RegularSet ..." << endl;
    testRegularSet();
    cout << "Testing IrregularXSet ..." << endl;
    testIrregularXSet();
    cout << "Testing IrregularYSet ..." << endl;
    testIrregularYSet();
    cout << "Testing IrregularXYSet ..." << endl;
    testIrregularXYSet();
    cout << "Testing CombineGrids ..." << endl;
    testCombineGrids();
  } catch (exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
