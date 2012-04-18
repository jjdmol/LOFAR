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
#include <BBSKernel/Expr/CachePolicy.h>
#include <BBSKernel/Expr/HamakerDipole.h>
#include <BBSKernel/Expr/Literal.h>
#include <BBSKernel/Expr/MatrixInverse.h>
#include <BBSKernel/Expr/MatrixMul2.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/AntennaFieldAzEl.h>
#include <BBSKernel/Expr/ITRFDirection.h>
#include <BBSKernel/Expr/ScalarMatrixMul.h>
#include <BBSKernel/Expr/StationBeamFormer.h>
#include <BBSKernel/Expr/TileArrayFactor.h>
#include <Common/LofarLogger.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaParse.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSObservation.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSPolarization.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <measures/Measures/MeasTable.h>

namespace LOFAR
{
namespace BBS
{

StationResponse::StationResponse(const casa::MeasurementSet &ms,
    bool inverse, bool useElementBeam, bool useArrayFactor, bool conjugateAF)
    :   itsRefDelay(new Dummy<Vector<2> >()),
        itsRefTile(new Dummy<Vector<2> >()),
        itsRefOrientation(new Dummy<Scalar>()),
        itsDirection(new Dummy<Vector<2> >())
{
    // Set pointing and beamformer reference directions towards NCP by default.
    setRefDelay(casa::MDirection());
    setRefTile(casa::MDirection());
    setDirection(casa::MDirection());

    // The positive X dipole direction is SE of the reference orientation, which
    // translates to an azimuth of 3/4*pi.
    setRefOrientation(3.0 * casa::C::pi_4);

    // Load observation details.
    Instrument::Ptr instrument = initInstrument(ms);
    double refFreq = getReferenceFreq(ms);

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

    // Load beam model coefficients.
    HamakerBeamCoeff coeffLBA, coeffHBA;
    coeffLBA.init(casa::Path("$LOFARROOT/share/element_beam_HAMAKER_LBA"
        ".coeff"));
    coeffHBA.init(casa::Path("$LOFARROOT/share/element_beam_HAMAKER_HBA"
        ".coeff"));

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
                Expr<Vector<2> >::Ptr exprAzEl(new AntennaFieldAzEl(exprDirITRF,
                    field));

                if(field->isHBA())
                {
                    exprElementBeam[j] =
                        Expr<JonesMatrix>::Ptr(new HamakerDipole(coeffHBA,
                        exprAzEl, itsRefOrientation));
                }
                else
                {
                    exprElementBeam[j] =
                        Expr<JonesMatrix>::Ptr(new HamakerDipole(coeffLBA,
                        exprAzEl, itsRefOrientation));
                }
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

void StationResponse::setRefOrientation(double orientation)
{
    // Update dipole reference orientation.
    itsRefOrientation->setValue(Scalar(Matrix(orientation)));

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

Instrument::Ptr StationResponse::initInstrument(const casa::MeasurementSet &ms)
    const
{
    // Get station names and positions in ITRF coordinates.
    casa::ROMSAntennaColumns antenna(ms.antenna());
    casa::ROMSObservationColumns observation(ms.observation());
    ASSERT(observation.nrow() > 0);
    ASSERT(!observation.flagRow()(0));

    // Get instrument name.
    string name = observation.telescopeName()(0);

    // Get station positions.
    casa::MVPosition centroid;
    vector<Station::Ptr> stations(antenna.nrow());
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        // Get station name and ITRF position.
        casa::MPosition position =
            casa::MPosition::Convert(antenna.positionMeas()(i),
            casa::MPosition::ITRF)();

        // Store station information.
        stations[i] = initStation(ms, i, antenna.name()(i), position);

        // Update ITRF centroid.
        centroid += position.getValue();
    }

    // Get the instrument position in ITRF coordinates, or use the centroid
    // of the station positions if the instrument position is unknown.
    casa::MPosition position;
    if(casa::MeasTable::Observatory(position, name))
    {
        position = casa::MPosition::Convert(position, casa::MPosition::ITRF)();
    }
    else
    {
        LOG_WARN("Instrument position unknown; will use centroid of stations.");
        ASSERT(antenna.nrow() != 0);
        centroid *= 1.0 / static_cast<double>(antenna.nrow());
        position = casa::MPosition(centroid, casa::MPosition::ITRF);
    }

    return Instrument::Ptr(new Instrument(name, position, stations.begin(),
        stations.end()));
}

Station::Ptr StationResponse::initStation(const casa::MeasurementSet &ms,
    unsigned int id, const string &name, const casa::MPosition &position) const
{
    if(!ms.keywordSet().isDefined("LOFAR_ANTENNA_FIELD"))
    {
        return Station::Ptr(new Station(name, position));
    }

    casa::Table tab_field(ms.keywordSet().asTable("LOFAR_ANTENNA_FIELD"));
    tab_field = tab_field(tab_field.col("ANTENNA_ID")
        == static_cast<casa::Int>(id));

    const size_t nFields = tab_field.nrow();
    if(nFields < 1 || nFields > 2)
    {
        LOG_WARN_STR("Antenna " << name << " consists of an incompatible number"
            " of antenna fields. Beam model simulation will not work for this"
            " antenna.");
        return Station::Ptr(new Station(name, position));
    }

    casa::ROScalarColumn<casa::String> c_name(tab_field, "NAME");
    casa::ROArrayQuantColumn<casa::Double> c_position(tab_field, "POSITION",
        "m");
    casa::ROArrayQuantColumn<casa::Double> c_axes(tab_field, "COORDINATE_AXES",
        "m");
    casa::ROArrayQuantColumn<casa::Double> c_tile_offset(tab_field,
        "TILE_ELEMENT_OFFSET", "m");
    casa::ROArrayQuantColumn<casa::Double> c_offset(tab_field, "ELEMENT_OFFSET",
        "m");
    casa::ROArrayColumn<casa::Bool> c_flag(tab_field, "ELEMENT_FLAG");

    AntennaField::Ptr field[2];
    for(size_t i = 0; i < nFields; ++i)
    {
        // Read antenna field center.
        casa::Vector<casa::Quantum<casa::Double> > aips_position =
            c_position(i);
        ASSERT(aips_position.size() == 3);

        Vector3 position = {{aips_position(0).getValue(),
            aips_position(1).getValue(), aips_position(2).getValue()}};

        // Read antenna field coordinate axes.
        casa::Matrix<casa::Quantum<casa::Double> > aips_axes = c_axes(i);
        ASSERT(aips_axes.shape().isEqual(casa::IPosition(2, 3, 3)));

        Vector3 P = {{aips_axes(0, 0).getValue(), aips_axes(1, 0).getValue(),
            aips_axes(2, 0).getValue()}};
        Vector3 Q = {{aips_axes(0, 1).getValue(), aips_axes(1, 1).getValue(),
            aips_axes(2, 1).getValue()}};
        Vector3 R = {{aips_axes(0, 2).getValue(), aips_axes(1, 2).getValue(),
            aips_axes(2, 2).getValue()}};

        // Store information as AntennaField.
        field[i] = AntennaField::Ptr(new AntennaField(c_name(i), position, P,
            Q, R));

        if(c_name(i) != "LBA")
        {
            // Read tile configuration for HBA antenna fields.
            casa::Matrix<casa::Quantum<casa::Double> > aips_offset =
                c_tile_offset(i);
            ASSERT(aips_offset.nrow() == 3);

            const size_t nElement = aips_offset.ncolumn();
            for(size_t j = 0; j < nElement; ++j)
            {
                Vector3 offset = {{aips_offset(0, j).getValue(),
                    aips_offset(1, j).getValue(),
                    aips_offset(2, j).getValue()}};

                field[i]->appendTileElement(offset);
            }
        }

        // Read element position offsets and flags.
        casa::Matrix<casa::Quantum<casa::Double> > aips_offset = c_offset(i);
        casa::Matrix<casa::Bool> aips_flag = c_flag(i);

        const size_t nElement = aips_offset.ncolumn();
        ASSERT(aips_offset.shape().isEqual(casa::IPosition(2, 3, nElement)));
        ASSERT(aips_flag.shape().isEqual(casa::IPosition(2, 2, nElement)));

        for(size_t j = 0; j < nElement; ++j)
        {
            AntennaField::Element element;
            element.offset[0] = aips_offset(0, j).getValue();
            element.offset[1] = aips_offset(1, j).getValue();
            element.offset[2] = aips_offset(2, j).getValue();
            element.flag[0] = aips_flag(0, j);
            element.flag[1] = aips_flag(1, j);

            field[i]->appendElement(element);
        }
    }

    return (nFields == 1 ? Station::Ptr(new Station(name, position, field[0]))
        : Station::Ptr(new Station(name, position, field[0], field[1])));
}

double StationResponse::getReferenceFreq(const casa::MeasurementSet &ms) const
{
    // Read polarization id and spectral window id.
    casa::ROMSDataDescColumns desc(ms.dataDescription());
    ASSERT(desc.nrow() > 0);
    ASSERT(!desc.flagRow()(0));

    const unsigned int idWindow = desc.spectralWindowId()(0);

    // Get spectral information.
    casa::ROMSSpWindowColumns window(ms.spectralWindow());
    ASSERT(window.nrow() > idWindow);
    ASSERT(!window.flagRow()(idWindow));

    return window.refFrequency()(idWindow);
}

} //# namespace BBS
} //# namespace LOFAR
