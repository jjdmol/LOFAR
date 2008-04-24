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


// Point: Class template that represents a point in a 2-D space.
template <typename T>
class Point
{
public:
    Point(const T &a, const T &b)
        :   first(a),
            second(b)
    {}
                    
    T   first, second;
};


// -----------------------------------------------------------------------------
// - Box: Class template for an axis aligned bounding box.                     -
// -----------------------------------------------------------------------------
template <typename T>
class Box;

template <typename T>
Box<T> unite(const Box<T> &lhs, const Box<T> &rhs);

template <typename T>
Box<T> intersect(const Box<T> &lhs, const Box<T> &rhs);

template <typename T>
class Box
{
public:
    Box()
        :   start(0, 0),
            end(0, 0)
    {}

                
    Box(const Point<T> &start, const Point<T> &end)
        :   start(start),
            end(end)
    {
        DBGASSERT(start.first <= end.first && start.second <= end.second);
    }            


    bool intersects(const Box<T> &other) const
    {
        return (other.start.first < end.first
            && !is_equal<T>::eval(other.start.first, end.first)
            && other.end.first > start.first
            && !is_equal<T>::eval(other.end.first, start.first)
            && other.start.second < end.second
            && !is_equal<T>::eval(other.start.second, end.second)
            && other.end.second > start.second
            && !is_equal<T>::eval(other.end.second, start.second));
    }    


    bool contains(const Box<T> &other) const
    {
        return ((other.start.first > start.first
                || is_equal<T>::eval(other.start.first, start.first))
            && (other.end.first < end.first
                || !is_equal<T>::eval(other.end.first, end.first))
            && (other.start.second > start.second
                || is_equal<T>::eval(other.start.second, start.second))
            && (other.end.second < end.second
                || is_equal<T>::eval(other.end.second, end.second)));
    }
    
    bool empty()
    {
        return (is_equal<T>::eval(start.first, end.first)
            || is_equal<T>::eval(start.second, end.second));
    }

    Box<T> operator&(const Box<T> &other) const
    { return intersect(*this, other); }

    
    Box<T> operator|(const Box<T> &other) const
    { return unite(*this, other); }

    
    Point<T>    start, end;
};


template <typename T>
Box<T> unite(const Box<T> &lhs, const Box<T> &rhs)
{

    Point<T> start(min(lhs.start.first, rhs.start.first),
        min(lhs.start.second, rhs.start.second));
        
    Point<T> end(max(lhs.end.first, rhs.end.first),
        max(lhs.end.second, rhs.end.second));

    return Box<T>(start, end);
}


template <typename T>
Box<T> intersect(const Box<T> &lhs, const Box<T> &rhs)
{
    Point<T> start(max(lhs.start.first, rhs.start.first),
        max(lhs.start.second, rhs.start.second));

    Point<T> end(min(lhs.end.first, rhs.end.first),
        min(lhs.end.second, rhs.end.second));

    if(start.first < end.first
        && !is_equal<T>::eval(start.first, end.first)
        && start.second < end.second
        && !is_equal<T>::eval(start.second, end.second))
    {
        return Box<T>(start, end);
    }

    return Box<T>();
}


// -----------------------------------------------------------------------------
// - Grid: Class template for a 2-D grid with regular/irregular axes.          -
// -----------------------------------------------------------------------------
template <typename T>
class Grid
{
public:
    Grid()
    {}
    
    Grid(typename Axis<T>::Pointer first, typename Axis<T>::Pointer second)
    {
        itsAxes[0] = first;
        itsAxes[1] = second;
    }
    
    typename Axis<T>::Pointer operator[](size_t n)
    { return itsAxes[n]; }
    
    const typename Axis<T>::Pointer operator[](size_t n) const
    { return itsAxes[n]; }

    pair<size_t, size_t> size() const
    { return make_pair(itsAxes[0]->size(), itsAxes[1]->size()); }
    
    size_t getCellCount() const
    { return itsAxes[0]->size() * itsAxes[1]->size(); }
    
    uint getCellId(const Location &location) const
    { return location.second * itsAxes[0]->size() + location.first; }
    
    Location getCellLocation(uint id) const
    { return Location(id % itsAxes[0]->size(), id / itsAxes[0]->size()); }

    Box<T> getCell(const Location &location) const
    {
        DBGASSERT(location.first < itsAxes[0]->size()
            && location.second < itsAxes[1]->size());
            
        return Box<T>(Point<T>(itsAxes[0]->lower(location.first),
            itsAxes[1]->lower(location.second)),
             Point<T>(itsAxes[0]->upper(location.first),
                itsAxes[1]->upper(location.second)));
    }
    
    Box<T> getBoundingBox() const
    {
        pair<T, T> range0 = itsAxes[0]->range();
        pair<T, T> range1 = itsAxes[1]->range();
        
        return Box<T>(Point<T>(range0.first, range1.first), 
            Point<T>(range0.second, range1.second));
    }
    
    Box<T> getBoundingBox(const Location &start, const Location &end) const
    {
        DBGASSERT(start.first <= end.first && start.second <= end.second);
        return unite(getCell(start), getCell(end));
    }

    Location locate(const Point<T> &point) const
    {
        return make_pair(itsAxes[0]->locate(point.first),
            itsAxes[1]->locate(point.second));
    }

private:
    typename Axis<T>::Pointer   itsAxes[2];
};


} //# namespace BBS
} //# namespace LOFAR

#endif
