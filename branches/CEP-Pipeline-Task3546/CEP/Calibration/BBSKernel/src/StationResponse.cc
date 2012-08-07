//# StationResponse.cc: A set of expressions to compute the response of a
//# station (JonesMatrix) to radiation from a specific direction.
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

#include <lofar_config.h>
#include <BBSKernel/StationResponse.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/MeasurementAIPS.h>
#include <BBSKernel/MeasurementExprLOFARUtil.h>
#include <BBSKernel/ModelConfig.h>
#include <BBSKernel/Expr/CachePolicy.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/ITRFDirection.h>
#include <BBSKernel/Expr/Literal.h>
#include <BBSKernel/Expr/MatrixInverse.h>
#include <Common/LofarLogger.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>

namespace LOFAR
{
namespace BBS
{

StationResponse::StationResponse(const casa::MeasurementSet &ms,
    bool inverse, bool useElementBeam, bool useArrayFactor, bool useChannelFreq,
    bool conjugateAF)
    :   itsRefDelay(new Dummy<Vector<2> >()),
        itsRefTile(new Dummy<Vector<2> >()),
        itsDirection(new Dummy<Vector<2> >())
{
    // Set pointing and beamformer reference directions towards NCP by default.
    setRefDelay(casa::MDirection());
    setRefTile(casa::MDirection());
    setDirection(casa::MDirection());

    // Load observation details.
    Instrument::Ptr instrument = readInstrument(ms);
    double refFreq = readFreqReference(ms);

    // The ITRF direction vectors for the direction of interest and the
    // reference direction are computed w.r.t. the center of the station
    // (the phase reference position).
    Expr<Vector<3> >::Ptr exprDirITRF(new ITRFDirection(instrument->position(),
        itsDirection));
    Expr<Vector<3> >::Ptr exprRefDelayITRF =
        Expr<Vector<3> >::Ptr(new ITRFDirection(instrument->position(),
        itsRefDelay));
    Expr<Vector<3> >::Ptr exprRefTileITRF =
        Expr<Vector<3> >::Ptr(new ITRFDirection(instrument->position(),
        itsRefTile));

    Expr<Scalar>::Ptr exprOne(new Literal(1.0));
    Expr<JonesMatrix>::Ptr exprIdentity(new AsDiagonalMatrix(exprOne, exprOne));

    itsExpr.resize(instrument->nStations(), exprIdentity);
    if(useElementBeam || useArrayFactor)
    {
        BeamConfig::Mode mode = (!useElementBeam ? BeamConfig::ARRAY_FACTOR
          : (!useArrayFactor ? BeamConfig::ELEMENT : BeamConfig::DEFAULT));
        BeamConfig beamConfig(mode, useChannelFreq, conjugateAF);

        for(size_t i = 0; i < instrument->nStations(); ++i)
        {
            Station::ConstPtr station = instrument->station(i);

            // Check preconditions.
            if(!station->isPhasedArray())
            {
                LOG_WARN_STR("Station " << station->name() << " is not a LOFAR"
                    " station or the additional information needed to compute"
                    " the station beam is missing. The station beam model will"
                    " NOT be applied.");
                continue;
            }

            itsExpr[i] = makeBeamExpr(station, refFreq, exprDirITRF,
                exprRefDelayITRF, exprRefTileITRF, beamConfig);

            if(inverse)
            {
                itsExpr[i] =
                    Expr<JonesMatrix>::Ptr(new MatrixInverse(itsExpr[i]));
            }
        }
    }

    // Apply default cache policy (caches the results of all shared nodes).
    DefaultCachePolicy policy;
    policy.apply(itsExpr.begin(), itsExpr.end());
}

void StationResponse::setRefDelay(const casa::MDirection &reference)
{
    // Convert to ensure the delay reference is specified with respect to the
    // correct epoch (J2000).
    casa::MDirection referenceJ2K(casa::MDirection::Convert(reference,
        casa::MDirection::J2000)());
    casa::Quantum<casa::Vector<casa::Double> > angles(referenceJ2K.getAngle());

    // Update pointing direction.
    Vector<2> radec;
    radec.assign(0, Matrix(angles.getBaseValue()(0)));
    radec.assign(1, Matrix(angles.getBaseValue()(1)));
    itsRefDelay->setValue(radec);

    // Clear cache.
    itsCache.clear();
    itsCache.clearStats();
}

void StationResponse::setRefTile(const casa::MDirection &reference)
{
    // Convert to ensure the delay reference is specified with respect to the
    // correct epoch (J2000).
    casa::MDirection referenceJ2K(casa::MDirection::Convert(reference,
        casa::MDirection::J2000)());
    casa::Quantum<casa::Vector<casa::Double> > angles(referenceJ2K.getAngle());

    // Update pointing direction.
    Vector<2> radec;
    radec.assign(0, Matrix(angles.getBaseValue()(0)));
    radec.assign(1, Matrix(angles.getBaseValue()(1)));
    itsRefTile->setValue(radec);

    // Clear cache.
    itsCache.clear();
    itsCache.clearStats();
}

void StationResponse::setDirection(const casa::MDirection &direction)
{
    // Convert to ensure the direction is specified with respect to the correct
    // reference (J2000).
    casa::MDirection directionJ2K(casa::MDirection::Convert(direction,
        casa::MDirection::J2000)());
    casa::Quantum<casa::Vector<casa::Double> > angles(directionJ2K.getAngle());

    // Update direction of interest.
    Vector<2> radec;
    radec.assign(0, Matrix(angles.getBaseValue()(0)));
    radec.assign(1, Matrix(angles.getBaseValue()(1)));
    itsDirection->setValue(radec);

    // Clear cache.
    itsCache.clear();
    itsCache.clearStats();
}

void StationResponse::setEvalGrid(const Grid &grid)
{
    itsRequest = Request(grid);

    // Clear cache.
    itsCache.clear();
    itsCache.clearStats();
}

unsigned int StationResponse::nStations() const
{
    return itsExpr.size();
}

const JonesMatrix::View StationResponse::evaluate(unsigned int i)
{
    ASSERT(i < itsExpr.size());
    const JonesMatrix result = itsExpr[i]->evaluate(itsRequest, itsCache, 0);

    return result.view();
}

} //# namespace BBS
} //# namespace LOFAR
