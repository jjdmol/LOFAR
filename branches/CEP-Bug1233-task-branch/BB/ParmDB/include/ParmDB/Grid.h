//# Grid.h: Class representing a regular or irregular 2-D grid.
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_PARMDB_GRID_H
#define LOFAR_PARMDB_GRID_H

#include <ParmDB/Box.h>
#include <ParmDB/Axis.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_vector.h>

namespace LOFAR {
namespace BBS {

  // Forward declaration.
  class Grid;

  // Define Location: A location on a 2-D grid.
  typedef pair<size_t, size_t> Location;


  // A 2-D grid with regular or irregular axes.                          -
  class GridRep
  {
  public:
    typedef shared_ptr<GridRep> ShPtr;

    // Default constructor uses two default RegularAxis objects.
    GridRep();

    // Create from given axes.
    GridRep(Axis::ShPtr first, Axis::ShPtr second);
    
    // Create a grid from a series of domains.
    // They have to be in order of startY,startX. If unsorted, they are sorted.
    // The domains in the vector must span a grid, otherwise an exception is
    // thrown.
    // Its axes can be regular (RegularAxis) or irregular (OrderedAxis).
    GridRep(const vector<Box>& domains, bool unsorted=false);

    // Create a grid from a series of domains.
    // They have to be in order of startY,startX which is given by the index.
    // The domains in the vector must span a grid, otherwise an exception is
    // thrown.
    // Its axes can be regular (RegularAxis) or irregular (OrderedAxis).
    GridRep(const vector<Box>& domains, const uint* indexVector)
      { setup (domains, indexVector); }

    // Is it the default grid?
    bool isDefault() const
      { return itsIsDefault; }

    // Get the hash value of the grid.
    int64 hash() const
      { return itsHash; }

    // Get the given axis.
    // <group>
    Axis::ShPtr getAxis (size_t n)
      { return itsAxes[n]; }
    const Axis::ShPtr getAxis (size_t n) const
      { return itsAxes[n]; }
    // </group>

  private:
    // Initialize from an ordered vector of domains.
    // The order is given by the index which has the same length as the vector.
    void setup (const vector<Box>& domains, const uint* index);

    // Set the id.
    void init();

    //# Data members.
    Axis::ShPtr itsAxes[2];
    //# The hash value of the grid (for faster comparison).
    int64       itsHash;
    // Is it the default grid?
    bool        itsIsDefault;
  };


  // A 2-D grid with regular or irregular axes.                          -
  class Grid
  {
  public:
    // Default constructor creates empty axes.
    Grid()
      : itsRep (new GridRep())
    {}

    // Create a grid using the given axes.
    Grid(Axis::ShPtr first, Axis::ShPtr second)
      : itsRep (new GridRep(first, second))
    {}

    // Create a grid from a series of domains.
    // They have to be in order of startY,startX. If unsorted, they are sorted.
    // The domains in the vector must span a grid, otherwise an exception is
    // thrown.
    // Its axes can be regular (RegularAxis) or irregular (OrderedAxis).
    Grid(const vector<Box>& domains, bool unsorted=false)
      : itsRep (new GridRep(domains, unsorted))
    {}

    // Create a grid from a series of domains.
    // They have to be in order of startY,startX which is given by the index.
    // The domains in the vector must span a grid, otherwise an exception is
    // thrown.
    // Its axes can be regular (RegularAxis) or irregular (OrderedAxis).
    Grid(const vector<Box>& domains, const uint* indexVector)
      : itsRep (new GridRep(domains, indexVector))
    {}

    // Check if two grids are equal. They are if their axes have the same
    // type and values.
    // <group>
    bool operator== (const Grid& that) const;
    bool operator!= (const Grid& that) const
      { return ! operator== (that); }
    // </group>

    // Check if the corresponding intervals in this and that grid are the same.
    bool checkIntervals (const Grid& that) const;

    // Is it the default grid?
    bool isDefault() const
    { return itsRep->isDefault(); }

    // Get the given axis.
    const Axis::ShPtr getAxis (size_t n) const
      { return itsRep->getAxis (n); }

    // Get the sizes of the axes.
    // <group>
    size_t nx() const
      { return getAxis(0)->size(); }
    size_t ny() const
      { return getAxis(1)->size(); }
    // </group>

    // Get the grid shape (nx,ny).
    pair<size_t, size_t> shape() const
      { return make_pair(nx(), ny()); }

    // Get the total number of cells.
    size_t size() const
      { return nx() * ny(); }

    // Get the cell id from an (x,y) location.
    uint getCellId (const Location& location) const
      { return location.second * nx() + location.first; }

    // Get the (x,y) location from a cell id.
    Location getCellLocation (uint id) const
      { return Location(id % nx(), id / nx()); }

    // Get the coordinates of the center of the given cell.
    Point getCellCenter (const Location& location) const
    {
      DBGASSERT(location.first < nx()  &&  location.second < ny());
      return Point (getAxis(0)->center(location.first),
		    getAxis(1)->center(location.second));
    }

    // Get the blc and trc coordinates of the given cell.
    // <group>
    Box getCell (const Location& location) const
    {
      DBGASSERT(location.first < nx()  &&  location.second < ny());
      return Box(Point(getAxis(0)->lower(location.first),
		       getAxis(1)->lower(location.second)),
		 Point(getAxis(0)->upper(location.first),
		       getAxis(1)->upper(location.second)));
    }

    
    Box getCell (uint id) const
      { return getCell(getCellLocation(id)); }
    // </group>

    // Get the bounding box of the grid.
    Box getBoundingBox() const
    {
      return Box(Point(getAxis(0)->start(), getAxis(1)->start()),
		 Point(getAxis(0)->end(),   getAxis(1)->end()));
    }

    // Get the bounding box of part of the grid.
    Box getBoundingBox (const Location& start, const Location& end) const
    {
      DBGASSERT(start.first <= end.first && start.second <= end.second);
      return unite(getCell(start), getCell(end));
    }

    // Give the (x,y) location of the cell containing the given point.
    // If the point is on the edge, the left or right cell is chosen
    // depending on the value of <src>biasRight</src>.
    Location locate (const Point& point, bool biasRight = true) const
    {
      return make_pair(getAxis(0)->locate(point.first, biasRight),
		       getAxis(1)->locate(point.second, biasRight));
    }

    // Apply the given domain to this grid.
    // It means that the subset of this grid is returned which is covered
    // by that domain.
    // Optionally the location of the start point in this grid is filled.
    // <group>
    Grid subset (const Box&) const;
    Grid subset (const Box&, Location& index) const;
    // </group>

    // Convert the grid to domain boxes and append them to the vector.
    void toDomains (vector<Box>& domains) const;

    // Get the hash value of the grid.
    int64 hash() const
      { return itsRep->hash(); }

    // Calculate the hash value of a set of grids.
    static int64 hash(const vector<Grid>&);

    // Calculate the hash value of a set of domains.
    static int64 hash(const vector<Box>&);

    // Define an ordering functions to be able to sort grids.
    // The ordering is on startY,startX.
    // <group>
    bool operator< (const Grid& that) const
      { return getBoundingBox().lowerY() < that.getBoundingBox().lowerY()
	  || (getBoundingBox().lowerY() == that.getBoundingBox().lowerY()  &&
	      getBoundingBox().lowerX() < that.getBoundingBox().lowerX()); }
    bool operator> (const Grid& that) const
      { return getBoundingBox().lowerY() > that.getBoundingBox().lowerY()
	  || (getBoundingBox().lowerY() == that.getBoundingBox().lowerY()  &&
	      getBoundingBox().lowerX() > that.getBoundingBox().lowerX()); }
    // </group>

  private:
    GridRep::ShPtr itsRep;
  };


} //# namespace BBS
} //# namespace LOFAR

#endif
