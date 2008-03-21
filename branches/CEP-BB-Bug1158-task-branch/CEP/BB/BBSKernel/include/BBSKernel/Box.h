//# Box.h:
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

#ifndef LOFAR_BBS_BBSKERNEL_BOX_H
#define LOFAR_BBS_BBSKERNEL_BOX_H

#include <BBSKernel/Types.h>

namespace LOFAR
{
namespace BBS
{

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
//        DBGASSERT(start.first <= end.first && start.second <= end.second);
    }            


    bool intersects(const Box<T> &other) const
    {
        return (other.start.first < end.first
            && !is_near<T>::eval(other.start.first, end.first)
            && other.end.first > start.first
            && !is_near<T>::eval(other.end.first, start.first)
            && other.start.second < end.second
            && !is_near<T>::eval(other.start.second, end.second)
            && other.end.second > start.second
            && !is_near<T>::eval(other.end.second, start.second));
    }    


    bool contains(const Box<T> &other) const
    {
        return ((other.start.first > start.first
            || is_near<T>::eval(other.start.first, start.first))
            && (other.end.first < end.first
            || is_near<T>::eval(other.end.first, end.first))
            (other.start.second > start.second
            || is_near<T>::eval(other.start.second, start.second))
            && (other.end.second < end.second
            || is_near<T>::eval(other.end.second, end.second)));
    }
    
    bool empty()
    {
        return (is_near<T>::eval(start.first, end.first)
            || is_near<T>::eval(start.second, end.second));
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
        && !is_near<T>::eval(start.first, end.first)
        && start.second < end.second
        && !is_near<T>::eval(start.second, end.second))
    {
        return Box<T>(start, end);
    }

    return Box<T>();
}

} //# namespace BBS
} //# namespace LOFAR

#endif



