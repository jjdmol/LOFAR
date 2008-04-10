//# Axis.h:
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

#ifndef LOFAR_BBS_BBSKERNEL_AXIS_H
#define LOFAR_BBS_BBSKERNEL_AXIS_H

#include <Common/lofar_vector.h>
#include <Common/lofar_smartptr.h>
#include <BBSKernel/Types.h>

namespace LOFAR
{
namespace BBS
{
using std::pair;
using std::make_pair;
using std::max;
using std::min;

template <typename T>
class Axis
{
public:
    typedef shared_ptr<Axis<T> >    Pointer;
    
    virtual ~Axis()
    {}
    
    virtual T center(size_t n) const = 0;
    virtual T lower(size_t n) const = 0;
    virtual T upper(size_t n) const = 0;
    virtual T width(size_t n) const = 0;
    virtual size_t size() const = 0;
    virtual pair<T, T> range() const = 0;
    virtual size_t locate(T x, bool open) const = 0;
};


template <typename T>
class RegularAxis: public Axis<T>
{
public:
    typedef shared_ptr<RegularAxis<T> > Pointer;
    
    RegularAxis()
        :   itsBegin(0),
            itsDelta(0),
            itsCount(0)
    {}
    
    RegularAxis(const T &begin, const T &delta, size_t count)
        :   itsBegin(begin),
            itsDelta(delta),
            itsCount(count)
    {}
    
    ~RegularAxis()
    {}
    
    T center(size_t n) const
    { return itsBegin + (2 * n * itsDelta + itsDelta) / 2; }
 
    T lower(size_t n) const
    { return itsBegin + n * itsDelta; }

    T upper(size_t n) const
    { return (itsBegin + itsDelta) + n * itsDelta; }

    T width(size_t n) const
    { return itsDelta; }

    size_t size() const
    { return itsCount; }

    pair<T, T> range() const
    { return make_pair(lower(0), (size() ? upper(size() - 1) : upper(0))); }

    size_t locate(T x, bool open) const
    {
        // Try to find x in the list of cell borders. This will return the index i
        // of the first border that is greater than or equal to x.
        // The upper border of a cell is open by convention. Therefore, if x is
        // located precisely on a cell border, it is located in the cell of which
        // that border is the _lower_ border. Equality is tested with casa::near(),
        // i.e. values of x that are 'very near' to a cell border are assumed to lie
        // on the cell border.

        size_t index =
            static_cast<size_t>(max(0.0, ceil(static_cast<double>(x - itsBegin) / itsDelta)));
    
        if(index > 0 && is_near<T>::eval(x, lower(index - 1)))
        {
            --index;
        }
    
        if(!is_near<T>::eval(x, lower(index)) || !open)
        {
            --index;
        }
    
        // If x was located outside of the range of the axis, size() will be
        // returned.
        if(index > size())
        {
            index = size();
        }
    
        return index;
    }
    
private:
    T       itsBegin, itsDelta;
    size_t  itsCount;
};


template <typename T>
class IrregularAxis: public Axis<T>
{
public:
    typedef shared_ptr<IrregularAxis<T> >   Pointer;
    
    IrregularAxis()
    {}

    IrregularAxis(const vector<T> &borders)
        :   itsBorders(borders)
    {}
    
    ~IrregularAxis()
    {}

    T center(size_t n) const
    { return 0.5 * (itsBorders[n] + itsBorders[n + 1]); }
 
    T lower(size_t n) const
    { return itsBorders[n]; }

    T upper(size_t n) const
    { return itsBorders[n + 1]; }

    T width(size_t n) const
    { return upper(n) - lower(n); }

    size_t size() const
    { return (itsBorders.size() ? itsBorders.size() - 1 : 0); }

    pair<T, T> range() const
    { return make_pair(lower(0), (size() ? upper(size() - 1) : upper(0))); }

    size_t locate(T x, bool open) const
    {
        // Try to find x in the list of cell borders. This will return the index i
        // of the first border that is greater than or equal to x.
        // The upper border of a cell is open by convention. Therefore, if x is
        // located precisely on a cell border, it is located in the cell of which
        // that border is the _lower_ border. Equality is tested with casa::near(),
        // i.e. values of x that are 'very near' to a cell border are assumed to lie
        // on the cell border.

        size_t index = lower_bound(itsBorders.begin(), itsBorders.end(), x)
            - itsBorders.begin();
    
        if(index > 0 && is_near<T>::eval(x, lower(index - 1)))
        {
            --index;
        }
    
        if(!is_near<T>::eval(x, lower(index)) || !open)
        {
            --index;
        }
    
        // If x was located outside of the range of the axis, size() will be
        // returned.
        if(index > size())
        {
            index = size();
        }
    
        return index;
    }

private:
    vector<T>   itsBorders;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
