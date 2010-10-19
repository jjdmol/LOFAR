//# Axis.cc: Class templates that represent a regular or irregular axis.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <BBSKernel/Axis.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobSTL.h>
#include <Common/LofarLogger.h> 
#include <Common/StreamUtil.h> 

namespace LOFAR
{
namespace BBS
{
    using LOFAR::operator<<;

    // Register with the BlobStreamableFactory. Use an anonymous namespace. This
    // ensures that the 'dummy' variables get their own private storage area and
    // are only visible in this compilation unit.
    namespace
    {
        bool dummy1 = BlobStreamableFactory::instance().registerClass<RegularAxis>("RegularAxis");
        bool dummy2 = BlobStreamableFactory::instance().registerClass<IrregularAxis>("IrregularAxis");
    }

    RegularAxis::RegularAxis()
        :   itsBegin(0),
            itsWidth(1),
            itsCount(1)
    {
    }     
    
    RegularAxis::RegularAxis(const double &begin, const double &width,
        uint32 count)
        :   itsBegin(begin),
            itsWidth(width),
            itsCount(count)
    {
        ASSERT(width > 0 && count > 0);
    }

    RegularAxis::~RegularAxis()
    {
    }

    double RegularAxis::center(size_t n) const
    {
        return itsBegin + 0.5 * ((2.0 * n + 1.0) * itsWidth);
    }
 
    double RegularAxis::lower(size_t n) const
    {
        return itsBegin + n * itsWidth;
    }

    double RegularAxis::upper(size_t n) const
    {
        return (itsBegin + itsWidth) + n * itsWidth;
    }

    double RegularAxis::width(size_t) const
    {
        return itsWidth;
    }

    size_t RegularAxis::size() const
    {
        return itsCount;
    }

    pair<double, double> RegularAxis::range() const
    { 
        return make_pair(lower(0), upper(size() - 1));
    }

    size_t RegularAxis::locate(double x, bool biasRight) const
    {
        // Find the cell that contains x. Values of x that are 'very near' (as
        // tested by casa::near()) to a cell border are assumed to lie on the
        // border. 
        // If x is located on a cell border, its location is decided according
        // to the value of biasRight. If biasRight is true, the decision is
        // 'biased to the right'. In other words, x is attributed to the cell
        // that has x as its lower border.
        
        const pair<double, double> axisRange = range();

        // Lower boundary case.
        if(casa::near(x, axisRange.first))
        {
            return biasRight ? 0 : size();
        }

        if(x < axisRange.first)
        {
            return size();
        }

        // Upper boundary case.
        if(casa::near(x, axisRange.second))
        {
            return biasRight ? size() : size() - 1;
        }
        
        if(x > axisRange.second)
        {
            return size();
        }
        
        // General case.
        // Find the index of the cell that contains x. Because the boundary
        // cases have already been dealt with, it is certain that x lies within
        // the range of the axis. Therefore, it is allowed to use std::min() in
        // the following.
        size_t index = std::min(static_cast<size_t>(floor((x + 0.5 * itsWidth
            - itsBegin) / itsWidth)), size() - 1);

        if(casa::near(x, lower(index)))
        {
            DBGASSERT(index > 0);
            return biasRight ? index : index - 1;
        }

        return x < lower(index) ? index - 1 : index;
    }

    Axis::Pointer RegularAxis::compress(size_t factor) const
    {
        // Is the resulting axis still regular?
        if(itsCount % factor == 0)
        {
            return Axis::Pointer(new RegularAxis(itsBegin, itsWidth * factor,
                itsCount / factor));
        }
        
        vector<double> centers(itsCount / factor + 1);
        for(size_t i = 0; i < itsCount / factor; ++i)
        {
            centers[i] = itsBegin + (i + 0.5) * factor * itsWidth;
        }
        centers.back() = lower(itsCount - (itsCount % factor)) +
            0.5 * (itsCount % factor) * itsWidth;

        vector<double> widths(itsCount / factor + 1, factor * itsWidth);
        widths.back() = (itsCount % factor) * itsWidth;

        return Axis::Pointer(new IrregularAxis(centers, widths));
    }


//  ----------------------------------------------------------------------------
//  - BlobStreamable implementation                                            -
//  ----------------------------------------------------------------------------
    // Write the contents of \c *this into the blob output stream \a bos.
    void RegularAxis::write(BlobOStream& bos) const
    {
        bos << itsBegin << itsWidth << itsCount;
    }

    // Read the contents from the blob input stream \a bis into \c *this.
    void RegularAxis::read(BlobIStream& bis)
    {
        bis >> itsBegin >> itsWidth >> itsCount;
    }

    // Return the type of \c *this as a string.
    const string& RegularAxis::classType() const
    {
        static string type("RegularAxis");
        return type;
    }


// -----------------------------------------------------------------------------
    IrregularAxis::IrregularAxis()
        :   itsCenters(1, 0.0),
            itsWidths(1, 1.0)
    {
    }

    IrregularAxis::IrregularAxis(const vector<double> &centers,
        const vector<double> &widths)
        :   itsCenters(centers),
            itsWidths(widths)
    {
        ASSERT(centers.size() > 0 && widths.size() > 0);
        ASSERT(centers.size() == widths.size());
    }

    IrregularAxis::~IrregularAxis()
    {
    }
    
    double IrregularAxis::center(size_t n) const
    {
        return itsCenters[n];
    }
 
    double IrregularAxis::lower(size_t n) const
    {
        return itsCenters[n] - 0.5 * itsWidths[n];
    }

    double IrregularAxis::upper(size_t n) const
    {
        return itsCenters[n] + 0.5 * itsWidths[n];
    }

    double IrregularAxis::width(size_t n) const
    {
        return itsWidths[n];
    }

    size_t IrregularAxis::size() const
    {
        return itsCenters.size();
    }

    pair<double, double> IrregularAxis::range() const
    { 
        return make_pair(lower(0), upper(size() - 1));
    }

    size_t IrregularAxis::locate(double x, bool biasRight) const
    {
        // Find the cell that contains x. Values of x that are 'very near' (as
        // tested by casa::near()) to a cell border are assumed to lie on the
        // border. 
        // If x is located on a cell border, its location is decided according
        // to the value of biasRight. If biasRight is true, the decision is
        // 'biased to the right'. In other words, x is attributed to the cell
        // that has x as its lower border.

        const pair<double, double> axisRange = range();

        // Lower boundary case.
        if(casa::near(x, axisRange.first))
        {
            return biasRight ? 0 : size();
        }

        if(x < axisRange.first)
        {
            return size();
        }

        // Upper boundary case.
        if(casa::near(x, axisRange.second))
        {
            return biasRight ? size() : size() - 1;
        }
        
        if(x > axisRange.second)
        {
            return size();
        }

        // General case.        
        // Try to find x in the list of cell centers. This will return the index
        // of the first cell center that is greater than or equal to x. Because
        // the boundary cases have already been dealt with, it is certain that
        // x lies within the range of the axis. Therefore, it is allowed to use
        // std::min() in the following.
        size_t index =
            std::min(static_cast<size_t>(lower_bound(itsCenters.begin(),
                itsCenters.end(), x) - itsCenters.begin()), size() - 1);
        
        if(casa::near(x, lower(index)))
        {
            DBGASSERT(index > 0);
            return biasRight ? index : index - 1;
        }

        return x < lower(index) ? index - 1 : index;
    }


    Axis::Pointer IrregularAxis::compress(size_t factor) const
    {
        size_t count = static_cast<size_t>(ceil(static_cast<double>(size())
            / factor));

        vector<double> centers(count), widths(count);
        
        for(size_t i = 0; i < count; ++i)
        {
            double centerSum = 0.0, widthSum = 0.0;
            for(size_t j = i * factor; j < min((i + 1) * factor, size()); ++j)
            {
                centerSum += center(j) * width(j);
                widthSum += width(j);
            }
            centers[i] = centerSum / widthSum;
            widths[i] = widthSum;
        }

        return Axis::Pointer(new IrregularAxis(centers, widths));
    }
    

//  ----------------------------------------------------------------------------
//  - BlobStreamable implementation                                            -
//  ----------------------------------------------------------------------------
    // Write the contents of \c *this into the blob output stream \a bos.
    void IrregularAxis::write(BlobOStream& bos) const
    {
        bos << itsCenters << itsWidths;
    }


    // Read the contents from the blob input stream \a bis into \c *this.
    void IrregularAxis::read(BlobIStream& bis)
    {
        bis >> itsCenters >> itsWidths;
    }


    // Return the type of \c *this as a string.
    const string& IrregularAxis::classType() const
    {
        static string type("IrregularAxis");
        return type;
    }

} // namespace BBS
} // namespace LOFAR
