//# Grid.h: Class template that represents a 2-D grid.
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

#ifndef LOFAR_BBS_BBSKERNEL_GRID_H
#define LOFAR_BBS_BBSKERNEL_GRID_H

#include <BBSKernel/Axis.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS
{
// Location: A location on a 2-D grid.
typedef pair<size_t, size_t>    Location;


// Point: A point in a 2-D space.
typedef pair<double, double>    Point;


// -----------------------------------------------------------------------------
// - Box: An axis aligned bounding box.                                        -
// -----------------------------------------------------------------------------
class Box;
Box unite(const Box &lhs, const Box &rhs);
Box intersect(const Box &lhs, const Box &rhs);


class Box
{
public:
    Box()
        :   start(0, 0),
            end(0, 0)
    {}

                
    Box(const Point &start, const Point &end)
        :   start(start),
            end(end)
    {
        DBGASSERT(start.first <= end.first && start.second <= end.second);
    }            


    bool intersects(const Box &other) const
    {
        // Box A only intersects box B if there is at least one point within or
        // on the border of A that falls within B (excluding its border).
        return (other.start.first < end.first
            && !casa::near(other.start.first, end.first)
            && other.end.first > start.first
            && !casa::near(other.end.first, start.first)
            && other.start.second < end.second
            && !casa::near(other.start.second, end.second)
            && other.end.second > start.second
            && !casa::near(other.end.second, start.second));
    }    


    bool contains(const Box &other) const
    {
        // A box A contains a box B if all points within or on the border of B
        // fall within or on the border of A.
        return ((other.start.first > start.first
                || casa::near(other.start.first, start.first))
            && (other.end.first < end.first
                || casa::near(other.end.first, end.first))
            && (other.start.second > start.second
                || casa::near(other.start.second, start.second))
            && (other.end.second < end.second
                || casa::near(other.end.second, end.second)));
    }
    
    bool empty()
    {
        return (casa::near(start.first, end.first)
            || casa::near(start.second, end.second));
    }

    Box operator&(const Box &other) const
    { return intersect(*this, other); }

    
    Box operator|(const Box &other) const
    { return unite(*this, other); }

    
    Point    start, end;
};


// -----------------------------------------------------------------------------
// - Grid: A 2-D grid with regular/irregular axes.                             -
// -----------------------------------------------------------------------------
class Grid
{
public:
    Grid()
    {}
    
    Grid(Axis::Pointer first, Axis::Pointer second)
    {
        itsAxes[0] = first;
        itsAxes[1] = second;
    }
    
    Axis::Pointer operator[](size_t n)
    { return itsAxes[n]; }
    
    const Axis::Pointer operator[](size_t n) const
    { return itsAxes[n]; }

    pair<size_t, size_t> size() const
    { return make_pair(itsAxes[0]->size(), itsAxes[1]->size()); }
    
    size_t getCellCount() const
    { return itsAxes[0]->size() * itsAxes[1]->size(); }
    
    uint getCellId(const Location &location) const
    { return location.second * itsAxes[0]->size() + location.first; }
    
    Location getCellLocation(uint id) const
    { return Location(id % itsAxes[0]->size(), id / itsAxes[0]->size()); }

    Box getCell(const Location &location) const
    {
        DBGASSERT(location.first < itsAxes[0]->size()
            && location.second < itsAxes[1]->size());
            
        return Box(Point(itsAxes[0]->lower(location.first),
            itsAxes[1]->lower(location.second)),
             Point(itsAxes[0]->upper(location.first),
                itsAxes[1]->upper(location.second)));
    }
    
    Box getBoundingBox() const
    {
        const pair<double, double> range0(itsAxes[0]->range());
        const pair<double, double> range1(itsAxes[1]->range());
        
        return Box(Point(range0.first, range1.first), 
            Point(range0.second, range1.second));
    }
    
    Box getBoundingBox(const Location &start, const Location &end) const
    {
        DBGASSERT(start.first <= end.first && start.second <= end.second);
        return unite(getCell(start), getCell(end));
    }

    Location locate(const Point &point, bool biasRight = true) const
    {
        return make_pair(itsAxes[0]->locate(point.first, biasRight),
            itsAxes[1]->locate(point.second, biasRight));
    }

private:
    Axis::Pointer   itsAxes[2];
};


} //# namespace BBS
} //# namespace LOFAR

#endif
