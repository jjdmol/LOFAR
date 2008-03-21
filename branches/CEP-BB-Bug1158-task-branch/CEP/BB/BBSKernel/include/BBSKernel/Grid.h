//# Grid.h:
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
#include <BBSKernel/Box.h>

namespace LOFAR
{
namespace BBS
{

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
    
    uint getCellId(const Location &location) const
    { return location.second * itsAxes[0]->size() + location.first; }
    
    Location getCellLocation(uint id) const
    {
        return Location(id % itsAxes[0]->size(), id / itsAxes[1]->size());
    }

    Box<T> getCell(const Location &location) const;
    
    Box<T> getBoundingBox() const;
    
    Box<T> getBoundingBox(const Location &start, const Location &end) const;

    pair<Location, bool> locate(const Point<T> &point, bool open) const;

private:
    typename Axis<T>::Pointer   itsAxes[2];
};


template <typename T>
pair<Location, bool> Grid<T>::locate(const Point<T> &point, bool open) const
{
    size_t first = itsAxes[0]->locate(point.first, open);
//    DBGASSERT(first <= itsAxis1.size());
    if(first == itsAxes[0]->size())
    {
        return make_pair(Location(), false);
    }

    size_t second = itsAxes[1]->locate(point.second, open);
//    DBGASSERT(second <= itsAxis2.size());
    if(second == itsAxes[1]->size())
    {
        return make_pair(Location(), false);
    }

    return make_pair(Location(first, second), true);
}


template <typename T>
Box<T> Grid<T>::getCell(const Location &location) const
{
    DBGASSERT(location.first < itsAxes[0]->size() && location.second < itsAxes[1]->size());
    return Box<T>(Point<T>(itsAxes[0]->lower(location.first), itsAxes[1]->lower(location.second)),
        Point<T>(itsAxes[0]->upper(location.first), itsAxes[1]->upper(location.second)));
}


template <typename T>
Box<T> Grid<T>::getBoundingBox() const
{
    pair<T, T> range0 = itsAxes[0]->range();
    pair<T, T> range1 = itsAxes[1]->range();
    
    return Box<T>(Point<T>(range0.first, range1.first), 
        Point<T>(range0.second, range1.second));
}


template <typename T>
Box<T> Grid<T>::getBoundingBox(const Location &start, const Location &end) const
{
    if(start.first > end.first || start.second > end.second)
    {
        return Box<T>();
    }

    return unite(getCell(start), getCell(end));
}

} //# namespace BBS
} //# namespace LOFAR

#endif
