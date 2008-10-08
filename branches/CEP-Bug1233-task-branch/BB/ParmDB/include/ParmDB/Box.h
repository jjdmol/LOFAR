//# Box.h: Class representing a 2-dim box
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

// @file
// @brief Class representing a 2-dim box
// @author Ger van Diepen (diepen AT astron nl)

#ifndef LOFAR_PARMDB_BOX_H
#define LOFAR_PARMDB_BOX_H

#include <Common/lofar_map.h>
#include <Common/lofar_algorithm.h>
#include <Common/LofarLogger.h>
#include <casa/BasicMath/Math.h>

namespace LOFAR {
namespace BBS {

  // Point: A point in a 2-D space.
  typedef pair<double, double>    Point;


  // @ingroup ParmDB
  // @{

  // @brief Class representing a 2-dim box
  class Box;
  Box unite(const Box& lhs, const Box& rhs);
  Box intersect(const Box& lhs, const Box& rhs);


  // A Box is a rectangular region defined by two Point objects defining
  // the bottom-left and top-rigth corner of the box.
  // The bounding box of a Grid is a Box object.
  class Box
  {
  public:
    // Default constructor creates an empty box.
    Box()
      : itsStart(0, 0),
	itsEnd  (0, 0)
    {}

    // Create a box from the bottom-left and top-right corner.
    // both coordinates in start must be <= end.
    Box (const Point& start, const Point& end)
      : itsStart(start),
	itsEnd  (end)
    {
      ASSERT(start.first <= end.first && start.second <= end.second);
    }            

    // Test if boxes are exactly the same.
    // <group>
    bool operator== (const Box& that) const
      { return itsStart==that.itsStart  &&  itsEnd==that.itsEnd; }
    bool operator!= (const Box& that) const
      { return ! operator== (that); }
    // </group>

    // Get start and end values.
    // <group>
    const Point& lower() const
      { return itsStart; }
    const Point& upper() const
      { return itsEnd; }
    double lowerX() const
      { return itsStart.first; }
    double lowerY() const
      { return itsStart.second; }
    double upperX() const
      { return itsEnd.first; }
    double upperY() const
      { return itsEnd.second; }
    // </group>

    // Get widths.
    // <group>
    double widthX() const
      { return itsEnd.first - itsStart.first; }
    double widthY() const
      { return itsEnd.second - itsStart.second; }
    // </group>

    // Box A only intersects box B if there is at least one point within or
    // on the border of A that falls within B (excluding its border).
    bool intersects (const Box& other) const
    {
      return (other.itsStart.first < itsEnd.first
	      && !casa::near(other.itsStart.first, itsEnd.first)
	      && other.itsEnd.first > itsStart.first
	      && !casa::near(other.itsEnd.first, itsStart.first)
	      && other.itsStart.second < itsEnd.second
	      && !casa::near(other.itsStart.second, itsEnd.second)
	      && other.itsEnd.second > itsStart.second
	      && !casa::near(other.itsEnd.second, itsStart.second));
    }

    // A box A contains a box B if all points within or on the border of B
    // fall within or on the border of A.
    bool contains (const Box& other) const
    {
      return ((other.itsStart.first >= itsStart.first
	       || casa::near(other.itsStart.first, itsStart.first))
	      && (other.itsEnd.first <= itsEnd.first
		  || casa::near(other.itsEnd.first, itsEnd.first))
	      && (other.itsStart.second >= itsStart.second
		  || casa::near(other.itsStart.second, itsStart.second))
	      && (other.itsEnd.second <= itsEnd.second
		  || casa::near(other.itsEnd.second, itsEnd.second)));
    }

    //Check if the box is empty.
    bool empty() const
    {
      return (casa::near(itsStart.first, itsEnd.first)
	      || casa::near(itsStart.second, itsEnd.second));
    }

    // Return the intersection of this and that box. An empty box is
    // returned if the boxes are disjoint.
    // Note that the operator has a low precedence, so it is advised to
    // enclose the expression in parentheses.
    Box operator& (const Box& that) const
      { return intersect(*this, that); }

    // Return the union of this and that box.
    // The union also contains the points between disjoint boxes.
    // Note that the operator has a low precedence, so it is advised to
    // enclose the expression in parentheses.
    Box operator|(const Box& that) const
      { return unite(*this, that); }

    // Define an ordering functions to be able to sort boxes.
    // The ordering is on startY,startX.
    // <group>
    bool operator< (const Box& that) const
      { return itsStart.second < that.itsStart.second
	  ||  (itsStart.second == that.itsStart.second  &&
	       itsStart.first < that.itsStart.first); }
    bool operator> (const Box& that) const
      { return itsStart.second > that.itsStart.second
	  ||  (itsStart.second == that.itsStart.second  &&
	       itsStart.first > that.itsStart.first); }
    // </group>

  private:
    Point itsStart;
    Point itsEnd;
  };

  // @}

} //# namespace BBS
} //# namespace LOFAR

#endif
