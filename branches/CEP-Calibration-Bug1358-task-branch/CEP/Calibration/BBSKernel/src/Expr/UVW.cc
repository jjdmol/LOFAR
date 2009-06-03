//# UVW.cc: Baseline UVW coordinates in wavelengths.
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
#include <BBSKernel/Expr/UVW.h>
#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace BBS 
{

UVW::UVW(const VisData::Pointer &chunk,
    const baseline_t &baseline)
    :   ExprTerminus(),
        itsChunk(chunk),
        itsBaseline(baseline)
{
}

ValueSet::ConstPtr UVW::evaluateImpl(const Request &request) const
{
    const size_t nFreq = request[FREQ]->size();
    const size_t nTime = request[TIME]->size();

    // Find the offset of the request grid relative to the chunk grid.
    const Grid &visGrid = itsChunk->getDimensions().getGrid();
    const size_t tstart = visGrid[TIME]->locate(request[TIME]->start());
    const size_t tend = visGrid[TIME]->locate(request[TIME]->end(), false);
    ASSERT(tend - tstart + 1 == nTime);

    const size_t bl = itsChunk->getDimensions().getBaselineIndex(itsBaseline);

    // Get a view on the relevant slice of uvw data.
    typedef boost::multi_array<double, 4>::index_range RangeType;
    typedef boost::multi_array<double, 4>::array_view<2>::type ViewType;
    ViewType uvw(itsChunk->uvw[boost::indices[bl][RangeType(tstart, tend + 1)]
        [RangeType()]]);

    Matrix U(real_t(), nFreq, nTime);
    Matrix V(real_t(), nFreq, nTime);
    Matrix W(real_t(), nFreq, nTime);

    double *Up = U.doubleStorage();
    double *Vp = V.doubleStorage();
    double *Wp = W.doubleStorage();
    
    for(int t = 0; t < static_cast<int>(nTime); ++t)
    {
        const real_t u = uvw[t][0] / casa::C::c;
        const real_t v = uvw[t][1] / casa::C::c;
        const real_t w = uvw[t][2] / casa::C::c;
        
        for(int f = 0; f < static_cast<int>(nFreq); ++f)
        {
            const real_t freq = request[FREQ]->center(f);
            Up[t * nFreq + f] = u * freq;
            Vp[t * nFreq + f] = v * freq;
            Wp[t * nFreq + f] = w * freq;
        }
    }
    
    ValueSet::Ptr result(new ValueSet(3));
    result->assign(0, U);
    result->assign(1, V);
    result->assign(2, W);
    
    return result;
}

} //# namespace BBS
} //# namespace LOFAR
