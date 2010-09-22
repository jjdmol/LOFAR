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

#include <BBSKernel/Expr/ArrayFactor.h>
#include <BBSKernel/Expr/AzEl.h>
#include <BBSKernel/Expr/CachePolicy.h>
#include <BBSKernel/Expr/ConditionNumber.h>
#include <BBSKernel/Expr/Delay.h>
#include <BBSKernel/Expr/EquatorialCentroid.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/ExprVisData.h>
#include <BBSKernel/Expr/FaradayRotation.h>
#include <BBSKernel/Expr/FlagIf.h>
#include <BBSKernel/Expr/GaussianCoherence.h>
#include <BBSKernel/Expr/GaussianSource.h>
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
#include <BBSKernel/Expr/TileArrayFactor.h>
#include <BBSKernel/Expr/StationShift.h>
#include <BBSKernel/Expr/StationUVW.h>

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

#include <functional>
// For std::distance().
#include <iterator>

namespace LOFAR
{
namespace BBS
{

MeasurementExprLOFAR::MeasurementExprLOFAR(const ModelConfig &config,
    const SourceDB &sourceDB, const Instrument &instrument,
    const BaselineSeq &baselines, const casa::MDirection &phaseRef,
    double refFreq, bool circular)
    :   itsInstrument(instrument),
        itsSourceDB(sourceDB),
        itsBaselines(baselines),
        itsCachePolicy(new DefaultCachePolicy())
{
    setCorrelations(circular);
    makeForwardExpr(config, phaseRef, refFreq, circular);
}

MeasurementExprLOFAR::MeasurementExprLOFAR(const ModelConfig &config,
    const SourceDB &sourceDB,
    const VisBuffer::Ptr &chunk, const BaselineMask &mask, bool forward)
    :   itsInstrument(chunk->instrument()),
        itsSourceDB(sourceDB),
        itsBaselines(filter(chunk->baselines(), mask)),
        itsCachePolicy(new DefaultCachePolicy())
{
    const bool circular = isCircular(chunk);
    ASSERTSTR(circular != isLinear(chunk), "The correlations in the measurement"
        " should be either all linear or all circular.");
    setCorrelations(circular);

    if(forward)
    {
        makeForwardExpr(config, chunk->getPhaseReference(),
            chunk->getReferenceFreq(), circular);
    }
    else
    {
        makeInverseExpr(config, chunk, chunk->getPhaseReference(),
            chunk->getReferenceFreq(), circular);
    }
}

void MeasurementExprLOFAR::solvablesChanged()
{
//    LOG_DEBUG_STR("" << itsCache);
    itsCache.clear(Cache::VOLATILE);
    itsCache.clearStats();
}

void MeasurementExprLOFAR::makeForwardExpr(const ModelConfig &config,
    const casa::MDirection &phaseRef, double refFreq, bool circular)
{
    // If no baselines are available, nothing needs to be done here.
    if(itsBaselines.empty())
    {
        return;
    }

    if(config.useFlagger())
    {
        LOG_WARN("Condition number flagging is only implemented for the inverse"
            " model.");
    }

    // Make a list of all the stations that are used in the given baseline
    // selection.
    vector<unsigned int> stations(makeUsedStationList());

    // Make a list of patches matching the selection criteria specified by the
    // user.
    vector<string> patches = makePatchList(config.getSources());
    if(patches.empty())
    {
        THROW(BBSKernelException, "No patches matching selection found in"
            " source database.");
    }

    // Create a linear to circular-RL transformation Jones matrix.
    Expr<JonesMatrix>::Ptr H(new LinearToCircularRL());

    // Create a UVW expression per station.
    casa::Vector<Expr<Vector<3> >::Ptr> exprUVW = makeUVWExpr(phaseRef,
        stations);

    ElementBeamExpr::Ptr exprElementBeam;
    casa::Vector<Expr<Vector<2> >::Ptr> exprRefAzEl;
    if(config.useBeam())
    {
        const BeamConfig &beamConfig = config.getBeamConfig();

        // Read antenna configurations.
        itsInstrument.readLOFARAntennaConfig(beamConfig.getConfigPath());

        // Create a functor to generate element beam expression nodes.
        exprElementBeam = ElementBeamExpr::create(beamConfig, itsScope);

        // Create an AZ, EL expression per station for the phase center.
        exprRefAzEl = makeRefAzElExpr(phaseRef, stations);
    }

    IonosphereExpr::Ptr exprIonosphere;
    if(config.useIonosphere())
    {
        exprIonosphere = IonosphereExpr::create(config.getIonosphereConfig(),
            itsScope);
    }

    // Direction dependent effects.
    const bool haveDependentEffects = config.useFaradayRotation()
        || config.useDirectionalGain() || config.useBeam()
        || config.useIonosphere();

    vector<MatrixSum::Ptr> coherence(itsBaselines.size());
    for(unsigned int i = 0; i < patches.size(); ++i)
    {
        vector<Source::Ptr> sources = makeSourceList(patches[i]);
        ASSERT(!sources.empty());

        casa::Vector<Expr<JonesMatrix>::Ptr> ddTransform;
        if(haveDependentEffects)
        {
            ddTransform.resize(stations.size());

            if(config.useFaradayRotation())
            {
                makeFaradayRotationExpr(config, stations, patches[i],
                    ddTransform);
            }

            if(config.useDirectionalGain())
            {
                makeDirectionalGainExpr(config, stations, patches[i],
                    ddTransform);
            }

            // Create an AZ, EL expression for each station for the centroid
            // direction of the patch.
            casa::Vector<Expr<Vector<2> >::Ptr> exprAzEl;
            if(config.useBeam() || config.useIonosphere())
            {
                exprAzEl = makeAzElExpr(stations,
                    makePatchCentroidExpr(sources));
            }

            if(config.useBeam())
            {
                makeBeamExpr(config.getBeamConfig(), refFreq, stations,
                    exprRefAzEl, exprAzEl, exprElementBeam, ddTransform);
            }

            if(config.useIonosphere())
            {
                makeIonosphereExpr(stations, itsInstrument.position(), exprAzEl,
                    exprIonosphere, ddTransform);
            }
        }

        // Create a station shift expression per (station, source) combination.
        casa::Matrix<Expr<Vector<2> >::Ptr> exprStationShift =
            makeStationShiftExpr(phaseRef, stations, sources, exprUVW);

        for(size_t j = 0; j < itsBaselines.size(); ++j)
        {
            pair<unsigned int, unsigned int> idx = findStationIndices(stations,
                itsBaselines[j]);

            // Construct an expression for the patch coherence on this baseline.
            Expr<JonesMatrix>::Ptr patchCoherence =
                makePatchCoherenceExpr(sources, exprUVW(idx.first),
                    exprStationShift.column(idx.first),
                    exprUVW(idx.second),
                    exprStationShift.column(idx.second));

            // Convert to circular-RL if required.
            if(circular)
            {
                patchCoherence = corrupt(H, patchCoherence, H);
            }

            // Apply direction dependent effects.
            if(haveDependentEffects)
            {
                patchCoherence = corrupt(ddTransform(idx.first), patchCoherence,
                    ddTransform(idx.second));
            }

            // Add patch coherence to the visibilities for this baseline.
            if(!coherence[j])
            {
                coherence[j] = MatrixSum::Ptr(new MatrixSum());
            }

            coherence[j]->connect(patchCoherence);
        }
    }

    // Direction independent effects.
    const bool haveIndependentEffects = config.useBandpass()
        || config.useGain();

    casa::Vector<Expr<JonesMatrix>::Ptr> diTransform;
    if(haveIndependentEffects)
    {
        diTransform.resize(stations.size());

        // Create a bandpass expression per station.
        if(config.useBandpass())
        {
            makeBandpassExpr(stations, diTransform);
        }

        // Create a clock delay expression per station.
        if(config.useClock())
        {
            makeClockExpr(stations, diTransform);
        }

        // Create a direction independent gain expression per station.
        if(config.useGain())
        {
            makeGainExpr(config, stations, diTransform);
        }
    }

    itsExpr.insert(itsExpr.begin(), coherence.begin(), coherence.end());
    for(size_t i = 0; i < itsBaselines.size(); ++i)
    {
        // Apply direction independent effects if available.
        if(haveIndependentEffects)
        {
            pair<unsigned int, unsigned int> idx = findStationIndices(stations,
                itsBaselines[i]);

            itsExpr[i] = corrupt(diTransform(idx.first), itsExpr[i],
                diTransform(idx.second));
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

void MeasurementExprLOFAR::makeInverseExpr(const ModelConfig &config,
    const VisBuffer::Ptr &chunk, const casa::MDirection &phaseRef,
    double refFreq, bool circular)
{
    // If no baselines are available, nothing needs to be done here.
    if(itsBaselines.empty())
    {
        return;
    }

    // Make a list of all the stations that are used in the given baseline
    // selection.
    const vector<unsigned int> stations = makeUsedStationList();

    // Create a single Jones matrix expression for each station, for the
    // selected direction.
    casa::Vector<Expr<JonesMatrix>::Ptr> transform(stations.size());

    // Direction independent effects.
    const bool haveIndependentEffects = config.useBandpass()
        || config.useGain();

    if(haveIndependentEffects)
    {
        // Create a bandpass expression per station.
        if(config.useBandpass())
        {
            makeBandpassExpr(stations, transform);
        }

        // Create a clock delay expression per station.
        if(config.useClock())
        {
            makeClockExpr(stations, transform);
        }

        // Create a direction independent gain expression per station.
        if(config.useGain())
        {
            makeGainExpr(config, stations, transform);
        }
    }

    // Direction dependent effects.
    const bool haveDependentEffects = config.useFaradayRotation()
        || config.useDirectionalGain() || config.useBeam()
        || config.useIonosphere();

    ElementBeamExpr::Ptr exprElementBeam;
    casa::Vector<Expr<Vector<2> >::Ptr> exprRefAzEl;
    if(config.useBeam())
    {
        const BeamConfig &beamConfig = config.getBeamConfig();

        // Read antenna configurations.
        itsInstrument.readLOFARAntennaConfig(beamConfig.getConfigPath());

        // Create a functor to generate element beam expression nodes.
        exprElementBeam = ElementBeamExpr::create(beamConfig, itsScope);

        // Create an AZ, EL expression per station for the phase center.
        exprRefAzEl = makeRefAzElExpr(phaseRef,
            stations);
    }

    IonosphereExpr::Ptr exprIonosphere;
    if(config.useIonosphere())
    {
        exprIonosphere = IonosphereExpr::create(config.getIonosphereConfig(),
            itsScope);
    }

    if(haveDependentEffects)
    {
        // Make a list of patches matching the selection criteria specified by
        // the user.
        vector<string> patches = makePatchList(config.getSources());
        if(patches.size() != 1)
        {
            THROW(BBSKernelException, "No patch, or more than one patch"
                " selected, yet corrections can only be applied for a single"
                " direction on the sky");
        }

        if(config.useFaradayRotation())
        {
            makeFaradayRotationExpr(config, stations, patches[0], transform);
        }

        if(config.useDirectionalGain())
        {
            makeDirectionalGainExpr(config, stations, patches[0], transform);
        }

        // Create an AZ, EL expression for each station for the centroid
        // direction of the patch.
        casa::Vector<Expr<Vector<2> >::Ptr> exprAzEl;
        if(config.useBeam() || config.useIonosphere())
        {
            vector<Source::Ptr> sources = makeSourceList(patches[0]);
            ASSERT(!sources.empty());

            exprAzEl = makeAzElExpr(stations, makePatchCentroidExpr(sources));
        }

        if(config.useBeam())
        {
            makeBeamExpr(config.getBeamConfig(), refFreq, stations, exprRefAzEl,
                exprAzEl, exprElementBeam, transform);
        }

        if(config.useIonosphere())
        {
            makeIonosphereExpr(stations, itsInstrument.position(), exprAzEl,
                exprIonosphere, transform);
        }
    }

    if(haveIndependentEffects || haveDependentEffects)
    {
        for(size_t i = 0; i < stations.size(); ++i)
        {
            if(config.useFlagger())
            {
                const FlaggerConfig &flagConfig = config.getFlaggerConfig();

                Expr<Scalar>::Ptr exprCond(new ConditionNumber(transform(i)));
                Expr<Scalar>::Ptr exprThreshold(makeFlagIf(exprCond,
                    std::bind2nd(std::greater_equal<double>(),
                        flagConfig.getThreshold())));

                typedef MergeFlags<JonesMatrix, Scalar> T_MERGEFLAGS;
                transform(i) = T_MERGEFLAGS::Ptr(new T_MERGEFLAGS(transform(i),
                    exprThreshold));
            }

            transform(i) =
                Expr<JonesMatrix>::Ptr(new MatrixInverse(transform(i)));
        }
    }

    itsExpr.reserve(itsBaselines.size());
    for(size_t i = 0; i < itsBaselines.size(); ++i)
    {
        Expr<JonesMatrix>::Ptr exprVisData;

        if(circular)
        {
            exprVisData.reset(new ExprVisData(chunk, itsBaselines[i],
                Correlation::RR, Correlation::RL, Correlation::LR,
                Correlation::LL));
        }
        else
        {
            exprVisData.reset(new ExprVisData(chunk, itsBaselines[i]));
        }

        if(haveIndependentEffects || haveDependentEffects)
        {
            pair<unsigned int, unsigned int> idx = findStationIndices(stations,
                itsBaselines[i]);

            exprVisData = corrupt(transform(idx.first), exprVisData,
                transform(idx.second));
        }

        itsExpr.push_back(exprVisData);
    }

    // Set caching policy.
    itsCachePolicy = CachePolicy::Ptr(new DefaultCachePolicy());
    if(config.useCache())
    {
        itsCachePolicy = CachePolicy::Ptr(new ExperimentalCachePolicy());
    }

    itsCachePolicy->apply(itsExpr.begin(), itsExpr.end());
}

unsigned int MeasurementExprLOFAR::size() const
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

bool MeasurementExprLOFAR::isLinear(const VisBuffer::Ptr &chunk) const
{
    for(size_t i = 0; i < chunk->nCorrelations(); ++i)
    {
        if(!Correlation::isLinear(chunk->correlations()[i]))
        {
            return false;
        }
    }

    return true;
}

bool MeasurementExprLOFAR::isCircular(const VisBuffer::Ptr &chunk) const
{
    for(size_t i = 0; i < chunk->nCorrelations(); ++i)
    {
        if(!Correlation::isCircular(chunk->correlations()[i]))
        {
            return false;
        }
    }

    return true;
}

vector<unsigned int> MeasurementExprLOFAR::makeUsedStationList() const
{
    set<unsigned int> used;
    for(size_t i = 0; i < itsBaselines.size(); ++i)
    {
        used.insert(itsBaselines[i].first);
        used.insert(itsBaselines[i].second);
    }

    return vector<unsigned int>(used.begin(), used.end());
}

pair<unsigned int, unsigned int>
MeasurementExprLOFAR::findStationIndices(const vector<unsigned int> &stations,
    const baseline_t &baseline) const
{
    // Find left and right hand side station index.
    vector<unsigned int>::const_iterator it;

    it = find(stations.begin(), stations.end(), baseline.first);
    unsigned int lhs = distance(stations.begin(), it);

    it = find(stations.begin(), stations.end(), baseline.second);
    unsigned int rhs = distance(stations.begin(), it);
    ASSERT(lhs < stations.size() && rhs < stations.size());

    return make_pair(lhs, rhs);
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

Expr<Vector<2> >::ConstPtr
MeasurementExprLOFAR::makePatchCentroidExpr(const vector<Source::Ptr> &sources)
    const
{
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

Expr<JonesMatrix>::Ptr
MeasurementExprLOFAR::makePatchCoherenceExpr(const vector<Source::Ptr> &sources,
    const Expr<Vector<3> >::Ptr &uvwLHS,
    const casa::Vector<Expr<Vector<2> >::Ptr> &shiftLHS,
    const Expr<Vector<3> >::Ptr &uvwRHS,
    const casa::Vector<Expr<Vector<2> >::Ptr> &shiftRHS) const
{
    if(sources.size() == 1)
    {
        // Source coherence.
        Expr<JonesMatrix>::Ptr coherence =
            sources[0]->coherence(uvwLHS, uvwRHS);

        // Phase shift (incorporates geometry and fringe stopping).
        Expr<Scalar>::Ptr shift(new PhaseShift(shiftLHS(0), shiftRHS(0)));

        // Phase shift the source coherence to the correct position.
        return Expr<JonesMatrix>::Ptr(new ScalarMatrixMul(shift, coherence));
    }

    MatrixSum::Ptr sum(new MatrixSum());
    for(size_t i = 0; i < sources.size(); ++i)
    {
        // Source coherence.
        Expr<JonesMatrix>::Ptr coherence =
            sources[i]->coherence(uvwLHS, uvwRHS);

        // Phase shift (incorporates geometry and fringe stopping).
        Expr<Scalar>::Ptr shift(new PhaseShift(shiftLHS(i), shiftRHS(i)));

        // Phase shift the source coherence to the correct position.
        coherence = Expr<JonesMatrix>::Ptr(new ScalarMatrixMul(shift,
            coherence));

        sum->connect(coherence);
    }

    return sum;
}

casa::Vector<Expr<Vector<3> >::Ptr>
MeasurementExprLOFAR::makeUVWExpr(const casa::MDirection &phaseRef,
    const vector<unsigned int> &stations)
{
    casa::Vector<Expr<Vector<3> >::Ptr> expr(stations.size());

    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const Station &station = itsInstrument[stations[i]];
        expr(i) = StationUVW::Ptr(new StationUVW(station.position(),
            itsInstrument.position(), phaseRef));
    }

    return expr;
}

casa::Vector<Expr<Vector<2> >::Ptr>
MeasurementExprLOFAR::makeAzElExpr(const vector<unsigned int> &stations,
    const Expr<Vector<2> >::ConstPtr &direction) const
{
    // Create an AzEl expression for each (station, source) combination.
    casa::Vector<Expr<Vector<2> >::Ptr> expr(stations.size());
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const casa::MPosition &position = itsInstrument[stations[i]].position();
        expr(i) = Expr<Vector<2> >::Ptr(new AzEl(position, direction));
    }

    return expr;
}

casa::Vector<Expr<Vector<2> >::Ptr>
MeasurementExprLOFAR::makeRefAzElExpr(const casa::MDirection &phaseRef,
    const vector<unsigned int> &stations) const
{
    casa::MDirection refJ2000(casa::MDirection::Convert(phaseRef,
        casa::MDirection::J2000)());
    casa::Quantum<casa::Vector<casa::Double> > refAngles = refJ2000.getAngle();

    Literal::Ptr refRa(new Literal(refAngles.getBaseValue()(0)));
    Literal::Ptr refDec(new Literal(refAngles.getBaseValue()(1)));

    AsExpr<Vector<2> >::Ptr exprRefDir(new AsExpr<Vector<2> >());
    exprRefDir->connect(0, refRa);
    exprRefDir->connect(1, refDec);

    return makeAzElExpr(stations, exprRefDir);
}

casa::Matrix<Expr<Vector<2> >::Ptr>
MeasurementExprLOFAR::makeStationShiftExpr(const casa::MDirection &phaseRef,
    const vector<unsigned int> &stations,
    const vector<Source::Ptr> &sources,
    const casa::Vector<Expr<Vector<3> >::Ptr> &exprUVW) const
{
    casa::Vector<LMN::Ptr> exprLMN(sources.size());
    for(unsigned int i = 0; i < sources.size(); ++i)
    {
        exprLMN(i) = LMN::Ptr(new LMN(phaseRef, sources[i]->position()));
    }

    casa::Matrix<Expr<Vector<2> >::Ptr> expr(sources.size(), stations.size());
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        for(unsigned int j = 0; j < sources.size(); ++j)
        {
            expr(j, i) =
                Expr<Vector<2> >::Ptr(new StationShift(exprUVW(i), exprLMN(j)));
        }
    }

    return expr;
}

void
MeasurementExprLOFAR::makeBandpassExpr(const vector<unsigned int> &stations,
    casa::Vector<Expr<JonesMatrix>::Ptr> &accumulator)
{
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const unsigned int station = stations[i];
        const string &suffix = itsInstrument[station].name();

        Expr<Scalar>::Ptr B00 = itsScope(INSTRUMENT, "Bandpass:0:0:"
            + suffix);
        Expr<Scalar>::Ptr B11 = itsScope(INSTRUMENT, "Bandpass:1:1:"
            + suffix);

        accumulator(i) = compose(accumulator(i),
            Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(B00, B11)));
    }
}

void MeasurementExprLOFAR::makeClockExpr(const vector<unsigned int> &stations,
    casa::Vector<Expr<JonesMatrix>::Ptr> &accumulator)
{
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const unsigned int station = stations[i];
        const string &suffix = itsInstrument[station].name();

        ExprParm::Ptr delay = itsScope(INSTRUMENT, "Clock:" + suffix);

        Expr<Scalar>::Ptr shift = Expr<Scalar>::Ptr(new Delay(delay));
        accumulator(i) = compose(accumulator(i),
            Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(shift, shift)));
    }
}

void MeasurementExprLOFAR::makeGainExpr(const ModelConfig &config,
    const vector<unsigned int> &stations,
    casa::Vector<Expr<JonesMatrix>::Ptr> &accumulator)
{
    string elem0(config.usePhasors() ? "Ampl"  : "Real");
    string elem1(config.usePhasors() ? "Phase" : "Imag");

    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        Expr<Scalar>::Ptr J00, J01, J10, J11;

        const unsigned int station = stations[i];
        const string &suffix = itsInstrument[station].name();

        ExprParm::Ptr J00_elem0 =
            itsScope(INSTRUMENT, "Gain:0:0:" + elem0 + ":" + suffix);
        ExprParm::Ptr J00_elem1 =
            itsScope(INSTRUMENT, "Gain:0:0:" + elem1 + ":" + suffix);
        ExprParm::Ptr J01_elem0 =
            itsScope(INSTRUMENT, "Gain:0:1:" + elem0 + ":" + suffix);
        ExprParm::Ptr J01_elem1 =
            itsScope(INSTRUMENT, "Gain:0:1:" + elem1 + ":" + suffix);
        ExprParm::Ptr J10_elem0 =
            itsScope(INSTRUMENT, "Gain:1:0:" + elem0 + ":" + suffix);
        ExprParm::Ptr J10_elem1 =
            itsScope(INSTRUMENT, "Gain:1:0:" + elem1 + ":" + suffix);
        ExprParm::Ptr J11_elem0 =
            itsScope(INSTRUMENT, "Gain:1:1:" + elem0 + ":" + suffix);
        ExprParm::Ptr J11_elem1 =
            itsScope(INSTRUMENT, "Gain:1:1:" + elem1 + ":" + suffix);

        if(config.usePhasors())
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

        accumulator(i) = compose(accumulator(i),
            Expr<JonesMatrix>::Ptr(new AsExpr<JonesMatrix>(J00, J01, J10,
                J11)));
    }
}

void MeasurementExprLOFAR::makeDirectionalGainExpr(const ModelConfig &config,
    const vector<unsigned int> &stations, const string &patch,
    casa::Vector<Expr<JonesMatrix>::Ptr> &accumulator)
{
    string elem0(config.usePhasors() ? "Ampl"  : "Real");
    string elem1(config.usePhasors() ? "Phase" : "Imag");

    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const unsigned int station = stations[i];

        Expr<Scalar>::Ptr J00, J01, J10, J11;

        string suffix(itsInstrument[station].name() + ":" + patch);

        ExprParm::Ptr J00_elem0 = itsScope(INSTRUMENT,
            "DirectionalGain:0:0:" + elem0 + ":" + suffix);
        ExprParm::Ptr J00_elem1 = itsScope(INSTRUMENT,
            "DirectionalGain:0:0:" + elem1 + ":" + suffix);
        ExprParm::Ptr J01_elem0 = itsScope(INSTRUMENT,
            "DirectionalGain:0:1:" + elem0 + ":" + suffix);
        ExprParm::Ptr J01_elem1 = itsScope(INSTRUMENT,
            "DirectionalGain:0:1:" + elem1 + ":" + suffix);
        ExprParm::Ptr J10_elem0 = itsScope(INSTRUMENT,
            "DirectionalGain:1:0:" + elem0 + ":" + suffix);
        ExprParm::Ptr J10_elem1 = itsScope(INSTRUMENT,
            "DirectionalGain:1:0:" + elem1 + ":" + suffix);
        ExprParm::Ptr J11_elem0 = itsScope(INSTRUMENT,
            "DirectionalGain:1:1:" + elem0 + ":" + suffix);
        ExprParm::Ptr J11_elem1 = itsScope(INSTRUMENT,
            "DirectionalGain:1:1:" + elem1 + ":" + suffix);

        if(config.usePhasors())
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

        accumulator(i) = compose(accumulator(i),
            Expr<JonesMatrix>::Ptr(new AsExpr<JonesMatrix>(J00, J01, J10,
                J11)));
    }
}

void MeasurementExprLOFAR::makeBeamExpr(const BeamConfig &config,
    double refFreq, const vector<unsigned int> &stations,
    const casa::Vector<Expr<Vector<2> >::Ptr> &exprRefAzEl,
    const casa::Vector<Expr<Vector<2> >::Ptr> &exprAzEl,
    const ElementBeamExpr::ConstPtr &exprElement,
    casa::Vector<Expr<JonesMatrix>::Ptr> &accumulator)
{
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const Station &station = itsInstrument[stations[i]];

        // Get element orientation.
        Expr<Scalar>::Ptr orientation = itsScope(INSTRUMENT,
            "AntennaOrientation:" + station.name());

        AntennaSelection selection;
        Expr<JonesMatrix>::Ptr exprBeam(exprElement->construct(exprAzEl(i),
            orientation));

        // Get LOFAR station name suffix.
        // NB. THIS IS A TEMPORARY SOLUTION THAT CAN BE REMOVED AS SOON AS THE
        // DIPOLE INFORMATION IS STORED AS META-DATA INSIDE THE MS.
        const string suffix = station.name().substr(5);
        if(suffix == "LBA")
        {
            try
            {
                selection = station.selection(config.getConfigName());
            }
            catch(BBSKernelException &ex)
            {
                selection = station.selection("LBA");
            }
        }
        else if(suffix == "HBA0")
        {
            selection = station.selection("HBA_0");

            Expr<JonesMatrix>::Ptr exprTileFactor =
                Expr<JonesMatrix>::Ptr(new TileArrayFactor(exprAzEl(i),
                    exprRefAzEl(i), station.tile(0)));

            exprBeam = Expr<JonesMatrix>::Ptr(new MatrixMul2(exprTileFactor,
                exprBeam));
        }
        else if(suffix == "HBA1")
        {
            selection = station.selection("HBA_1");

            Expr<JonesMatrix>::Ptr exprTileFactor =
                Expr<JonesMatrix>::Ptr(new TileArrayFactor(exprAzEl(i),
                    exprRefAzEl(i), station.tile(1)));

            exprBeam = Expr<JonesMatrix>::Ptr(new MatrixMul2(exprTileFactor,
                exprBeam));
        }
        else if(suffix == "HBA")
        {
            selection = station.selection("HBA");

            Expr<JonesMatrix>::Ptr exprTileFactor =
                Expr<JonesMatrix>::Ptr(new TileArrayFactor(exprAzEl(i),
                    exprRefAzEl(i), station.tile(0)));

            exprBeam = Expr<JonesMatrix>::Ptr(new MatrixMul2(exprTileFactor,
                exprBeam));
        }
        else
        {
            THROW(BBSKernelException, "Illegal LOFAR station name encoutered: "
                << station.name());
        }

        // Create ArrayFactor expression.
        Expr<JonesMatrix>::Ptr exprArrayFactor(new ArrayFactor(exprAzEl(i),
            exprRefAzEl(i), selection, refFreq));

        accumulator(i) = compose(accumulator(i),
            Expr<JonesMatrix>::Ptr(new MatrixMul2(exprArrayFactor, exprBeam)));
    }
}

void
MeasurementExprLOFAR::makeIonosphereExpr(const vector<unsigned int> &stations,
    const casa::MPosition &refPosition,
    const casa::Vector<Expr<Vector<2> >::Ptr> &exprAzEl,
    const IonosphereExpr::ConstPtr &exprIonosphere,
    casa::Vector<Expr<JonesMatrix>::Ptr> &accumulator)
{
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const unsigned int station = stations[i];
        accumulator(i) =
            compose(accumulator(i), exprIonosphere->construct(refPosition,
                itsInstrument[station].position(), exprAzEl(i)));
    }
}

void MeasurementExprLOFAR::makeFaradayRotationExpr(const ModelConfig&,
    const vector<unsigned int> &stations, const string &patch,
    casa::Vector<Expr<JonesMatrix>::Ptr> &accumulator)
{
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const unsigned int station = stations[i];

        ExprParm::Ptr rm = itsScope(INSTRUMENT, "RotationMeasure:"
            + itsInstrument[station].name() + ":" + patch);

        accumulator(i) = compose(accumulator(i),
            Expr<JonesMatrix>::Ptr(new FaradayRotation(rm)));
    }
}

Expr<JonesMatrix>::Ptr
MeasurementExprLOFAR::compose(const Expr<JonesMatrix>::Ptr &accumulator,
    const Expr<JonesMatrix>::Ptr &effect) const
{
    if(accumulator)
    {
        return Expr<JonesMatrix>::Ptr(new MatrixMul2(accumulator, effect));
    }
    else
    {
        return effect;
    }
}

Expr<JonesMatrix>::Ptr
MeasurementExprLOFAR::corrupt(const Expr<JonesMatrix>::Ptr &lhs,
    const Expr<JonesMatrix>::Ptr &coherence,
    const Expr<JonesMatrix>::Ptr &rhs) const
{
    return Expr<JonesMatrix>::Ptr(new MatrixMul3(lhs, coherence, rhs));
}

} // namespace BBS
} // namespace LOFAR
