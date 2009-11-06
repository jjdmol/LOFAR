//# Grid.cc:
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

namespace LOFAR {
namespace BBS {


  GridRep::GridRep()
    : itsIsDefault (true)
  {
    itsAxes[0] = Axis::ShPtr(new RegularAxis());
    itsAxes[1] = Axis::ShPtr(new RegularAxis());
    init();
  }
    
  GridRep::GridRep(Axis::ShPtr first, Axis::ShPtr second)
    : itsIsDefault (false)
  {
    itsAxes[0] = first;
    itsAxes[1] = second;
    init();
  }
    
  GridRep::GridRep (const vector<Box>& domains, bool unsorted)
    : itsIsDefault (false)
  {
    if (unsorted) {
      vector<Box> sortDomains(domains);
      sort (sortDomains.begin(), sortDomains.end());
      setup (sortDomains);
    } else {
      setup (domains);
    }
    init();
  }

  GridRep::GridRep (const vector<Grid>& grids, bool unsorted)
    : itsIsDefault (false)
  {
    if (unsorted) {
      vector<Grid> sortGrids(grids);
      sort (sortGrids.begin(), sortGrids.end());
      setup (sortGrids);
    } else {
      setup (grids);
    }
    init();
  }

  void GridRep::setup (const vector<Box>& domains)
  {
    double sx = domains[0].lowerX();
    double ex = domains[0].upperX();
    double dx = ex-sx;
    double sy = domains[0].lowerY();
    double ey = domains[0].upperY();
    double dy = ey-sy;
    // Determine nr of cells in X and test if the cells are regular or not.
    // They are regular if equal width and consecutive.
    uint nx = 1;
    bool xregular = true;
    while (nx < domains.size()) {
      if (sy != domains[nx].lowerY()) {
        break;
      }
      if (! (casa::near(ex, domains[nx].lowerX())
         &&  casa::near(dx, domains[nx].upperX() - domains[nx].lowerX()))) {
        xregular = false;
      }
      ex = domains[nx].upperX();
      ++nx;
    }
    // Assure there is an integer nr of y domains.
    ASSERT (domains.size() % nx == 0);
    uint ny = domains.size() / nx;
    // Check if the x axis is the same for all y-s.
    vector<double> xaxisStart;
    vector<double> xaxisEnd;
    xaxisStart.reserve (nx);
    xaxisEnd.reserve (nx);
    for (uint i=0; i<nx; ++i) {
      xaxisStart.push_back (domains[i].lowerX());
      xaxisEnd.push_back   (domains[i].upperX());
    }
    for (uint j=1; j<ny; ++j) {
      for (uint i=0; i<nx; ++i) {
        uint inx = j*nx + i;
        ASSERT (casa::near(xaxisStart[i], domains[inx].lowerX()));
        ASSERT (casa::near(xaxisEnd[i], domains[inx].upperX()));
      }
    }
    // Determine the start/end for Y and if it is regular.
    // Check if the y axis is the same for all x-s.
    vector<double> yaxisStart;
    vector<double> yaxisEnd;
    yaxisStart.reserve (ny);
    yaxisEnd.reserve (ny);
    bool yregular = true;
    ey = sy;
    for (uint i=0; i<ny; ++i) {
      uint inx = i*nx;
      yaxisStart.push_back (domains[inx].lowerY());
      yaxisEnd.push_back (domains[inx].upperY());
      if (! (casa::near(ey, domains[inx].lowerY())
         &&  casa::near(dy, domains[inx].upperY() - domains[inx].lowerY()))) {
        yregular = false;
      }
      ey = domains[inx].upperY();
    }
    for (uint j=0; j<ny; ++j) {
      for (uint i=1; i<nx; ++i) {
        uint inx = j*nx + i;
        ASSERT (casa::near(yaxisStart[j], domains[inx].lowerY()));
        ASSERT (casa::near(yaxisEnd[j], domains[inx].upperY()));
      }
    }
    // Create the (ir)regular axis.
    // Note that OrderedAxis checks if the intervals are ordered.
    if (xregular) {
      itsAxes[0] = Axis::ShPtr(new RegularAxis (xaxisStart[0], dx, nx));
    } else {
      itsAxes[0] = Axis::ShPtr(new OrderedAxis (xaxisStart, xaxisEnd, true));
    }
    if (yregular) {
      itsAxes[1] = Axis::ShPtr(new RegularAxis (yaxisStart[0], dy, ny));
    } else {
      itsAxes[1] = Axis::ShPtr(new OrderedAxis (yaxisStart, yaxisEnd, true));
    }
  }

  void GridRep::setup (const vector<Grid>& grids)
  {
    // First form a grid from the bounding boxes.
    // This checks if it is regular and divides in x and y.
    vector<Box> domains;
    domains.reserve (grids.size());
    for (vector<Grid>::const_iterator iter = grids.begin();
         iter != grids.end(); ++iter) {
      domains.push_back (iter->getBoundingBox());
    }
    setup (domains);
    // Now combine the grid axes.
    uint nx = itsAxes[0]->size();
    uint ny = itsAxes[1]->size();
    itsAxes[0] = combineAxes (grids, 0, nx, 1);
    itsAxes[1] = combineAxes (grids, 1, ny, nx);
    // Check if the grids themselves are equal for all x and y.
    // Check if the x axis is the same for all y-s.
    Axis::ShPtr xaxis = grids[0].getAxis(0);
    for (uint j=1; j<ny; ++j) {
      for (uint i=0; i<nx; ++i) {
        uint inx = j*nx + i;
        ASSERT (*grids[i].getAxis(0) == *grids[inx].getAxis(0));
      }
    }
    // Check if the y axis is the same for all x-s.
    for (uint j=0; j<ny; ++j) {
      uint inx = j*nx;
      for (uint i=1; i<nx; ++i) {
        ASSERT (*grids[inx].getAxis(1) == *grids[inx+i].getAxis(1));
      }
    }
  }

  Axis::ShPtr GridRep::combineAxes (const vector<Grid>& grids, uint axnr,
                                    uint n, uint step) const
  {
    // Nothing to be done if only one cell.
    const Axis::ShPtr& faxis = grids[0].getAxis(axnr);
    if (n == 1) {
      return faxis;
    }
    // Count total number of cells and check if fully regular.
    bool isRegular = faxis->isRegular();
    double width   = faxis->width(0);
    double last    = faxis->upper(faxis->size() - 1);
    uint ncells    = faxis->size();
    for (uint i=1; i<n; ++i) {
      const Axis::ShPtr& axis = grids[i*step].getAxis(axnr);
      ncells += axis->size();
      if (isRegular) {
        isRegular = axis->isRegular() && (casa::near(width, axis->width(0)) &&
                                          casa::near(last,  axis->lower(0)));
        last = axis->upper(axis->size() - 1);
      }
    }
    // If regular, return as such.
    if (isRegular) {
      return Axis::ShPtr(new RegularAxis (faxis->lower(0), width, ncells));
    }
    // Alas irregular, so create an ordered axis.
    vector<double> starts;
    vector<double> ends;
    starts.reserve (ncells);
    ends.reserve   (ncells);
    for (uint i=0; i<n; ++i) {
      const Axis::ShPtr& axis = grids[i*step].getAxis(axnr);
      for (uint j=0; j<axis->size(); ++j) {
        starts.push_back (axis->lower(j));
        ends.push_back   (axis->upper(j));
      }
    }
    return Axis::ShPtr(new OrderedAxis (starts, ends, true));
  }

  void GridRep::init()
  {
    // Calculate the hash value as a set of individual domains.
    // Thus add up the start and end values of all cells.
    const Axis& x = *itsAxes[0];
    const Axis& y = *itsAxes[1];
    int64 xval = 0;
    int64 yval = 0;
    for (uint i=0; i<x.size(); ++i) {
      xval += int64(x.lower(i)) + int64(x.upper(i));
    }
    for (uint i=0; i<y.size(); ++i) {
      yval += int64(y.lower(i)) + int64(y.upper(i));
    }
    itsHash = x.size() * yval + y.size() * xval;
  }



  Grid::Grid (const vector<Grid>& grids, bool unsorted)
  {
    // If only one entry, we can simply make a copy.
    if (grids.size() == 1) {
      this->operator= (grids[0]);
    } else {
      itsRep = GridRep::ShPtr (new GridRep(grids, unsorted));
    }
  }

  bool Grid::operator== (const Grid& that) const
  {
    if (&(*itsRep) == &(*that.itsRep)) return true;
    if (hash()     != that.hash())     return false;
    if (getAxis(0) != that.getAxis(0)) return false;
    if (getAxis(1) != that.getAxis(1)) return false;
    return true;
  }

  bool Grid::checkIntervals (const Grid& that) const
  {
    // Check per axis.
    return (getAxis(0)->checkIntervals (*that.getAxis(0))  &&
            getAxis(1)->checkIntervals (*that.getAxis(1)));
  }

  int64 Grid::hash (const vector<Grid>& grids)
  {
    double val = 0;
    for (uint i=0; i<grids.size(); ++i) {
      val += grids[i].hash();
    }
    return val;
  }

  int64 Grid::hash (const vector<Box>& domains)
  {
    double val = 0;
    for (uint i=0; i<domains.size(); ++i) {
      const Box& box = domains[i];
      val += int64(box.lowerX()) + int64(box.upperX()) +
             int64(box.lowerY()) + int64(box.upperY());
    }
    return val;
  }

  Grid Grid::subset (const Box& domain) const
  {
    Location index;
    return subset(domain, index);
  }

  Grid Grid::subset (const Box& domain, Location& index) const
  {
    return Grid (getAxis(0)->subset (domain.lowerX(),
                                     domain.upperX(), index.first),
                 getAxis(1)->subset (domain.lowerY(),
                                     domain.upperY(), index.second));
  }

  Grid Grid::subset (const Location& start, const Location& end) const
  {
    DBGASSERT(start.first <= end.first  &&  start.second <= end.second);
    return Grid (getAxis(0)->subset (start.first, end.first),
                 getAxis(1)->subset (start.second, end.second));
  }

  void Grid::toDomains (vector<Box>& domains) const
  {
    const Axis& xaxis = *(getAxis(0));
    const Axis& yaxis = *(getAxis(1));
    uint nrx = nx();
    uint nry = ny();
    // Prefetch the lower and upper values for both axes.
    vector<double> sx(nrx), ex(nrx);
    vector<double> sy(nrx), ey(nrx);
    for (uint i=0; i<nrx; ++i) {
      sx[i] = xaxis.lower(i);
      ex[i] = xaxis.upper(i);
    }
    for (uint i=0; i<nry; ++i) {
      sy[i] = yaxis.lower(i);
      ey[i] = yaxis.upper(i);
    }
    // Create the domains and append them to the vector.
    domains.reserve (domains.size() + size());
    for (uint iy=0; iy<nry; ++iy) {
      for (uint ix=0; ix<nrx; ++ix) {
        domains.push_back (Box(Point(sx[ix], sy[iy]), Point(ex[ix], ey[iy])));
      }
    }
  }

} //# namespace BBS
} //# namespace LOFAR
