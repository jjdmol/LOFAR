//# MeasurementExprLOFAR.cc: Measurement equation for the LOFAR telescope and
//# its environment.
//#
//# Copyright (C) 2007
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
#include <BBSKernel/MeasurementExprLOFAR.h>

#include <BBSKernel/Correlation.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/Measurement.h>
#include <BBSKernel/Expr/AntennaFieldAzEl.h>
#include <BBSKernel/Expr/AzEl.h>
#include <BBSKernel/Expr/ConditionNumber.h>
#include <BBSKernel/Expr/Delay.h>
#include <BBSKernel/Expr/EquatorialCentroid.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/ExprVisData.h>
#include <BBSKernel/Expr/FaradayRotation.h>
#include <BBSKernel/Expr/FlagIf.h>
#include <BBSKernel/Expr/GaussianCoherence.h>
#include <BBSKernel/Expr/GaussianSource.h>
#include <BBSKernel/Expr/ITRFDirection.h>
#include <BBSKernel/Expr/LinearToCircularRL.h>
#include <BBSKernel/Expr/Literal.h>
#include <BBSKernel/Expr/LMN.h>
#include <BBSKernel/Expr/MatrixInverse.h>
#include <BBSKernel/Expr/MatrixMul2.h>
#include <BBSKernel/Expr/MatrixMul3.h>
#include <BBSKernel/Expr/MatrixSum.h>
#include <BBSKernel/Expr/MergeFlags.h>
#include <BBSKernel/Expr/PhaseShift.h>
#include <BBSKernel/Expr/PointCoherence.h>
#include <BBSKernel/Expr/PointSource.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/ScalarMatrixMul.h>
#include <BBSKernel/Expr/SpectralIndex.h>
#include <BBSKernel/Expr/StationBeamFormer.h>
#include <BBSKernel/Expr/StationShift.h>
#include <BBSKernel/Expr/StationUVW.h>
#include <BBSKernel/Expr/TileArrayFactor.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>
#include <Common/lofar_set.h>
#include <Common/lofar_fstream.h>
#include <Common/lofar_sstream.h>
#include <Common/lofar_complex.h>
#include <Common/lofar_algorithm.h>
#include <Common/lofar_smartptr.h>

#include <casa/Arrays/Vector.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>

namespace LOFAR
{
namespace BBS
{

MeasurementExprLOFAR::MeasurementExprLOFAR(SourceDB &sourceDB,
        const ModelConfig &config, const Instrument::ConstPtr &instrument,
        const BaselineSeq &baselines, const casa::MDirection &phaseReference,
        double referenceFreq, bool circular)
    :   itsSourceDB(sourceDB),
        itsInstrument(instrument),
        itsBaselines(baselines),
        itsCachePolicy(new DefaultCachePolicy())
{
    setCorrelations(circular);
    makeForwardExpr(config, phaseReference, referenceFreq, circular);
}

MeasurementExprLOFAR::MeasurementExprLOFAR(SourceDB &sourceDB,
    const ModelConfig &config, const VisBuffer::Ptr &buffer,
    const BaselineMask &mask, bool inverse)
    :   itsSourceDB(sourceDB),
        itsInstrument(buffer->instrument()),
        itsBaselines(filter(buffer->baselines(), mask)),
        itsCachePolicy(new DefaultCachePolicy())
{
    const bool circular = isCircular(buffer);
    ASSERTSTR(circular != isLinear(buffer), "The correlations in the"
        " measurement should be either all linear or all circular.");
    setCorrelations(circular);

    if(inverse)
    {
        makeInverseExpr(config, buffer, buffer->getPhaseReference(),
            buffer->getReferenceFreq(), circular);
    }
    else
    {
        makeForwardExpr(config, buffer->getPhaseReference(),
            buffer->getReferenceFreq(), circular);
    }
}

void MeasurementExprLOFAR::solvablesChanged()
{
//    LOG_DEBUG_STR("" << itsCache);
    itsCache.clear(Cache::VOLATILE);
    itsCache.clearStats();
}

void MeasurementExprLOFAR::makeForwardExpr(const ModelConfig &config,
    const casa::MDirection &phaseReference, double referenceFreq, bool circular)
{
    if(config.useFlagger())
    {
        LOG_WARN("Condition number flagging is only implemented for the inverse"
            " model.");
    }

    // Make a list of patches matching the selection criteria specified by the
    // user.
    vector<string> patches = makePatchList(config.getSources());
    LOG_DEBUG_STR("No. of patches in the catalog: " << patches.size());
    if(patches.empty())
    {
        THROW(BBSKernelException, "No patches matching selection found in"
            " source database.");
    }

    // Create a linear to circular-RL transformation Jones matrix.
    Expr<JonesMatrix>::Ptr H(new LinearToCircularRL());

    // Phase reference position on the sky.
    Expr<Vector<2> >::Ptr exprRefPosition = makeRefPositionExpr(phaseReference);

    // Phase reference position on the sky in local station coordinates.
    vector<Expr<Vector<2> >::Ptr> exprRefAzEl(itsInstrument->nStations());
    for(size_t i = 0; i < itsInstrument->nStations(); ++i)
    {
        exprRefAzEl[i] = makeAzElExpr(itsInstrument->station(i),
            exprRefPosition);
    }

    // Create an UVW expression per station.
    vector<Expr<Vector<3> >::Ptr> exprUVW(itsInstrument->nStations());
    for(size_t i = 0; i < itsInstrument->nStations(); ++i)
    {
        exprUVW[i] = makeUVWExpr(itsInstrument->position(),
            itsInstrument->station(i)->position(), phaseReference);
    }

    HamakerBeamCoeff coeffLBA, coeffHBA;
    if(config.useBeam())
    {
        // Read LBA beam model coefficients.
        casa::Path path;
        path = config.getBeamConfig().getElementPath();
        path.append("element_beam_HAMAKER_LBA.coeff");
        coeffLBA.init(path);

        // Read HBA beam model coefficients.
        path = config.getBeamConfig().getElementPath();
        path.append("element_beam_HAMAKER_HBA.coeff");
        coeffHBA.init(path);
    }

    IonosphereExpr::Ptr exprIonosphere;
    if(config.useIonosphere())
    {
        exprIonosphere = IonosphereExpr::create(config.getIonosphereConfig(),
            itsScope);
    }

    vector<MatrixSum::Ptr> coherenceExpr(itsBaselines.size());
    for(size_t i = 0; i < patches.size(); ++i)
    {
        const string &patch = patches[i];
        vector<Source::Ptr> sources = makeSourceList(patch);

        Expr<Vector<2> >::Ptr exprPatchPosition =
            makePatchPositionExpr(sources);

        vector<Expr<Vector<3> >::Ptr> exprLMN(sources.size());
        for(unsigned int j = 0; j < sources.size(); ++j)
        {
            exprLMN[j] = makeLMNExpr(phaseReference, sources[j]->position());
        }

        vector<Expr<JonesMatrix>::Ptr> exprDDE(itsInstrument->nStations());
        for(size_t j = 0; j < itsInstrument->nStations(); ++j)
        {
            // Directional gain.
            if(config.useDirectionalGain())
            {
                exprDDE[j] = compose(exprDDE[j],
                    makeDirectionalGainExpr(itsInstrument->station(j), patch,
                    config.usePhasors()));
            }

            // Beam.
            if(config.useBeam())
            {
                exprDDE[j] = compose(exprDDE[j],
                    makeBeamExpr(itsInstrument->station(j), referenceFreq,
                    config.getBeamConfig(), coeffLBA, coeffHBA,
                    exprPatchPosition, exprRefPosition));
            }

            // Faraday rotation.
            if(config.useFaradayRotation())
            {
                exprDDE[j] = compose(exprDDE[j],
                    makeFaradayRotationExpr(itsInstrument->station(j), patch));
            }

            // Ionosphere.
            if(config.useIonosphere())
            {
                // Create an AZ, EL expression for the centroid direction of the
                // patch.
                Expr<Vector<2> >::Ptr exprAzEl =
                    makeAzElExpr(itsInstrument->station(j), exprPatchPosition);

                exprDDE[j] = compose(exprDDE[j],
                    makeIonosphereExpr(itsInstrument->station(j),
                    itsInstrument->position(), exprAzEl, exprIonosphere));
            }
        }

        // Create a station shift expression per (station, source) combination.
        vector<vector<Expr<Vector<2> >::Ptr> >
            exprStationShift(itsInstrument->nStations());
        for(size_t j = 0; j < itsInstrument->nStations(); ++j)
        {
            exprStationShift[j].reserve(sources.size());
            for(size_t k = 0; k < sources.size(); ++k)
            {
                exprStationShift[j].push_back(makeStationShiftExpr(exprUVW[j],
                    exprLMN[k]));
            }
        }

        for(size_t j = 0; j < itsBaselines.size(); ++j)
        {
            const baseline_t baseline = itsBaselines[j];

            // Construct an expression for the patch coherence on this baseline.
            Expr<JonesMatrix>::Ptr patchCoherenceExpr =
                makePatchCoherenceExpr(exprUVW[baseline.first],
                    exprStationShift[baseline.first],
                    exprUVW[baseline.second],
                    exprStationShift[baseline.second],
                    sources);

            // Convert to circular-RL if required.
            if(circular)
            {
                patchCoherenceExpr = corrupt(H, patchCoherenceExpr, H);
            }

            // Apply direction dependent effects.
            patchCoherenceExpr = corrupt(exprDDE[baseline.first],
                patchCoherenceExpr, exprDDE[baseline.second]);

            // Add patch coherence to the visibilities for this baseline.
            if(!coherenceExpr[j])
            {
                coherenceExpr[j] = MatrixSum::Ptr(new MatrixSum());
            }

            coherenceExpr[j]->connect(patchCoherenceExpr);
        }
    }

    // Direction independent effects (DIE).
    vector<Expr<JonesMatrix>::Ptr> exprDIE(itsInstrument->nStations());
    for(size_t i = 0; i < itsInstrument->nStations(); ++i)
    {
        // Create a clock delay expression per station.
        if(config.useClock())
        {
            exprDIE[i] = compose(exprDIE[i],
                makeClockExpr(itsInstrument->station(i)));
        }

        // Bandpass.
        if(config.useBandpass())
        {
            exprDIE[i] = compose(exprDIE[i],
                makeBandpassExpr(itsInstrument->station(i)));
        }

        // Create a direction independent gain expression per station.
        if(config.useGain())
        {
            exprDIE[i] = compose(exprDIE[i],
                makeGainExpr(itsInstrument->station(i), config.usePhasors()));
        }
    }

    itsExpr = vector<Expr<JonesMatrix>::Ptr>(itsBaselines.size());
    for(size_t i = 0; i < itsBaselines.size(); ++i)
    {
        const baseline_t baseline = itsBaselines[i];
        itsExpr[i] = corrupt(exprDIE[baseline.first], coherenceExpr[i],
            exprDIE[baseline.second]);
    }

    // Set caching policy.
    itsCachePolicy = CachePolicy::Ptr(new DefaultCachePolicy());
    if(config.useCache())
    {
        itsCachePolicy = CachePolicy::Ptr(new ExperimentalCachePolicy());
    }

    itsCachePolicy->apply(itsExpr.begin(), itsExpr.end());
}

void MeasurementExprLOFAR::makeInverseExpr(const ModelConfig &config,
    const VisBuffer::Ptr &buffer, const casa::MDirection &phaseReference,
    double referenceFreq, bool circular)
{
    // Allocate space for the station response expressions.
    vector<Expr<JonesMatrix>::Ptr> stationExpr(itsInstrument->nStations());

    // Direction independent effects (DIE).
    const bool haveDIE = config.useClock() || config.useBandpass()
        || config.useGain();

    for(size_t i = 0; i < itsInstrument->nStations(); ++i)
    {
        // Create a clock delay expression per station.
        if(config.useClock())
        {
            stationExpr[i] = compose(stationExpr[i],
                makeClockExpr(itsInstrument->station(i)));
        }

        // Bandpass.
        if(config.useBandpass())
        {
            stationExpr[i] = compose(stationExpr[i],
                makeBandpassExpr(itsInstrument->station(i)));
        }

        // Create a direction independent gain expression per station.
        if(config.useGain())
        {
            stationExpr[i] = compose(stationExpr[i],
                makeGainExpr(itsInstrument->station(i), config.usePhasors()));
        }
    }

    // Direction dependent effects (DDE).
    const bool haveDDE = config.useDirectionalGain()
        || config.useBeam() || config.useFaradayRotation()
        || config.useIonosphere();

    if(haveDDE)
    {
        // Position of interest on the sky (given as patch name).
        if(config.getSources().size() != 1)
        {
            THROW(BBSKernelException, "No patch, or more than one patch"
                " selected, yet corrections can only be applied for a single"
                " direction on the sky");
        }
        const string &patch = config.getSources().front();
        Expr<Vector<2> >::Ptr exprPatchPosition =
            makePatchPositionExpr(makeSourceList(patch));

        // Phase reference position on the sky.
        Expr<Vector<2> >::Ptr exprRefPosition =
            makeRefPositionExpr(phaseReference);

        HamakerBeamCoeff coeffLBA, coeffHBA;
        if(config.useBeam())
        {
            // Read LBA beam model coefficients.
            casa::Path path;
            path = config.getBeamConfig().getElementPath();
            path.append("element_beam_HAMAKER_LBA.coeff");
            coeffLBA.init(path);

            // Read HBA beam model coefficients.
            path = config.getBeamConfig().getElementPath();
            path.append("element_beam_HAMAKER_HBA.coeff");
            coeffHBA.init(path);
        }

        // Functor for the creation of the ionosphere sub-expression.
        IonosphereExpr::Ptr exprIonosphere;
        if(config.useIonosphere())
        {
            exprIonosphere =
                IonosphereExpr::create(config.getIonosphereConfig(), itsScope);
        }

        for(size_t i = 0; i < stationExpr.size(); ++i)
        {
            // Directional gain.
            if(config.useDirectionalGain())
            {
                stationExpr[i] = compose(stationExpr[i],
                    makeDirectionalGainExpr(itsInstrument->station(i), patch,
                    config.usePhasors()));
            }

            // Beam.
            if(config.useBeam())
            {
                stationExpr[i] = compose(stationExpr[i],
                    makeBeamExpr(itsInstrument->station(i), referenceFreq,
                    config.getBeamConfig(), coeffLBA, coeffHBA,
                    exprPatchPosition, exprRefPosition));
            }

            // Faraday rotation.
            if(config.useFaradayRotation())
            {
                stationExpr[i] = compose(stationExpr[i],
                    makeFaradayRotationExpr(itsInstrument->station(i), patch));
            }

            // Ionosphere.
            if(config.useIonosphere())
            {
                // Create an AZ, EL expression for the centroid direction of the
                // patch.
                Expr<Vector<2> >::Ptr exprAzEl =
                    makeAzElExpr(itsInstrument->station(i), exprPatchPosition);

                stationExpr[i] = compose(stationExpr[i],
                    makeIonosphereExpr(itsInstrument->station(i),
                    itsInstrument->position(), exprAzEl, exprIonosphere));
            }
        }
    }

    if(haveDIE || haveDDE)
    {
        for(size_t i = 0; i < itsInstrument->nStations(); ++i)
        {
            if(config.useFlagger())
            {
                const FlaggerConfig &flagConfig = config.getFlaggerConfig();

                Expr<Scalar>::Ptr exprCond =
                    Expr<Scalar>::Ptr(new ConditionNumber(stationExpr[i]));
                Expr<Scalar>::Ptr exprThreshold(makeFlagIf(exprCond,
                    std::bind2nd(std::greater_equal<double>(),
                    flagConfig.getThreshold())));

                typedef MergeFlags<JonesMatrix, Scalar> T_MERGEFLAGS;
                stationExpr[i] =
                    T_MERGEFLAGS::Ptr(new T_MERGEFLAGS(stationExpr[i],
                    exprThreshold));
            }

            stationExpr[i] =
                Expr<JonesMatrix>::Ptr(new MatrixInverse(stationExpr[i]));
        }
    }

    itsExpr = vector<Expr<JonesMatrix>::Ptr>(itsBaselines.size());
    for(size_t i = 0; i < itsBaselines.size(); ++i)
    {
        const baseline_t baseline = itsBaselines[i];

        Expr<JonesMatrix>::Ptr exprVisData;
        if(circular)
        {
            itsExpr[i] = Expr<JonesMatrix>::Ptr(new ExprVisData(buffer,
                baseline, Correlation::RR, Correlation::RL, Correlation::LR,
                Correlation::LL));
        }
        else
        {
            itsExpr[i] = Expr<JonesMatrix>::Ptr(new ExprVisData(buffer,
                baseline));
        }

        if(haveDIE || haveDDE)
        {
            itsExpr[i] = corrupt(stationExpr[baseline.first], itsExpr[i],
                stationExpr[baseline.second]);
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

size_t MeasurementExprLOFAR::size() const
{
    return itsExpr.size();
}

Box MeasurementExprLOFAR::domain() const
{
    return ParmManager::instance().domain();
}

const BaselineSeq &MeasurementExprLOFAR::baselines() const
{
    return itsBaselines;
}

const CorrelationSeq &MeasurementExprLOFAR::correlations() const
{
    return itsCorrelations;
}

ParmGroup MeasurementExprLOFAR::parms() const
{
    ParmGroup result;

    for(Scope::const_iterator parm_it = itsScope.begin(),
        parm_end = itsScope.end(); parm_it != parm_end; ++parm_it)
    {
        result.insert(parm_it->first);
    }

    return result;
}

size_t MeasurementExprLOFAR::nParms() const
{
    return itsScope.size();
}

ParmGroup MeasurementExprLOFAR::solvables() const
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

void MeasurementExprLOFAR::setSolvables(const ParmGroup &solvables)
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
        if(parm_it == itsScope.end())
        {
            clearSolvables();
            THROW(BBSKernelException, "Model does not depend on parameter: "
                << ParmManager::instance().get(*sol_it)->getName());
        }

        parm_it->second->setPValueFlag();
    }

    // Clear any cached results and reinitialize the caching policy.
    itsCache.clear();
    itsCache.clearStats();
    itsCachePolicy->apply(itsExpr.begin(), itsExpr.end());
}

void MeasurementExprLOFAR::clearSolvables()
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

void MeasurementExprLOFAR::setEvalGrid(const Grid &grid)
{
    itsRequest = Request(grid);

    itsCache.clear();
    itsCache.clearStats();

    // TODO: Set cache size in number of Matrix instances... ?
}

const JonesMatrix MeasurementExprLOFAR::evaluate(unsigned int i)
{
    JonesMatrix result;

    // Evaluate the model.
    ASSERT(i < itsExpr.size());
    const JonesMatrix model = itsExpr[i]->evaluate(itsRequest, itsCache, 0);

    EXPR_TIMER_START_NAMED("MeasurementExprLOFAR::evaluate()");

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

    EXPR_TIMER_STOP_NAMED("MeasurementExprLOFAR::evaluate()");

    return result;
}

void MeasurementExprLOFAR::setCorrelations(bool circular)
{
    itsCorrelations.clear();

    if(circular)
    {
        LOG_DEBUG_STR("Visibilities will be simulated using circular (RL)"
            " correlations.");
        itsCorrelations.append(Correlation::RR);
        itsCorrelations.append(Correlation::RL);
        itsCorrelations.append(Correlation::LR);
        itsCorrelations.append(Correlation::LL);
    }
    else
    {
        LOG_DEBUG_STR("Visibilities will be simulated using linear"
            " correlations.");
        itsCorrelations.append(Correlation::XX);
        itsCorrelations.append(Correlation::XY);
        itsCorrelations.append(Correlation::YX);
        itsCorrelations.append(Correlation::YY);
    }
}

bool MeasurementExprLOFAR::isLinear(const VisBuffer::Ptr &buffer) const
{
    for(size_t i = 0; i < buffer->nCorrelations(); ++i)
    {
        if(!Correlation::isLinear(buffer->correlations()[i]))
        {
            return false;
        }
    }

    return true;
}

bool MeasurementExprLOFAR::isCircular(const VisBuffer::Ptr &buffer) const
{
    for(size_t i = 0; i < buffer->nCorrelations(); ++i)
    {
        if(!Correlation::isCircular(buffer->correlations()[i]))
        {
            return false;
        }
    }

    return true;
}

vector<string>
MeasurementExprLOFAR::makePatchList(const vector<string> &patterns)
{
    if(patterns.empty())
    {
        return itsSourceDB.getPatches(-1, "*");
    }

    // Create a list of all unique patches that match the given patterns.
    set<string> patches;
    for(size_t i = 0; i < patterns.size(); ++i)
    {
        vector<string> match(itsSourceDB.getPatches(-1, patterns[i]));
        patches.insert(match.begin(), match.end());
    }

    return vector<string>(patches.begin(), patches.end());
}

vector<Source::Ptr> MeasurementExprLOFAR::makeSourceList(const string &patch)
{
    vector<SourceInfo> sources(itsSourceDB.getPatchSources(patch));

    vector<Source::Ptr> result;
    result.reserve(sources.size());
    for(size_t i = 0; i < sources.size(); ++i)
    {
        result.push_back(Source::create(sources[i], itsScope));
    }

    return result;
}

Expr<JonesMatrix>::Ptr
MeasurementExprLOFAR::makePatchCoherenceExpr(const Expr<Vector<3> >::Ptr &uvwLHS,
    const vector<Expr<Vector<2> >::Ptr> &shiftLHS,
    const Expr<Vector<3> >::Ptr &uvwRHS,
    const vector<Expr<Vector<2> >::Ptr> &shiftRHS,
    const vector<Source::Ptr> &sources) const
{
    if(sources.size() == 1)
    {
        // Source coherence.
        Expr<JonesMatrix>::Ptr coherence = sources.front()->coherence(uvwLHS,
            uvwRHS);

        // Phase shift (incorporates geometry and fringe stopping).
        Expr<Scalar>::Ptr shift(new PhaseShift(shiftLHS.front(),
            shiftRHS.front()));

        // Phase shift the source coherence to the correct position.
        return Expr<JonesMatrix>::Ptr(new ScalarMatrixMul(shift, coherence));
    }

    MatrixSum::Ptr sum(new MatrixSum());
    for(size_t i = 0; i < sources.size(); ++i)
    {
        // Source coherence.
        Expr<JonesMatrix>::Ptr coherence = sources[i]->coherence(uvwLHS,
            uvwRHS);

        // Phase shift (incorporates geometry and fringe stopping).
        Expr<Scalar>::Ptr shift(new PhaseShift(shiftLHS[i], shiftRHS[i]));

        // Phase shift the source coherence to the correct position.
        coherence = Expr<JonesMatrix>::Ptr(new ScalarMatrixMul(shift,
            coherence));

        sum->connect(coherence);
    }

    return sum;
}

Expr<Vector<2> >::Ptr
MeasurementExprLOFAR::makePatchPositionExpr(const vector<Source::Ptr> &sources)
    const
{
    ASSERTSTR(!sources.empty(), "Cannot determine position for empty patch.");
    if(sources.size() == 1)
    {
        return sources[0]->position();
    }

    EquatorialCentroid::Ptr centroid(new EquatorialCentroid());
    for(size_t i = 0; i < sources.size(); ++i)
    {
        centroid->connect(sources[i]->position());
    }
    return centroid;
}

Expr<Vector<3> >::Ptr
MeasurementExprLOFAR::makeUVWExpr(const casa::MPosition &array,
    const casa::MPosition &station, const casa::MDirection &direction) const
{
    return Expr<Vector<3> >::Ptr(new StationUVW(station, array, direction));
}

Expr<Vector<3> >::Ptr
MeasurementExprLOFAR::makeLMNExpr(const casa::MDirection &reference,
    const Expr<Vector<2> >::Ptr &direction) const
{
    return Expr<Vector<3> >::Ptr(new LMN(reference, direction));
}

Expr<Vector<2> >::Ptr
MeasurementExprLOFAR::makeStationShiftExpr(const Expr<Vector<3> >::Ptr &exprUVW,
    const Expr<Vector<3> >::Ptr &exprLMN) const
{
    return Expr<Vector<2> >::Ptr(new StationShift(exprUVW, exprLMN));
}

Expr<Vector<2> >::Ptr
MeasurementExprLOFAR::makeRefPositionExpr(const casa::MDirection &reference)
    const
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
MeasurementExprLOFAR::makeAzElExpr(const Station::ConstPtr &station,
    const Expr<Vector<2> >::Ptr &direction) const
{
    return Expr<Vector<2> >::Ptr(new AzEl(station->position(), direction));
}

Expr<JonesMatrix>::Ptr
MeasurementExprLOFAR::makeBandpassExpr(const Station::ConstPtr &station)
{
    const string &suffix = station->name();

    Expr<Scalar>::Ptr B00 = itsScope(INSTRUMENT, "Bandpass:0:0:" + suffix);
    Expr<Scalar>::Ptr B11 = itsScope(INSTRUMENT, "Bandpass:1:1:" + suffix);

    return Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(B00, B11));
}

Expr<JonesMatrix>::Ptr
MeasurementExprLOFAR::makeClockExpr(const Station::ConstPtr &station)
{
    ExprParm::Ptr delay = itsScope(INSTRUMENT, "Clock:" + station->name());

    Expr<Scalar>::Ptr shift = Expr<Scalar>::Ptr(new Delay(delay));
    return Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(shift, shift));
}

Expr<JonesMatrix>::Ptr
MeasurementExprLOFAR::makeGainExpr(const Station::ConstPtr &station,
    bool phasors)
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
MeasurementExprLOFAR::makeDirectionalGainExpr(const Station::ConstPtr &station,
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
MeasurementExprLOFAR::makeBeamExpr(const Station::ConstPtr &station,
    double referenceFreq, const BeamConfig &config,
    const HamakerBeamCoeff &coeffLBA, const HamakerBeamCoeff &coeffHBA,
    const Expr<Vector<2> >::Ptr &exprRaDec,
    const Expr<Vector<2> >::Ptr &exprRefRaDec) const
{
    // Check if the beam model can be computed for this station.
    if(!station->isPhasedArray())
    {
        THROW(BBSKernelException, "Station " << station->name() << " is not a"
            " LOFAR station or the additional information needed to compute the"
            " station beam is missing.");
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

            if(field->isHBA())
            {
                exprElementBeam[i] =
                    Expr<JonesMatrix>::Ptr(new HamakerDipole(coeffHBA, exprAzEl,
                    exprOrientation));
            }
            else
            {
                exprElementBeam[i] =
                    Expr<JonesMatrix>::Ptr(new HamakerDipole(coeffLBA, exprAzEl,
                    exprOrientation));
            }
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
MeasurementExprLOFAR::makeIonosphereExpr(const Station::ConstPtr &station,
    const casa::MPosition &refPosition,
    const Expr<Vector<2> >::Ptr &exprAzEl,
    const IonosphereExpr::Ptr &exprIonosphere) const
{
    return exprIonosphere->construct(refPosition, station->position(),
        exprAzEl);
}

Expr<JonesMatrix>::Ptr
MeasurementExprLOFAR::makeFaradayRotationExpr(const Station::ConstPtr &station,
    const string &patch)
{
    ExprParm::Ptr rm = itsScope(INSTRUMENT, "RotationMeasure:"
        + station->name() + ":" + patch);

    return Expr<JonesMatrix>::Ptr(new FaradayRotation(rm));
}

Expr<JonesMatrix>::Ptr
MeasurementExprLOFAR::compose(const Expr<JonesMatrix>::Ptr &accumulator,
    const Expr<JonesMatrix>::Ptr &effect) const
{
    if(accumulator)
    {
        return Expr<JonesMatrix>::Ptr(new MatrixMul2(accumulator, effect));
    }

    return effect;
}

Expr<JonesMatrix>::Ptr
MeasurementExprLOFAR::corrupt(const Expr<JonesMatrix>::Ptr &lhs,
    const Expr<JonesMatrix>::Ptr &coherence,
    const Expr<JonesMatrix>::Ptr &rhs) const
{
    if(lhs && rhs)
    {
        return Expr<JonesMatrix>::Ptr(new MatrixMul3(lhs, coherence, rhs));
    }

    return coherence;
}

} // namespace BBS
} // namespace LOFAR
