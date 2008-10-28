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
    vector<uint> index(domains.size());
    uint* indexp = &(index[0]);
    for (uint i=0; i<domains.size(); ++i) {
      indexp[i] = i;
    }
    if (unsorted) {
      vector<Box> sortDomains(domains);
      sort (sortDomains.begin(), sortDomains.end());
      setup (sortDomains, indexp);
    } else {
      setup (domains, indexp);
    }
    init();
  }

  void GridRep::setup (const vector<Box>& domains, const uint* index)
  {
    uint first = index[0];
    double sx = domains[first].lowerX();
    double ex = domains[first].upperX();
    double dx = ex-sx;
    double sy = domains[first].lowerY();
    double ey = domains[first].upperY();
    double dy = ey-sy;
    // Determine nr of cells in X and test if the cells are regular or not.
    // They are regular if equal width and consecutive.
    uint nx = 1;
    bool xregular = true;
    while (nx < domains.size()) {
      uint inx = index[nx];
      if (sy != domains[inx].lowerY()) {
        break;
      }
      if (! (casa::near(ex, domains[inx].lowerX())
         &&  casa::near(dx, domains[inx].upperX() - domains[inx].lowerX()))) {
        xregular = false;
      }
      ex = domains[inx].upperX();
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
      uint inx = index[i];
      xaxisStart.push_back (domains[inx].lowerX());
      xaxisEnd.push_back (domains[inx].upperX());
    }
    for (uint j=1; j<ny; ++j) {
      for (uint i=0; i<nx; ++i) {
        uint inx = index[j*nx + i];
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
      uint inx = index[i*nx];
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
        uint inx = index[j*nx + i];
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

  void GridRep::init()
  {
    // Calculate the hash value as a set of individual domains.
    // Thus add up the start and end values of all cells.
    const Axis& x = *itsAxes[0];
    const Axis& y = *itsAxes[1];
    double xval = 0;
    double yval = 0;
    for (uint i=0; i<x.size(); ++i) {
      xval += int64(x.lower(i)) + int64(x.upper(i));
    }
    for (uint i=0; i<y.size(); ++i) {
      yval += int64(y.lower(i)) + int64(y.upper(i));
    }
    itsHash = x.size() * yval + y.size() * xval;
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
