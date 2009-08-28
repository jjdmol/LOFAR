//# StatUVW.cc: UVW coordinates of a station in meters.
//#
//# Copyright (C) 2002
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
    Vector<3> result;

    Matrix U, V, W;

    // Allocate result.
    size_t nTime = request[TIME]->size();

    double *u = U.setDoubleFormat(1, nTime);
    double *v = V.setDoubleFormat(1, nTime);
    double *w = W.setDoubleFormat(1, nTime);

    result.assign(0, U);
    result.assign(1, V);
    result.assign(2, W);

    size_t nDone = 0;
    for(size_t i = 0; i < nTime; ++i)
    {
        Time time(request[TIME]->center(i));
        map<Time, Uvw>::const_iterator it = itsUvwCache.find(time);

        if(it != itsUvwCache.end())
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
            map<Time, Uvw>::iterator it = itsUvwCache.find(Time(time));

            if(it == itsUvwCache.end())
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
                itsUvwCache[Time(time)] = Uvw(xyz(0), xyz(1), xyz(2));
            }
        }
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
