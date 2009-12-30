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

StationUVW::StationUVW(const casa::MPosition &position,
    const casa::MPosition &array, const casa::MDirection &reference)
    :   itsPosition(casa::MPosition::Convert(position,
            casa::MPosition::ITRF)()),
        itsArrayPosition(casa::MPosition::Convert(array,
            casa::MPosition::ITRF)()),
        itsPhaseReference(casa::MDirection::Convert(reference,
            casa::MDirection::J2000)())
{
}

const Vector<3> StationUVW::evaluateExpr(const Request &request, Cache &cache,
    unsigned int grid) const
{
    // Get the station position relative to the array reference position
    // (to keep values small).
    const casa::MPosition mDeltaPos(itsPosition.getValue()
        - itsArrayPosition.getValue(), casa::MPosition::ITRF);

    // Setup coordinate transformation engine.
    casa::Quantum<casa::Double> qEpoch(0.0, "s");
    casa::MEpoch mEpoch(qEpoch, casa::MEpoch::UTC);

    casa::MeasFrame frame(itsArrayPosition);
    frame.set(itsPhaseReference);
    frame.set(mEpoch);

    casa::MVBaseline mvBaseline(mDeltaPos.getValue());
    casa::MBaseline mBaseline(mvBaseline, casa::MBaseline::ITRF);
    mBaseline.getRefPtr()->set(frame);

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
        const double time = timeAxis->center(i);

        qEpoch.setValue(time);
        mEpoch.set(qEpoch);
        frame.set(mEpoch);

        casa::MVuvw uvw2000(convertor().getValue(),
            itsPhaseReference.getValue());
        const casa::Vector<casa::Double> &xyz = uvw2000.getValue();

        *u++ = xyz(0);
        *v++ = xyz(1);
        *w++ = xyz(2);
    }

    Vector<3> result;
    result.assign(0, U);
    result.assign(1, V);
    result.assign(2, W);

    return result;
}

} //# namespace BBS
} //# namespace LOFAR
