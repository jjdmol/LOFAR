//# Axis.h: Class templates that represent a regular or irregular axis.
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

#include <Common/lofar_algorithm.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_smartptr.h>

#include <limits>
#include <utility>

#include <casa/BasicMath/Math.h>

namespace LOFAR
{
namespace BBS
{
using std::pair;
using std::make_pair;
using std::numeric_limits;


// Class template that encapsulates the comparison of two values for equality.
// For floating point types, values 'very near' to each other are assumed to be
// equal. The notion of 'very near' is captured by casa::near().
template <typename T, bool is_integer = std::numeric_limits<T>::is_integer>
struct is_equal
{
    static bool eval(const T &a, const T &b);
};

template <typename T>
struct is_equal<T, true>
{
    static bool eval(const T &a, const T &b)
    { return (a == b); }
};

template <typename T>
struct is_equal<T, false>
{
    static bool eval(const T &a, const T &b)
    { return casa::near(a, b); }
};


// Abstract base class for a cell centered axis.
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
    virtual size_t locate(const T &x) const = 0;
};


// Regularly strided cell centered axis.
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

    T width(size_t) const
    { return itsDelta; }

    size_t size() const
    { return itsCount; }

    pair<T, T> range() const
    { 
        if(size() == 0)
        {
            return make_pair(0, 0);
        }
        
        return make_pair(lower(0), upper(size() - 1));
    }

    size_t locate(const T &x) const
    {
        // Find the cell that contains x. The upper border of a cell is open by
        // convention. Therefore, if x is located precisely on a cell border,
        // it is located in the cell of which that border is the _lower_ border.
        // Equality is tested with is_equal<T>, which does a fuzzy comparison
        // for floating point types. Values of x that are 'very near' to a cell
        // border are assumed to lie on the cell border.

        // T may be unsigned. If x < itsBegin, this will create a problem when
        // evaluating x - itsBegin. For floating point types, even though
        // x < itsBegin, x may still be 'very close' to the lower border of cell
        // 0, in which case it should be attributed to cell 0.
        if(x < itsBegin)
        {
            if(is_equal<T>::eval(x, itsBegin))
            {
                return 0;
            }                
            else
            {
                return size();
            }
        }        

        // Find the index of the cell that contains x.
        size_t index = static_cast<size_t>(floor((x - itsBegin) / itsDelta));

        // If T is a floating point type, x could be 'very close' to the upper
        // cell border, in which case it should be attributed to the next cell.
        if(is_equal<T>::eval(x, upper(index)))
        {
            ++index;
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


// Irregularly strided cell centered axis.
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
    { return (itsBorders[n] + itsBorders[n + 1]) / 2; }
 
    T lower(size_t n) const
    { return itsBorders[n]; }

    T upper(size_t n) const
    { return itsBorders[n + 1]; }

    T width(size_t n) const
    { return upper(n) - lower(n); }

    size_t size() const
    { return (itsBorders.size() ? itsBorders.size() - 1 : 0); }

    pair<T, T> range() const
    { 
        if(size() == 0)
        {
            return make_pair(0, 0);
        }

        return make_pair(lower(0), upper(size() - 1));
    }

    size_t locate(const T &x) const
    {
        // Try to find x in the list of cell borders. This will return the index
        // of the first border that is greater than or equal to x. The upper
        // border of a cell is open by convention. Therefore, if x is located
        // precisely on a cell border, it is located in the cell of which that
        // border is the _lower_ border.
        // Equality is tested with is_equal<T>, which does a fuzzy comparison
        // for floating point types. Values of x that are 'very near' to a cell
        // border are assumed to lie on the cell border.

        size_t index = upper_bound(itsBorders.begin(), itsBorders.end(), x) -
            itsBorders.begin();

        if(index == itsBorders.size())
        {
            return size();
        }

        if(!is_equal<T>::eval(x, lower(index)))
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
