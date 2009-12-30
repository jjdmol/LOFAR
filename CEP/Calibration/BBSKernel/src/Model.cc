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
#include <BBSKernel/Expr/HamakerDipole.h>
//#include <BBSKernel/Expr/JonesVisData.h>
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
#include <BBSKernel/Expr/PolynomialPhaseScreen.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/ScalarMatrixMul.h>
#include <BBSKernel/Expr/SpectralIndex.h>
#include <BBSKernel/Expr/StationShift.h>
#include <BBSKernel/Expr/StationUVW.h>
//#include <BBSKernel/Expr/StatUVW.h>
#include <BBSKernel/Expr/YatawattaDipole.h>

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
    // Make station UVW expression for all stations.
//    makeStationUVW();
}

void Model::clear()
{
    itsExpr.clear();
    itsParms.clear();

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
    const vector<unsigned int> stations = makeUsedStationList(baselines);
    if(stations.empty())
    {
        THROW(BBSKernelException, "Baseline selection is empty.");
    }

    // Create Source objects for all selected sources.
    vector<Source::Ptr> sources = makeSourceList(config.getSources());
    if(sources.empty())
    {
        THROW(BBSKernelException, "No sources matching selection found in"
            " source database.");
    }

    LOG_INFO_STR("Number of sources in the model: " << sources.size());

    // Create a coherence expression for all point sources.
    map<unsigned int, Expr<JonesMatrix>::Ptr> exprPointCoherence;
    for(unsigned int i = 0; i < sources.size(); ++i)
    {
        if(typeid(*sources[i]) == typeid(PointSource))
        {
            // A point source's coherence is independent of baseline UVW
            // coordinates. Therefore, they are constructed here and shared
            // between baselines.
            PointSource::ConstPtr source =
                static_pointer_cast<PointSource::ConstPtr::element_type>
                    (sources[i]);

            Expr<JonesMatrix>::Ptr coherence(new PointCoherence(source));
            exprPointCoherence[i] = coherence;
        }
    }

    // Direction independent effects.
    const bool haveIndependentEffects = config.useBandpass()
        || config.useGain();

    // Create a bandpass expression per station.
    casa::Vector<Expr<JonesMatrix>::Ptr> exprBandpass;
    if(config.useBandpass())
    {
        exprBandpass = makeBandpassExpr(stations);
    }

    // Create a direction independent gain expression per station.
    casa::Vector<Expr<JonesMatrix>::Ptr> exprGain;
    if(config.useGain())
    {
        exprGain = makeGainExpr(config, stations);
    }

    // Create a single Expr<JonesMatrix> per station that is the composition of
    // all direction independent effects.
    casa::Vector<Expr<JonesMatrix>::Ptr> indepTransform;
    if(haveIndependentEffects)
    {
        indepTransform.resize(stations.size());
        for(size_t i = 0; i < stations.size(); ++i)
        {
            if(config.useBandpass())
            {
                indepTransform(i) = corrupt(indepTransform(i), exprBandpass(i));
            }

            if(config.useGain())
            {
                indepTransform(i) = corrupt(indepTransform(i), exprGain(i));
            }

            // TODO: Add other direction independent effects here.
        }
    }

    // Direction dependent effects.
    const bool haveDependentEffects = config.useDirectionalGain()
        || config.useBeam() || config.useIonosphere();

    // Create an UVW expression per station.
    casa::Vector<Expr<Vector<3> >::Ptr> exprUVW = makeUVWExpr(stations);

    // Create a station shift expression per (station, source) combination.
    casa::Matrix<Expr<Vector<2> >::Ptr> exprStationShift =
        makeStationShiftExpr(stations, sources, exprUVW);

    // Create a direction dependent gain expression per (station, source)
    // combination.
    casa::Matrix<Expr<JonesMatrix>::Ptr> exprDirectionalGain;
    if(config.useDirectionalGain())
    {
        exprDirectionalGain =
            makeDirectionalGainExpr(config, stations, sources);
    }

    // Create an AzEl node per (station, source) combination.
    casa::Matrix<Expr<Vector<2> >::Ptr> exprAzEl;
    if(config.useBeam() || config.useIonosphere())
    {
        exprAzEl = makeAzElExpr(stations, sources);
    }

    // Create a beam expression per (station, source) combination.
    casa::Matrix<Expr<JonesMatrix>::Ptr> exprBeam;
    if(config.useBeam())
    {
        exprBeam = makeBeamExpr(config.getBeamConfig(), stations, exprAzEl);
    }

    // Create a PolynomialPhaseScreen expression per (station, source)
    // combination.
    casa::Matrix<Expr<JonesMatrix>::Ptr> exprIonosphere;
    if(config.useIonosphere())
    {
        exprIonosphere = makeIonosphereExpr(config.getIonosphereConfig(),
            stations, exprAzEl);
    }

    // Create a single Expr<JonesMatrix> per (station, source) combination that
    // is the composition of all direction dependent effects in the direction of
    // the source.
    casa::Matrix<Expr<JonesMatrix>::Ptr> depTransform;
    if(haveDependentEffects)
    {
        depTransform.resize(sources.size(), stations.size());

        for(size_t i = 0; i < stations.size(); ++i)
        {
            for(size_t j = 0; j < sources.size(); ++j)
            {
                if(config.useDirectionalGain())
                {
                    depTransform(j, i) = corrupt(depTransform(j, i),
                        exprDirectionalGain(j, i));
                }

                if(config.useBeam())
                {
                    depTransform(j, i) = corrupt(depTransform(j, i),
                        exprBeam(j, i));
                }

                if(config.useIonosphere())
                {
                    depTransform(j, i) = corrupt(depTransform(j, i),
                        exprIonosphere(j, i));
                }

                // TODO: Add other direction dependent effects here.
            }
        }
    }

    // Create an expression tree for each baseline.
    itsExpr.reserve(baselines.size());
    for(size_t i = 0; i < baselines.size(); ++i)
    {
        const baseline_t &baseline = baselines[i];

        // Find left and right hand side station index.
        vector<unsigned int>::const_iterator it;

        it = find(stations.begin(), stations.end(), baseline.first);
        unsigned int lhs = distance(stations.begin(), it);

        it = find(stations.begin(), stations.end(), baseline.second);
        unsigned int rhs = distance(stations.begin(), it);

        ASSERT(lhs < stations.size() && rhs < stations.size());

        // Make baseline expression.
        MatrixSum::Ptr sum(new MatrixSum());

        for(size_t j = 0; j < sources.size(); ++j)
        {
            Expr<JonesMatrix>::Ptr term;

            if(typeid(*sources[j]) == typeid(PointSource))
            {
                term = exprPointCoherence[j];
            }
            else if(typeid(*sources[j]) == typeid(GaussianSource))
            {
                GaussianSource::ConstPtr source =
                    static_pointer_cast<GaussianSource::ConstPtr::element_type>
                        (sources[j]);

                term = Expr<JonesMatrix>::Ptr(new GaussianCoherence(source,
                    exprUVW(lhs), exprUVW(rhs)));

//                term = Expr<JonesMatrix>::Ptr(new GaussianCoherence(source,
//                    itsStationUVW[baseline.first],
//                    itsStationUVW[baseline.second]));
            }
            else
            {
                THROW(BBSKernelException, "Unknown source type encountered");
            }

            // Phase shift (incorporates geometry and fringe stopping).
            PhaseShift::Ptr shift(new PhaseShift(exprStationShift(j, lhs),
                exprStationShift(j, rhs)));

            // Phase shift the source coherence.
            term = Expr<JonesMatrix>::Ptr(new ScalarMatrixMul(shift, term));

            // Apply direction dependent effects.
            if(haveDependentEffects)
            {
                term = Expr<JonesMatrix>::Ptr(new MatrixMul3(depTransform(j,
                    lhs), term, depTransform(j, rhs)));
            }

            sum->connect(term);
        }

        Expr<JonesMatrix>::Ptr coherence(sum);

        // Apply direction independent effects.
        if(haveIndependentEffects)
        {
            coherence =
                Expr<JonesMatrix>::Ptr(new MatrixMul3(indepTransform(lhs),
                    coherence, indepTransform(rhs)));
        }

        itsExpr.push_back(coherence);
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

    // Create a bandpass expression per station.
    casa::Vector<Expr<JonesMatrix>::Ptr> exprBandpass;
    if(config.useBandpass())
    {
        exprBandpass = makeBandpassExpr(stations);
    }

    // Create a direction independent gain expression per station.
    casa::Vector<Expr<JonesMatrix>::Ptr> exprGain;
    if(config.useGain())
    {
        exprGain = makeGainExpr(config, stations);
    }

    // Create a single Expr<JonesMatrix> per station that is the composition of
    // all direction independent effects.
    if(haveIndependentEffects)
    {
        for(size_t i = 0; i < stations.size(); ++i)
        {
            if(config.useBandpass())
            {
                transform(i) = corrupt(transform(i), exprBandpass(i));
            }

            if(config.useGain())
            {
                transform(i) = corrupt(transform(i), exprGain(i));
            }

            // TODO: Add other direction independent effects here.
        }
    }

    // Direction dependent effects.
    const bool haveDependentEffects = config.useDirectionalGain()
        || config.useBeam() || config.useIonosphere();

    if(haveDependentEffects)
    {
        // Create Source objects for all selected sources.
        vector<Source::Ptr> sources = makeSourceList(config.getSources());
        if(sources.size() != 1)
        {
            THROW(BBSKernelException, "No source, or more than one source"
            " selected, yet corrections can only be applied for a single"
            " direction on the sky");
        }

        // Create a direction dependent gain expression per (station, source)
        // combination.
        casa::Matrix<Expr<JonesMatrix>::Ptr> exprDirectionalGain;
        if(config.useDirectionalGain())
        {
            exprDirectionalGain =
                makeDirectionalGainExpr(config, stations, sources);
        }

        // Create an AzEl node per (station, source) combination.
        casa::Matrix<Expr<Vector<2> >::Ptr> exprAzEl;
        if(config.useBeam() || config.useIonosphere())
        {
            exprAzEl = makeAzElExpr(stations, sources);
        }

        // Create a beam expression per (station, source) combination.
        casa::Matrix<Expr<JonesMatrix>::Ptr> exprBeam;
        if(config.useBeam())
        {
            exprBeam = makeBeamExpr(config.getBeamConfig(), stations, exprAzEl);
        }

        // Create a PolynomialPhaseScreen expression per (station, source)
        // combination.
        casa::Matrix<Expr<JonesMatrix>::Ptr> exprIonosphere;
        if(config.useIonosphere())
        {
            exprIonosphere = makeIonosphereExpr(config.getIonosphereConfig(),
                stations, exprAzEl);
        }

        // Create a single Expr<JonesMatrix> per (station, source) combination
        // that is the composition of all direction dependent effects in the
        // direction of the source.
        for(size_t i = 0; i < stations.size(); ++i)
        {
            if(config.useDirectionalGain())
            {
                transform(i) = corrupt(transform(i), exprDirectionalGain(0, i));
            }

            if(config.useBeam())
            {
                transform(i) = corrupt(transform(i), exprBeam(0, i));
            }

            if(config.useIonosphere())
            {
                transform(i) = corrupt(transform(i), exprIonosphere(0, i));
            }

            // TODO: Add other direction dependent effects here.
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

    // Create an expression tree for each baseline.
    itsExpr.reserve(baselines.size());
    for(size_t i = 0; i < baselines.size(); ++i)
    {
        const baseline_t &baseline = baselines[i];

        // Find left and right hand side station index.
        vector<unsigned int>::const_iterator it;

        it = find(stations.begin(), stations.end(), baseline.first);
        unsigned int lhs = distance(stations.begin(), it);

        it = find(stations.begin(), stations.end(), baseline.second);
        unsigned int rhs = distance(stations.begin(), it);

        ASSERT(lhs < stations.size() && rhs < stations.size());

//        Expr<JonesMatrix>::Ptr exprVisData(new JonesVisData(chunk, baseline));
        Expr<JonesMatrix>::Ptr exprVisData(new ExprVisData(chunk, baseline));

        if(haveIndependentEffects || haveDependentEffects)
        {
            exprVisData = Expr<JonesMatrix>::Ptr(new MatrixMul3(transform(lhs),
                exprVisData, transform(rhs)));
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

    map<unsigned int, ExprParm::Ptr>::const_iterator parmIt = itsParms.begin();
    while(parmIt != itsParms.end())
    {
        result.insert(parmIt->first);
        ++parmIt;
    }

    return result;
}

ParmGroup Model::getSolvableParms() const
{
    ParmGroup result;

    map<unsigned int, ExprParm::Ptr>::const_iterator parmIt = itsParms.begin();
    while(parmIt != itsParms.end())
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
        map<unsigned int, ExprParm::Ptr>::iterator parmIt =
            itsParms.find(*solIt);
        ASSERT(parmIt != itsParms.end());

        if(parmIt != itsParms.end())
        {
            parmIt->second->setPValueFlag();
        }

        ++solIt;
    }
}

void Model::clearSolvableParms()
{
    map<unsigned int, ExprParm::Ptr>::iterator parmIt = itsParms.begin();
    while(parmIt != itsParms.end())
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

ExprParm::Ptr Model::makeExprParm(unsigned int category, const string &name)
{
    ParmProxy::ConstPtr proxy(ParmManager::instance().get(category, name));

    pair<map<unsigned int, ExprParm::Ptr>::const_iterator, bool> status =
        itsParms.insert(make_pair(proxy->getId(),
            ExprParm::Ptr(new ExprParm(proxy))));

    return status.first->second;
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

Expr<JonesMatrix>::Ptr Model::corrupt(Expr<JonesMatrix>::Ptr &accumulator,
    Expr<JonesMatrix>::Ptr &effect)
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

vector<Source::Ptr> Model::makeSourceList(const vector<string> &patterns)
{
    vector<Source::Ptr> result;

    if(patterns.empty())
    {
        // Create a Source object for all available sources.
        vector<SourceInfo> sources(itsSourceDb.getSources("*"));
        for(size_t i = 0; i < sources.size(); ++i)
        {
            Source::Ptr source(makeSource(sources[i]));
            if(source)
            {
                result.push_back(source);
            }
        }
    }
    else
    {
        // Create a list of all unique sources that match the given patterns.
        map<string, SourceInfo> sources;

        for(size_t i = 0; i < patterns.size(); ++i)
        {
            vector<SourceInfo> tmp(itsSourceDb.getSources(patterns[i]));
            for(size_t j = 0; j < tmp.size(); ++j)
            {
                sources.insert(make_pair(tmp[j].getName(), tmp[j]));
            }
        }

        // Create a Source object for each unique source.
        map<string, SourceInfo>::const_iterator srcIt = sources.begin();
        map<string, SourceInfo>::const_iterator srcItEnd = sources.end();
        while(srcIt != srcItEnd)
        {
            Source::Ptr source = makeSource(srcIt->second);
            if(source)
            {
                result.push_back(source);
            }

            ++srcIt;
        }
    }

    return result;
}

Source::Ptr Model::makeSource(const SourceInfo &source)
{
    switch(source.getType())
    {
    case SourceInfo::POINT:
        {
            ExprParm::Ptr ra = makeExprParm(SKY, "Ra:" + source.getName());
            ExprParm::Ptr dec = makeExprParm(SKY, "Dec:" + source.getName());
            Expr<Scalar>::Ptr stokesI = makeSpectralIndexExpr(source, "I");
            ExprParm::Ptr stokesQ = makeExprParm(SKY, "Q:" + source.getName());
            ExprParm::Ptr stokesU = makeExprParm(SKY, "U:" + source.getName());
            ExprParm::Ptr stokesV = makeExprParm(SKY, "V:" + source.getName());

            AsExpr<Vector<2> >::Ptr position(new AsExpr<Vector<2> >());
            position->connect(0, ra);
            position->connect(1, dec);

            AsExpr<Vector<4> >::Ptr stokes(new AsExpr<Vector<4> >());
            stokes->connect(0, stokesI);
            stokes->connect(1, stokesQ);
            stokes->connect(2, stokesU);
            stokes->connect(3, stokesV);

            return PointSource::Ptr(new PointSource(source.getName(), position,
                stokes));
        }
        break;

    case SourceInfo::GAUSSIAN:
        {
            ExprParm::Ptr ra = makeExprParm(SKY, "Ra:" + source.getName());
            ExprParm::Ptr dec = makeExprParm(SKY, "Dec:" + source.getName());
            Expr<Scalar>::Ptr stokesI = makeSpectralIndexExpr(source, "I");
            ExprParm::Ptr stokesQ = makeExprParm(SKY, "Q:" + source.getName());
            ExprParm::Ptr stokesU = makeExprParm(SKY, "U:" + source.getName());
            ExprParm::Ptr stokesV = makeExprParm(SKY, "V:" + source.getName());
            ExprParm::Ptr major =
                makeExprParm(SKY, "MajorAxis:" + source.getName());
            ExprParm::Ptr minor =
                makeExprParm(SKY, "MinorAxis:" + source.getName());
            ExprParm::Ptr orientation =
                makeExprParm(SKY, "Orientation:" + source.getName());

            AsExpr<Vector<2> >::Ptr position(new AsExpr<Vector<2> >());
            position->connect(0, ra);
            position->connect(1, dec);

            AsExpr<Vector<4> >::Ptr stokes(new AsExpr<Vector<4> >());
            stokes->connect(0, stokesI);
            stokes->connect(1, stokesQ);
            stokes->connect(2, stokesU);
            stokes->connect(3, stokesV);

            AsExpr<Vector<2> >::Ptr dimensions(new AsExpr<Vector<2> >());
            dimensions->connect(0, major);
            dimensions->connect(1, minor);

            return GaussianSource::Ptr(new GaussianSource(source.getName(),
                position, stokes, dimensions, orientation));
        }
        break;

    default:
        LOG_WARN_STR("Unable to construct source: " << source.getName());
        break;
    }

    return Source::Ptr();
}

Expr<Scalar>::Ptr Model::makeSpectralIndexExpr(const SourceInfo &source,
    const string &stokesParm)
{
    unsigned int degree =
        static_cast<unsigned int>(ParmManager::instance().getDefaultValue(SKY,
            "SpectralIndexDegree:" + source.getName()));

    // Reference frequency.
    ExprParm::Ptr refFreq =
        makeExprParm(SKY, "ReferenceFrequency:" + source.getName());
    // Stokes parameter value at the reference frequency.
    ExprParm::Ptr refStokes =
        makeExprParm(SKY, stokesParm + ":" + source.getName());

    vector<Expr<Scalar>::Ptr> coeff;
    coeff.reserve(degree + 1);
    for(unsigned int i = 0; i <= degree; ++i)
    {
        ostringstream name;
        name << "SpectralIndex:" << i << ":" << source.getName();
        coeff.push_back(makeExprParm(SKY, name.str()));
    }

    return Expr<Scalar>::Ptr(new SpectralIndex(refFreq, refStokes,
        coeff.begin(), coeff.end()));
}

//void Model::makeStationUVW()
//{
//    itsStationUVW.resize(itsInstrument.size());
//    for(size_t i = 0; i < itsStationUVW.size(); ++i)
//    {
//        itsStationUVW[i].reset(new StatUVW(itsInstrument[i].position(),
//            itsInstrument.position(), itsPhaseReference));
//    }
//}

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

casa::Matrix<Expr<Vector<2> >::Ptr>
Model::makeStationShiftExpr(const vector<unsigned int> &stations,
    const vector<Source::Ptr> &sources,
    const casa::Vector<Expr<Vector<3> >::Ptr> &exprUVW) const
{
    casa::Vector<LMN::Ptr> exprLMN(sources.size());
    for(unsigned int i = 0; i < sources.size(); ++i)
    {
        exprLMN(i) = LMN::Ptr(new LMN(itsPhaseReference,
            sources[i]->getPosition()));
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
//            // NOTE: itsStationUVW is indexed on station number, not used
//            // station index (hence the look-up through the provided used
//            // station list).
//            expr(j, i) =
//                Expr<Vector<2> >::Ptr(new StationShift(itsStationUVW[stations[i]],
//                    exprLMN(j)));
}

casa::Vector<Expr<JonesMatrix>::Ptr>
Model::makeBandpassExpr(const vector<unsigned int> &stations)
{
    casa::Vector<Expr<JonesMatrix>::Ptr> expr(stations.size());
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const unsigned int station = stations[i];
        const string &suffix = itsInstrument[station].name();

        Expr<Scalar>::Ptr B00 = makeExprParm(INSTRUMENT, "Bandpass:0:0:"
            + suffix);
        Expr<Scalar>::Ptr B11 = makeExprParm(INSTRUMENT, "Bandpass:1:1:"
            + suffix);

        expr(i) = Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(B00, B11));
    }

    return expr;
}

casa::Vector<Expr<JonesMatrix>::Ptr>
Model::makeGainExpr(const ModelConfig &config,
    const vector<unsigned int> &stations)
{
    string elem0(config.usePhasors() ? "Ampl"  : "Real");
    string elem1(config.usePhasors() ? "Phase" : "Imag");

    casa::Vector<Expr<JonesMatrix>::Ptr> expr(stations.size());
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        Expr<Scalar>::Ptr J00, J01, J10, J11;

        const unsigned int station = stations[i];
        const string &suffix = itsInstrument[station].name();

        ExprParm::Ptr J00_elem0 =
            makeExprParm(INSTRUMENT, "Gain:0:0:" + elem0 + ":" + suffix);
        ExprParm::Ptr J00_elem1 =
            makeExprParm(INSTRUMENT, "Gain:0:0:" + elem1 + ":" + suffix);
        ExprParm::Ptr J01_elem0 =
            makeExprParm(INSTRUMENT, "Gain:0:1:" + elem0 + ":" + suffix);
        ExprParm::Ptr J01_elem1 =
            makeExprParm(INSTRUMENT, "Gain:0:1:" + elem1 + ":" + suffix);
        ExprParm::Ptr J10_elem0 =
            makeExprParm(INSTRUMENT, "Gain:1:0:" + elem0 + ":" + suffix);
        ExprParm::Ptr J10_elem1 =
            makeExprParm(INSTRUMENT, "Gain:1:0:" + elem1 + ":" + suffix);
        ExprParm::Ptr J11_elem0 =
            makeExprParm(INSTRUMENT, "Gain:1:1:" + elem0 + ":" + suffix);
        ExprParm::Ptr J11_elem1 =
            makeExprParm(INSTRUMENT, "Gain:1:1:" + elem1 + ":" + suffix);

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

        expr(i) = Expr<JonesMatrix>::Ptr(new AsExpr<JonesMatrix>(J00, J01, J10,
            J11));
    }

    return expr;
}

casa::Matrix<Expr<JonesMatrix>::Ptr>
Model::makeDirectionalGainExpr(const ModelConfig &config,
    const vector<unsigned int> &stations, const vector<Source::Ptr> &sources)
{
    string elem0(config.usePhasors() ? "Ampl"  : "Real");
    string elem1(config.usePhasors() ? "Phase" : "Imag");

    casa::Matrix<Expr<JonesMatrix>::Ptr> expr(sources.size(), stations.size());
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const unsigned int station = stations[i];
        for(unsigned int j = 0; j < sources.size(); ++j)
        {
            Expr<Scalar>::Ptr J00, J01, J10, J11;

            string suffix(itsInstrument[station].name() + ":"
                + sources[j]->getName());

            ExprParm::Ptr J00_elem0 = makeExprParm(INSTRUMENT,
                "DirectionalGain:0:0:" + elem0 + ":" + suffix);
            ExprParm::Ptr J00_elem1 = makeExprParm(INSTRUMENT,
                "DirectionalGain:0:0:" + elem1 + ":" + suffix);
            ExprParm::Ptr J01_elem0 = makeExprParm(INSTRUMENT,
                "DirectionalGain:0:1:" + elem0 + ":" + suffix);
            ExprParm::Ptr J01_elem1 = makeExprParm(INSTRUMENT,
                "DirectionalGain:0:1:" + elem1 + ":" + suffix);
            ExprParm::Ptr J10_elem0 = makeExprParm(INSTRUMENT,
                "DirectionalGain:1:0:" + elem0 + ":" + suffix);
            ExprParm::Ptr J10_elem1 = makeExprParm(INSTRUMENT,
                "DirectionalGain:1:0:" + elem1 + ":" + suffix);
            ExprParm::Ptr J11_elem0 = makeExprParm(INSTRUMENT,
                "DirectionalGain:1:1:" + elem0 + ":" + suffix);
            ExprParm::Ptr J11_elem1 = makeExprParm(INSTRUMENT,
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

            expr(j, i) = Expr<JonesMatrix>::Ptr(new AsExpr<JonesMatrix>(J00,
                J01, J10, J11));
        }
    }

    return expr;
}

casa::Matrix<Expr<Vector<2> >::Ptr>
Model::makeAzElExpr(const vector<unsigned int> &stations,
    const vector<Source::Ptr> &sources) const
{
    // Create an AzEl expression for each (station, source) combination.
    casa::Matrix<Expr<Vector<2> >::Ptr> expr(sources.size(), stations.size());
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const unsigned int station = stations[i];
        const casa::MPosition &position = itsInstrument[station].position();

        for(unsigned int j = 0; j < sources.size(); ++j)
        {
            expr(j, i) = Expr<Vector<2> >::Ptr(new AzEl(position,
                sources[j]->getPosition()));
        }
    }

    return expr;
}

//casa::Matrix<Expr<JonesMatrix>::Ptr>
//Model::makeDipoleBeamExpr(const ModelConfig &config,
//    const vector<unsigned int> &stations,
//    const casa::Matrix<Expr<Vector<2> >::Ptr> &exprAzEl)
//{
//    ASSERT(config.useBeam() && config.getBeamType()
//        != ModelConfig::UNKNOWN_BEAM_TYPE);

//    const unsigned int nSources = exprAzEl.shape()(0);
//    const unsigned int nStations = exprAzEl.shape()(1);
//    ASSERT(nStations == stations.size());

//    // Allocate result.
//    casa::Matrix<Expr<JonesMatrix>::Ptr> expr(nSources, nStations);

//    switch(config.getBeamType())
//    {
//        case ModelConfig::HAMAKER_DIPOLE:
//        {
//            HamakerDipoleConfig beamConfig;
//            config.getBeamConfig(beamConfig);

//            // Read beam coefficients from file.
//            HamakerBeamCoeff coeff;
//            coeff.init(beamConfig.getCoeffFile());

//            // Create a dipole beam expression per (station, source)
//            // combination.
//            for(unsigned int i = 0; i < nStations; ++i)
//            {
//                // Get dipole orientation.
//                const unsigned int station = stations[i];
//                Expr<Scalar>::Ptr orientation = makeExprParm(INSTRUMENT,
//                    "Orientation:" + itsInstrument[station].name());

//                for(unsigned int j = 0; j < nSources; ++j)
//                {
//                    expr(j, i) = Expr<JonesMatrix>::Ptr(new HamakerDipole(coeff,
//                        exprAzEl(j, i), orientation));
//                }
//            }
//            break;
//        }

//        case ModelConfig::YATAWATTA_DIPOLE:
//        {
//            YatawattaDipoleConfig beamConfig;
//            config.getBeamConfig(beamConfig);

//            // TODO: Where is this scale factor coming from (see global_model.py
//            // in EJones_droopy_comp and EJones_HBA)?
////            const double scaleFactor = options[DIPOLE_BEAM_LBA] ? 1.0 / 88.0
////                : 1.0 / 600.0;

//            // Create a dipole beam expression per (station, source)
//            // combination.
//            for(unsigned int i = 0; i < nStations; ++i)
//            {
//                // Get dipole orientation.
//                const unsigned int station = stations[i];
//                Expr<Scalar>::Ptr orientation = makeExprParm(INSTRUMENT,
//                    "Orientation:" + itsInstrument[station].name());

//                for(unsigned int j = 0; j < nSources; ++j)
//                {
//                    expr(j, i) = Expr<JonesMatrix>::Ptr
//                        (new YatawattaDipole(beamConfig.getModuleTheta(),
//                            beamConfig.getModulePhi(), 1.0, exprAzEl(j, i),
//                            orientation));
//                }
//            }
//            break;
//        }

//        default:
//            THROW(BBSKernelException, "Unsupported beam type encountered.");
//    }

//    return expr;
//}

casa::Matrix<Expr<JonesMatrix>::Ptr>
Model::makeBeamExpr(const BeamConfig &config,
    const vector<unsigned int> &stations,
    const casa::Matrix<Expr<Vector<2> >::Ptr> &exprAzEl)
{
    ASSERT(config.getElementType() != BeamConfig::UNKNOWN);

    const unsigned int nDirections = exprAzEl.shape()(0);
    const unsigned int nStations = exprAzEl.shape()(1);
    ASSERT(nStations == stations.size());

    LOG_INFO_STR("Using element type: "
        << config.getElementTypeAsString());

    // Create element beam expressions.
    casa::Matrix<Expr<JonesMatrix>::Ptr> exprElement(nDirections, nStations);
    switch(config.getElementType())
    {
        case BeamConfig::HAMAKER_LBA:
        case BeamConfig::HAMAKER_HBA:
        {
            casa::Path coeffFile = config.getElementPath();
            coeffFile.append("element_beam_" + config.getElementTypeAsString()
                + ".coeff");
            LOG_INFO_STR("Element beam config file: "
                << coeffFile.expandedName());

            // Read beam coefficients from file.
            HamakerBeamCoeff coeff;
            coeff.init(coeffFile);

            // Create an element beam expression per (station, source)
            // combination.
            for(unsigned int i = 0; i < nStations; ++i)
            {
                // Get element orientation.
                const unsigned int station = stations[i];
                Expr<Scalar>::Ptr orientation = makeExprParm(INSTRUMENT,
                    "AntennaOrientation:" + itsInstrument[station].name());

                for(unsigned int j = 0; j < nDirections; ++j)
                {
                    exprElement(j, i) =
                        Expr<JonesMatrix>::Ptr(new HamakerDipole(coeff,
                            exprAzEl(j, i), orientation));
                }
            }
            break;
        }

        case BeamConfig::YATAWATTA_LBA:
        case BeamConfig::YATAWATTA_HBA:
        {
            // TODO: Transparantly handle platforms that use a different
            // extension for loadable modules.
            casa::Path moduleTheta = config.getElementPath();
            moduleTheta.append("element_beam_" + config.getElementTypeAsString()
                + "_theta.so");

            casa::Path modulePhi = config.getElementPath();
            modulePhi.append("element_beam_" + config.getElementTypeAsString()
                + "_phi.so");

            LOG_INFO_STR("Element beam loadable modules: ["
                << moduleTheta.expandedName() << ","
                << modulePhi.expandedName() << "]");

            // Create an element beam expression per (station, source)
            // combination.
            for(unsigned int i = 0; i < nStations; ++i)
            {
                // Get element orientation.
                const unsigned int station = stations[i];
                Expr<Scalar>::Ptr orientation = makeExprParm(INSTRUMENT,
                    "AntennaOrientation:" + itsInstrument[station].name());

                for(unsigned int j = 0; j < nDirections; ++j)
                {
                    exprElement(j, i) =
                        Expr<JonesMatrix>::Ptr(new YatawattaDipole(moduleTheta,
                            modulePhi, exprAzEl(j, i), orientation));
                }
            }
            break;
        }

        default:
            THROW(BBSKernelException, "Unsupported element type encountered");
    }

    itsInstrument.readAntennaConfigurations(config.getConfigPath());

    casa::MDirection refJ2000(casa::MDirection::Convert(itsPhaseReference,
        casa::MDirection::J2000)());
    casa::Quantum<casa::Vector<casa::Double> > refAngles =
        refJ2000.getAngle();
    Literal::Ptr refRa(new Literal(refAngles.getBaseValue()(0)));
    Literal::Ptr refDec(new Literal(refAngles.getBaseValue()(1)));

    AsExpr<Vector<2> >::Ptr exprRefDir(new AsExpr<Vector<2> >());
    exprRefDir->connect(0, refRa);
    exprRefDir->connect(1, refDec);

    casa::Matrix<Expr<JonesMatrix>::Ptr> expr(nDirections, nStations);
    for(unsigned int i = 0; i < nStations; ++i)
    {
        const unsigned int station = stations[i];
        const AntennaConfig &antennaConfig =
            itsInstrument[station].config(config.getConfigName());

        Expr<Vector<2> >::Ptr
            refAzEl(new AzEl(itsInstrument[station].position(), exprRefDir));

        for(unsigned int j = 0; j < nDirections; ++j)
        {
            Expr<JonesMatrix>::Ptr exprArrayFactor(new ArrayFactor(exprAzEl(j, i),
                refAzEl, antennaConfig, itsReferenceFreq));

            expr(j, i) = Expr<JonesMatrix>::Ptr(new MatrixMul2(exprArrayFactor,
                exprElement(j, i)));
        }
    }

    return expr;
}

casa::Matrix<Expr<JonesMatrix>::Ptr>
Model::makeIonosphereExpr(const IonosphereConfig &config,
    const vector<unsigned int> &stations,
    const casa::Matrix<Expr<Vector<2> >::Ptr> &exprAzEl)
{
    // Use station 0 as reference position on earth.
    // TODO: Is station 0 a sensible choice for the reference position? Should
    // we use the array reference position instead? Or should it be different
    // for each station?
    casa::MPosition reference = itsInstrument[0].position();

    // Get ionosphere model parameters.
    unsigned int degree = config.getDegree() + 1;

    // Make sure rank is at least 1 (i.e. a linear gradient over the field of
    // view).
    if(degree <= 1)
    {
        THROW(BBSKernelException, "Ionosphere model degree should be at least 1"
            " (linear gradient)");
    }

    unsigned int nParms = degree * degree - 1;
    vector<Expr<Scalar>::Ptr> MIMParms(nParms);
    for(unsigned int i = 0; i < degree; ++i)
    {
        for(unsigned int j = 0; j < degree; ++j)
        {
            // For the moment we do not include MIM:0:0 (absolute TEC).
            if(i == 0 && j == 0)
            {
                continue;
            }

            ostringstream oss;
            // TODO: Shouldn't j and i be interchanged here?
            oss << "MIM:" << j << ":" << i;
            MIMParms[i * degree + j - 1] = makeExprParm(INSTRUMENT, oss.str());
        }
    }

    // Create a PolynomialPhaseScreen expression per (station, source)
    // combination.
    unsigned int nSources = exprAzEl.shape()(0);
    unsigned int nStations = exprAzEl.shape()(1);
    ASSERT(nStations == stations.size());

    casa::Matrix<Expr<JonesMatrix>::Ptr> expr(nSources, nStations);
    for(size_t i = 0; i < nStations; ++i)
    {
        const unsigned int station = stations[i];
        const casa::MPosition &position = itsInstrument[station].position();

        for(size_t j = 0; j < nSources; ++j)
        {
            PiercePoint::Ptr pp(new PiercePoint(position, exprAzEl(j, i)));
            PolynomialPhaseScreen::Ptr phi(new PolynomialPhaseScreen(reference,
                pp, MIMParms.begin(), MIMParms.end()));
            expr(j, i) = AsDiagonalMatrix::Ptr(new AsDiagonalMatrix(phi, phi));
        }
    }

    return expr;
}


} // namespace BBS
} // namespace LOFAR
