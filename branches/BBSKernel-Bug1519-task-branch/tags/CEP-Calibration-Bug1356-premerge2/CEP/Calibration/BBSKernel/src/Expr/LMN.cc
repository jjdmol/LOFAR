//# LMN.cc: LMN-coordinates of a direction on the sky.
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

#include <BBSKernel/Expr/LMN.h>
#include <Common/lofar_math.h>

#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::sin;
using LOFAR::cos;

LMN::LMN(const casa::MDirection &reference,
    const Expr<Vector<2> >::ConstPtr &direction)
    :   BasicUnaryExpr<Vector<2>, Vector<3> >(direction),
        itsPhaseReference(casa::MDirection::Convert(reference,
            casa::MDirection::J2000)())
{
}

const Vector<3>::View LMN::evaluateImpl(const Request &request,
    const Vector<2>::View &direction) const
{
    casa::Quantum<casa::Vector<casa::Double> > angles =
        itsPhaseReference.getAngle();
    const double refRa = angles.getBaseValue()(0);
    const double refDec = angles.getBaseValue()(1);
    const double refCosDec = cos(refDec);
    const double refSinDec = sin(refDec);

    Matrix cosDec(cos(direction(1)));
    Matrix deltaRa(direction(0) - refRa);

    Vector<3>::View result;
    result.assign(0, cosDec * sin(deltaRa));
    result.assign(1, sin(direction(1)) * refCosDec - cosDec * refSinDec
        * cos(deltaRa));
    Matrix n = 1.0 - sqr(result(0)) - sqr(result(1));
    ASSERT(min(n).getDouble() >= 0.0);
    result.assign(2, sqrt(n));

    return result;
}

} // namespace BBS
} // namespace LOFAR
