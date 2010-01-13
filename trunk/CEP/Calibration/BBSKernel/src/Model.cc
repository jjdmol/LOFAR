//# Model.cc: Measurement equation for the LOFAR telescope and its environment.
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
#include <BBSKernel/Model.h>

#include <BBSKernel/Exceptions.h>
#include <BBSKernel/ModelConfig.h>
#include <BBSKernel/Measurement.h>

#include <BBSKernel/Expr/ArrayFactor.h>
#include <BBSKernel/Expr/AzEl.h>
#include <BBSKernel/Expr/ConditionNumber.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/ExprVisData.h>
#include <BBSKernel/Expr/FlagIf.h>
#include <BBSKernel/Expr/GaussianCoherence.h>
#include <BBSKernel/Expr/GaussianSource.h>
#include <BBSKernel/Expr/Literal.h>
#include <BBSKernel/Expr/LMN.h>
#include <BBSKernel/Expr/MatrixInverse.h>
#include <BBSKernel/Expr/MatrixMul2.h>
#include <BBSKernel/Expr/MatrixMul3.h>
#include <BBSKernel/Expr/MatrixSum.h>
#include <BBSKernel/Expr/MergeFlags.h>
#include <BBSKernel/Expr/PhaseShift.h>
#include <BBSKernel/Expr/PiercePoint.h>
#include <BBSKernel/Expr/PointCoherence.h>
#include <BBSKernel/Expr/PointSource.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/ScalarMatrixMul.h>
#include <BBSKernel/Expr/SpectralIndex.h>
#include <BBSKernel/Expr/EquatorialCentroid.h>
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
// For std::advance()...
#include <iterator>

namespace LOFAR
{
namespace BBS
{

Model::Model(const Instrument &instrument, const SourceDB &sourceDb,
    const casa::MDirection &reference, double referenceFreq)
    :   itsInstrument(instrument),
        itsSourceDb(sourceDb),
        itsPhaseReference(reference),
        itsReferenceFreq(referenceFreq)
{
}

void Model::clear()
{
    itsExpr.clear();
    itsScope.clear();

    LOG_DEBUG_STR("" << itsCache);
    itsCache.clear();
    itsCache.clearStats();
}

void Model::makeForwardExpr(const ModelConfig &config, const VisData::Ptr&,
    const vector<baseline_t> &baselines)
{
    // Clear previously created expressions and cached results.
    clear();

    if(config.useFlagger())
    {
        LOG_WARN("Condition number flagging is only implemented for the inverse"
            " model.");
    }

    // Make a list of all the stations that are used in the given baseline
    // selection.
    vector<unsigned int> stations = makeUsedStationList(baselines);
    if(stations.empty())
    {
        THROW(BBSKernelException, "Baseline selection is empty.");
    }

    // Make a list of patches matching the selection criteria specified by the
    // user.
    vector<string> patches = makePatchList(config.getSources());
    if(patches.empty())
    {
        THROW(BBSKernelException, "No patches matching selection found in"
            " source database.");
    }

    LOG_INFO_STR("Number of patches in the model: " << patches.size());

    // Create a UVW expression per station.
    casa::Vector<Expr<Vector<3> >::Ptr> exprUVW = makeUVWExpr(stations);

    ElementBeamExpr::Ptr exprElementBeam;
    casa::Vector<Expr<Vector<2> >::Ptr> exprRefAzEl;
    if(config.useBeam())
    {
        const BeamConfig &beamConfig = config.getBeamConfig();

        // Read antenna configurations.
        itsInstrument.readAntennaConfigurations(beamConfig.getConfigPath());

        // Create a functor to generate element beam expression nodes.
        exprElementBeam = ElementBeamExpr::create(beamConfig, itsScope);

        // Create an AZ, EL expression per station for the phase center.
        exprRefAzEl = makeRefAzElExpr(stations);
    }

    IonosphereExpr::Ptr exprIonosphere;
    if(config.useIonosphere())
    {
        exprIonosphere = IonosphereExpr::create(config.getIonosphereConfig(),
            itsScope);
    }

    // Direction dependent effects.
    const bool haveDependentEffects = config.useDirectionalGain()
        || config.useBeam() || config.useIonosphere();

    vector<MatrixSum::Ptr> coherence(baselines.size());
    for(unsigned int i = 0; i < patches.size(); ++i)
    {
        vector<Source::Ptr> sources = makeSourceList(patches[i]);
        ASSERT(!sources.empty());

        casa::Vector<Expr<JonesMatrix>::Ptr> ddTransform;
        if(haveDependentEffects)
        {
            ddTransform.resize(stations.size());

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
                makeBeamExpr(config.getBeamConfig(), stations, exprRefAzEl,
                    exprAzEl, exprElementBeam, ddTransform);
            }

            if(config.useIonosphere())
            {
                makeIonosphereExpr(stations, itsInstrument.position(), exprAzEl,
                    exprIonosphere, ddTransform);
            }
        }

        // Create a station shift expression per (station, source) combination.
        casa::Matrix<Expr<Vector<2> >::Ptr> exprStationShift =
            makeStationShiftExpr(stations, sources, exprUVW);

        for(size_t j = 0; j < baselines.size(); ++j)
        {
            pair<unsigned int, unsigned int> idx = findStationIndices(stations,
                baselines[j]);

            // Construct an expression for the patch coherence on this baseline.
            Expr<JonesMatrix>::Ptr patchCoherence =
                makePatchCoherenceExpr(sources, exprUVW(idx.first),
                    exprStationShift.column(idx.first),
                    exprUVW(idx.second),
                    exprStationShift.column(idx.second));

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

        // Create a direction independent gain expression per station.
        if(config.useGain())
        {
            makeGainExpr(config, stations, diTransform);
        }
    }

    itsExpr.insert(itsExpr.begin(), coherence.begin(), coherence.end());
    for(size_t i = 0; i < baselines.size(); ++i)
    {
        // Apply direction independent effects if available.
        if(haveIndependentEffects)
        {
            pair<unsigned int, unsigned int> idx = findStationIndices(stations,
                baselines[i]);

            itsExpr[i] = corrupt(diTransform(idx.first), itsExpr[i],
                diTransform(idx.second));
        }
    }
}

void Model::makeInverseExpr(const ModelConfig &config,
    const VisData::Ptr &chunk, const vector<baseline_t> &baselines)
{
    // Clear previously created expressions and cached results.
    clear();

    // Make a list of all the stations that are used in the given baseline
    // selection.
    const vector<unsigned int> stations = makeUsedStationList(baselines);
    if(stations.empty())
    {
        THROW(BBSKernelException, "Baseline selection is empty.");
    }

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

        // Create a direction independent gain expression per station.
        if(config.useGain())
        {
            makeGainExpr(config, stations, transform);
        }
    }

    // Direction dependent effects.
    const bool haveDependentEffects = config.useDirectionalGain()
        || config.useBeam() || config.useIonosphere();

    ElementBeamExpr::Ptr exprElementBeam;
    casa::Vector<Expr<Vector<2> >::Ptr> exprRefAzEl;
    if(config.useBeam())
    {
        const BeamConfig &beamConfig = config.getBeamConfig();

        // Read antenna configurations.
        itsInstrument.readAntennaConfigurations(beamConfig.getConfigPath());

        // Create a functor to generate element beam expression nodes.
        exprElementBeam = ElementBeamExpr::create(beamConfig, itsScope);

        // Create an AZ, EL expression per station for the phase center.
        exprRefAzEl = makeRefAzElExpr(stations);
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
            makeBeamExpr(config.getBeamConfig(), stations, exprRefAzEl,
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

    itsExpr.reserve(baselines.size());
    for(size_t i = 0; i < baselines.size(); ++i)
    {
        Expr<JonesMatrix>::Ptr exprVisData(new ExprVisData(chunk,
            baselines[i]));

        if(haveIndependentEffects || haveDependentEffects)
        {
            pair<unsigned int, unsigned int> idx = findStationIndices(stations,
                baselines[i]);

            exprVisData = corrupt(transform(idx.first), exprVisData,
                transform(idx.second));
        }

        itsExpr.push_back(exprVisData);
    }
}

unsigned int Model::size() const
{
    return itsExpr.size();
}

Box Model::domain() const
{
    return ParmManager::instance().domain();
}

ParmGroup Model::getParms() const
{
    ParmGroup result;

    Scope::const_iterator parmIt = itsScope.begin();
    while(parmIt != itsScope.end())
    {
        result.insert(parmIt->first);
        ++parmIt;
    }

    return result;
}

ParmGroup Model::getSolvableParms() const
{
    ParmGroup result;

    Scope::const_iterator parmIt = itsScope.begin();
    while(parmIt != itsScope.end())
    {
        if(parmIt->second->getPValueFlag())
        {
            result.insert(parmIt->first);
        }

        ++parmIt;
    }

    return result;
}

void Model::setSolvableParms(const ParmGroup &solvables)
{
    ParmGroup::const_iterator solIt = solvables.begin();
    while(solIt != solvables.end())
    {
        Scope::iterator parmIt = itsScope.find(*solIt);
        ASSERT(parmIt != itsScope.end());

        if(parmIt != itsScope.end())
        {
            parmIt->second->setPValueFlag();
        }

        ++solIt;
    }
}

void Model::clearSolvableParms()
{
    Scope::iterator parmIt = itsScope.begin();
    while(parmIt != itsScope.end())
    {
        parmIt->second->clearPValueFlag();
        ++parmIt;
    }
}

void Model::setEvalGrid(const Grid &grid)
{
    itsRequest = Request(grid);
    LOG_DEBUG_STR("" << itsCache);
    itsCache.clearStats();
    itsCache.clear();

    // TODO: Set cache size in number of Matrix instances... ?
}

const JonesMatrix Model::evaluate(unsigned int i)
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

vector<unsigned int>
Model::makeUsedStationList(const vector<baseline_t> &baselines) const
{
    set<unsigned int> used;
    for(vector<baseline_t>::const_iterator it = baselines.begin(),
        end = baselines.end();
        it != end; ++it)
    {
        used.insert(it->first);
        used.insert(it->second);
    }

    return vector<unsigned int>(used.begin(), used.end());
}

pair<unsigned int, unsigned int>
Model::findStationIndices(const vector<unsigned int> &stations,
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

vector<string> Model::makePatchList(const vector<string> &patterns)
{
    if(patterns.empty())
    {
        return itsSourceDb.getPatches(-1, "*");
    }

    // Create a list of all unique patches that match the given patterns.
    set<string> patches;
    for(size_t i = 0; i < patterns.size(); ++i)
    {
        vector<string> match(itsSourceDb.getPatches(-1, patterns[i]));
        patches.insert(match.begin(), match.end());
    }

    return vector<string>(patches.begin(), patches.end());
}

vector<Source::Ptr> Model::makeSourceList(const string &patch)
{
    vector<SourceInfo> sources(itsSourceDb.getPatchSources(patch));

    vector<Source::Ptr> result;
    result.reserve(sources.size());
    for(size_t i = 0; i < sources.size(); ++i)
    {
        result.push_back(Source::create(sources[i], itsScope));
    }

    return result;
}

Expr<Vector<2> >::ConstPtr
Model::makePatchCentroidExpr(const vector<Source::Ptr> &sources) const
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
Model::makePatchCoherenceExpr(const vector<Source::Ptr> &sources,
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
Model::makeUVWExpr(const vector<unsigned int> &stations)
{
    casa::Vector<Expr<Vector<3> >::Ptr> expr(stations.size());

    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const Station &station = itsInstrument[stations[i]];
        expr(i) = StationUVW::Ptr(new StationUVW(station.position(),
            itsInstrument.position(), itsPhaseReference));
    }

    return expr;
}

casa::Vector<Expr<Vector<2> >::Ptr>
Model::makeAzElExpr(const vector<unsigned int> &stations,
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
Model::makeRefAzElExpr(const vector<unsigned int> &stations) const
{
    casa::MDirection refJ2000(casa::MDirection::Convert(itsPhaseReference,
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
Model::makeStationShiftExpr(const vector<unsigned int> &stations,
    const vector<Source::Ptr> &sources,
    const casa::Vector<Expr<Vector<3> >::Ptr> &exprUVW) const
{
    casa::Vector<LMN::Ptr> exprLMN(sources.size());
    for(unsigned int i = 0; i < sources.size(); ++i)
    {
        exprLMN(i) = LMN::Ptr(new LMN(itsPhaseReference,
            sources[i]->position()));
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

void Model::makeBandpassExpr(const vector<unsigned int> &stations,
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

void Model::makeGainExpr(const ModelConfig &config,
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

void Model::makeDirectionalGainExpr(const ModelConfig &config,
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

void Model::makeBeamExpr(const BeamConfig &config,
    const vector<unsigned int> &stations,
    const casa::Vector<Expr<Vector<2> >::Ptr> &exprRefAzEl,
    const casa::Vector<Expr<Vector<2> >::Ptr> &exprAzEl,
    const ElementBeamExpr::ConstPtr &exprElement,
    casa::Vector<Expr<JonesMatrix>::Ptr> &accumulator)
{
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const unsigned int station = stations[i];

        // Get station element configuration.
        const AntennaConfig &antennaConfig =
            itsInstrument[station].config(config.getConfigName());

        // Get element orientation.
        Expr<Scalar>::Ptr orientation = itsScope(INSTRUMENT,
            "AntennaOrientation:" + itsInstrument[station].name());

        // Create ArrayFactor expression.
        Expr<JonesMatrix>::Ptr exprArrayFactor(new ArrayFactor(exprAzEl(i),
            exprRefAzEl(i), antennaConfig, itsReferenceFreq));

        accumulator(i) = compose(accumulator(i),
            Expr<JonesMatrix>::Ptr(new MatrixMul2(exprArrayFactor,
                exprElement->construct(exprAzEl(i), orientation))));
    }
}

void Model::makeIonosphereExpr(const vector<unsigned int> &stations,
    const casa::MPosition &refPosition,
    const casa::Vector<Expr<Vector<2> >::Ptr> &exprAzEl,
    const IonosphereExpr::ConstPtr &exprIonosphere,
    casa::Vector<Expr<JonesMatrix>::Ptr> &accumulator)
{
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const unsigned int station = stations[i];

        const casa::MPosition &position = itsInstrument[station].position();
        PiercePoint::Ptr piercePoint(new PiercePoint(position, exprAzEl(i)));

        accumulator(i) = compose(accumulator(i),
            exprIonosphere->construct(refPosition, piercePoint));
    }
}

Expr<JonesMatrix>::Ptr Model::compose(const Expr<JonesMatrix>::Ptr &accumulator,
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

Expr<JonesMatrix>::Ptr Model::corrupt(const Expr<JonesMatrix>::Ptr &lhs,
    const Expr<JonesMatrix>::Ptr &coherence,
    const Expr<JonesMatrix>::Ptr &rhs) const
{
    return Expr<JonesMatrix>::Ptr(new MatrixMul3(lhs, coherence, rhs));
}

} // namespace BBS
} // namespace LOFAR
