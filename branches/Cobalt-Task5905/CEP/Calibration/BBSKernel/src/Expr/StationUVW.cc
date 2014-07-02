//# StationUVW.cc: Station UVW coordinates.
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
#include <BBSKernel/Expr/StationUVW.h>

#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MCBaseline.h>
#include <casa/Quanta/MVuvw.h>

namespace LOFAR
{
namespace BBS
{

StationUVW::StationUVW(const casa::MPosition &arrayPosition,
    const casa::MPosition &stationPosition,
    const casa::MDirection &direction)
    :   itsArrayPosition(casa::MPosition::Convert(arrayPosition,
            casa::MPosition::ITRF)()),
        itsStationPosition(casa::MPosition::Convert(stationPosition,
            casa::MPosition::ITRF)()),
        itsDirection(casa::MDirection::Convert(direction,
            casa::MDirection::J2000)())
{
}

const Vector<3> StationUVW::evaluateExpr(const Request &request, Cache&,
    unsigned int grid) const
{
    EXPR_TIMER_START();

    // Initialize reference frame.
    casa::Quantum<casa::Double> qEpoch(0.0, "s");
    casa::MEpoch mEpoch(qEpoch, casa::MEpoch::UTC);
    casa::MeasFrame mFrame(mEpoch, itsArrayPosition, itsDirection);

    // Use baseline coordinates relative to the array reference position (to
    // keep values small). The array reference position will drop out when
    // computing baseline UVW coordinates from a pair of "station" UVW
    // coordinates.
    casa::MVBaseline mvBaseline(itsStationPosition.getValue(),
        itsArrayPosition.getValue());
    casa::MBaseline mBaseline(mvBaseline,
        casa::MBaseline::Ref(casa::MBaseline::ITRF, mFrame));

    // Setup coordinate transformation engine.
    casa::MBaseline::Convert convertor(mBaseline, casa::MBaseline::J2000);

    // Allocate space for the result.
    // TODO: This is a hack! The Matrix class does not support 1xN or Nx1
    // "matrices".
    Axis::ShPtr timeAxis(request[grid][TIME]);
    const size_t nTime = timeAxis->size();

    Matrix U, V, W;
    double *u = U.setDoubleFormat(1, nTime);
    double *v = V.setDoubleFormat(1, nTime);
    double *w = W.setDoubleFormat(1, nTime);

    // Compute UVW coordinates.
    for(size_t i = 0; i < nTime; ++i)
    {
        // Update reference frame.
        qEpoch.setValue(timeAxis->center(i));
        mEpoch.set(qEpoch);
        mFrame.set(mEpoch);

        // Compute UVW coordinates (J2000).
        casa::MBaseline mBaselineJ2000(convertor());
        casa::MVuvw mvUVW(mBaselineJ2000.getValue(), itsDirection.getValue());

        *u++ = mvUVW(0);
        *v++ = mvUVW(1);
        *w++ = mvUVW(2);
    }

    Vector<3> result;
    result.assign(0, U);
    result.assign(1, V);
    result.assign(2, W);

    EXPR_TIMER_STOP();

    return result;
}

} //# namespace BBS
} //# namespace LOFAR
