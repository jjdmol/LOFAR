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
#include <Common/LofarTypes.h>

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
    virtual size_t locate(double x, bool biasRight = true) const = 0;
    virtual Axis::Pointer compress(size_t factor) const = 0;
};


// Regularly strided cell centered axis.
class RegularAxis: public Axis
{
public:
    typedef shared_ptr<RegularAxis> Pointer;
    
    RegularAxis();
    RegularAxis(const double &begin, const double &width, uint32 count);

    ~RegularAxis();
    
    double center(size_t n) const;
    double lower(size_t n) const;
    double upper(size_t n) const;
    double width(size_t) const;
    size_t size() const;
    pair<double, double> range() const;
    size_t locate(double x, bool biasRight = true) const;
    Axis::Pointer compress(size_t factor) const;
    
private:
    //# -------- BlobStreamable interface implementation -------- 

    // Write the contents of \c *this into the blob output stream \a bos.
    virtual void write(BlobOStream& bos) const;

    // Read the contents from the blob input stream \a bis into \c *this.
    virtual void read(BlobIStream& bis);

    // Return the type of \c *this as a string.
    virtual const string& classType() const;
    
    //# -------- BlobStreamable interface implementation -------- 

    double  itsBegin, itsWidth;
    uint32  itsCount;
};


// Irregularly strided cell centered axis.
class IrregularAxis: public Axis
{
public:
    typedef shared_ptr<IrregularAxis>   Pointer;
    
    IrregularAxis();
    IrregularAxis(const vector<double> &centers, const vector<double> &widths);
    
    ~IrregularAxis();

    double center(size_t n) const;
    double lower(size_t n) const;
    double upper(size_t n) const;
    double width(size_t n) const;
    size_t size() const;
    pair<double, double> range() const;
    size_t locate(double x, bool biasRight = true) const;
    Axis::Pointer compress(size_t factor) const;

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
