//# Rotation.cc: Ionospheric Faraday rotation.
//#
//# Copyright (C) 2009
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
#include <BBSKernel/Expr/Rotation.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

Rotation::Rotation(const Expr<Scalar>::ConstPtr &chi)
    :   BasicUnaryExpr<Scalar, JonesMatrix>(chi)
{
}

const JonesMatrix::View Rotation::evaluateImpl(const Grid&,
    const Scalar::View &chi) const
{
    Matrix cosChi = cos(chi());
    Matrix sinChi = sin(chi());

    JonesMatrix::View result;
    result.assign(0, 0, cosChi);
    result.assign(0, 1, -sinChi);
    result.assign(1, 0, sinChi);
    result.assign(1, 1, cosChi);

    return result;
}

} //# namespace BBS
} //# namespace LOFAR
