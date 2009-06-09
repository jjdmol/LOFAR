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

#include <BBSKernel/Expr/PointSource.h>
#include <BBSKernel/Expr/Mul.h>
#include <BBSKernel/Expr/MatrixMul3.h>
#include <BBSKernel/Expr/MatrixSum.h>
#include <BBSKernel/Expr/PointCoherence.h>
//#include <BBSKernel/Expr/GaussianSource.h>
//#include <BBSKernel/Expr/GaussianCoherence.h>

//#include <BBSKernel/Expr/ExprParm.h>
//#include <BBSKernel/Expr/AzEl.h>
//#include <BBSKernel/Expr/PiercePoint.h>
//#include <BBSKernel/Expr/MIM.h>
//#include <BBSKernel/Expr/YatawattaDipole.h>
//#include <BBSKernel/Expr/HamakerDipole.h>
#include <BBSKernel/Expr/PhaseRef.h>
//#include <BBSKernel/Expr/UVW.h>
#include <BBSKernel/Expr/PhaseShift.h>
#include <BBSKernel/Expr/ConditionNumber.h>
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
    :   itsSourceDb(sourceDb),
        itsInstrument(instrument)
{
    // Store phase reference.
    itsPhaseRef = PhaseRef::Ptr(new PhaseRef(reference));

    // Make station UVW expr for all stations.
    makeStationUVW();
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

void Model::setRequestGrid(const Grid &grid)
{
    itsRequest = Request(grid);
    itsCache.printStats();
    itsCache.clearStats();
    itsCache.clear();
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

    const JonesMatrix::proxy value = model.value();
    result.assign(value);

//    cout << "Computing partials:";

    ExprBase::const_solvables_iterator solIt = (exprIt->second)->begin();
    ExprBase::const_solvables_iterator solItEnd = (exprIt->second)->end();
    while(solIt != solItEnd)
    {
//        cout << " " << solvables[i].parmId << "|" << solvables[i].coeffId;

        // Get the perturbed value associated with *solIt (the current
        // solvable).
        const JonesMatrix::proxy pert = model.value(*solIt);

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
        JonesMatrix::proxy partial;
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

boost::multi_array<Expr<JonesMatrix>::Ptr, 2>
Model::makeDirectionDependentGainExpr(const ModelConfig &config,
    const vector<unsigned int> &stations, const vector<Source::Ptr> &sources)
{
    string elem1(config.usePhasors ? "Ampl"  : "Real");
    string elem2(config.usePhasors ? "Phase" : "Imag");

    boost::multi_array<Expr<JonesMatrix>::Ptr, 2>
        result(boost::extents[stations.size()][sources.size()]);

    for(size_t i = 0; i < stations.size(); ++i)
    {
        for(size_t j = 0; j < sources.size(); ++j)
        {
            Expr<Scalar>::Ptr J00, J01, J10, J11;

            string suffix(itsInstrument.stations[stations[i]].name + ":"
                + sources[j]->getName());

            ExprParm::Ptr J00_elem1 =
                makeExprParm(INSTRUMENT, "Gain:0:0:" + elem1 + ":" + suffix);
            ExprParm::Ptr J00_elem2 =
                makeExprParm(INSTRUMENT, "Gain:0:0:" + elem2 + ":" + suffix);
            ExprParm::Ptr J01_elem1 =
                makeExprParm(INSTRUMENT, "Gain:0:1:" + elem1 + ":" + suffix);
            ExprParm::Ptr J01_elem2 =
                makeExprParm(INSTRUMENT, "Gain:0:1:" + elem2 + ":" + suffix);
            ExprParm::Ptr J10_elem1 =
                makeExprParm(INSTRUMENT, "Gain:1:0:" + elem1 + ":" + suffix);
            ExprParm::Ptr J10_elem2 =
                makeExprParm(INSTRUMENT, "Gain:1:0:" + elem2 + ":" + suffix);
            ExprParm::Ptr J11_elem1 =
                makeExprParm(INSTRUMENT, "Gain:1:1:" + elem1 + ":" + suffix);
            ExprParm::Ptr J11_elem2 =
                makeExprParm(INSTRUMENT, "Gain:1:1:" + elem2 + ":" + suffix);

            ASSERT(!config.usePhasors);
            if(config.usePhasors)
            {
                J00.reset(new AsPolar(J00_elem1, J00_elem2));
                J01.reset(new AsPolar(J01_elem1, J01_elem2));
                J10.reset(new AsPolar(J10_elem1, J10_elem2));
                J11.reset(new AsPolar(J11_elem1, J11_elem2));
            }
            else
            {
                J00.reset(new AsComplex(J00_elem1, J00_elem2));
                J01.reset(new AsComplex(J01_elem1, J01_elem2));
                J10.reset(new AsComplex(J10_elem1, J10_elem2));
                J11.reset(new AsComplex(J11_elem1, J11_elem2));
            }

            AsExpr<JonesMatrix>::Ptr jones(new AsExpr<JonesMatrix>());
            jones->connect(0, 0, J00);
            jones->connect(0, 1, J01);
            jones->connect(1, 0, J10);
            jones->connect(1, 1, J11);

            result[i][j] = jones;
        }
    }

    return result;
}

vector<unsigned int> Model::stationsUsed(const vector<baseline_t> &baselines)
    const
{
    vector<bool> flags(itsInstrument.stations.size(), false);

    vector<baseline_t>::const_iterator blIt = baselines.begin();
    while(blIt != baselines.end())
    {
        DBGASSERT(blIt->first < flags.size() && blIt->second < flags.size());
        flags[blIt->first] = true;
        flags[blIt->second] = true;
        ++blIt;
    }

    vector<unsigned int> result;
    for(unsigned int i = 0; i < flags.size(); ++i)
    {
        if(flags[i])
        {
            result.push_back(i);
        }
    }

    return result;
}

void Model::makeForwardExpr(const ModelConfig &config,
    const VisData::Ptr &chunk, const vector<baseline_t> &baselines)
{
    // Clear previously created expressions and cached results.
    clear();

    // Get a vector of indices of those stations that are referenced in the
    // baselines selection.
    const vector<unsigned int> stations = stationsUsed(baselines);

    // Create Source objects for all selected sources.
    vector<Source::Ptr> sources = makeSourceList(config.sources);
    if(sources.empty())
    {
        THROW(BBSKernelException, "No sources matching selection found in"
            " source database.");
    }

    // Parse user supplied model component selection.
    vector<bool> mask(parseComponents(config.components));

    // Make a UVW expression for each baseline.
//    vector<Expr::Ptr> uvw = makeUVWExpr(chunk, baselines);

    // Create a station shift node per station-source combination.
    boost::multi_array<Expr<Vector<2> >::Ptr, 2> stationShift;
        makeStationShiftNodes(stationShift, stations, sources);

    boost::multi_array<Expr<JonesMatrix>::Ptr, 2> gainExpr =
        makeDirectionDependentGainExpr(config, stations, sources);

    // Make an LMN and coherence expression for each source.
//    vector<Expr::Ptr> lmn;
    vector<Expr<JonesMatrix>::Ptr> coherence;

    for(size_t i = 0; i < sources.size(); ++i)
    {
//        LMN::Ptr lmnExpr(new LMN(itsPhaseRef));
//        lmnExpr->connect(LMN::POSITION, sources[i]->getPosition());
//        lmn.push_back(lmnExpr);

        PointSource::ConstPtr point =
            dynamic_pointer_cast<const PointSource>(sources[i]);

        PointCoherence::Ptr coherenceExpr(new PointCoherence(point->getStokesVector()));
        coherence.push_back(coherenceExpr);
    }

    // Make an expression that computes the complex visibilities for each
    // baseline.
    for(size_t i = 0; i < baselines.size(); ++i)
    {
        MatrixSum::Ptr sum(new MatrixSum());

        for(size_t j = 0; j < sources.size(); ++j)
        {
//            PhaseShift::Ptr shift(new PhaseShift());
//            shift->connect(PhaseShift::UVW, uvw[i]);
//            shift->connect(PhaseShift::LMN, lmn[j]);

            // Phase shift (incorporates geometry and fringe stopping).
            PhaseShiftOld::Ptr shift(new PhaseShiftOld(stationShift[baselines[i].first][j], stationShift[baselines[i].second][j]));

            Mul::Ptr mul(new Mul(shift, coherence[j]));

            if(mask[DIRECTIONAL_GAIN])
            {
                MatrixMul3::Ptr matmul3(new MatrixMul3(gainExpr[baselines[i].first][j],
                    mul, gainExpr[baselines[i].second][j]));

//                matmul3->connect(MatrixMul3::LEFT, gainExpr[baselines[i].first][j]);
//                matmul3->connect(MatrixMul3::MID, mul);
//                matmul3->connect(MatrixMul3::RIGHT, gainExpr[baselines[i].second][j]);
                sum->connect(matmul3);
            }
            else
            {
                sum->connect(mul);
            }
        }

//        itsExpr[baselines[i]] = makeExprParm(SKY, "I:CasA");
        itsExpr[baselines[i]] = sum;
    }

//    LMN::Ptr lmnCasA(new LMN(itsPhaseRef));
//    lmnCasA->connect(LMN::RA, makeExprParm(SKY, "Ra:CasA"));
//    lmnCasA->connect(LMN::DEC, makeExprParm(SKY, "Dec:CasA"));

//    PointCoherence::Ptr cohCasA(new PointCoherence());
//    cohCasA->connect(PointCoherence::STOKES_I, makeExprParm(SKY, "I:CasA"));
//    cohCasA->connect(PointCoherence::STOKES_Q, makeExprParm(SKY, "Q:CasA"));
//    cohCasA->connect(PointCoherence::STOKES_U, makeExprParm(SKY, "U:CasA"));
//    cohCasA->connect(PointCoherence::STOKES_V, makeExprParm(SKY, "V:CasA"));

//    LMN::Ptr lmnCygA(new LMN(itsPhaseRef));
//    lmnCygA->connect(LMN::RA, makeExprParm(SKY, "Ra:CygA"));
//    lmnCygA->connect(LMN::DEC, makeExprParm(SKY, "Dec:CygA"));

//    PointCoherence::Ptr cohCygA(new PointCoherence());
//    cohCygA->connect(PointCoherence::STOKES_I, makeExprParm(SKY, "I:CygA"));
//    cohCygA->connect(PointCoherence::STOKES_Q, makeExprParm(SKY, "Q:CygA"));
//    cohCygA->connect(PointCoherence::STOKES_U, makeExprParm(SKY, "U:CygA"));
//    cohCygA->connect(PointCoherence::STOKES_V, makeExprParm(SKY, "V:CygA"));


//    // Create source nodes for all selected sources.
//    vector<Source::Ptr> sources;
//    makeSources(sources, config.sources);
//    if(sources.empty())
//    {
//        LOG_ERROR_STR("Source selection is empty.");
//        return false;
//    }

//    const size_t nSources = sources.size();
//    const size_t nStations = itsInstrument.stations.size();

////    cout << "nSources: " << nSources << endl;

//    // Create coherence nodes for all point sources.
//    map<size_t, JonesExpr> pointCoh;
//    for(size_t i = 0; i < nSources; ++i)
//    {
//        // A point source's coherence is independent of baseline UVW
//        // coordinates. Therefore, they are constructed here and shared
//        // between baselines.
//        PointSource::ConstPtr point =
//            dynamic_pointer_cast<const PointSource>(sources[i]);

//        if(point)
//        {
//            // Create point source coherence.
//            pointCoh[i] = new PointCoherence(point);
//        }
//    }

//    // Direction independent (uv-plane) effects.
//    bool uvEffects = mask[BANDPASS] || mask[GAIN];
//
//    // Create a bandpass node per station.
//    vector<JonesExpr> bandpass;
//    if(mask[BANDPASS])
//    {
//        makeBandpassNodes(bandpass);
//    }
//
//    // Create a gain node per station.
//    vector<JonesExpr> gain;
//    if(mask[GAIN])
//    {
//        makeGainNodes(gain, config);
//    }

//    // Create a single JonesExpr per station that is the accumulation of all
//    // uv-plane effects.
//    vector<JonesExpr> uvJones;
//    if(uvEffects)
//    {
//        uvJones.resize(nStations);
//        for(size_t i = 0; i < nStations; ++i)
//        {
//            if(mask[BANDPASS])
//            {
//                uvJones[i] = bandpass[i];
//            }
//
//            if(mask[GAIN])
//            {
//                uvJones[i] = uvJones[i].isNull() ? gain[i]
//                    : JonesExpr(new JonesMul2(uvJones[i], gain[i]));
//            }

//            // TODO: Add other uv-plane effects here.
//        }
//    }
//
//    // Direction dependent (image-plane) effects.
//    bool imgEffects = mask[DIRECTIONAL_GAIN] || mask[BEAM] || mask[IONOSPHERE];
//
//    // Create a station shift node per station-source combination.
//    boost::multi_array<Expr, 2> stationShift;
//    makeStationShiftNodes(stationShift, sources);

//    // Create an AzEl node per station-source combination.
//    boost::multi_array<Expr, 2> azel;
//    if(mask[BEAM] || mask[IONOSPHERE])
//    {
//        makeAzElNodes(azel, sources);
//    }
//
//    // Create a directional gain node per station-source combination.
//    boost::multi_array<JonesExpr, 2> gainDirectional;
//    if(mask[DIRECTIONAL_GAIN])
//    {
//        makeDirectionalGainNodes(gainDirectional, config, sources);
//    }
//
//    // Create a dipole beam node per station0-source combination.
//    boost::multi_array<JonesExpr, 2> dipoleBeam;
//    if(mask[BEAM])
//    {
//        makeDipoleBeamNodes(dipoleBeam, config, azel);
//    }
//
//    // Create a MIM node per station-source combination.
//    boost::multi_array<JonesExpr, 2> ionosphere;
//    if(mask[IONOSPHERE])
//    {
//      makeIonosphereNodes(ionosphere, config, azel);
//    }

//    // Create a single JonesExpr per station-source combination that is the
//    // accumulation of all image-plane effects.
//    // TODO: Incorporate station phase shift by splitting it per station-source.
//    boost::multi_array<JonesExpr, 2> imgJones;
//    if(imgEffects)
//    {
//        imgJones.resize(boost::extents[nStations][nSources]);

//        for(size_t i = 0; i < nStations; ++i)
//        {
//            for(size_t j = 0; j < nSources; ++j)
//            {
//                if(mask[DIRECTIONAL_GAIN])
//                {
//                    imgJones[i][j] = gainDirectional[i][j];
//                }
//
//                if(mask[BEAM])
//                {
//                    imgJones[i][j] = imgJones[i][j].isNull() ? dipoleBeam[i][j]
//                        : JonesExpr(new JonesMul2(imgJones[i][j],
//                            dipoleBeam[i][j]));
//                }

//                if(mask[IONOSPHERE])
//                {
//                    imgJones[i][j] = imgJones[i][j].isNull() ? ionosphere[i][j]
//                        : JonesExpr(new JonesMul2(imgJones[i][j],
//                            ionosphere[i][j]));
//                }
//
//                // TODO: Add other image-plane effects here.
//            }
//        }
//    }
//
//    // Create an expression tree for each baseline.
//    for(size_t i = 0; i < baselines.size(); ++i)
//    {
//        const baseline_t &baseline = baselines[i];

//        vector<JonesExpr> terms;
//        for(size_t j = 0; j < nSources; ++j)
//        {
//            JonesExpr coherence;
//
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
//
//                coherence = new GaussianCoherence(gauss,
//                    itsStationUVW[baseline.first],
//                    itsStationUVW[baseline.second]);
//            }

//            // Phase shift (incorporates geometry and fringe stopping).
//            Expr shift(new PhaseShift(stationShift[baseline.first][j],
//                stationShift[baseline.second][j]));

//            // Phase shift the source coherence.
//            ASSERT(!coherence.isNull());
//            coherence = new JonesMul(shift, coherence);
//
//            // Apply direction dependent (image-plane) effects.
//            if(imgEffects)
//            {
//                coherence = new JonesCMul3(imgJones[baseline.first][j],
//                    coherence, imgJones[baseline.second][j]);
//            }

//            terms.push_back(coherence);
//        }

//        // Sum all source coherences.
//        JonesExpr sum;
//        if(terms.size() == 1)
//        {
//            sum = terms.front();
//        }
//        else
//        {
//            sum = new JonesSum(terms);
//        }

//        // Apply direction independent (uv-plane) effects.
//        if(uvEffects)
//        {
//            sum = new JonesCMul3(uvJones[baseline.first], sum,
//                uvJones[baseline.second]);
//        }

//        itsExpr[baseline] = sum;
//    }
}

void Model::makeInverseExpr(const ModelConfig &config,
    const VisData::Ptr &chunk, const vector<baseline_t> &baselines)
{
    // Clear previously created expressions and cached results.
    clear();

    // Get a vector of indices of those stations that are referenced in the
    // baselines selection.
    const vector<unsigned int> stations = stationsUsed(baselines);

    // Create Source objects for all selected sources.
    vector<Source::Ptr> sources = makeSourceList(config.sources);
    ASSERTSTR(sources.size() == 1, "No direction, or more than one direction"
        " specified. A correction can only be applied for a single direction on"
        " the sky");

    // Parse user supplied model component selection.
    vector<bool> mask(parseComponents(config.components));

    // Create a single JonesExpr per station-source combination that is the
    // accumulation of all uv-plane and image-plane effects.
    vector<Expr<JonesMatrix>::Ptr> jJones(stations.size());

    if(mask[DIRECTIONAL_GAIN])
    {
        boost::multi_array<Expr<JonesMatrix>::Ptr, 2> gainExpr =
            makeDirectionDependentGainExpr(config, stations, sources);

        for(size_t i = 0; i < stations.size(); ++i)
        {
            if(mask[COND_NUM_FLAG])
            {
                ASSERT(config.condNumFlagConfig);
                ConditionNumber::Ptr cn(new ConditionNumber(gainExpr[i][0],
                    config.condNumFlagConfig->threshold));
                jJones[i].reset(new JonesInvert(cn));
            }
            else
            {
                jJones[i].reset(new JonesInvert(gainExpr[i][0]));
            }
        }
    }

    // Create an expression tree for each baseline.
    for(size_t i = 0; i < baselines.size(); ++i)
    {
        const baseline_t &baseline = baselines[i];

        Expr<JonesMatrix>::Ptr vdata(new JonesVisData(chunk, baseline));

        if(mask[DIRECTIONAL_GAIN])
        {
            vdata.reset(new MatrixMul3(jJones[baseline.first], vdata,
                jJones[baseline.second]));
        }

        itsExpr[baseline] = vdata;
    }

//    ASSERTSTR(itsExpr.empty(), "Model already initialized; call Model::"
//        "clearExpressions() first");

//    const size_t nStations = itsInstrument.stations.size();

//    // Parse user supplied model component selection.
//    vector<bool> mask(parseComponents(config.components));

//    // Direction independent (uv-plane) effects.
//    bool uvEffects = mask[BANDPASS] || mask[GAIN];
//
//    // Create a bandpass node per station.
//    vector<JonesExpr> bandpass;
//    if(mask[BANDPASS])
//    {
//        makeBandpassNodes(bandpass);
//    }
//
//    // Create a gain node per station.
//    vector<JonesExpr> gain;
//    if(mask[GAIN])
//    {
//        makeGainNodes(gain, config);
//    }

//    // Create a single JonesExpr per station-source combination that is the
//    // accumulation of all uv-plane and image-plane effects.
//    // Accumulate all uv-plane effects.
//    boost::multi_array<JonesExpr, 1> jJones(boost::extents[nStations]);
//    if(uvEffects)
//    {
//        for(size_t i = 0; i < nStations; ++i)
//        {
//            if(mask[BANDPASS])
//            {
//                jJones[i] = bandpass[i];
//            }
//
//            if(mask[GAIN])
//            {
//                jJones[i] = jJones[i].isNull() ? gain[i]
//                    : JonesExpr(new JonesMul2(jJones[i], gain[i]));
//            }

//            // TODO: Add other uv-plane effects here.
//        }
//    }
//
//    // Direction dependent (image-plane) effects.
//    bool imgEffects = mask[DIRECTIONAL_GAIN] || mask[BEAM] || mask[IONOSPHERE];

//    if(imgEffects)
//    {
//        // Create source nodes for all selected sources.
//        vector<Source::Ptr> sources;
//        makeSources(sources, config.sources);
//        if(sources.size() != 1)
//        {
//            LOG_ERROR_STR("No direction, or more than one direction specified."
//                " A correction can only be applied for a single direction on"
//                " the sky");
//            return false;
//        }

//        // Create an AzEl node per station-source combination.
//        boost::multi_array<Expr, 2> azel;
//        if(mask[BEAM] || mask[IONOSPHERE])
//        {
//            makeAzElNodes(azel, sources);
//        }

//        // Create a directional gain node per station-source combination.
//        boost::multi_array<JonesExpr, 2> gainDirectional;
//        if(mask[DIRECTIONAL_GAIN])
//        {
//            makeDirectionalGainNodes(gainDirectional, config, sources);
//        }
//
//        // Create a dipole beam node per station0-source combination.
//        boost::multi_array<JonesExpr, 2> dipoleBeam;
//        if(mask[BEAM])
//        {
//            makeDipoleBeamNodes(dipoleBeam, config, azel);
//        }

//        // Create a MIM node per station-source combination.
//        boost::multi_array<JonesExpr, 2> ionosphere;
//        if(mask[IONOSPHERE])
//        {
//	  makeIonosphereNodes(ionosphere, config, azel);
//        }

//        // Accumulate the image-plane effects.
//        for(size_t i = 0; i < nStations; ++i)
//        {
//            if(mask[DIRECTIONAL_GAIN])
//            {
//                jJones[i] = jJones[i].isNull() ? gainDirectional[i][0]
//                    : JonesExpr(new JonesMul2(jJones[i],
//                        gainDirectional[i][0]));
//            }
//
//            if(mask[BEAM])
//            {
//                jJones[i] = jJones[i].isNull() ? dipoleBeam[i][0]
//                    : JonesExpr(new JonesMul2(jJones[i], dipoleBeam[i][0]));
//            }

//            if(mask[IONOSPHERE])
//            {
//                jJones[i] = jJones[i].isNull() ? ionosphere[i][0]
//                    : JonesExpr(new JonesMul2(jJones[i], ionosphere[i][0]));
//            }

//            // TODO: Add other image-plane effects here.
//        }
//    }
//
//    // Create an inverse J-Jones matrix for each station.
//    if(uvEffects || imgEffects)
//    {
//        for(size_t i = 0; i < nStations; ++i)
//        {
//            ASSERT(!jJones[i].isNull());
//            jJones[i] = new JonesInvert(jJones[i]);
//        }
//    }
//
//    // Create an expression tree for each baseline.
//    for(size_t i = 0; i < baselines.size(); ++i)
//    {
//        const baseline_t &baseline = baselines[i];
//
//        JonesExpr vdata(new JonesVisData(chunk, baseline));
//
//        if(uvEffects || imgEffects)
//        {
//            vdata = new JonesCMul3(jJones[baseline.first], vdata,
//                jJones[baseline.second]);
//        }
//
//        itsExpr[baseline] = vdata;
//    }
}

void Model::clear()
{
    itsExpr.clear();
    itsParms.clear();
    itsCache.clear();
    itsCache.clearStats();
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


vector<bool> Model::parseComponents(const vector<string> &components) const
{
    vector<bool> mask(N_ModelComponent, false);

    for(vector<string>::const_iterator it = components.begin();
        it != components.end();
        ++it)
    {
        if(*it == "BANDPASS")
        {
            mask[BANDPASS] = true;
        }
        else if(*it == "GAIN")
        {
            mask[GAIN] = true;
        }
        else if(*it == "DIRECTIONAL_GAIN")
        {
            mask[DIRECTIONAL_GAIN] = true;
        }
        else if(*it == "BEAM")
        {
            mask[BEAM] = true;
        }
        else if(*it == "IONOSPHERE")
        {
            mask[IONOSPHERE] = true;
        }
        else if(*it == "COND_NUM_FLAG")
        {
            mask[COND_NUM_FLAG] = true;
        }
        else
        {
            LOG_WARN_STR("Ignored unsupported model component \"" << *it
                << "\".");
        }
    }

    return mask;
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
        LOG_WARN_STR("Unable to construct source !!: " << source.getName());
        break;
    }

    return Source::Ptr();
}

void Model::makeStationUVW()
{
    itsStationUVW.resize(itsInstrument.stations.size());
    for(size_t i = 0; i < itsStationUVW.size(); ++i)
    {
        itsStationUVW[i].reset(new StatUVW(itsInstrument.stations[i],
            itsInstrument.position, itsPhaseRef));
    }
}

//void Model::makeAzElNodes(boost::multi_array<Expr, 2> &result,
//    const vector<Source::Ptr> &sources) const
//{
//    const size_t nSources = sources.size();
//    const size_t nStations = itsInstrument.stations.size();

//    // Create an AzEl node for each source-station combination.
//    result.resize(boost::extents[nStations][nSources]);
//    for(size_t i = 0; i < nStations; ++i)
//    {
//        for(size_t j = 0; j < nSources; ++j)
//        {
//            result[i][j] = new AzEl(itsInstrument.stations[i], sources[j]);
//        }
//    }
//}

void Model::makeStationShiftNodes(boost::multi_array<Expr<Vector<2> >::Ptr, 2> &result,
    const vector<unsigned int> &stations, const vector<Source::Ptr> &sources)
    const
{
    result.resize(boost::extents[stations.size()][sources.size()]);

    for(size_t j = 0; j < sources.size(); ++j)
    {
        LMN::Ptr lmn(new LMN(itsPhaseRef, sources[j]->getPosition()));

        for(size_t i = 0; i < stations.size(); ++i)
        {
            result[i][j].reset(new DFTPS(itsStationUVW[stations[i]], lmn));
        }
    }
}

//void Model::makeBandpassNodes(vector<JonesExpr> &result)
//{
//    const size_t nStations = itsInstrument.stations.size();

//    result.resize(nStations);
//    for(size_t i = 0; i < nStations; ++i)
//    {
//        Expr B11, B22;
//
//        const string &suffix = itsInstrument.stations[i].name;

//        B11 = makeExprParm(INSTRUMENT, "Bandpass:11:" + suffix);
//        B22 = makeExprParm(INSTRUMENT, "Bandpass:22:" + suffix);

//        result[i] = new Diag(B11, B22);
//    }
//}

//void Model::makeGainNodes(vector<JonesExpr> &result, const ModelConfig &config)
//{
//    string elem1(config.usePhasors ? "Ampl"  : "Real");
//    string elem2(config.usePhasors ? "Phase" : "Imag");

//    const size_t nStations = itsInstrument.stations.size();
//    result.resize(nStations);

//    for(size_t i = 0; i < nStations; ++i)
//    {
//        Expr J11, J12, J21, J22;
//
//        const string &suffix = itsInstrument.stations[i].name;

//        Expr J11_elem1 =
//            makeExprParm(INSTRUMENT, "Gain:11:" + elem1 + ":" + suffix);
//        Expr J11_elem2 =
//            makeExprParm(INSTRUMENT, "Gain:11:" + elem2 + ":" + suffix);
//        Expr J12_elem1 =
//            makeExprParm(INSTRUMENT, "Gain:12:" + elem1 + ":" + suffix);
//        Expr J12_elem2 =
//            makeExprParm(INSTRUMENT, "Gain:12:" + elem2 + ":" + suffix);
//        Expr J21_elem1 =
//            makeExprParm(INSTRUMENT, "Gain:21:" + elem1 + ":" + suffix);
//        Expr J21_elem2 =
//            makeExprParm(INSTRUMENT, "Gain:21:" + elem2 + ":" + suffix);
//        Expr J22_elem1 =
//            makeExprParm(INSTRUMENT, "Gain:22:" + elem1 + ":" + suffix);
//        Expr J22_elem2 =
//            makeExprParm(INSTRUMENT, "Gain:22:" + elem2 + ":" + suffix);

//        if(config.usePhasors)
//        {
//            J11 = new ExprAPToComplex(J11_elem1, J11_elem2);
//            J12 = new ExprAPToComplex(J12_elem1, J12_elem2);
//            J21 = new ExprAPToComplex(J21_elem1, J21_elem2);
//            J22 = new ExprAPToComplex(J22_elem1, J22_elem2);
//        }
//        else
//        {
//            J11 = new ExprToComplex(J11_elem1, J11_elem2);
//            J12 = new ExprToComplex(J12_elem1, J12_elem2);
//            J21 = new ExprToComplex(J21_elem1, J21_elem2);
//            J22 = new ExprToComplex(J22_elem1, J22_elem2);
//        }

//        result[i] = new JonesNode(J11, J12, J21, J22);
//    }
//}

//void Model::makeDirectionalGainNodes(boost::multi_array<JonesExpr, 2> &result,
//    const ModelConfig &config, const vector<Source::Ptr> &sources)
//{
//    string elem1(config.usePhasors ? "Ampl"  : "Real");
//    string elem2(config.usePhasors ? "Phase" : "Imag");

//    const size_t nStations = itsInstrument.stations.size();
//    const size_t nSources = sources.size();
//    result.resize(boost::extents[nStations][nSources]);

//    for(size_t i = 0; i < nStations; ++i)
//    {
//        for(size_t j = 0; j < nSources; ++j)
//        {
//            Expr J11, J12, J21, J22;
//
//            string suffix(itsInstrument.stations[i].name + ":"
//                + sources[j]->getName());

//            Expr J11_elem1 =
//                makeExprParm(INSTRUMENT, "Gain:11:" + elem1 + ":" + suffix);
//            Expr J11_elem2 =
//                makeExprParm(INSTRUMENT, "Gain:11:" + elem2 + ":" + suffix);
//            Expr J12_elem1 =
//                makeExprParm(INSTRUMENT, "Gain:12:" + elem1 + ":" + suffix);
//            Expr J12_elem2 =
//                makeExprParm(INSTRUMENT, "Gain:12:" + elem2 + ":" + suffix);
//            Expr J21_elem1 =
//                makeExprParm(INSTRUMENT, "Gain:21:" + elem1 + ":" + suffix);
//            Expr J21_elem2 =
//                makeExprParm(INSTRUMENT, "Gain:21:" + elem2 + ":" + suffix);
//            Expr J22_elem1 =
//                makeExprParm(INSTRUMENT, "Gain:22:" + elem1 + ":" + suffix);
//            Expr J22_elem2 =
//                makeExprParm(INSTRUMENT, "Gain:22:" + elem2 + ":" + suffix);

//            if(config.usePhasors)
//            {
//                J11 = new ExprAPToComplex(J11_elem1, J11_elem2);
//                J12 = new ExprAPToComplex(J12_elem1, J12_elem2);
//                J21 = new ExprAPToComplex(J21_elem1, J21_elem2);
//                J22 = new ExprAPToComplex(J22_elem1, J22_elem2);
//            }
//            else
//            {
//                J11 = new ExprToComplex(J11_elem1, J11_elem2);
//                J12 = new ExprToComplex(J12_elem1, J12_elem2);
//                J21 = new ExprToComplex(J21_elem1, J21_elem2);
//                J22 = new ExprToComplex(J22_elem1, J22_elem2);
//            }

//            result[i][j] = new JonesNode(J11, J12, J21, J22);
//        }
//    }
//}

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

//void Model::makeIonosphereNodes(boost::multi_array<JonesExpr, 2> &result,
//    const ModelConfig &config, const boost::multi_array<Expr, 2> &azel)
//{
//    ASSERT(azel.shape()[0] == itsInstrument.stations.size());
//
//    const size_t nStations = azel.shape()[0];
//    const size_t nSources = azel.shape()[1];

//    // Create reference Station.
//    Station reference = itsInstrument.stations[0];

//    IonoConfig::ConstPtr ionoConfig =
//      dynamic_pointer_cast<const IonoConfig>(config.ionoConfig);
//    ASSERT(ionoConfig);
//    // Get parameters.
//    uint rank = ionoConfig->rank+1;
//    //make sure rank is at least 1, which means a linear gradient over the field of view.
//    ASSERTSTR(rank>1,"Ionosphere rank should be at least 1 (linear gradient)");
//    // for the moment we do not include MIM:0:0 (i.e. absolute TEC)
//    uint NParms=(rank)*(rank)-1;
//    vector<Expr> MIMParms(NParms);
//    for(uint i = 0; i < rank; ++i)
//      for(uint j = 0; j < rank; ++j)
//	{
//	  if (i==0 && j==0) continue;
//	  ostringstream oss;
//	  oss << "MIM:" << j<<":"<<i;
//	  MIMParms[i*rank+j-1] = makeExprParm(INSTRUMENT, oss.str());
//	}

//    // Create a MIM node per (station, source) combination.
//    result.resize(boost::extents[nStations][nSources]);
//    for(size_t i = 0; i < nStations; ++i)
//    {
//        for(size_t j = 0; j < nSources; ++j)
//        {
//            Expr point(new PiercePoint(itsInstrument.stations[i], azel[i][j]));
//            result[i][j] = new MIM(point, MIMParms, reference);
//        }
//    }
//}

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
