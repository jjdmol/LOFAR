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


    RegularAxis::RegularAxis(const double &begin, const double &delta,
        uint32 count)
        :   itsBegin(begin),
            itsDelta(delta),
            itsCount(count)
    {
        ASSERT(delta > 0 && count > 0);
    }


    // Write the contents of \c *this into the blob output stream \a bos.
    void RegularAxis::write(BlobOStream& bos) const
    {
        bos << itsBegin << itsDelta << itsCount;
    }


    // Read the contents from the blob input stream \a bis into \c *this.
    void RegularAxis::read(BlobIStream& bis)
    {
        bis >> itsBegin >> itsDelta >> itsCount;
    }


    // Return the type of \c *this as a string.
    const string& RegularAxis::classType() const
    {
        static string type("RegularAxis");
        return type;
    }


// -----------------------------------------------------------------------------
    IrregularAxis::IrregularAxis(const vector<double> &centers,
        const vector<double> &widths)
        :   itsCenters(centers),
            itsWidths(widths)
    {
        ASSERT(centers.size() > 0 && widths.size() > 0);
        ASSERT(centers.size() == widths.size());
    }


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
