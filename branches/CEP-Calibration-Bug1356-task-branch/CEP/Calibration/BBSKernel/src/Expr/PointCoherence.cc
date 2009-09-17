//# PointCoherence.h: Spatial coherence function of a point source.
//#
//# Copyright (C) 2005
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

#include <BBSKernel/Expr/PointCoherence.h>

namespace LOFAR
{
namespace BBS
{

PointCoherence::PointCoherence(const PointSource::ConstPtr &source)
    :   BasicBinaryExpr<Vector<4>, Scalar, JonesMatrix>
            (source->getStokesVector(), source->getSpectralIndex())
{
}

const JonesMatrix::View PointCoherence::evaluateImpl(const Request &request,
    const Vector<4>::View &stokes, const Scalar::View &spectral) const
{
    JonesMatrix::View result;

    if(spectral.bound() || stokes.bound(0) || stokes.bound(1))
    {
        result.assign(0, 0, spectral() * 0.5 * (stokes(0) + stokes(1)));
        result.assign(1, 1, spectral() * 0.5 * (stokes(0) - stokes(1)));
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
