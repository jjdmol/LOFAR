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
#include <BBSKernel/MeasurementExprLOFARUtil.h>
#include <BBSKernel/Correlation.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/Measurement.h>
#include <BBSKernel/Expr/ConditionNumber.h>
#include <BBSKernel/Expr/ExprVisData.h>
#include <BBSKernel/Expr/FlagIf.h>
#include <BBSKernel/Expr/GaussianCoherence.h>
#include <BBSKernel/Expr/GaussianSource.h>
#include <BBSKernel/Expr/LinearToCircularRL.h>
#include <BBSKernel/Expr/MatrixInverse.h>
#include <BBSKernel/Expr/MatrixSum.h>
#include <BBSKernel/Expr/MergeFlags.h>
#include <BBSKernel/Expr/PointCoherence.h>
#include <BBSKernel/Expr/PointSource.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/SpectralIndex.h>
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
    const BaselineSeq &baselines, double refFreq,
    const casa::MDirection &refPhase, const casa::MDirection &refDelay,
    const casa::MDirection &refTile, bool circular)
    :   itsBaselines(baselines),
        itsCachePolicy(new DefaultCachePolicy())
{
    setCorrelations(circular);
    makeForwardExpr(config, sourceDB, instrument, refFreq, refPhase, refDelay,
        refTile, circular);
}

MeasurementExprLOFAR::MeasurementExprLOFAR(SourceDB &sourceDB,
    const ModelConfig &config, const VisBuffer::Ptr &buffer,
    const BaselineMask &mask, bool inverse)
    :   itsBaselines(filter(buffer->baselines(), mask)),
        itsCachePolicy(new DefaultCachePolicy())
{
    const bool circular = buffer->isCircular();
    ASSERTSTR(circular != buffer->isLinear(), "The correlations in the"
        " measurement should be either all linear or all circular.");
    setCorrelations(circular);

    if(inverse)
    {
        makeInverseExpr(config, sourceDB, buffer);
    }
    else
    {
        makeForwardExpr(config, sourceDB, buffer->instrument(),
            buffer->getReferenceFreq(), buffer->getPhaseReference(),
            buffer->getDelayReference(), buffer->getTileReference(), circular);
    }
}

void MeasurementExprLOFAR::solvablesChanged()
{
//    LOG_DEBUG_STR("" << itsCache);
    itsCache.clear(Cache::VOLATILE);
    itsCache.clearStats();
}

void MeasurementExprLOFAR::makeForwardExpr(const ModelConfig &config,
    SourceDB &sourceDB, const Instrument::Ptr &instrument, double refFreq,
    const casa::MDirection &refPhase, const casa::MDirection &refDelay,
    const casa::MDirection &refTile, bool circular)
{
    if(config.useFlagger())
    {
        LOG_WARN("Condition number flagging is only implemented for the inverse"
            " model.");
    }

    // Make a list of patches matching the selection criteria specified by the
    // user.
    vector<string> patches = makePatchList(sourceDB, config.getSources());
    LOG_DEBUG_STR("No. of patches in the catalog: " << patches.size());
    if(patches.empty())
    {
        THROW(BBSKernelException, "No patches matching selection found in"
            " source database.");
    }

    // Create a linear to circular-RL transformation Jones matrix.
    Expr<JonesMatrix>::Ptr H(new LinearToCircularRL());

    // Beam reference position on the sky.
    Expr<Vector<2> >::Ptr exprRefDelay = makeDirectionExpr(refDelay);
    Expr<Vector<3> >::Ptr exprRefDelayITRF =
        makeITRFExpr(instrument->position(), exprRefDelay);

    // Tile beam reference position on the sky.
    Expr<Vector<2> >::Ptr exprRefTile = makeDirectionExpr(refTile);
    Expr<Vector<3> >::Ptr exprRefTileITRF =
        makeITRFExpr(instrument->position(), exprRefTile);

    // Create an UVW expression per station.
    vector<Expr<Vector<3> >::Ptr> exprUVW(instrument->nStations());
    for(size_t i = 0; i < instrument->nStations(); ++i)
    {
        exprUVW[i] = makeStationUVWExpr(instrument->position(),
            instrument->station(i)->position(), refPhase);
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
        vector<Source::Ptr> sources = makeSourceList(sourceDB, patch);

        Expr<Vector<2> >::Ptr exprPatchPosition =
            makePatchCentroidExpr(sources);
        Expr<Vector<3> >::Ptr exprPatchPositionITRF =
            makeITRFExpr(instrument->position(), exprPatchPosition);

        vector<Expr<Vector<3> >::Ptr> exprLMN(sources.size());
        for(unsigned int j = 0; j < sources.size(); ++j)
        {
            exprLMN[j] = makeLMNExpr(refPhase, sources[j]->position());
        }

        vector<Expr<JonesMatrix>::Ptr> exprDDE(instrument->nStations());
        for(size_t j = 0; j < instrument->nStations(); ++j)
        {
            // Directional gain.
            if(config.useDirectionalGain())
            {
                exprDDE[j] = compose(exprDDE[j],
                    makeDirectionalGainExpr(itsScope, instrument->station(j),
                    patch, config.usePhasors()));
            }

            // Beam.
            if(config.useBeam())
            {
                exprDDE[j] = compose(exprDDE[j],
                    makeBeamExpr(itsScope, instrument->station(j), refFreq,
                    exprPatchPositionITRF, exprRefDelayITRF, exprRefTileITRF,
                    config.getBeamConfig(), coeffLBA, coeffHBA));
            }

            // Faraday rotation.
            if(config.useFaradayRotation())
            {
                exprDDE[j] = compose(exprDDE[j],
                    makeFaradayRotationExpr(itsScope, instrument->station(j),
                    patch));
            }

            // Ionosphere.
            if(config.useIonosphere())
            {
                // Create an AZ, EL expression for the centroid direction of the
                // patch.
                Expr<Vector<2> >::Ptr exprAzEl =
                    makeAzElExpr(instrument->station(j)->position(),
                    exprPatchPosition);

                exprDDE[j] = compose(exprDDE[j],
                    makeIonosphereExpr(itsScope, instrument->station(j),
                    instrument->position(), exprAzEl, exprIonosphere));
            }
        }

        // Create a station shift expression per (station, source) combination.
        vector<vector<Expr<Vector<2> >::Ptr> >
            exprStationShift(instrument->nStations());
        for(size_t j = 0; j < instrument->nStations(); ++j)
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
                patchCoherenceExpr = apply(H, patchCoherenceExpr, H);
            }

            // Apply direction dependent effects.
            patchCoherenceExpr = apply(exprDDE[baseline.first],
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
    vector<Expr<JonesMatrix>::Ptr> exprDIE(instrument->nStations());
    for(size_t i = 0; i < instrument->nStations(); ++i)
    {
        // Create a clock delay expression per station.
        if(config.useClock())
        {
            exprDIE[i] = compose(exprDIE[i],
                makeClockExpr(itsScope, instrument->station(i)));
        }

        // Bandpass.
        if(config.useBandpass())
        {
            exprDIE[i] = compose(exprDIE[i],
                makeBandpassExpr(itsScope, instrument->station(i)));
        }

        // Create a direction independent gain expression per station.
        if(config.useGain())
        {
            exprDIE[i] = compose(exprDIE[i],
                makeGainExpr(itsScope, instrument->station(i),
                config.usePhasors()));
        }
    }

    itsExpr = vector<Expr<JonesMatrix>::Ptr>(itsBaselines.size());
    for(size_t i = 0; i < itsBaselines.size(); ++i)
    {
        const baseline_t baseline = itsBaselines[i];
        itsExpr[i] = apply(exprDIE[baseline.first], coherenceExpr[i],
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
    SourceDB &sourceDB, const VisBuffer::Ptr &buffer)
{
    const bool circular = buffer->isCircular();
    Instrument::Ptr instrument = buffer->instrument();

    // Allocate space for the station response expressions.
    vector<Expr<JonesMatrix>::Ptr> stationExpr(instrument->nStations());

    // Direction independent effects (DIE).
    const bool haveDIE = config.useClock() || config.useBandpass()
        || config.useGain();

    for(size_t i = 0; i < instrument->nStations(); ++i)
    {
        // Create a clock delay expression per station.
        if(config.useClock())
        {
            stationExpr[i] = compose(stationExpr[i],
                makeClockExpr(itsScope, instrument->station(i)));
        }

        // Bandpass.
        if(config.useBandpass())
        {
            stationExpr[i] = compose(stationExpr[i],
                makeBandpassExpr(itsScope, instrument->station(i)));
        }

        // Create a direction independent gain expression per station.
        if(config.useGain())
        {
            stationExpr[i] = compose(stationExpr[i],
                makeGainExpr(itsScope, instrument->station(i),
                config.usePhasors()));
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
            makePatchCentroidExpr(makeSourceList(sourceDB, patch));
        Expr<Vector<3> >::Ptr exprPatchPositionITRF =
            makeITRFExpr(instrument->position(), exprPatchPosition);

        // Beam reference position on the sky.
        Expr<Vector<2> >::Ptr exprRefDelay =
            makeDirectionExpr(buffer->getDelayReference());
        Expr<Vector<3> >::Ptr exprRefDelayITRF =
            makeITRFExpr(instrument->position(), exprRefDelay);

        // Tile beam reference position on the sky.
        Expr<Vector<2> >::Ptr exprRefTile =
            makeDirectionExpr(buffer->getTileReference());
        Expr<Vector<3> >::Ptr exprRefTileITRF =
            makeITRFExpr(instrument->position(), exprRefTile);

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
                    makeDirectionalGainExpr(itsScope, instrument->station(i),
                    patch, config.usePhasors()));
            }

            // Beam.
            if(config.useBeam())
            {
                stationExpr[i] = compose(stationExpr[i],
                    makeBeamExpr(itsScope, instrument->station(i),
                    buffer->getReferenceFreq(), exprPatchPositionITRF,
                    exprRefDelayITRF, exprRefTileITRF, config.getBeamConfig(),
                    coeffLBA, coeffHBA));
            }

            // Faraday rotation.
            if(config.useFaradayRotation())
            {
                stationExpr[i] = compose(stationExpr[i],
                    makeFaradayRotationExpr(itsScope, instrument->station(i),
                    patch));
            }

            // Ionosphere.
            if(config.useIonosphere())
            {
                // Create an AZ, EL expression for the centroid direction of the
                // patch.
                Expr<Vector<2> >::Ptr exprAzEl =
                    makeAzElExpr(instrument->station(i)->position(),
                    exprPatchPosition);

                stationExpr[i] = compose(stationExpr[i],
                    makeIonosphereExpr(itsScope, instrument->station(i),
                    instrument->position(), exprAzEl, exprIonosphere));
            }
        }
    }

    if(haveDIE || haveDDE)
    {
        for(size_t i = 0; i < instrument->nStations(); ++i)
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
            itsExpr[i] = apply(stationExpr[baseline.first], itsExpr[i],
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

vector<string>
MeasurementExprLOFAR::makePatchList(SourceDB &sourceDB,
    const vector<string> &patterns)
{
    if(patterns.empty())
    {
        return sourceDB.getPatches(-1, "*");
    }

    // Create a list of all unique patches that match the given patterns.
    set<string> patches;
    for(size_t i = 0; i < patterns.size(); ++i)
    {
        vector<string> match(sourceDB.getPatches(-1, patterns[i]));
        patches.insert(match.begin(), match.end());
    }

    return vector<string>(patches.begin(), patches.end());
}

vector<Source::Ptr> MeasurementExprLOFAR::makeSourceList(SourceDB &sourceDB,
    const string &patch)
{
    vector<SourceInfo> sources(sourceDB.getPatchSources(patch));

    vector<Source::Ptr> result;
    result.reserve(sources.size());
    for(size_t i = 0; i < sources.size(); ++i)
    {
        result.push_back(Source::create(sources[i], itsScope));
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
