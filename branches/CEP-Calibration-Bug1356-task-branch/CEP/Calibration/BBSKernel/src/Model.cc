//# Model.cc:
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <BBSKernel/Model.h>

#include <BBSKernel/Exceptions.h>
#include <BBSKernel/ModelConfig.h>
#include <BBSKernel/Measurement.h>

#include <BBSKernel/Expr/StatUVW.h>
#include <BBSKernel/Expr/StationUVW.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/PointSource.h>
#include <BBSKernel/Expr/MergeFlags.h>
#include <BBSKernel/Expr/Mul.h>
#include <BBSKernel/Expr/MatrixMul3.h>
#include <BBSKernel/Expr/MatrixMul2.h>
#include <BBSKernel/Expr/MatrixSum.h>
#include <BBSKernel/Expr/PointCoherence.h>
//#include <BBSKernel/Expr/GaussianSource.h>
//#include <BBSKernel/Expr/GaussianCoherence.h>

//#include <BBSKernel/Expr/ExprParm.h>
#include <BBSKernel/Expr/AzEl.h>
#include <BBSKernel/Expr/PiercePoint.h>
#include <BBSKernel/Expr/MIM.h>
#include <BBSKernel/Expr/YatawattaDipole.h>
#include <BBSKernel/Expr/HamakerDipole.h>
#include <BBSKernel/Expr/PhaseRef.h>
//#include <BBSKernel/Expr/UVW.h>
#include <BBSKernel/Expr/SpectralIndex.h>
#include <BBSKernel/Expr/PhaseShift.h>
#include <BBSKernel/Expr/ConditionNumber.h>
#include <BBSKernel/Expr/Threshold.h>
//#include <BBSKernel/Expr/JonesMul.h>
//#include <BBSKernel/Expr/JonesMul2.h>
#include <BBSKernel/Expr/JonesVisData.h>
//#include <BBSKernel/Expr/JonesSum.h>

#include <BBSKernel/Expr/LMN.h>
#include <BBSKernel/Expr/DFTPS.h>
//#include <BBSKernel/Expr/Diag.h>
//#include <BBSKernel/Expr/JonesNode.h>
#include <BBSKernel/Expr/JonesInvert.h>
//#include <BBSKernel/Expr/JonesCMul3.h>
#include <BBSKernel/Expr/Request.h>
//#include <BBSKernel/Expr/JonesExpr.h>
#include <BBSKernel/Expr/SpectralIndex.h>

#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>
#include <Common/lofar_fstream.h>
#include <Common/lofar_sstream.h>
#include <Common/lofar_complex.h>
#include <Common/lofar_algorithm.h>
#include <Common/lofar_smartptr.h>

#include <measures/Measures/MDirection.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Arrays/Vector.h>

#include <iterator>

namespace LOFAR
{
namespace BBS
{

Model::Model(const Instrument &instrument, const SourceDB &sourceDb,
    const casa::MDirection &reference)
    :   itsInstrument(instrument),
        itsSourceDb(sourceDb),
        itsPhaseReference(reference)
{
//    // Make station UVW expression for all stations.
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

void Model::makeForwardExpr(const ModelConfig &config,
    const VisData::Ptr &chunk, const vector<baseline_t> &baselines)
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

    // Create a coherence expression for all point sources.
    casa::Vector<Expr<JonesMatrix>::Ptr> exprCoherence(sources.size());
    for(unsigned int i = 0; i < sources.size(); ++i)
    {
        PointSource::ConstPtr point =
            dynamic_pointer_cast<const PointSource>(sources[i]);

        exprCoherence(i) =
            PointCoherence::Ptr(new PointCoherence(point->getStokesVector(),
                point->getSpectralIndex()));
    }

    // Isotropic (direction independent) transform.
    const bool useIsotropicTransform = config.useBandpass()
        || config.useIsotropicGain();

    // Create a bandpass expression per station.
    casa::Vector<Expr<JonesMatrix>::Ptr> exprBandpass;
    if(config.useBandpass())
    {
        exprBandpass = makeBandpassExpr(stations);
    }

    // Create an isotropic gain expression per station.
    casa::Vector<Expr<JonesMatrix>::Ptr> exprIsotropicGain;
    if(config.useIsotropicGain())
    {
        exprIsotropicGain = makeIsotropicGainExpr(config, stations);
    }

    // Create a single Expr<JonesMatrix> per station that is the composition of
    // all isotropic effects.
    casa::Vector<Expr<JonesMatrix>::Ptr> isoTransform;
    if(useIsotropicTransform)
    {
        isoTransform.resize(stations.size());
        for(size_t i = 0; i < stations.size(); ++i)
        {
            if(config.useBandpass())
            {
                isoTransform(i) = corrupt(isoTransform(i), exprBandpass(i));
            }

            if(config.useIsotropicGain())
            {
                isoTransform(i) =
                    corrupt(isoTransform(i), exprIsotropicGain(i));
            }

            // TODO: Add other isotropic effects here.
        }
    }

    // Anisotropic (direction dependent) effects.
    const bool useAnisotropicTransform = config.useAnisotropicGain()
        || config.useBeam() || config.useIonosphere();

    // Create a UVW expression per station.
    casa::Vector<Expr<Vector<3> >::Ptr> exprStationUVW =
        makeStationUVWExpr(stations);

    // Create a station shift expression per (station, source) combination.
    casa::Matrix<Expr<Vector<2> >::Ptr> exprStationShift =
        makeStationShiftExpr(exprStationUVW, sources);

    // Create an anisotropic gain expression per (station, source) combination.
    casa::Matrix<Expr<JonesMatrix>::Ptr> exprAnisotropicGain;
    if(config.useAnisotropicGain())
    {
        exprAnisotropicGain =
            makeAnisotropicGainExpr(config, stations, sources);
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
        exprBeam = makeDipoleBeamExpr(config, stations, exprAzEl);
    }

    // Create a MIM expression per (station, source) combination.
    casa::Matrix<Expr<JonesMatrix>::Ptr> exprIonosphere;
    if(config.useIonosphere())
    {
        exprIonosphere = makeIonosphereNodes(config, stations, exprAzEl);
    }

    // Create a single Expr<JonesMatrix> per (station, source) combination that
    // is the composition of all anisotropic effects in the direction of the
    // source.
    casa::Matrix<Expr<JonesMatrix>::Ptr> anisoTransform;
    if(useAnisotropicTransform)
    {
        anisoTransform.resize(sources.size(), stations.size());

        for(size_t i = 0; i < stations.size(); ++i)
        {
            for(size_t j = 0; j < sources.size(); ++j)
            {
                if(config.useAnisotropicGain())
                {
                    anisoTransform(j, i) = corrupt(anisoTransform(j, i),
                        exprAnisotropicGain(j, i));
                }

                if(config.useBeam())
                {
                    anisoTransform(j, i) = corrupt(anisoTransform(j, i),
                        exprBeam(j, i));
                }

                if(config.useIonosphere())
                {
                    anisoTransform(j, i) = corrupt(anisoTransform(j, i),
                        exprIonosphere(j, i));
                }

                // TODO: Add other image-plane effects here.
            }
        }
    }

    // Create an expression tree for each baseline.
    for(size_t i = 0; i < baselines.size(); ++i)
    {
        const baseline_t &baseline = baselines[i];

        // Find left and right hand side station index.
        vector<unsigned int>::const_iterator it;

        it = find(stations.begin(), stations.end(), baseline.first);
        ASSERT(it != stations.end());
        unsigned int lhs = it - stations.begin();

        it = find(stations.begin(), stations.end(), baseline.second);
        ASSERT(it != stations.end());
        unsigned int rhs = it - stations.begin();

        // Make baseline expression.
        MatrixSum::Ptr sum(new MatrixSum());

        for(size_t j = 0; j < sources.size(); ++j)
        {
//            map<size_t, JonesExpr>::iterator pointCohIt = pointCoh.find(j);
//            if(pointCohIt != pointCoh.end())
//            {
//                coherence = pointCohIt->second;
//            }
//            else
//            {
//                GaussianSource::ConstPtr gauss =
//                    dynamic_pointer_cast<const GaussianSource>(sources[j]);
//                ASSERT(gauss);

//                coherence = new GaussianCoherence(gauss,
//                    itsStationUVW[baseline.first],
//                    itsStationUVW[baseline.second]);
//            }

            // Phase shift (incorporates geometry and fringe stopping).
            PhaseShiftOld::Ptr shift(new PhaseShiftOld(exprStationShift(j, lhs),
                exprStationShift(j, rhs)));

            // Phase shift the source coherence.
            Expr<JonesMatrix>::Ptr term(new Mul(shift, exprCoherence(j)));

            // Apply anisotropic (direction dependent) effects.
            if(useAnisotropicTransform)
            {
                term = Expr<JonesMatrix>::Ptr(new MatrixMul3(anisoTransform(j,
                    lhs), term, anisoTransform(j, rhs)));
            }

            sum->connect(term);
        }

        Expr<JonesMatrix>::Ptr coherence(sum);

        // Apply isotropic (direction independent) effects.
        if(useIsotropicTransform)
        {
            coherence = Expr<JonesMatrix>::Ptr(new MatrixMul3(isoTransform(lhs),
                coherence, isoTransform(rhs)));
        }

        itsExpr[baseline] = coherence;
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

    // Isotropic (direction independent) transform.
    const bool useIsotropicTransform = config.useBandpass()
        || config.useIsotropicGain();

    // Create a bandpass expression per station.
    casa::Vector<Expr<JonesMatrix>::Ptr> exprBandpass;
    if(config.useBandpass())
    {
        exprBandpass = makeBandpassExpr(stations);
    }

    // Create an isotropic gain expression per station.
    casa::Vector<Expr<JonesMatrix>::Ptr> exprIsotropicGain;
    if(config.useIsotropicGain())
    {
        exprIsotropicGain = makeIsotropicGainExpr(config, stations);
    }

    // Create a single Expr<JonesMatrix> per station that is the composition of
    // all isotropic effects.
    if(useIsotropicTransform)
    {
        for(unsigned int i = 0; i < stations.size(); ++i)
        {
            if(config.useBandpass())
            {
                transform(i) = corrupt(transform(i), exprBandpass(i));
            }

            if(config.useIsotropicGain())
            {
                transform(i) = corrupt(transform(i), exprIsotropicGain(i));
            }

            // TODO: Add other isotropic effects here.
        }
    }

    // Anisotropic (direction dependent) transform.
    const bool useAnisotropicTransform = config.useAnisotropicGain()
        || config.useBeam() || config.useIonosphere();

    if(useAnisotropicTransform)
    {
        // Create Source objects for all selected sources.
        vector<Source::Ptr> sources = makeSourceList(config.getSources());
        if(sources.size() != 1)
        {
            THROW(BBSKernelException, "No source, or more than one source"
            " selected, yet corrections can only be applied for a single"
            " direction on the sky");
        }

        // Create an anisotropic gain expression per (station, source)
        // combination.
        casa::Matrix<Expr<JonesMatrix>::Ptr> exprAnisotropicGain;
        if(config.useAnisotropicGain())
        {
            exprAnisotropicGain =
                makeAnisotropicGainExpr(config, stations, sources);
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
            exprBeam = makeDipoleBeamExpr(config, stations, exprAzEl);
        }

        // Create a MIM expression per (station, source) combination.
        casa::Matrix<Expr<JonesMatrix>::Ptr> exprIonosphere;
        if(config.useIonosphere())
        {
            exprIonosphere = makeIonosphereNodes(config, stations, exprAzEl);
        }

        // Create a single Expr<JonesMatrix> per (station, source) combination
        // that is the composition of all anisotropic effects in the direction
        // of the source.
        for(unsigned int i = 0; i < stations.size(); ++i)
        {
            if(config.useAnisotropicGain())
            {
                transform(i) = corrupt(transform(i), exprAnisotropicGain(0, i));
            }

            if(config.useBeam())
            {
                transform(i) = corrupt(transform(i), exprBeam(0, i));
            }

            if(config.useIonosphere())
            {
                transform(i) = corrupt(transform(i), exprIonosphere(0, i));
            }

            // TODO: Add other image-plane effects here.
        }
    }

    if(useIsotropicTransform || useAnisotropicTransform)
    {
        for(unsigned int i = 0; i < stations.size(); ++i)
        {
            if(config.useFlagger())
            {
                FlaggerConfig flagConfig;
                config.getFlaggerConfig(flagConfig);

                typedef Threshold<OpGreaterOrEqual> T_THRESHOLD;
                typedef MergeFlags<JonesMatrix, Scalar> T_MERGEFLAGS;

                Expr<Scalar>::Ptr exprCond(new ConditionNumber(transform(i)));
                Expr<Scalar>::Ptr exprThreshold(new T_THRESHOLD(exprCond,
                    flagConfig.getThreshold()));

                transform(i) =
                    Expr<JonesMatrix>::Ptr(new T_MERGEFLAGS(transform(i),
                        exprThreshold));
            }

            transform(i) =
                Expr<JonesMatrix>::Ptr(new JonesInvert(transform(i)));
        }
    }

    // Create an expression tree for each baseline.
    for(size_t i = 0; i < baselines.size(); ++i)
    {
        const baseline_t &baseline = baselines[i];

        // Find left and right hand side station index.
        vector<unsigned int>::const_iterator it;

        it = find(stations.begin(), stations.end(), baseline.first);
        ASSERT(it != stations.end());
        unsigned int lhs = it - stations.begin();

        it = find(stations.begin(), stations.end(), baseline.second);
        ASSERT(it != stations.end());
        unsigned int rhs = it - stations.begin();

        Expr<JonesMatrix>::Ptr exprVisData(new JonesVisData(chunk, baseline));

        if(useIsotropicTransform || useAnisotropicTransform)
        {
            exprVisData = Expr<JonesMatrix>::Ptr(new MatrixMul3(transform(lhs),
                exprVisData, transform(rhs)));
        }

        itsExpr[baseline] = exprVisData;
    }
}

void Model::setRequestGrid(const Grid &grid)
{
    itsRequest = Request(grid);
    LOG_DEBUG_STR("" << itsCache);
    itsCache.clearStats();
    itsCache.clear();

    // TODO: Set cache size in number of Matrix instances... ?
}

const JonesMatrix Model::evaluate(const baseline_t &baseline)
{
    map<baseline_t, Expr<JonesMatrix>::Ptr>::const_iterator exprIt =
        itsExpr.find(baseline);
    ASSERT(exprIt != itsExpr.end());

    const JonesMatrix model = (exprIt->second)->evaluate(itsRequest, itsCache);

    JonesMatrix result;
    result.setFlags(model.flags());
//    LOG_DEBUG_STR("BL: " << baseline.first << " - " << baseline.second
//        << " FLAGS: " << value.flags());

    const JonesMatrix::View value(model.view());
    result.assign(value);

    PValueKey key;
    JonesMatrix::Iterator it(model);

//    cout << "Computing partials:";
    while(!it.atEnd())
    {
        key = it.key();

//        cout << " " << key.parmId << "|" << key.coeffId;
//        cout << " " << solvables[i].parmId << "|" << solvables[i].coeffId;

        // Get the perturbed value associated with *solIt (the current
        // solvable).
//        const JonesMatrix::View pert = model.value(*solIt);
        const JonesMatrix::View pert(it.value(key));

        // Get perturbation.
//        map<unsigned int, ExprParm::Ptr>::const_iterator parmIt =
//            itsParms.find(it->parmId);
//        ASSERT(parmIt != itsParms.end());
//        ASSERT(parmIt->second);
//        const double inversePert =
//            1.0 / parmIt->second->getPerturbation(it->coeffId);

//        ParmProxy::ConstPtr parm =
//            ParmManager::instance().get(solIt->parmId);
//        const double inversePert =
//            1.0 / parm->getPerturbation(solIt->coeffId);
        ParmProxy::ConstPtr parm =
            ParmManager::instance().get(key.parmId);
        const double inversePert =
            1.0 / parm->getPerturbation(key.coeffId);

        // Approximate partial derivative by forward differences.
        JonesMatrix::View partial;
        partial.assign(0, 0, Matrix((pert(0, 0) - value(0, 0)) * inversePert));
        partial.assign(0, 1, Matrix((pert(0, 1) - value(0, 1)) * inversePert));
        partial.assign(1, 0, Matrix((pert(1, 0) - value(1, 0)) * inversePert));
        partial.assign(1, 1, Matrix((pert(1, 1) - value(1, 1)) * inversePert));

//        cout << "main: " << value(0, 0) << endl;
//        cout << "pert: " << pert(0, 0) << endl;
//        cout << "partial: " << partial(0, 0) << endl;

//        result.assign(*solIt, partial);
//        ++solIt;
        result.assign(key, partial);
        it.advance(key);
    }
//    cout << endl;

    return result;
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

ExprParm::Ptr Model::makeExprParm(uint category, const string &name)
{
    ParmProxy::ConstPtr proxy(ParmManager::instance().get(category, name));

    pair<map<uint, ExprParm::Ptr>::const_iterator, bool> status =
        itsParms.insert(make_pair(proxy->getId(),
            ExprParm::Ptr(new ExprParm(proxy))));

    return status.first->second;
}

//vector<Expr::Ptr> Model::makeUVWExpr(const VisData::Ptr &chunk,
//    const vector<baseline_t> &baselines)
//{
//    vector<Expr::Ptr> result;

//    vector<baseline_t>::const_iterator it = baselines.begin();
//    vector<baseline_t>::const_iterator itEnd = baselines.end();
//    while(it != itEnd)
//    {
//        result.push_back(Expr::Ptr(new UVW(chunk, *it)));
//        ++it;
//    }
//
//    return result;
//}

vector<unsigned int>
Model::makeUsedStationList(const vector<baseline_t> &baselines) const
{
    vector<bool> used(itsInstrument.stations.size(), false);

    vector<baseline_t>::const_iterator blIt = baselines.begin();
    while(blIt != baselines.end())
    {
        DBGASSERT(blIt->first < used.size() && blIt->second < used.size());
        used[blIt->first] = true;
        used[blIt->second] = true;
        ++blIt;
    }

    vector<unsigned int> list;
    for(unsigned int i = 0; i < used.size(); ++i)
    {
        if(used[i])
        {
            list.push_back(i);
        }
    }
//    int usedStation = 0;
//    for(unsigned int i = 0; i < itsInstrument.stations.size(); ++i)
//    {
//        index[i] = used[i] ? usedStation++ : -1;
//    }

    return list;
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
            ExprParm::Ptr stokesI = makeExprParm(SKY, "I:" + source.getName());
            ExprParm::Ptr stokesQ = makeExprParm(SKY, "Q:" + source.getName());
            ExprParm::Ptr stokesU = makeExprParm(SKY, "U:" + source.getName());
            ExprParm::Ptr stokesV = makeExprParm(SKY, "V:" + source.getName());
            Expr<Scalar>::Ptr spectral = makeSpectralIndexExpr(source);

            AsExpr<Vector<2> >::Ptr position(new AsExpr<Vector<2> >());
            position->connect(0, ra);
            position->connect(1, dec);

            AsExpr<Vector<4> >::Ptr stokes(new AsExpr<Vector<4> >());
            stokes->connect(0, stokesI);
            stokes->connect(1, stokesQ);
            stokes->connect(2, stokesU);
            stokes->connect(3, stokesV);

            return PointSource::Ptr(new PointSource(source.getName(), position,
                stokes, spectral));
        }
        break;

//    case SourceInfo::GAUSSIAN:
//        {
//            Expr ra = makeExprParm(SKY, "Ra:" + source.getName());
//            Expr dec = makeExprParm(SKY, "Dec:" + source.getName());
//            Expr i = makeExprParm(SKY, "I:" + source.getName());
//            Expr q = makeExprParm(SKY, "Q:" + source.getName());
//            Expr u = makeExprParm(SKY, "U:" + source.getName());
//            Expr v = makeExprParm(SKY, "V:" + source.getName());
//            Expr maj = makeExprParm(SKY, "Major:" + source.getName());
//            Expr min = makeExprParm(SKY, "Minor:" + source.getName());
//            Expr phi = makeExprParm(SKY, "Phi:" + source.getName());
//            Expr spectral = makeSpectralIndexExpr(source);

//            return GaussianSource::Ptr(new GaussianSource(source.getName(),
//                ra, dec, i, q, u, v, maj, min, phi));
//        }
//        break;

    default:
        LOG_WARN_STR("Unable to construct source: " << source.getName());
        break;
    }

    return Source::Ptr();
}

//void Model::makeStationUVW()
//{
//    itsStationUVW.resize(itsInstrument.stations.size());
//    for(size_t i = 0; i < itsStationUVW.size(); ++i)
//    {
////        itsStationUVW[i].reset(new StatUVW(itsInstrument.stations[i],
////            itsInstrument.position, itsPhaseRef));
//        itsStationUVW[i].reset
//            (new StationUVW(itsInstrument.stations[i].position,
//                itsInstrument.position, itsPhaseReference));
//    }
//}

Expr<Scalar>::Ptr Model::makeSpectralIndexExpr(const SourceInfo &source)
{
    unsigned int degree =
        static_cast<unsigned int>(ParmManager::instance().getDefaultValue(SKY,
            "SpectralIndexDegree:" + source.getName()));

    ExprParm::Ptr reference = makeExprParm(SKY, "ReferenceFrequency:"
        + source.getName());

    vector<Expr<Scalar>::Ptr> coeff;
    coeff.reserve(degree + 1);
    for(unsigned int i = 0; i <= degree; ++i)
    {
        ostringstream name;
        name << "SpectralIndex:" << i << ":" << source.getName();
        coeff.push_back(makeExprParm(SKY, name.str()));
    }

    return Expr<Scalar>::Ptr(new SpectralIndex(reference, coeff.begin(),
        coeff.end()));
}

casa::Vector<Expr<Vector<3> >::Ptr>
Model::makeStationUVWExpr(const vector<unsigned int> &stations) const
{
    casa::Vector<Expr<Vector<3> >::Ptr> expr(stations.size());
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const casa::MPosition &position =
            itsInstrument.stations[stations[i]].position;
//        expr[i] = Expr<Vector<3> >::Ptr(new StationUVW(position,
//            itsInstrument.position, itsPhaseReference));
        expr[i] = Expr<Vector<3> >::Ptr(new StatUVW(position,
            itsInstrument.position, itsPhaseReference));
    }

    return expr;
}

casa::Matrix<Expr<Vector<2> >::Ptr>
Model::makeStationShiftExpr(const casa::Vector<Expr<Vector<3> >::Ptr> &uvw,
    const vector<Source::Ptr> &sources) const
{
    casa::Vector<LMN::Ptr> exprLMN(sources.size());
    for(unsigned int i = 0; i < sources.size(); ++i)
    {
        exprLMN(i) = LMN::Ptr(new LMN(itsPhaseReference,
            sources[i]->getPosition()));
    }

    casa::Matrix<Expr<Vector<2> >::Ptr> expr(sources.size(), uvw.size());
    for(unsigned int i = 0; i < uvw.size(); ++i)
    {
        for(unsigned int j = 0; j < sources.size(); ++j)
        {
            expr(j, i) = Expr<Vector<2> >::Ptr(new DFTPS(uvw(i), exprLMN(j)));
        }
    }

    return expr;
}

casa::Vector<Expr<JonesMatrix>::Ptr>
Model::makeBandpassExpr(const vector<unsigned int> &stations)
{
    casa::Vector<Expr<JonesMatrix>::Ptr> expr(stations.size());
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        const unsigned int station = stations[i];
        const string &suffix = itsInstrument.stations[station].name;

        Expr<Scalar>::Ptr B00 = makeExprParm(INSTRUMENT, "Bandpass:0:0:"
            + suffix);
        Expr<Scalar>::Ptr B11 = makeExprParm(INSTRUMENT, "Bandpass:1:1:"
            + suffix);

        expr(i) = Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(B00, B11));
    }

    return expr;
}

casa::Vector<Expr<JonesMatrix>::Ptr>
Model::makeIsotropicGainExpr(const ModelConfig &config,
    const vector<unsigned int> &stations)
{
    string elem0(config.usePhasors() ? "Ampl"  : "Real");
    string elem1(config.usePhasors() ? "Phase" : "Imag");

    casa::Vector<Expr<JonesMatrix>::Ptr> expr(stations.size());
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        Expr<Scalar>::Ptr J00, J01, J10, J11;

        const unsigned int station = stations[i];
        const string &suffix = itsInstrument.stations[station].name;

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
Model::makeAnisotropicGainExpr(const ModelConfig &config,
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

            string suffix(itsInstrument.stations[station].name + ":"
                + sources[j]->getName());

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
        const casa::MPosition &position =
            itsInstrument.stations[station].position;

        for(unsigned int j = 0; j < sources.size(); ++j)
        {
            expr(j, i) = Expr<Vector<2> >::Ptr(new AzEl(position,
                sources[j]->getPosition()));
        }
    }

    return expr;
}

casa::Matrix<Expr<JonesMatrix>::Ptr>
Model::makeDipoleBeamExpr(const ModelConfig &config,
    const vector<unsigned int> &stations,
    const casa::Matrix<Expr<Vector<2> >::Ptr> &azel)
{
    ASSERT(config.useBeam() && config.getBeamType()
        != ModelConfig::UNKNOWN_BEAM_TYPE);

    const unsigned int nSources = azel.shape()(0);
    const unsigned int nStations = azel.shape()(1);
    ASSERT(nStations == stations.size());

    // Allocate result.
    casa::Matrix<Expr<JonesMatrix>::Ptr> expr(nSources, nStations);

    switch(config.getBeamType())
    {
        case ModelConfig::HAMAKER_DIPOLE:
        {
            HamakerDipoleConfig beamConfig;
            config.getBeamConfig(beamConfig);

            // Read beam coefficients from file.
            HamakerBeamCoeff coeff;
            coeff.init(beamConfig.getCoeffFile());

            // Create a dipole beam expression per (station, source)
            // combination.
            for(unsigned int i = 0; i < nStations; ++i)
            {
                // Get dipole orientation.
                const unsigned int station = stations[i];
                Expr<Scalar>::Ptr orientation = makeExprParm(INSTRUMENT,
                    "Orientation:" + itsInstrument.stations[station].name);

                for(unsigned int j = 0; j < nSources; ++j)
                {
                    expr(j, i) = Expr<JonesMatrix>::Ptr(new HamakerDipole(coeff,
                        azel(j, i), orientation));
                }
            }
            break;
        }

        case ModelConfig::YATAWATTA_DIPOLE:
        {
            YatawattaDipoleConfig beamConfig;
            config.getBeamConfig(beamConfig);

            // TODO: Where is this scale factor coming from (see global_model.py
            // in EJones_droopy_comp and EJones_HBA)?
//            const double scaleFactor = options[DIPOLE_BEAM_LBA] ? 1.0 / 88.0
//                : 1.0 / 600.0;

            // Create a dipole beam expression per (station, source)
            // combination.
            for(unsigned int i = 0; i < nStations; ++i)
            {
                // Get dipole orientation.
                const unsigned int station = stations[i];
                Expr<Scalar>::Ptr orientation = makeExprParm(INSTRUMENT,
                    "Orientation:" + itsInstrument.stations[station].name);

                for(unsigned int j = 0; j < nSources; ++j)
                {
                    expr(j, i) = Expr<JonesMatrix>::Ptr
                        (new YatawattaDipole(beamConfig.getModuleTheta(),
                            beamConfig.getModulePhi(), 1.0, azel(j, i),
                            orientation));
                }
            }
            break;
        }

        default:
            THROW(BBSKernelException, "Unsupported beam type encountered.");
    }

    return expr;
}

casa::Matrix<Expr<JonesMatrix>::Ptr>
Model::makeIonosphereNodes(const ModelConfig &config,
    const vector<unsigned int> &stations,
    const casa::Matrix<Expr<Vector<2> >::Ptr> &azel)
{
    // Use station 0 as reference position on earth.
    // TODO: Is station 0 a sensible choice for the reference position? Should
    // we use the array reference position instead? Or should it be different
    // for each station?
    casa::MPosition reference = itsInstrument.stations[0].position;

    // Get ionosphere model parameters.
    IonosphereConfig ionoConfig;
    config.getIonosphereConfig(ionoConfig);
    unsigned int degree = ionoConfig.getDegree() + 1;

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

    // Create a MIM expression per (station, source) combination.
    unsigned int nSources = azel.shape()(0);
    unsigned int nStations = azel.shape()(1);
    ASSERT(nStations == stations.size());

    casa::Matrix<Expr<JonesMatrix>::Ptr> expr(nSources, nStations);
    for(size_t i = 0; i < nStations; ++i)
    {
        const unsigned int station = stations[i];
        const casa::MPosition &position =
            itsInstrument.stations[station].position;

        for(size_t j = 0; j < nSources; ++j)
        {
            PiercePoint::Ptr pp(new PiercePoint(position, azel(j, i)));
            MIM::Ptr mim(new MIM(reference, pp, MIMParms.begin(),
                MIMParms.end()));
            expr(j, i) = AsDiagonalMatrix::Ptr(new AsDiagonalMatrix(mim, mim));
        }
    }

    return expr;
}


} // namespace BBS
} // namespace LOFAR
