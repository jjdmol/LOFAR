//# StationResponse.h: Voltage response of a LOFAR phased array station.
//#
//# Copyright (C) 2013
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

#ifndef LOFAR_BBSKERNEL_EXPR_STATIONRESPONSE_H
#define LOFAR_BBSKERNEL_EXPR_STATIONRESPONSE_H

// \file
// Voltage response of a LOFAR phased array station.

#include <BBSKernel/Expr/BasicExpr.h>
#include <BBSKernel/StationLOFAR.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class StationResponse: public BasicTernaryExpr<Vector<3>, Vector<3>, Vector<3>,
    JonesMatrix>
{
public:
    typedef shared_ptr<StationResponse>       Ptr;
    typedef shared_ptr<const StationResponse> ConstPtr;

    StationResponse(const Expr<Vector<3> >::ConstPtr &direction,
        const Expr<Vector<3> >::ConstPtr &station0,
        const Expr<Vector<3> >::ConstPtr &tile0,
        const LOFAR::StationResponse::Station::ConstPtr &station);

    /*!
     *  \brief Use the specified reference frequency to compute the station
     *  beam former weights, instead of the channel frequencies.
     *
     *  \see useChannelFreq()
     */
    void useReferenceFreq(double freq);

    /*!
     *  \brief Use the channel frequencies to compute the station beam former
     *  weights.
     *
     *  \see useReferenceFreq(double freq)
     */
    void useChannelFreq();

    /*!
     *  \brief Switch computation of the array factor on or off.
     *
     *  When switched off, the array factor defaults to 1.0, i.e. the array
     *  factor of a single element "array". By default, computation of the array
     *  factor is switched on.
     */
    void useArrayFactor(bool use);

    /*!
     *  \brief Switch computation of the element response on or off.
     *
     *  When switched off, the element response defaults to the identity matrix,
     *  i.e. the response of an idealized isotropic element. By default,
     *  computation of the element response is switched on.
     */
    void useElementResponse(bool use);

protected:
    const JonesMatrix::View evaluateImpl(const Grid &grid,
        const Vector<3>::View &direction, const Vector<3>::View &station0,
        const Vector<3>::View &tile0) const;

private:
    const JonesMatrix::View evaluateImplResponse(const Grid &grid,
        const Vector<3>::View &direction, const Vector<3>::View &station0,
        const Vector<3>::View &tile0) const;

    const JonesMatrix::View evaluateImplArrayFactor(const Grid &grid,
        const Vector<3>::View &direction, const Vector<3>::View &station0,
        const Vector<3>::View &tile0) const;

    const JonesMatrix::View evaluateImplElementResponse(const Grid &grid,
        const Vector<3>::View &direction) const;

    LOFAR::StationResponse::Station::ConstPtr   itsStation;
    bool                                        itsUseChannelFreq;
    bool                                        itsUseArrayFactor;
    bool                                        itsUseElementResponse;
    double                                      itsReferenceFreq;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
