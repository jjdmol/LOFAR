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

#include <Blob/BlobStreamable.h>
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

// Abstract base class for a cell centered axis.
class Axis: public BlobStreamable
{
public:
    typedef shared_ptr<Axis>    Pointer;
    
    virtual ~Axis()
    {}
    
    virtual double center(size_t n) const = 0;
    virtual double lower(size_t n) const = 0;
    virtual double upper(size_t n) const = 0;
    virtual double width(size_t n) const = 0;
    virtual size_t size() const = 0;
    virtual pair<double, double> range() const = 0;
    virtual size_t locate(const double &x) const = 0;
};


// Regularly strided cell centered axis.
class RegularAxis: public Axis
{
public:
    typedef shared_ptr<RegularAxis> Pointer;
    
    RegularAxis()
        :   itsBegin(0),
            itsDelta(1),
            itsCount(1)
    {}     
    
    RegularAxis(const double &begin, const double &delta, size_t count);
    
    ~RegularAxis()
    {}
    
    double center(size_t n) const
    { return itsBegin + 0.5 * ((2.0 * n + 1.0) * itsDelta); }
 
    double lower(size_t n) const
    { return itsBegin + n * itsDelta; }

    double upper(size_t n) const
    { return (itsBegin + itsDelta) + n * itsDelta; }

    double width(size_t) const
    { return itsDelta; }

    size_t size() const
    { return itsCount; }

    pair<double, double> range() const
    { 
        return make_pair(lower(0), upper(size() - 1));
    }

    size_t locate(const double &x) const
    {
        // Find the cell that contains x. The upper border of a cell is open by
        // convention. Therefore, if x is located precisely on a cell border,
        // it is located in the cell of which that border is the _lower_ border.
        // Equality is tested with is_equal<T>, which does a fuzzy comparison
        // for floating point types. Values of x that are 'very near' to a cell
        // border are assumed to lie on the cell border.

        const size_t axisSize = size();
        
        // T may be unsigned. If x < itsBegin, this will create a problem when
        // evaluating x - itsBegin. For floating point types, even though
        // x < itsBegin, x may still be 'very close' to the lower border of cell
        // 0, in which case it should be attributed to cell 0.
        if(x < itsBegin)
        {
            return casa::near(x, itsBegin) ? 0 : axisSize;
        }        

        // Find the index of the cell that contains x.
        size_t index = static_cast<size_t>(floor((x - itsBegin) / itsDelta));

        // If T is a floating point type, x could be 'very close' to the upper
        // cell border, in which case it should be attributed to the next cell.
        if(casa::near(x, upper(index)))
        {
            ++index;
        }

        // If x was located outside of the range of the axis, size() will be
        // returned.
        if(index > axisSize)
        {
            index = axisSize;
        }
    
        return index;
    }
    
private:
    //# -------- BlobStreamable interface implementation -------- 

    // Write the contents of \c *this into the blob output stream \a bos.
    virtual void write(BlobOStream& bos) const;

    // Read the contents from the blob input stream \a bis into \c *this.
    virtual void read(BlobIStream& bis);

    // Return the type of \c *this as a string.
    virtual const string& classType() const;
    
    //# -------- BlobStreamable interface implementation -------- 

    double  itsBegin, itsDelta;
    size_t  itsCount;
};


// Irregularly strided cell centered axis.
class IrregularAxis: public Axis
{
public:
    typedef shared_ptr<IrregularAxis>   Pointer;
    
    IrregularAxis()
        :   itsCenters(1, 0.0),
            itsWidths(1, 1.0)
    {}

    IrregularAxis(const vector<double> &centers, const vector<double> &widths);
    
    ~IrregularAxis()
    {}

    double center(size_t n) const
    { return itsCenters[n]; }
 
    double lower(size_t n) const
    { return itsCenters[n] - 0.5 * itsWidths[n]; }

    double upper(size_t n) const
    { return itsCenters[n] + 0.5 * itsWidths[n]; }

    double width(size_t n) const
    { return itsWidths[n]; }

    size_t size() const
    { return itsCenters.size(); }

    pair<double, double> range() const
    { 
        return make_pair(lower(0), upper(size() - 1));
    }

    size_t locate(const double &x) const
    {
        // Try to find x in the list of cell borders. This will return the index
        // of the first border that is greater than or equal to x. The upper
        // border of a cell is open by convention. Therefore, if x is located
        // precisely on a cell border, it is located in the cell of which that
        // border is the _lower_ border.
        // Equality is tested with is_equal<T>, which does a fuzzy comparison
        // for floating point types. Values of x that are 'very near' to a cell
        // border are assumed to lie on the cell border.

        const size_t axisSize = size();

        // General case.
        if(x > lower(0) && x < upper(axisSize - 1))
        {
            const size_t index =
                upper_bound(itsCenters.begin(), itsCenters.end(), x)
                - itsCenters.begin();
            const double lowerBorder = lower(index);            
            
            return x < lowerBorder && !casa::near(x, lowerBorder) ?
                index - 1 : index;
        }

        const pair<double,double> axisRange = range();

        // Left border.
        if(x <= lower(0))
        {
            return x > axisRange.first || casa::near(x, axisRange.first) ?
                0 : axisSize;
        }

        // Right border.
        return x < axisRange.second && !casa::near(x, axisRange.second) ?
            axisSize - 1 : axisSize;
    }

private:
    //# -------- BlobStreamable interface implementation -------- 

    // Write the contents of \c *this into the blob output stream \a bos.
    virtual void write(BlobOStream& bos) const;

    // Read the contents from the blob input stream \a bis into \c *this.
    virtual void read(BlobIStream& bis);

    // Return the type of \c *this as a string.
    virtual const string& classType() const;
    
    //# -------- BlobStreamable interface implementation -------- 

    vector<double>   itsCenters, itsWidths;
};


} //# namespace BBS
} //# namespace LOFAR

#endif
