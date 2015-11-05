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
#include <BBSKernel/Expr/AntennaElementLBA.h>
#include <BBSKernel/Expr/AntennaElementHBA.h>
#include <BBSKernel/Expr/AntennaFieldThetaPhi.h>
#include <BBSKernel/Expr/CachePolicy.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/ITRFDirection.h>
#include <BBSKernel/Expr/Literal.h>
#include <BBSKernel/Expr/MatrixInverse.h>
#include <BBSKernel/Expr/MatrixMul2.h>
#include <BBSKernel/Expr/ParallacticRotation.h>
#include <BBSKernel/Expr/ScalarMatrixMul.h>
#include <BBSKernel/Expr/StationBeamFormer.h>
#include <BBSKernel/Expr/TileArrayFactor.h>
#include <Common/LofarLogger.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>

namespace LOFAR
{
namespace BBS
{

StationResponse::StationResponse(const casa::MeasurementSet &ms,
    bool inverse, bool useElementBeam, bool useArrayFactor, bool conjugateAF)
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

    itsExpr.reserve(instrument->nStations());
    for(size_t i = 0; i < instrument->nStations(); ++i)
    {
        Station::ConstPtr station = instrument->station(i);

        // Check preconditions.
        if(!station->isPhasedArray())
        {
            LOG_WARN_STR("Station " << station->name() << " is not a LOFAR"
                " station or the additional information needed to compute the"
                " station beam is missing. The station beam model will NOT be"
                " applied.");

            Expr<Scalar>::Ptr exprOne(new Literal(1.0));
            Expr<JonesMatrix>::Ptr exprIdentity(new AsDiagonalMatrix(exprOne,
                exprOne));
            itsExpr.push_back(exprIdentity);
            continue;
        }

        // Build expressions for the dual-dipole or tile beam of each antenna
        // field.
        Expr<JonesMatrix>::Ptr exprElementBeam[2];
        for(size_t j = 0; j < station->nField(); ++j)
        {
            AntennaField::ConstPtr field = station->field(j);

            // Element (dual-dipole) beam expression.
            if(useElementBeam)
            {
                Expr<Vector<2> >::Ptr exprThetaPhi =
                    Expr<Vector<2> >::Ptr(new AntennaFieldThetaPhi(exprDirITRF,
                    field));

                if(field->isHBA())
                {
                    exprElementBeam[j] =
                        Expr<JonesMatrix>::Ptr(new AntennaElementHBA(exprThetaPhi));
                }
                else
                {
                    exprElementBeam[j] =
                        Expr<JonesMatrix>::Ptr(new AntennaElementLBA(exprThetaPhi));
                }

                Expr<JonesMatrix>::Ptr exprRotation =
                    Expr<JonesMatrix>::Ptr(new ParallacticRotation(exprDirITRF,
                    field));

                exprElementBeam[j] =
                    Expr<JonesMatrix>::Ptr(new MatrixMul2(exprElementBeam[j],
                    exprRotation));
            }
            else
            {
                Expr<Scalar>::Ptr exprOne(new Literal(1.0));
                Expr<JonesMatrix>::Ptr exprIdentity =
                    Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(exprOne,
                    exprOne));
                exprElementBeam[j] = exprIdentity;
            }

            // Tile array factor.
            if(field->isHBA() && useArrayFactor)
            {
                Expr<Scalar>::Ptr exprTileFactor =
                    Expr<Scalar>::Ptr(new TileArrayFactor(exprDirITRF,
                        exprRefTileITRF, field, conjugateAF));
                exprElementBeam[j] =
                    Expr<JonesMatrix>::Ptr(new ScalarMatrixMul(exprTileFactor,
                    exprElementBeam[j]));
            }
        }

        Expr<JonesMatrix>::Ptr exprBeam;
        if(!useArrayFactor)
        {
            // If the station consists of multiple antenna fields, but beam
            // forming is disabled, then we have to decide which antenna field
            // to use. By default the first antenna field will be used. The
            // differences between the dipole beam response of the antenna
            // fields of a station should only vary as a result of differences
            // in the field coordinate systems (because all dipoles are oriented
            // the same way).

            exprBeam = exprElementBeam[0];
        }
        else if(station->nField() == 1)
        {
            exprBeam = Expr<JonesMatrix>::Ptr(new StationBeamFormer(exprDirITRF,
                exprRefDelayITRF, exprElementBeam[0], station, refFreq,
                conjugateAF));
        }
        else
        {
            exprBeam = Expr<JonesMatrix>::Ptr(new StationBeamFormer(exprDirITRF,
                exprRefDelayITRF, exprElementBeam[0], exprElementBeam[1],
                station, refFreq, conjugateAF));
        }

        if(inverse)
        {
            exprBeam = Expr<JonesMatrix>::Ptr(new MatrixInverse(exprBeam));
        }

        itsExpr.push_back(exprBeam);
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

Expr<JonesMatrix>::Ptr
StationResponse::compose(const Expr<JonesMatrix>::Ptr &lhs,
    const Expr<JonesMatrix>::Ptr &rhs) const
{
    if(lhs)
    {
        return Expr<JonesMatrix>::Ptr(new MatrixMul2(lhs, rhs));
    }

    return rhs;
}

} //# namespace BBS
} //# namespace LOFAR
