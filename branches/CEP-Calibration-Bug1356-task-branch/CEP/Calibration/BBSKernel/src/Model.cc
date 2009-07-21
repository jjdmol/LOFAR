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
//#include <BBSKernel/Expr/YatawattaDipole.h>
//#include <BBSKernel/Expr/HamakerDipole.h>
#include <BBSKernel/Expr/PhaseRef.h>
//#include <BBSKernel/Expr/UVW.h>
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
        itsPhaseReference(reference),
{
    // Make station UVW expression for all stations.
    makeStationUVW();
}

void Model::clear()
{
    itsExpr.clear();
    itsParms.clear();

    itsCache.printStats();
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
            PointCoherence::Ptr(new PointCoherence(point->getStokesVector()));
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

    // Create a station shift node per (station, source) combination.
    casa::Matrix<Expr<Vector<2> >::Ptr> exprStationShift =
        makeStationShiftExpr(stations, sources);

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

//    // Create a dipole beam node per (station0, source) combination.
//    casa::Matrix<Expr<Vector<2> >::Ptr> exprBeam;
//    if(config.useBeam())
//    {
//        exprBeam = makeBeamExpr(config, stations, exprAzEl);
//    }

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

//                if(config.useBeam())
//                {
//                    anisoTransform(j, i) = corrupt(anisoTransform(j, i),
//                        exprDipoleBeam(j, i));
//                }

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

//        // Create a dipole beam node per (station0, source) combination.
//        casa::Matrix<Expr<Vector<2> >::Ptr> exprBeam;
//        if(config.useBeam())
//        {
//            exprBeam = makeBeamExpr(config, stations, exprAzEl);
//        }

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

//            if(config.useBeam())
//            {
//                transform(i) = corrupt(transform(0, i), exprDipoleBeam(0, i));
//            }

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
    itsCache.printStats();
    itsCache.clearStats();
    itsCache.clear();

    // TODO: Set cache size in number of Matrix instances... ?
}

JonesMatrix Model::evaluate(const baseline_t &baseline)
{
    map<baseline_t, Expr<JonesMatrix>::Ptr>::const_iterator exprIt =
        itsExpr.find(baseline);
    ASSERT(exprIt != itsExpr.end());

    const JonesMatrix model = (exprIt->second)->evaluate(itsRequest, itsCache);

    JonesMatrix result;
    result.setFlags(model.flags());
//    LOG_DEBUG_STR("BL: " << baseline.first << " - " << baseline.second
//        << " FLAGS: " << value.flags());

    const JonesMatrix::view value = model.value();
    result.assign(value);

//    cout << "Computing partials:";

    ExprBase::const_solvable_iterator solIt = (exprIt->second)->begin();
    ExprBase::const_solvable_iterator solItEnd = (exprIt->second)->end();
    while(solIt != solItEnd)
    {
//        cout << " " << solvables[i].parmId << "|" << solvables[i].coeffId;

        // Get the perturbed value associated with *solIt (the current
        // solvable).
        const JonesMatrix::view pert = model.value(*solIt);

        // Get perturbation.
//        map<unsigned int, ExprParm::Ptr>::const_iterator parmIt =
//            itsParms.find(it->parmId);
//        ASSERT(parmIt != itsParms.end());
//        ASSERT(parmIt->second);
//        const double inversePert =
//            1.0 / parmIt->second->getPerturbation(it->coeffId);

        ParmProxy::ConstPtr parm =
            ParmManager::instance().get(solIt->parmId);
        const double inversePert =
            1.0 / parm->getPerturbation(solIt->coeffId);

        // Approximate partial derivative by forward differences.
        JonesMatrix::view partial;
        partial.assign(0, 0, Matrix((pert(0, 0) - value(0, 0)) * inversePert));
        partial.assign(0, 1, Matrix((pert(0, 1) - value(0, 1)) * inversePert));
        partial.assign(1, 0, Matrix((pert(1, 0) - value(1, 0)) * inversePert));
        partial.assign(1, 1, Matrix((pert(1, 1) - value(1, 1)) * inversePert));

//        cout << "main: " << main(0) << endl;
//        cout << "pert: " << pert(0) << endl;
//        cout << "partial: " << partial(0) << endl;

        result.assign(*solIt, partial);
        ++solIt;
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

    map<baseline_t, Expr<JonesMatrix>::Ptr>::const_iterator exprIt =
        itsExpr.begin();
    while(exprIt != itsExpr.end())
    {
        exprIt->second->updateSolvables();
        ++exprIt;
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

    map<baseline_t, Expr<JonesMatrix>::Ptr>::const_iterator exprIt =
        itsExpr.begin();
    while(exprIt != itsExpr.end())
    {
        exprIt->second->updateSolvables();
        ++exprIt;
    }
}

//ParmGroup Model::getPerturbedParms() const
//{
//    ParmGroup result;
//
//    map<uint, Expr>::const_iterator it = itsParms.begin();
//    map<uint, Expr>::const_iterator itEnd = itsParms.end();
//    while(it != itEnd)
//    {
//        const ExprParm *parmPtr =
//            static_cast<const ExprParm*>(it->second.getPtr());
//        ASSERT(parmPtr);
//        if(parmPtr->getPValueFlag())
//        {
//            result.insert(it->first);
//        }
//        ++it;
//    }

//    return result;
//}

//void Model::precalculate(const Request &request)
//{
//    if(itsExpr.empty())
//    {
//        return;
//    }
//
//    // First clear the levels of all nodes in the tree.
//    for(map<baseline_t, JonesExpr>::iterator it = itsExpr.begin();
//        it != itsExpr.end();
//        ++it)
//    {
//        JonesExpr &expr = it->second;
//        ASSERT(!expr.isNull());
//        expr.clearDone();
//    }

//    // Now set the levels of all nodes in the tree.
//    // The root nodes have level 0; child nodes have level 1 or higher.
//    int nrLev = -1;
//    for(map<baseline_t, JonesExpr>::iterator it = itsExpr.begin();
//        it != itsExpr.end();
//        ++it)
//    {
//        JonesExpr &expr = it->second;
//        ASSERT(!expr.isNull());
//        nrLev = std::max(nrLev, expr.setLevel(0));
//    }
//    ++nrLev;
//    ASSERT(nrLev > 0);

//    // Find the nodes to be precalculated at each level.
//    // That is not needed for the root nodes (the baselines).
//    // The nodes used by the baselines are always precalculated (even if
//    // having one parent).
//    // It may happen that a station is used by only one baseline. Calculating
//    // such a baseline is much more work if the station was not precalculated.
//    vector<vector<ExprRep*> > precalcNodes(nrLev);
//    for(int level = 1; level < nrLev; ++level)
//    {
//        for(map<baseline_t, JonesExpr>::iterator it = itsExpr.begin();
//            it != itsExpr.end();
//            ++it)
//        {
//            JonesExpr &expr = it->second;
//            ASSERT(!expr.isNull());
//            expr.getCachingNodes(precalcNodes[level], level, false);
//        }
//    }

//    LOG_TRACE_FLOW_STR("#levels=" << nrLev);
//    for(int i = 0; i < nrLev; ++i)
//    {
//        LOG_TRACE_FLOW_STR("#expr on level " << i << " is "
//            << precalcNodes[i].size());
//    }

//#pragma omp parallel
//    {
//        // Loop through expressions to be precalculated.
//        // At each level the expressions can be executed in parallel.
//        // Level 0 is formed by itsExpr which are not calculated here.
//        for(size_t level = precalcNodes.size(); --level > 0;)
//        {
//            vector<ExprRep*> &nodes = precalcNodes[level];
//
//            if(!nodes.empty())
//            {
//#pragma omp for schedule(dynamic)
//                // NOTE: OpenMP will only parallelize for loops that use type
//                // 'int' for the loop counter.
//                for(int i = 0; i < static_cast<int>(nodes.size()); ++i)
//                {
//                    nodes[i]->precalculate(request);
//                }
//            }
//        }
//    } // omp parallel
//}

//JonesResult Model::evaluate(const baseline_t &baseline,
//    const Request &request)
//{
//    map<baseline_t, JonesExpr>::iterator it = itsExpr.find(baseline);
//    ASSERTSTR(it != itsExpr.end(), "Result requested for unknown"
//        " baseline " << baseline.first << " - " << baseline.second);

//    return it->second.getResult(request);
//}


//vector<bool> Model::parseComponents(const vector<string> &components) const
//{
//    vector<bool> mask(N_ModelComponent, false);

//    for(vector<string>::const_iterator it = components.begin();
//        it != components.end();
//        ++it)
//    {
//        if(*it == "BANDPASS")
//        {
//            mask[BANDPASS] = true;
//        }
//        else if(*it == "GAIN")
//        {
//            mask[GAIN] = true;
//        }
//        else if(*it == "DIRECTIONAL_GAIN")
//        {
//            mask[DIRECTIONAL_GAIN] = true;
//        }
//        else if(*it == "BEAM")
//        {
//            mask[BEAM] = true;
//        }
//        else if(*it == "IONOSPHERE")
//        {
//            mask[IONOSPHERE] = true;
//        }
//        else if(*it == "COND_NUM_FLAG")
//        {
//            mask[COND_NUM_FLAG] = true;
//        }
//        else
//        {
//            LOG_WARN_STR("Ignored unsupported model component \"" << *it
//                << "\".");
//        }
//    }

//    return mask;
//}

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
//    LOG_DEBUG_STR("Creating source: " << source.getName() << " ["
//        << source.getType() << "]);

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

//    case SourceInfo::GAUSSIAN:
//        {
//            Expr ra(makeExprParm(SKY, "Ra:" + source.getName()));
//            Expr dec(makeExprParm(SKY, "Dec:" + source.getName()));
//            Expr i(makeExprParm(SKY, "I:" + source.getName()));
//            Expr q(makeExprParm(SKY, "Q:" + source.getName()));
//            Expr u(makeExprParm(SKY, "U:" + source.getName()));
//            Expr v(makeExprParm(SKY, "V:" + source.getName()));
//            Expr maj(makeExprParm(SKY, "Major:" + source.getName()));
//            Expr min(makeExprParm(SKY, "Minor:" + source.getName()));
//            Expr phi(makeExprParm(SKY, "Phi:" + source.getName()));

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

void Model::makeStationUVW()
{
    itsStationUVW.resize(itsInstrument.stations.size());
    for(size_t i = 0; i < itsStationUVW.size(); ++i)
    {
//        itsStationUVW[i].reset(new StatUVW(itsInstrument.stations[i],
//            itsInstrument.position, itsPhaseRef));
        itsStationUVW[i].reset
            (new StationUVW(itsInstrument.stations[i].position,
                itsInstrument.position, itsPhaseReference));
    }
}

casa::Matrix<Expr<Vector<2> >::Ptr>
Model::makeStationShiftExpr(const vector<unsigned int> &stations,
    const vector<Source::Ptr> &sources) const
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
        const unsigned int station = stations[i];
        for(unsigned int j = 0; j < sources.size(); ++j)
        {
            expr(j, i) = Expr<Vector<2> >::Ptr(new DFTPS(itsStationUVW[station],
                exprLMN(j)));
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

//void Model::makeDipoleBeamNodes(boost::multi_array<JonesExpr, 2> &result,
//    const ModelConfig &config, const boost::multi_array<Expr, 2> &azel)
//{
//    ASSERT(config.beamConfig);
//    ASSERT(config.beamConfig->type() == "HamakerDipole"
//        || config.beamConfig->type() == "YatawattaDipole");
//    ASSERT(azel.shape()[0] == itsInstrument.stations.size());
//
//    const size_t nStations = azel.shape()[0];
//    const size_t nSources = azel.shape()[1];

//    // TODO: This code uses station 0 as reference position on earth
//    // (see global_model.py in EJones_HBA). Is this accurate enough?

//    if(config.beamConfig->type() == "HamakerDipole")
//    {
//        HamakerDipoleConfig::ConstPtr beamConfig =
//            dynamic_pointer_cast<const HamakerDipoleConfig>(config.beamConfig);
//        ASSERT(beamConfig);
//
//        // Read beam model coefficients.
//        BeamCoeff coeff = readBeamCoeffFile(beamConfig->coeffFile);

//        // Create a beam node for each source-station combination.
//        result.resize(boost::extents[nStations][nSources]);
//        for(size_t i = 0; i < nStations; ++i)
//        {
//            // Get dipole orientation.
//            Expr orientation(makeExprParm(INSTRUMENT, "Orientation:"
//                + itsInstrument.stations[i].name));

//            for(size_t j = 0; j < nSources; ++j)
//            {
//                result[i][j] = new HamakerDipole(coeff, azel[0][j],
//                    orientation);
//            }
//        }
//    }
//    else if(config.beamConfig->type() == "YatawattaDipole")
//    {
//        YatawattaDipoleConfig::ConstPtr beamConfig =
//            dynamic_pointer_cast<const YatawattaDipoleConfig>
//                (config.beamConfig);
//        ASSERT(beamConfig);

//        // TODO: Where is this scale factor coming from (see global_model.py
//        // in EJones_droopy_comp and EJones_HBA)?
////        const double scaleFactor = options[DIPOLE_BEAM_LBA] ? 1.0 / 88.0
////            : 1.0 / 600.0;

//        // Create a beam node for each source-station combination.
//        result.resize(boost::extents[nStations][nSources]);
//        for(size_t i = 0; i < nStations; ++i)
//        {
//            // Get dipole orientation.
//            Expr orientation(makeExprParm(INSTRUMENT, "Orientation:"
//                + itsInstrument.stations[i].name));

//            for(size_t j = 0; j < nSources; ++j)
//            {
////                result[i][j] = new DipoleBeam(azel[j], 1.706, 1.38,
////                    casa::C::pi / 4.001, -casa::C::pi_4);
//                result[i][j] = new YatawattaDipole(beamConfig->moduleTheta,
//                    beamConfig->modulePhi, azel[0][j], orientation, 1.0);
//            }
//        }
//    }
//}

casa::Matrix<Expr<JonesMatrix>::Ptr>
Model::makeIonosphereNodes(const ModelConfig &config,
    const vector<unsigned int> &stations,
    const casa::Matrix<Expr<Vector<2> >::Ptr> &azel)
{
    // Use station 0 as reference position on earth.
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
            // For the moment we do not include MIM:0:0 (i.e. absolute TEC).
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

    // Create a MIM node per (station, source) combination.
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

//BeamCoeff Model::readBeamCoeffFile(const string &filename) const
//{
//    LOG_DEBUG_STR("Reading beam coefficients...");

//    // Open file.
//    ifstream in(filename.c_str());
//    if(!in)
//    {
//        THROW(BBSKernelException, "Unable to open file: " << filename << ".");
//    }

//    // Read file header.
//    string header, token0, token1, token2, token3, token4, token5;
//    getline(in, header);
//
//    size_t nElements, nHarmonics, nPowerTime, nPowerFreq;
//    double freqAvg, freqRange;
//
//    istringstream iss(header);
//    iss >> token0 >> nElements >> token1 >> nHarmonics >> token2 >> nPowerTime
//        >> token3 >> nPowerFreq >> token4 >> freqAvg >> token5 >> freqRange;

//    if(!in || !iss || token0 != "d" || token1 != "k" || token2 != "pwrT"
//        || token3 != "pwrF" || token4 != "freqAvg" || token5 != "freqRange")
//    {
//        THROW(BBSKernelException, "Unable to parse header");
//    }

//    if(nElements * nHarmonics * nPowerTime * nPowerFreq == 0)
//    {
//        THROW(BBSKernelException, "The number of coefficients should be larger"
//            " than zero.");
//    }

//    LOG_DEBUG_STR("nElements: " << nElements << " nHarmonics: " << nHarmonics
//        << " nPowerTime: " << nPowerTime << " nPowerFreq: " << nPowerFreq);

//    // Allocate coefficient matrix.
//    shared_ptr<boost::multi_array<dcomplex, 4> > coeff
//        (new boost::multi_array<dcomplex, 4>(boost::extents[nElements]
//            [nHarmonics][nPowerTime][nPowerFreq]));

//    size_t nCoeff = 0;
//    while(in.good())
//    {
//        // Read line from file.
//        string line;
//        getline(in, line);

//        // Skip lines that contain only whitespace.
//        if(line.find_last_not_of(" ") == string::npos)
//        {
//            continue;
//        }

//        // Parse line.
//        size_t element, harmonic, powerTime, powerFreq;
//        double re, im;
//
//        iss.clear();
//        iss.str(line);
//        iss >> element >> harmonic >> powerTime >> powerFreq >> re >> im;
//
//        if(!iss || element >= nElements || harmonic >= nHarmonics
//            || powerTime >= nPowerTime || powerFreq >= nPowerFreq)
//        {
//            THROW(BBSKernelException, "Error reading file.");
//        }
//
//        // Store coefficient.
//        (*coeff)[element][harmonic][powerTime][powerFreq] =
//            makedcomplex(re, im);

//        // Update coefficient counter.
//        ++nCoeff;
//    }

//    if(!in.eof())
//    {
//        THROW(BBSKernelException, "Error reading file.");
//    }

//    if(nCoeff != nElements * nHarmonics * nPowerTime * nPowerFreq)
//    {
//        THROW(BBSKernelException, "Number of coefficients in header does not"
//            " match number of coefficients in file.");
//    }

//    return BeamCoeff(freqAvg, freqRange, coeff);
//}

} // namespace BBS
} // namespace LOFAR
