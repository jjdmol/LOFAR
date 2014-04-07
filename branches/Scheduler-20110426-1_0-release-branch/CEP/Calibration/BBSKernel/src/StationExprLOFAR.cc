//# StationExprLOFAR.cc: Expression for the response (Jones matrix) of a set of
//# LOFAR stations.
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
#include <BBSKernel/StationExprLOFAR.h>
#include <BBSKernel/Exceptions.h>

#include <BBSKernel/Expr/AntennaFieldAzEl.h>
#include <BBSKernel/Expr/AzEl.h>
#include <BBSKernel/Expr/CachePolicy.h>
#include <BBSKernel/Expr/Delay.h>
#include <BBSKernel/Expr/EquatorialCentroid.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/FaradayRotation.h>
#include <BBSKernel/Expr/ITRFDirection.h>
#include <BBSKernel/Expr/Literal.h>
#include <BBSKernel/Expr/MatrixInverse.h>
#include <BBSKernel/Expr/MatrixMul2.h>
#include <BBSKernel/Expr/ScalarMatrixMul.h>
#include <BBSKernel/Expr/StationBeamFormer.h>
#include <BBSKernel/Expr/TileArrayFactor.h>

#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>

namespace LOFAR
{
namespace BBS
{

StationExprLOFAR::StationExprLOFAR(SourceDB &sourceDB,
    const ModelConfig &config, const Instrument::ConstPtr &instrument,
    const casa::MDirection &phaseReference, double referenceFreq, bool inverse)
    :   itsInstrument(instrument),
        itsPhaseReference(phaseReference),
        itsReferenceFreq(referenceFreq)
{
    initialize(sourceDB, config, inverse);
}

StationExprLOFAR::StationExprLOFAR(SourceDB &sourceDB,
    const ModelConfig &config, const VisBuffer::Ptr &buffer, bool inverse)
    :   itsInstrument(buffer->instrument()),
        itsPhaseReference(buffer->getPhaseReference()),
        itsReferenceFreq(buffer->getReferenceFreq())
{
    initialize(sourceDB, config, inverse);
}

void StationExprLOFAR::initialize(SourceDB &sourceDB, const ModelConfig &config,
    bool inverse)
{
    // Allocate space for the station response expressions.
    itsExpr.resize(itsInstrument->nStations());

    // Direction independent effects (DIE).
    for(size_t i = 0; i < itsExpr.size(); ++i)
    {
        // Create a clock delay expression per station.
        if(config.useClock())
        {
            itsExpr[i] = compose(itsExpr[i],
                makeClockExpr(itsInstrument->station(i)));
        }

        // Bandpass.
        if(config.useBandpass())
        {
            itsExpr[i] = compose(itsExpr[i],
                makeBandpassExpr(itsInstrument->station(i)));
        }

        // Create a direction independent gain expression per station.
        if(config.useGain())
        {
            itsExpr[i] = compose(itsExpr[i],
                makeGainExpr(itsInstrument->station(i), config.usePhasors()));
        }
    }

    // Direction dependent effects (DDE).
    if(config.useDirectionalGain() || config.useBeam()
        || config.useFaradayRotation() || config.useIonosphere())
    {
        // Phase reference position on the sky.
        Expr<Vector<2> >::Ptr exprRefPosition =
            makeRefPositionExpr(itsPhaseReference);

        // Position of interest on the sky (given as patch name).
        if(config.getSources().size() != 1)
        {
            THROW(BBSKernelException, "No patch, or more than one patch"
                " selected, yet corrections can only be applied for a single"
                " direction on the sky");
        }
        string patch = config.getSources().front();
        Expr<Vector<2> >::Ptr exprPatchPosition =
            makePatchPositionExpr(sourceDB, patch);

        // Functor for the creation of the ionosphere sub-expression.
        IonosphereExpr::Ptr exprIonosphere;
        if(config.useIonosphere())
        {
            exprIonosphere =
                IonosphereExpr::create(config.getIonosphereConfig(), itsScope);
        }

        for(size_t i = 0; i < itsExpr.size(); ++i)
        {
            if(config.useDirectionalGain())
            {
                itsExpr[i] = compose(itsExpr[i],
                    makeDirectionalGainExpr(itsInstrument->station(i), patch,
                    config.usePhasors()));
            }

            if(config.useBeam())
            {
                // Create beam expression.
                itsExpr[i] = compose(itsExpr[i],
                    makeBeamExpr(itsInstrument->station(i), itsReferenceFreq,
                    config.getBeamConfig(), exprPatchPosition,
                    exprRefPosition));
            }

            if(config.useFaradayRotation())
            {
                itsExpr[i] = compose(itsExpr[i],
                    makeFaradayRotationExpr(itsInstrument->station(i), patch));
            }

            if(config.useIonosphere())
            {
                // Create an AZ, EL expression per station for the centroid
                // direction of the patch.
                Expr<Vector<2> >::Ptr exprAzEl =
                    makeAzElExpr(itsInstrument->station(i), exprPatchPosition);

                itsExpr[i] = compose(itsExpr[i],
                    makeIonosphereExpr(itsInstrument->station(i),
                    itsInstrument->position(), exprAzEl, exprIonosphere));
            }
        }
    }

    Expr<Scalar>::Ptr exprOne(new Literal(1.0));
    Expr<JonesMatrix>::Ptr exprIdentity(new AsDiagonalMatrix(exprOne, exprOne));
    for(size_t i = 0; i < itsExpr.size(); ++i)
    {
        if(!itsExpr[i])
        {
            itsExpr[i] = exprIdentity;
        }

        if(inverse)
        {
            itsExpr[i] = Expr<JonesMatrix>::Ptr(new MatrixInverse(itsExpr[i]));
        }
    }

    // Set caching policy.
    itsCachePolicy = CachePolicy::Ptr(new DefaultCachePolicy());
    if(config.useCache())
    {
        itsCachePolicy = CachePolicy::Ptr(new ExperimentalCachePolicy());
    }

    itsCachePolicy->apply(itsExpr.begin(), itsExpr.end());
}

size_t StationExprLOFAR::size() const
{
    return itsExpr.size();
}

Box StationExprLOFAR::domain() const
{
    return ParmManager::instance().domain();
}

ParmGroup StationExprLOFAR::parms() const
{
    ParmGroup result;

    for(Scope::const_iterator parm_it = itsScope.begin(),
        parm_end = itsScope.end(); parm_it != parm_end; ++parm_it)
    {
        result.insert(parm_it->first);
    }

    return result;
}

ParmGroup StationExprLOFAR::solvables() const
{
    ParmGroup result;

    for(Scope::const_iterator parm_it = itsScope.begin(),
        parm_end = itsScope.end(); parm_it != parm_end; ++parm_it)
    {
        if(parm_it->second->getPValueFlag())
        {
            result.insert(parm_it->first);
        }
    }

    return result;
}

size_t StationExprLOFAR::nParms() const
{
    return itsScope.size();
}

void StationExprLOFAR::setSolvables(const ParmGroup &solvables)
{
    // Clear the flag that controls whether or not partial derivatives are
    // computed for all parameters.
    for(Scope::const_iterator parm_it = itsScope.begin(),
        parm_end = itsScope.end(); parm_it != parm_end; ++parm_it)
    {
        parm_it->second->clearPValueFlag();
    }

    // Make sure a partial derivative is computed for each solvable that is
    // part of this expression (i.e. that can be found in itsScope).
    for(ParmGroup::const_iterator sol_it = solvables.begin(),
        sol_end = solvables.end(); sol_it != sol_end; ++sol_it)
    {
        Scope::iterator parm_it = itsScope.find(*sol_it);
        if(parm_it != itsScope.end())
        {
            parm_it->second->setPValueFlag();
        }
    }

    // Clear any cached results and reinitialize the caching policy.
    itsCache.clear();
    itsCache.clearStats();
    itsCachePolicy->apply(itsExpr.begin(), itsExpr.end());
}

void StationExprLOFAR::clearSolvables()
{
    // Clear the flag that controls whether or not partial derivatives are
    // computed for all parameters.
    for(Scope::const_iterator parm_it = itsScope.begin(),
        parm_end = itsScope.end(); parm_it != parm_end; ++parm_it)
    {
        parm_it->second->clearPValueFlag();
    }

    // Clear any cached results and reinitialize the caching policy.
    itsCache.clear();
    itsCache.clearStats();
    itsCachePolicy->apply(itsExpr.begin(), itsExpr.end());
}

void StationExprLOFAR::solvablesChanged()
{
    itsCache.clear(Cache::VOLATILE);
    itsCache.clearStats();
}

void StationExprLOFAR::setEvalGrid(const Grid &grid)
{
    itsRequest = Request(grid);

    itsCache.clear();
    itsCache.clearStats();

    // TODO: Set cache size in number of Matrix instances... ?
}

const JonesMatrix StationExprLOFAR::evaluate(unsigned int i)
{
    JonesMatrix result;

    // Evaluate the model.
    ASSERT(i < itsExpr.size());
    const JonesMatrix model = itsExpr[i]->evaluate(itsRequest, itsCache, 0);

    // Pass-through the flags.
    result.setFlags(model.flags());

    // Pass-through the model value.
    const JonesMatrix::View value(model.view());
    result.assign(value);

    // Compute (approximate) partial derivatives using forward differences.
    PValueKey key;
    JonesMatrix::Iterator it(model);
    while(!it.atEnd())
    {
        key = it.key();

        // Get the perturbed value associated with the current (parameter,
        // coefficient) pair.
        const JonesMatrix::View pert(it.value(key));

        // Get perturbation.
        ParmProxy::ConstPtr parm = ParmManager::instance().get(key.parmId);
        const double inversePert = 1.0 / parm->getPerturbation(key.coeffId);

        // Approximate partial derivative using forward differences.
        JonesMatrix::View partial;
        partial.assign(0, 0, (pert(0, 0) - value(0, 0)) * inversePert);
        partial.assign(0, 1, (pert(0, 1) - value(0, 1)) * inversePert);
        partial.assign(1, 0, (pert(1, 0) - value(1, 0)) * inversePert);
        partial.assign(1, 1, (pert(1, 1) - value(1, 1)) * inversePert);

        result.assign(key, partial);
        it.advance(key);
    }

    return result;
}

Expr<Vector<2> >::Ptr
StationExprLOFAR::makeSourcePositionExpr(const string &source)
{
    ExprParm::Ptr ra = itsScope(SKY, "Ra:" + source);
    ExprParm::Ptr dec = itsScope(SKY, "Dec:" + source);

    AsExpr<Vector<2> >::Ptr position(new AsExpr<Vector<2> >());
    position->connect(0, ra);
    position->connect(1, dec);

    return position;
}

Expr<Vector<2> >::Ptr
StationExprLOFAR::makePatchPositionExpr(SourceDB &sourceDB, const string &patch)
{
    vector<SourceInfo> sources = sourceDB.getPatchSources(patch);
    ASSERTSTR(!sources.empty(), "Cannot determine position for empty patch: "
        << patch);

    if(sources.size() == 1)
    {
        return makeSourcePositionExpr(sources.front().getName());
    }

    EquatorialCentroid::Ptr centroid(new EquatorialCentroid());
    for(size_t i = 0; i < sources.size(); ++i)
    {
        centroid->connect(makeSourcePositionExpr(sources[i].getName()));
    }

    return centroid;
}

Expr<Vector<2> >::Ptr
StationExprLOFAR::makeRefPositionExpr(const casa::MDirection &reference) const
{
    casa::MDirection refJ2000(casa::MDirection::Convert(reference,
        casa::MDirection::J2000)());
    casa::Quantum<casa::Vector<casa::Double> > refAngles = refJ2000.getAngle();

    Literal::Ptr refRa(new Literal(refAngles.getBaseValue()(0)));
    Literal::Ptr refDec(new Literal(refAngles.getBaseValue()(1)));

    AsExpr<Vector<2> >::Ptr position(new AsExpr<Vector<2> >());
    position->connect(0, refRa);
    position->connect(1, refDec);

    return position;
}

Expr<Vector<2> >::Ptr
StationExprLOFAR::makeAzElExpr(const Station::ConstPtr &station,
    const Expr<Vector<2> >::Ptr &direction) const
{
    return Expr<Vector<2> >::Ptr(new AzEl(station->position(), direction));
}

Expr<JonesMatrix>::Ptr
StationExprLOFAR::makeBandpassExpr(const Station::ConstPtr &station)
{
    const string &suffix = station->name();

    Expr<Scalar>::Ptr B00 = itsScope(INSTRUMENT, "Bandpass:0:0:" + suffix);
    Expr<Scalar>::Ptr B11 = itsScope(INSTRUMENT, "Bandpass:1:1:" + suffix);

    return Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(B00, B11));
}

Expr<JonesMatrix>::Ptr
StationExprLOFAR::makeClockExpr(const Station::ConstPtr &station)
{
    ExprParm::Ptr delay = itsScope(INSTRUMENT, "Clock:" + station->name());

    Expr<Scalar>::Ptr shift = Expr<Scalar>::Ptr(new Delay(delay));
    return Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(shift, shift));
}

Expr<JonesMatrix>::Ptr
StationExprLOFAR::makeGainExpr(const Station::ConstPtr &station, bool phasors)
{
    Expr<Scalar>::Ptr J00, J01, J10, J11;

    string suffix0 = string(phasors ? "Ampl"  : "Real") + ":" + station->name();
    string suffix1 = string(phasors ? "Phase"  : "Imag") + ":"
        + station->name();

    ExprParm::Ptr J00_elem0 = itsScope(INSTRUMENT, "Gain:0:0:" + suffix0);
    ExprParm::Ptr J00_elem1 = itsScope(INSTRUMENT, "Gain:0:0:" + suffix1);
    ExprParm::Ptr J01_elem0 = itsScope(INSTRUMENT, "Gain:0:1:" + suffix0);
    ExprParm::Ptr J01_elem1 = itsScope(INSTRUMENT, "Gain:0:1:" + suffix1);
    ExprParm::Ptr J10_elem0 = itsScope(INSTRUMENT, "Gain:1:0:" + suffix0);
    ExprParm::Ptr J10_elem1 = itsScope(INSTRUMENT, "Gain:1:0:" + suffix1);
    ExprParm::Ptr J11_elem0 = itsScope(INSTRUMENT, "Gain:1:1:" + suffix0);
    ExprParm::Ptr J11_elem1 = itsScope(INSTRUMENT, "Gain:1:1:" + suffix1);

    if(phasors)
    {
        J00.reset(new AsPolar(J00_elem0, J00_elem1));
        J01.reset(new AsPolar(J01_elem0, J01_elem1));
        J10.reset(new AsPolar(J10_elem0, J10_elem1));
        J11.reset(new AsPolar(J11_elem0, J11_elem1));
    }
    else
    {
        J00.reset(new AsComplex(J00_elem0, J00_elem1));
        J01.reset(new AsComplex(J01_elem0, J01_elem1));
        J10.reset(new AsComplex(J10_elem0, J10_elem1));
        J11.reset(new AsComplex(J11_elem0, J11_elem1));
    }

    return Expr<JonesMatrix>::Ptr(new AsExpr<JonesMatrix>(J00, J01, J10, J11));
}

Expr<JonesMatrix>::Ptr
StationExprLOFAR::makeDirectionalGainExpr(const Station::ConstPtr &station,
    const string &patch, bool phasors)
{
    Expr<Scalar>::Ptr J00, J01, J10, J11;

    string suffix0 = string(phasors ? "Ampl"  : "Real") + ":" + station->name()
        + ":" + patch;
    string suffix1 = string(phasors ? "Phase"  : "Imag") + ":" + station->name()
        + ":" + patch;

    ExprParm::Ptr J00_elem0 = itsScope(INSTRUMENT,
        "DirectionalGain:0:0:" + suffix0);
    ExprParm::Ptr J00_elem1 = itsScope(INSTRUMENT,
        "DirectionalGain:0:0:" + suffix1);
    ExprParm::Ptr J01_elem0 = itsScope(INSTRUMENT,
        "DirectionalGain:0:1:" + suffix0);
    ExprParm::Ptr J01_elem1 = itsScope(INSTRUMENT,
        "DirectionalGain:0:1:" + suffix1);
    ExprParm::Ptr J10_elem0 = itsScope(INSTRUMENT,
        "DirectionalGain:1:0:" + suffix0);
    ExprParm::Ptr J10_elem1 = itsScope(INSTRUMENT,
        "DirectionalGain:1:0:" + suffix1);
    ExprParm::Ptr J11_elem0 = itsScope(INSTRUMENT,
        "DirectionalGain:1:1:" + suffix0);
    ExprParm::Ptr J11_elem1 = itsScope(INSTRUMENT,
        "DirectionalGain:1:1:" + suffix1);

    if(phasors)
    {
        J00.reset(new AsPolar(J00_elem0, J00_elem1));
        J01.reset(new AsPolar(J01_elem0, J01_elem1));
        J10.reset(new AsPolar(J10_elem0, J10_elem1));
        J11.reset(new AsPolar(J11_elem0, J11_elem1));
    }
    else
    {
        J00.reset(new AsComplex(J00_elem0, J00_elem1));
        J01.reset(new AsComplex(J01_elem0, J01_elem1));
        J10.reset(new AsComplex(J10_elem0, J10_elem1));
        J11.reset(new AsComplex(J11_elem0, J11_elem1));
    }

    return Expr<JonesMatrix>::Ptr(new AsExpr<JonesMatrix>(J00, J01, J10, J11));
}

Expr<JonesMatrix>::Ptr
StationExprLOFAR::makeBeamExpr(const Station::ConstPtr &station,
    double referenceFreq, const BeamConfig &config,
    const Expr<Vector<2> >::Ptr &exprRaDec,
    const Expr<Vector<2> >::Ptr &exprRefRaDec) const
{
    // Check if the beam model can be computed for this station.
    if(!station->isPhasedArray())
    {
        LOG_WARN_STR("Station " << station->name() << " is not a LOFAR station"
            " or the additional information needed to compute the station beam"
            " is missing. The station beam model will NOT be applied.");

        Expr<Scalar>::Ptr exprOne(new Literal(1.0));
        return Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(exprOne, exprOne));
    }

    // The positive X dipole direction is SE of the reference orientation, which
    // translates to an azimuth of 3/4*pi.
    Expr<Scalar>::Ptr exprOrientation(new Literal(3.0 * casa::C::pi_4));

    // The ITRF direction vectors for the direction of interest and the
    // reference direction are computed w.r.t. the center of the station (the
    // phase reference position).
    Expr<Vector<3> >::Ptr exprITRFDir(new ITRFDirection(station->position(),
        exprRaDec));
    Expr<Vector<3> >::Ptr exprITRFRef(new ITRFDirection(station->position(),
        exprRefRaDec));

    // Build expressions for the dual-dipole or tile beam of each antenna field.
    Expr<JonesMatrix>::Ptr exprElementBeam[2];
    for(size_t i = 0; i < station->nField(); ++i)
    {
        AntennaField::ConstPtr field = station->field(i);

        // Element (dual-dipole) beam expression.
        if(config.mode() != BeamConfig::ARRAY_FACTOR)
        {
            Expr<Vector<2> >::Ptr exprAzEl(new AntennaFieldAzEl(exprITRFDir,
                field));
            HamakerBeamCoeff coeff = loadBeamModelCoeff(config.getElementPath(),
                field);
            exprElementBeam[i] = Expr<JonesMatrix>::Ptr(new HamakerDipole(coeff,
                exprAzEl, exprOrientation));
        }
        else
        {
            Expr<Scalar>::Ptr exprOne(new Literal(1.0));
            Expr<JonesMatrix>::Ptr exprIdentity(new AsDiagonalMatrix(exprOne,
                exprOne));
            exprElementBeam[i] = exprIdentity;
        }

        // Tile array factor.
        if(field->isHBA() && config.mode() != BeamConfig::ELEMENT)
        {
            Expr<Scalar>::Ptr exprTileFactor(new TileArrayFactor(exprITRFDir,
                exprITRFRef, field, config.conjugateAF()));
            exprElementBeam[i] =
                Expr<JonesMatrix>::Ptr(new ScalarMatrixMul(exprTileFactor,
                exprElementBeam[i]));
        }
    }

    if(config.mode() == BeamConfig::ELEMENT)
    {
        // If the station consists of multiple antenna fields, but beam forming
        // is disabled, then we have to decide which antenna field to use. By
        // default the first antenna field will be used. The differences between
        // the dipole beam response of the antenna fields of a station should
        // only vary as a result of differences in the field coordinate systems
        // (because all dipoles are oriented the same way).

        return exprElementBeam[0];
    }

    if(station->nField() == 1)
    {
        return Expr<JonesMatrix>::Ptr(new StationBeamFormer(exprITRFDir,
            exprITRFRef, exprElementBeam[0], station, referenceFreq,
            config.conjugateAF()));
    }

    return Expr<JonesMatrix>::Ptr(new StationBeamFormer(exprITRFDir,
        exprITRFRef, exprElementBeam[0], exprElementBeam[1], station,
        referenceFreq, config.conjugateAF()));
}

Expr<JonesMatrix>::Ptr
StationExprLOFAR::makeIonosphereExpr(const Station::ConstPtr &station,
    const casa::MPosition &refPosition,
    const Expr<Vector<2> >::Ptr &exprAzEl,
    const IonosphereExpr::Ptr &exprIonosphere) const
{
    return exprIonosphere->construct(refPosition, station->position(),
        exprAzEl);
}

Expr<JonesMatrix>::Ptr
StationExprLOFAR::makeFaradayRotationExpr(const Station::ConstPtr &station,
    const string &patch)
{
    ExprParm::Ptr rm = itsScope(INSTRUMENT, "RotationMeasure:"
        + station->name() + ":" + patch);

    return Expr<JonesMatrix>::Ptr(new FaradayRotation(rm));
}

Expr<JonesMatrix>::Ptr
StationExprLOFAR::compose(const Expr<JonesMatrix>::Ptr &accumulator,
    const Expr<JonesMatrix>::Ptr &effect) const
{
    if(accumulator)
    {
        return Expr<JonesMatrix>::Ptr(new MatrixMul2(accumulator, effect));
    }

    return effect;
}

HamakerBeamCoeff StationExprLOFAR::loadBeamModelCoeff(casa::Path path,
    const AntennaField::ConstPtr &field) const
{
    if(field->isHBA())
    {
        path.append("element_beam_HAMAKER_HBA.coeff");
    }
    else
    {
        path.append("element_beam_HAMAKER_LBA.coeff");
    }

    HamakerBeamCoeff coeff;
    coeff.init(path);
    return coeff;
}

} //# namespace BBS
} //# namespace LOFAR
