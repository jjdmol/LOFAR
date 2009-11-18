//# StatUVW.cc: UVW coordinates of a station in meters.
//#
//# Copyright (C) 2002
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

#include <BBSKernel/Expr/StatUVW.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>

#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MCBaseline.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasFrame.h>
#include <casa/Quanta/MVuvw.h>
#include <stdint.h>

//using namespace casa;

namespace LOFAR
{
namespace BBS
{

StatUVW::StatUVW(const casa::MPosition &position, const casa::MPosition &array,
    const casa::MDirection &reference)
    :   itsPosition(casa::MPosition::Convert(position,
            casa::MPosition::ITRF)()),
        itsArrayPosition(casa::MPosition::Convert(array,
            casa::MPosition::ITRF)()),
        itsPhaseReference(casa::MDirection::Convert(reference,
            casa::MDirection::J2000)())
{
}

const Vector<3> StatUVW::evaluateExpr(const Request &request, Cache &cache)
    const
{
    // Allocate result.
    size_t nTime = request[TIME]->size();

    Matrix U, V, W;
    double *u = U.setDoubleFormat(1, nTime);
    double *v = V.setDoubleFormat(1, nTime);
    double *w = W.setDoubleFormat(1, nTime);

    Vector<3> result;
    result.assign(0, U);
    result.assign(1, V);
    result.assign(2, W);

    size_t nDone = 0;
    for(size_t i = 0; i < nTime; ++i)
    {
        Timestamp time(request[TIME]->center(i));
        map<Timestamp, UVW>::const_iterator it = itsUVWCache.find(time);

        if(it != itsUVWCache.end())
        {
            u[i] = it->second.u;
            v[i] = it->second.v;
            w[i] = it->second.w;
            ++nDone;
        }
    }

    // If all done then return.
    if(nDone != nTime)
    {
        // Compute missing UVW coordinates using the AIPS++ measures.

        // Get the station position relative to the array reference position
        // (to keep values small).
        const casa::MPosition mPos(itsPosition.getValue()
            - itsArrayPosition.getValue());

        //# Setup coordinate transformation engine.
        casa::Quantum<double> qEpoch(0.0, "s");
        casa::MEpoch mEpoch(qEpoch, casa::MEpoch::UTC);

        casa::MeasFrame frame(itsArrayPosition);
        frame.set(itsPhaseReference);
        frame.set(mEpoch);

        casa::MVBaseline mvBaseline(mPos.getValue());
        casa::MBaseline mBaseline(mvBaseline, casa::MBaseline::ITRF);
        mBaseline.getRefPtr()->set(frame);

        casa::MBaseline::Convert convertor(mBaseline, casa::MBaseline::J2000);

        //# Compute missing UVW coordinates.
        for(size_t i = 0; i < nTime; ++i)
        {
            const double time = request[TIME]->center(i);
            map<Timestamp, UVW>::iterator it =
                itsUVWCache.find(Timestamp(time));

            if(it == itsUVWCache.end())
            {
                qEpoch.setValue(time);
                mEpoch.set(qEpoch);
                frame.set(mEpoch);

                casa::MVuvw uvw2000(convertor().getValue(),
                    itsPhaseReference.getValue());
                const casa::Vector<double> &xyz = uvw2000.getValue();

                u[i] = xyz(0);
                v[i] = xyz(1);
                w[i] = xyz(2);

                // Update UVW cache.
                itsUVWCache[Timestamp(time)] = UVW(xyz(0), xyz(1), xyz(2));
            }
        }
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
