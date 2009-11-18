//# PointCoherence.h: Spatial coherence function of a point source.
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include <BBSKernel/Expr/PointCoherence.h>

namespace LOFAR
{
namespace BBS
{

PointCoherence::PointCoherence(const PointSource::ConstPtr &source)
    :   BasicUnaryExpr<Vector<4>, JonesMatrix>(source->getStokesVector())
{
}

const JonesMatrix::View PointCoherence::evaluateImpl(const Request &request,
    const Vector<4>::View &stokes) const
{
    JonesMatrix::View result;

    if(stokes.bound(0) || stokes.bound(1))
    {
        result.assign(0, 0, 0.5 * (stokes(0) + stokes(1)));
        result.assign(1, 1, 0.5 * (stokes(0) - stokes(1)));
    }

    if(stokes.bound(2) || stokes.bound(3))
    {
        Matrix uv = 0.5 * tocomplex(stokes(2), stokes(3));
        result.assign(0, 1, uv);
        result.assign(1, 0, conj(uv));
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
