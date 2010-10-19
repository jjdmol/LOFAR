//# SpectralIndex.cc: Frequency dependent scale factor for the base flux given
//# for a specific reference frequency.
//#
//# Copyright (C) 2009
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

#include <lofar_config.h>
#include <BBSKernel/Expr/SpectralIndex.h>
#include <BBSKernel/Expr/MatrixTmp.h>

namespace LOFAR
{
namespace BBS
{

Matrix SpectralIndex::getResultValue(const Request &request,
    const std::vector<const Matrix*> &args)
{
    ASSERT(args.size() == getChildCount());

    // Special case: No coefficients or a single real coefficient with value
    // zero.
    if(args.size() == 1 || (args.size() == 2 && !args.back()->isArray()
        && !args.back()->isComplex() && args.back()->getDouble() == 0.0))
    {
        return Matrix(1.0);
    }

    // Create a 2-D Matrix that contains the frequency for each sample. (We
    // _must_ expand to a 2-D buffer because 1-D buffers are not supported).
    const Grid &reqGrid = request.getGrid();

    Matrix freq;
    double *it = freq.setDoubleFormat(reqGrid[FREQ]->size(),
        reqGrid[TIME]->size());

    for(unsigned int t = 0; t < request.getTimeslotCount(); ++t)
    {
        for(unsigned int f = 0; f < request.getChannelCount(); ++f)
        {
            *it++ = reqGrid[FREQ]->center(f);
        }
    }

    // Compute flux scale factor as:
    // (v / v0) ^ (-1.0 * [c0 + c1 * log(v / v0) + c2 * log(v / v0)^2 + ...])
    // Where v is the frequency and v0 is the reference frequency.

    // Compute log(v / v0).
    Matrix base = log(freq / *args[0]);

    // Compute c0 + log(v / v0) * c1 + log(v / v0)^2 * c2 + ... using Horner's
    // rule.
    Matrix exponent = *args.back();
    for(unsigned int i = args.size() - 2; i >= 1; --i)
    {
        exponent = exponent * base + *args[i];
    }

    // Compute (v / v0) ^ -exponent.
    return exp(base * (-exponent));
}

} //# namespace BBS
} //# namespace LOFAR
