//# StationResponse.h: A set of expressions to compute the response of a station
//# (JonesMatrix) to radiation from a specific direction.
//#
//# Copyright (C) 2010
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

#ifndef LOFAR_BBSKERNEL_STATIONRESPONSE_H
#define LOFAR_BBSKERNEL_STATIONRESPONSE_H

// \file
// A set of expressions to compute the response of a station (JonesMatrix) to
// radiation from a specific direction.

#include <BBSKernel/Instrument.h>
#include <BBSKernel/ModelConfig.h>
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/Scope.h>

#include <Common/lofar_smartptr.h>
#include <Common/lofar_vector.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/MDirection.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

// Dummy class used to allow updates to the reference direction and the
// direction of interest.
template <typename T_EXPR>
class Dummy: public Expr<T_EXPR>
{
public:
    typedef shared_ptr<Dummy>       Ptr;
    typedef shared_ptr<const Dummy> ConstPtr;

    Dummy()
    {
    }

    explicit Dummy(const T_EXPR &value)
        :   itsValue(value)
    {
    }

    void setValue(const T_EXPR &value)
    {
        itsValue = value;
    }

protected:
    virtual unsigned int nArguments() const
    {
        return 0;
    }

    virtual ExprBase::ConstPtr argument(unsigned int) const
    {
        ASSERT(false);
    }

    virtual const T_EXPR evaluateExpr(const Request&, Cache&, unsigned int)
        const
    {
        return itsValue;
    }

private:
    T_EXPR  itsValue;
};

class StationResponse
{
public:
    typedef shared_ptr<StationResponse>         Ptr;
    typedef shared_ptr<const StationResponse>   ConstPtr;

    StationResponse(const casa::MeasurementSet &ms, bool inverse = false,
        bool useElementBeam = true, bool useArrayFactor = true,
        bool useChannelFreq = false, bool conjugateAF = false);

    // Set the delay reference direction (used by the station beamformer).
    void setRefDelay(const casa::MDirection &reference);

    // Set the tile delay reference direction (used by the tile beamformer).
    void setRefTile(const casa::MDirection &reference);

    // Set the direction of interest.
    void setDirection(const casa::MDirection &direction);

    // Set the grid on which the station responses will be evaluated.
    void setEvalGrid(const Grid &grid);

    // Return the number of stations.
    unsigned int nStations() const;

    // Compute the response of the station with index i on the evaluation grid
    // and return the result.
    const JonesMatrix::View evaluate(unsigned int i);

private:
    Dummy<Vector<2> >::Ptr          itsRefDelay;
    Dummy<Vector<2> >::Ptr          itsRefTile;
    Dummy<Vector<2> >::Ptr          itsDirection;
    vector<Expr<JonesMatrix>::Ptr>  itsExpr;
    Request                         itsRequest;
    Cache                           itsCache;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
