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
#include <BBSKernel/Expr/ArrayFactor.h>
#include <BBSKernel/Expr/AzEl.h>
#include <BBSKernel/Expr/CachePolicy.h>
#include <BBSKernel/Expr/HamakerDipole.h>
#include <BBSKernel/Expr/Literal.h>
#include <BBSKernel/Expr/MatrixInverse.h>
#include <BBSKernel/Expr/MatrixMul2.h>
#include <BBSKernel/Expr/TileArrayFactor.h>

#include <Common/LofarLogger.h>

#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>

namespace LOFAR
{
namespace BBS
{

StationResponse::StationResponse(Instrument instrument,
    const string &config,
    const casa::Path &configPath,
    double referenceFreq,
    bool inverse)
    :   itsPointing(new Dummy<Vector<2> >()),
        itsDirection(new Dummy<Vector<2> >())
{
    LOG_DEBUG_STR("Config name: " << config);
    LOG_DEBUG_STR("Config path: " << configPath.originalName());

    // Load antenna configurations from disk.
    instrument.readLOFARAntennaConfig(configPath);

    // Load element beam model coefficients from disk.
    HamakerBeamCoeff coeff;
    bool observationLBA = true;
    if(referenceFreq >= 10e6 && referenceFreq <= 90e6)
    {
        LOG_DEBUG_STR("Using LBA element beam model.");
        if(referenceFreq < 32.5e6 || referenceFreq > 77.5e6)
        {
            LOG_WARN_STR("Reference frequency outside of model domain"
                " [32.5 MHz, 77.5 MHz].");
        }

        coeff.init(casa::Path("$LOFARROOT/share/element_beam_HAMAKER_LBA"
            ".coeff"));
    }
    else if(referenceFreq >= 110e6 && referenceFreq <= 270e6)
    {
        LOG_DEBUG_STR("Using HBA element beam model.");
        if(referenceFreq < 150e6 || referenceFreq > 210e6)
        {
            LOG_WARN_STR("Reference frequency outside of model domain [150 MHz,"
                " 210 MHz].");
        }

        observationLBA = false;
        coeff.init(casa::Path("$LOFARROOT/share/element_beam_HAMAKER_HBA"
            ".coeff"));
    }
    else
    {
        THROW(BBSKernelException, "Reference frequency not contained in any"
            " valid LOFAR frequency range.");
    }

    // Default positive X-dipole orientation.
    Expr<Scalar>::Ptr exprOrientation(new Literal(-casa::C::pi_2 / 2.0));

    itsExpr.reserve(instrument.size());
    for(size_t i = 0; i < instrument.size(); ++i)
    {
        const Station &station = instrument[i];

        // The (azimuth, elevantion) coordinates of the pointing direction as
        // seen from this station.
        Expr<Vector<2> >::Ptr exprRefAzEl(new AzEl(station.position(),
            itsPointing));

        // The (azimuth, elevantion) coordinates of the direction of interest as
        // seen from this station.
        Expr<Vector<2> >::Ptr exprAzEl(new AzEl(station.position(),
            itsDirection));

        // Element beam.
        Expr<JonesMatrix>::Ptr exprBeam(new HamakerDipole(coeff, exprAzEl,
            exprOrientation));

        // Get LOFAR station name suffix.
        // NB. THIS IS A TEMPORARY SOLUTION THAT CAN BE REMOVED AS SOON AS THE
        // DIPOLE INFORMATION IS STORED AS META-DATA INSIDE THE MS.
        AntennaSelection selection;
        const string suffix = station.name().substr(5);
        if(suffix == "LBA")
        {
            ASSERT(observationLBA);

            try
            {
                selection = station.selection(config);
            }
            catch(BBSKernelException &ex)
            {
                // If a specific LBA configuration is not available, use the
                // default configuration (this seems to be what is also done
                // when observing).
                selection = station.selection("LBA");
            }
        }
        else if(suffix == "HBA0")
        {
            ASSERT(!observationLBA);

            selection = station.selection("HBA_0");

            Expr<JonesMatrix>::Ptr exprTileAF(new TileArrayFactor(exprAzEl,
                exprRefAzEl, station.tile(0)));
            exprBeam = compose(exprTileAF, exprBeam);
        }
        else if(suffix == "HBA1")
        {
            ASSERT(!observationLBA);

            selection = station.selection("HBA_1");

            Expr<JonesMatrix>::Ptr exprTileAF(new TileArrayFactor(exprAzEl,
                exprRefAzEl, station.tile(1)));
            exprBeam = compose(exprTileAF, exprBeam);
        }
        else if(suffix == "HBA")
        {
            ASSERT(!observationLBA);

            selection = station.selection("HBA");

            Expr<JonesMatrix>::Ptr exprTileAF(new TileArrayFactor(exprAzEl,
                exprRefAzEl, station.tile(0)));
            exprBeam = compose(exprTileAF, exprBeam);
        }
        else
        {
            THROW(BBSKernelException, "Illegal LOFAR station name: "
                << station.name());
        }

        // Create ArrayFactor expression.
        Expr<JonesMatrix>::Ptr exprStationAF(new ArrayFactor(exprAzEl,
            exprRefAzEl, selection, referenceFreq));

        exprBeam = compose(exprStationAF, exprBeam);

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

void StationResponse::setPointing(const casa::MDirection &pointing)
{
    // Convert to ensure the pointing is specified with respect to the correct
    // reference (J2000).
    casa::MDirection pointingJ2K(casa::MDirection::Convert(pointing,
        casa::MDirection::J2000)());
    casa::Quantum<casa::Vector<casa::Double> > angles(pointingJ2K.getAngle());

    // Update pointing direction.
    Vector<2> radec;
    radec.assign(0, Matrix(angles.getBaseValue()(0)));
    radec.assign(1, Matrix(angles.getBaseValue()(1)));
    itsPointing->setValue(radec);

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
StationResponse::compose(const Expr<JonesMatrix>::Ptr &accumulator,
    const Expr<JonesMatrix>::Ptr &effect) const
{
    if(accumulator)
    {
        return Expr<JonesMatrix>::Ptr(new MatrixMul2(accumulator, effect));
    }

    return effect;
}

} //# namespace BBS
} //# namespace LOFAR
