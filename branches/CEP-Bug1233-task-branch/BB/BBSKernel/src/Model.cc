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

#include <BBSKernel/MNS/MeqPointSource.h>
#include <BBSKernel/MNS/MeqGaussianSource.h>

#include <BBSKernel/MNS/ExprParm.h>
#include <BBSKernel/MNS/MeqPhaseRef.h>
#include <BBSKernel/MNS/MeqPhaseShift.h>
#include <BBSKernel/MNS/MeqPointCoherency.h>
#include <BBSKernel/MNS/MeqJonesMul.h>
#include <BBSKernel/MNS/MeqJonesSum.h>

#include <BBSKernel/MNS/MeqLMN.h>
#include <BBSKernel/MNS/MeqDFTPS.h>
#include <BBSKernel/MNS/MeqJonesNode.h>
#include <BBSKernel/MNS/MeqJonesInvert.h>
#include <BBSKernel/MNS/MeqJonesCMul3.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <BBSKernel/MNS/MeqJonesExpr.h>

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

Model::Model(const Instrument &instrument, const casa::MDirection &phaseRef)
    : itsInstrument(instrument)
{
    // Store phase reference.
    itsPhaseRef = PhaseRef::ConstPointer(new PhaseRef(phaseRef));

    // Make UVW nodes for all stations.
    makeStationUvw();
}

bool Model::makeFwdExpressions(const ModelConfig &config,
    const vector<baseline_t> &baselines)
{
    // Remove all existing expressions.
    clearExpressions();

    // Create source nodes for all selected sources.
    vector<Source::Pointer> sources;
    sources.push_back(makeSource(SourceDescriptor("CasA", SourceDescriptor::POINT)));
//    sources.push_back(makeSource(SourceDescriptor("CygA", SourceDescriptor::POINT)));
//    makeSources(sources, config.sources);
    if(sources.empty())
    {
        LOG_ERROR_STR("Source selection is empty.");
        return false;
    }

    const size_t nSources = sources.size();
    const size_t nStations = itsStationUvw.size();

    cout << "nSources: " << nSources << endl;
    cout << "nStations: " << nStations << endl;

    // Create LMN nodes for all sources and coherence nodes for all point
    // sources.
    vector<Expr> lmn(nSources);
    map<size_t, JonesExpr> pointCoherences;

    for(size_t i = 0; i < nSources; ++i)
    {
        // Create LMN node.
        lmn[i] = new LMN(sources[i], itsPhaseRef);

        // A point source's coherence is independent of baseline UVW
        // coordinates. Therefore, they are constructed here and shared
        // between baselines.
        PointSource::ConstPointer pointSource =
            dynamic_pointer_cast<const PointSource>(sources[i]);

        if(pointSource)
        {
            // Create point source coherence.
            pointCoherences[i] = new PointCoherency(pointSource);
        }
    }

    // Direction independent (uv-plane) effects.
    vector<JonesExpr> gain;
    makeGainNodes(gain, config);

    // Direction dependent (image-plane) effects.
    boost::multi_array<Expr, 2>
        stationShift(boost::extents[nStations][nSources]);

    // Create a station shift node per station-source combination.
    for(size_t i = 0; i < nStations; ++i)
    {
        for(size_t j = 0; j < nSources; ++j)
        {
            stationShift[i][j] = new DFTPS(itsStationUvw[i], lmn[j]);
        }
    }
    
    // Create an expression tree for each baseline.
    for(size_t i = 0; i < baselines.size(); ++i)
    {
        const baseline_t &baseline = baselines[i];

        vector<JonesExpr> terms;
        for(size_t j = 0; j < nSources; ++j)
        {
            // Phase shift (incorporates geometry and fringe stopping).
            Expr shift(new PhaseShift(stationShift[baseline.first][j],
                stationShift[baseline.second][j]));

            JonesExpr coherence = pointCoherences[j];
            ASSERT(!coherence.isNull());

            // Phase shift the source coherence.
            coherence = new JonesMul(coherence, shift);

            terms.push_back(coherence);
        }

        // Sum all source coherences.
        JonesExpr sum;
        if(terms.size() == 1)
        {
            sum = terms.front();
        }
        else
        {
            sum = new JonesSum(terms);
        }

        sum = new JonesCMul3(gain[baseline.first], sum,
            gain[baseline.second]);

        itsExpressions[baseline] = sum;
    }

    return true;
}


/*
void Model::makeEquations(EquationType type, const ModelConfig &config,
    const vector<baseline_t> &baselines, PhaseRef *phaseRef,
    VisData::Pointer buffer)
{
    // Parse component names.
    vector<bool> mask(N_ModelComponent, false);
    for(vector<string>::const_iterator it = config.components.begin();
        it != config.components.end();
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
        else if(*it == "BEAM" && type == SIMULATE)
        {
            mask[BEAM] = true;
        }
        else
        {
            LOG_WARN_STR("Ignored unknown model component '" << *it << "'.");
        }
    }
    
    string part1(config.usePhasors ? "ampl:" : "real:");
    string part2(config.usePhasors ? "phase:" : "imag:");

    // Clear all equations.
    clearEquations();

    itsEquationType = type;

    // Make nodes for all specified sources (use all if none specified).
    if(config.sources.empty())
        makeSourceNodes(itsSourceList->getSourceNames(), phaseRef);
    else
        makeSourceNodes(config.sources, phaseRef);

    // Make baseline equations.
    size_t nStations = itsStationNodes.size();
    size_t nSources = itsSourceNodes.size();

    ASSERT(nStations > 0 && nSources > 0);

    vector<Expr> azel(nSources);
    vector<Expr> dft(nStations * nSources);
    vector<JonesExpr> bandpass, gain, dir_gain;
    vector<vector<JonesExpr> > beam;

    if(mask[BANDPASS])
    {
        bandpass.resize(nStations);
    }
    
    if(mask[GAIN])
    {
        gain.resize(nStations);
    }
        
    if(mask[DIRECTIONAL_GAIN])
    {
        dir_gain.resize(nStations * nSources);
    }

    if(mask[BEAM])
    {
        makeBeamNodes(config, instrumentDBase, parmGroup, beam);
    }

    for(size_t i = 0; i < nStations; ++i)
    {
        // Make a phase term per station per source.
        for(size_t j = 0; j < nSources; ++j)
            dft[i * nSources + j] = Expr(new DFTPS(itsLMNNodes[j],
                itsUVWNodes[i].get()));

        // Make a bandpass expression per station.
        if(mask[BANDPASS])
        {
            string suffix = itsStationNodes[i]->getName();

            Expr B11(ParmFunklet::create("bandpass:11:" + suffix,
                parmGroup, instrumentDBase));
            Expr B22(ParmFunklet::create("bandpass:22:" + suffix,
                parmGroup, instrumentDBase));

            bandpass[i] = new Diag(B11, B22);
        }

        // Make a complex gain expression per station and possibly per source.
        if(mask[GAIN])
        {
            Expr J11, J12, J21, J22;
            string suffix = itsStationNodes[i]->getName();

            // Make a J-jones expression per station
            Expr J11_part1(ParmFunklet::create("gain:11:" + part1
                + suffix, parmGroup, instrumentDBase));
            Expr J11_part2(ParmFunklet::create("gain:11:" + part2
                + suffix, parmGroup, instrumentDBase));
            Expr J12_part1(ParmFunklet::create("gain:12:" + part1
                + suffix, parmGroup, instrumentDBase));
            Expr J12_part2(ParmFunklet::create("gain:12:" + part2
                + suffix, parmGroup, instrumentDBase));
            Expr J21_part1(ParmFunklet::create("gain:21:" + part1
                + suffix, parmGroup, instrumentDBase));
            Expr J21_part2(ParmFunklet::create("gain:21:" + part2
                + suffix, parmGroup, instrumentDBase));
            Expr J22_part1(ParmFunklet::create("gain:22:" + part1
                + suffix, parmGroup, instrumentDBase));
            Expr J22_part2(ParmFunklet::create("gain:22:" + part2
                + suffix, parmGroup, instrumentDBase));

            if(mask[config.usePhasors])
            {
                J11 = new ExprAPToComplex(J11_part1, J11_part2);
                J12 = new ExprAPToComplex(J12_part1, J12_part2);
                J21 = new ExprAPToComplex(J21_part1, J21_part2);
                J22 = new ExprAPToComplex(J22_part1, J22_part2);
            }
            else
            {
                J11 = new ExprToComplex(J11_part1, J11_part2);
                J12 = new ExprToComplex(J12_part1, J12_part2);
                J21 = new ExprToComplex(J21_part1, J21_part2);
                J22 = new ExprToComplex(J22_part1, J22_part2);
            }

            gain[i] = new JonesNode(J11, J12, J21, J22);
//            inv_gain[i] = new JonesInvert(gain[i]);
        }
        
        if(mask[DIRECTIONAL_GAIN])
        {
            // Make a J-jones expression per station per source. Eventually,
            // patches of several sources will be supported as well.
            for(size_t j = 0; j < nSources; ++j)
            {
                Expr J11, J12, J21, J22;
                string suffix = itsStationNodes[i]->getName() + ":"
                    + itsSourceNodes[j]->getName();

                Expr J11_part1(ParmFunklet::create("gain:11:" + part1
                    + suffix, parmGroup, instrumentDBase));
                Expr J11_part2(ParmFunklet::create("gain:11:" + part2
                    + suffix, parmGroup, instrumentDBase));
                Expr J12_part1(ParmFunklet::create("gain:12:" + part1
                    + suffix, parmGroup, instrumentDBase));
                Expr J12_part2(ParmFunklet::create("gain:12:" + part2
                    + suffix, parmGroup, instrumentDBase));
                Expr J21_part1(ParmFunklet::create("gain:21:" + part1
                    + suffix, parmGroup, instrumentDBase));
                Expr J21_part2(ParmFunklet::create("gain:21:" + part2
                    + suffix, parmGroup, instrumentDBase));
                Expr J22_part1(ParmFunklet::create("gain:22:" + part1
                    + suffix, parmGroup, instrumentDBase));
                Expr J22_part2(ParmFunklet::create("gain:22:" + part2
                    + suffix, parmGroup, instrumentDBase));

                if(mask[config.usePhasors])
                {
                    J11 = new ExprAPToComplex(J11_part1, J11_part2);
                    J12 = new ExprAPToComplex(J12_part1, J12_part2);
                    J21 = new ExprAPToComplex(J21_part1, J21_part2);
                    J22 = new ExprAPToComplex(J22_part1, J22_part2);
                }
                else
                {
                    J11 = new ExprToComplex(J11_part1, J11_part2);
                    J12 = new ExprToComplex(J12_part1, J12_part2);
                    J21 = new ExprToComplex(J21_part1, J21_part2);
                    J22 = new ExprToComplex(J22_part1, J22_part2);
                }

                dir_gain[i * nSources + j] =
                    new JonesNode(J11, J12, J21, J22);

                // Gain correction is always performed with respect to the
                // direction of the first source (patch).
//                if(j == 0)
//                    inv_gain[i] = new JonesInvert(gain[i * nSources]);
            }
        }
    }

    if(type == CORRECT)
    {
        ASSERTSTR(mask[GAIN] || mask[DIRECTIONAL_GAIN] || mask[BANDPASS],
            "Need at least one of GAIN, DIRECTIONAL_GAIN, BANDPASS to"
            " correct for.");
            
        vector<JonesExpr> inv_bandpass, inv_gain, inv_dir_gain;
        
        if(mask[BANDPASS])
        {
            inv_bandpass.resize(nStations);
            for(size_t i = 0; i < nStations; ++i)
            {
                inv_bandpass[i] = new JonesInvert(bandpass[i]);
            }
        }

        if(mask[GAIN])
        {
            inv_gain.resize(nStations);
            for(size_t i = 0; i < nStations; ++i)
            {
                inv_gain[i] = new JonesInvert(gain[i]);
            }
        }
        
        if(mask[DIRECTIONAL_GAIN])
        {
            inv_dir_gain.resize(nStations);
            for(size_t i = 0; i < nStations; ++i)
            {
                // Always correct for the first source (direction).
                inv_dir_gain[i] = new JonesInvert(dir_gain[i * nSources]);
            }
        }

        for(vector<baseline_t>::const_iterator it = baselines.begin();
            it != baselines.end();
            ++it)
        {
            const baseline_t &baseline = *it;
            JonesExpr vis = new JonesVisData(buffer, baseline);
            
            if(mask[BANDPASS])
            {
                vis = new JonesCMul3(inv_bandpass[baseline.first], vis,
                    inv_bandpass[baseline.second]);
            }

            if(mask[GAIN])
            {
                vis = new JonesCMul3(inv_gain[baseline.first], vis,
                    inv_gain[baseline.second]);
            }
            
            if(mask[DIRECTIONAL_GAIN])
            {
                vis = new JonesCMul3(inv_dir_gain[baseline.first], vis,
                    inv_dir_gain[baseline.second]);
            }

            itsExpressions[baseline] = vis;
        }
    }
    else
    {
        for(vector<baseline_t>::const_iterator it = baselines.begin();
            it != baselines.end();
            ++it)
        {
            const baseline_t &baseline = *it;

            vector<JonesExpr> terms;
            for(size_t j = 0; j < nSources; ++j)
            {
                // Phase shift (incorporates geometry and fringe stopping).
                Expr shift
                    (new PhaseShift(dft[baseline.first * nSources + j],
                    dft[baseline.second * nSources + j]));

                JonesExpr sourceExpr;

                PointSource *source =
                    dynamic_cast<PointSource*>(itsSourceNodes[j]);

                if(source)
                {
                    // Point source.
                    sourceExpr = new PointCoherency(source);
                }
                else
                {
                    GaussianSource *source = dynamic_cast<GaussianSource*>(itsSourceNodes[j]);
                    ASSERT(source);
                    sourceExpr = new GaussianCoherency(source, itsUVWNodes[baseline.first].get(), itsUVWNodes[baseline.second].get());
                }

                // Phase shift the source coherency.
                sourceExpr = new JonesMul(sourceExpr, shift);
                
                // Apply image plane effects if required.
                if(mask[BEAM])
                {
                    sourceExpr = new JonesCMul3
                        (beam[baseline.first][j],
                        sourceExpr,
                        beam[baseline.second][j]);
                }

                if(mask[DIRECTIONAL_GAIN])
                {
                    sourceExpr = new JonesCMul3
                        (dir_gain[baseline.first * nSources + j],
                        sourceExpr,
                        dir_gain[baseline.second * nSources + j]);
                }
                
                terms.push_back(sourceExpr);
            }

            JonesExpr sum;
            if(terms.size() == 1)
                sum = terms.front();
            else
                sum = JonesExpr(new JonesSum(terms));

            // Apply UV-plane effects if required.
            if(mask[GAIN])
            {
                sum = new JonesCMul3(gain[baseline.first], sum,
                    gain[baseline.second]);
            }

            if(mask[BANDPASS])
            {
                sum = new JonesCMul3(bandpass[baseline.first], sum,
                    bandpass[baseline.second]);
            }
            
            itsExpressions[baseline] = sum;
        }
    }
}
*/    

void Model::clearExpressions()
{
    itsExpressions.clear();
    itsParms.clear();
}

void Model::setPerturbedParms(const ParmGroup &perturbed)
{
    ParmGroup::const_iterator pertIt = perturbed.begin();
    ParmGroup::const_iterator pertItEnd = perturbed.end();
    while(pertIt != pertItEnd)
    {
        map<uint, Expr>::iterator parmIt = itsParms.find(*pertIt);
        if(parmIt != itsParms.end())
        {
            ExprParm *parmPtr = static_cast<ExprParm*>(parmIt->second.getPtr());
            ASSERT(parmPtr);
            parmPtr->setPValueFlag();
        }
        ++pertIt;
    }
}

void Model::clearPerturbedParms()
{
    map<uint, Expr>::iterator it = itsParms.begin();
    map<uint, Expr>::iterator itEnd = itsParms.end();
    while(it != itEnd)
    {
        ExprParm *parmPtr = static_cast<ExprParm*>(it->second.getPtr());
        ASSERT(parmPtr);
        parmPtr->clearPValueFlag();
        ++it;
    }
}

ParmGroup Model::getPerturbedParms() const
{
    ParmGroup result;
    
    map<uint, Expr>::const_iterator it = itsParms.begin();
    map<uint, Expr>::const_iterator itEnd = itsParms.end();
    while(it != itEnd)
    {
        const ExprParm *parmPtr =
            static_cast<const ExprParm*>(it->second.getPtr());
        ASSERT(parmPtr);
        if(parmPtr->getPValueFlag())
        {
            result.insert(it->first);
        }
        ++it;
    }

    return result;
}

ParmGroup Model::getParms() const
{
    ParmGroup result;
    
    map<uint, Expr>::const_iterator it = itsParms.begin();
    map<uint, Expr>::const_iterator itEnd = itsParms.end();
    while(it != itEnd)
    {
        result.insert(it->first);
        ++it;
    }

    return result;
}

void Model::precalculate(const Request &request)
{
    if(itsExpressions.empty())
    {
        return;
    }
    
    // First clear the levels of all nodes in the tree.
    for(map<baseline_t, JonesExpr>::iterator it = itsExpressions.begin();
        it != itsExpressions.end();
        ++it)
    {
        JonesExpr &expr = it->second;
        ASSERT(!expr.isNull());
        expr.clearDone();
    }

    // Now set the levels of all nodes in the tree.
    // The root nodes have level 0; child nodes have level 1 or higher.
    int nrLev = -1;
    for(map<baseline_t, JonesExpr>::iterator it = itsExpressions.begin();
        it != itsExpressions.end();
        ++it)
    {
        JonesExpr &expr = it->second;
        ASSERT(!expr.isNull());
        nrLev = std::max(nrLev, expr.setLevel(0));
    }
    ++nrLev;
    ASSERT(nrLev > 0);

    // Find the nodes to be precalculated at each level.
    // That is not needed for the root nodes (the baselines).
    // The nodes used by the baselines are always precalculated (even if
    // having one parent).
    // It may happen that a station is used by only one baseline. Calculating
    // such a baseline is much more work if the station was not precalculated.
    vector<vector<ExprRep*> > precalcNodes(nrLev);
    for(int level = 1; level < nrLev; ++level)
    {
        for(map<baseline_t, JonesExpr>::iterator it = itsExpressions.begin();
            it != itsExpressions.end();
            ++it)
        {
            JonesExpr &expr = it->second;
            ASSERT(!expr.isNull());
            expr.getCachingNodes(precalcNodes[level], level, false);
        }
    }

    LOG_TRACE_FLOW_STR("#levels=" << nrLev);
    for(int i = 0; i < nrLev; ++i)
    {
        LOG_TRACE_FLOW_STR("#expr on level " << i << " is "
            << precalcNodes[i].size());
    }

#pragma omp parallel
    {
        // Loop through expressions to be precalculated.
        // At each level the expressions can be executed in parallel.
        // Level 0 is formed by itsExpr which are not calculated here.
        for(size_t level = precalcNodes.size(); --level > 0;)
        {
            vector<ExprRep*> &nodes = precalcNodes[level];
            
            if(!nodes.empty())
            {
#pragma omp for schedule(dynamic)
                // NOTE: OpenMP will only parallelize for loops that use type
                // 'int' for the loop counter.
                for(int i = 0; i < static_cast<int>(nodes.size()); ++i)
                {
                    nodes[i]->precalculate(request);
                }
            }
        }
    } // omp parallel
}

JonesResult Model::evaluate(const baseline_t &baseline,
    const Request &request)
{
    map<baseline_t, JonesExpr>::iterator it = itsExpressions.find(baseline);
    ASSERTSTR(it != itsExpressions.end(), "Result requested for unknown"
        " baseline " << baseline.first << " - " << baseline.second);

    return it->second.getResult(request);
}

/*
void Model::makeStationNodes(const Instrument &instrument,
    const PhaseRef &phaseRef)
{
    // Workaround to facilitate comparison with earlier versions of BBS
    // and with Tree.
    casa::Quantum<casa::Vector<casa::Double> > angles =
        phaseRef.direction().getAngle();
    casa::MDirection phaseRef2(casa::MVDirection(angles.getBaseValue()(0),
        angles.getBaseValue()(1)), casa::MDirection::J2000);
    LOG_WARN_STR("Superfluous conversion of phase center from/to"
        " casa::MDirection to allow comparison with earlier version of BBS and"
        " with Tree. Should be removed after validation.");

    itsStationNodes.resize(instrument.stations.size());
    itsUVWNodes.resize(instrument.stations.size());
    for(size_t i = 0; i < instrument.stations.size(); ++i)
    {
        const casa::MVPosition &position =
            instrument.stations[i].position.getValue();

        ParmSingle *x = new ParmSingle("position:x:" +
            instrument.stations[i].name, position(0));
        ParmSingle *y = new ParmSingle("position:y:" +
            instrument.stations[i].name, position(1));
        ParmSingle *z = new ParmSingle("position:z:" +
            instrument.stations[i].name, position(2));

        itsStationNodes[i].reset(new Station(x, y, z,
            instrument.stations[i].name));

        itsUVWNodes[i].reset(new StatUVW(itsStationNodes[i].get(),
            phaseRef2, instrument.position));
    }
}
*/    


/*
void Model::makeSourceNodes(const vector<string> &names, PhaseRef *phaseRef)
{
    itsSourceNodes.resize(names.size());
    itsLMNNodes.resize(names.size());
    for(size_t i = 0; i < names.size(); ++i)
    {
        Source *source = itsSourceList->getSource(names[i]);
        itsSourceNodes[i] = source;

        // Create an LMN node for the source.
        itsLMNNodes[i] = new LMN(source);
        itsLMNNodes[i]->setPhaseRef(phaseRef);
    }
}
*/    


/*
void Model::setStationUVW(const Instrument &instrument, VisData::Pointer buffer)
{
    const size_t nStations = instrument.stations.size();
    vector<bool> statDone(nStations);
    vector<double> statUVW(3 * nStations);

    const VisDimensions &dims = buffer->getDimensions();
    const Grid &grid = dims.getGrid();
    const vector<baseline_t> &baselines = dims.getBaselines();
    
    // Step through the MS by timeslot.
    for (size_t tslot = 0; tslot < grid[TIME]->size(); ++tslot)
    {
        const double time = grid[TIME]->center(tslot);
        fill(statDone.begin(), statDone.end(), false);

        // Set UVW of first station used to 0 (UVW coordinates are relative!).
        size_t station0 = baselines[0].first;
        statUVW[3 * station0] = 0.0;
        statUVW[3 * station0 + 1] = 0.0;
        statUVW[3 * station0 + 2] = 0.0;
        itsUVWNodes[station0]->set(time, 0.0, 0.0, 0.0);
        statDone[station0] = true;

        size_t nDone;
        do
        {
            nDone = 0;
            for(size_t idx = 0; idx < baselines.size(); ++idx)
            {
                // If the contents of the UVW column is uninitialized, skip
                // it.
                if(buffer->tslot_flag[idx][tslot] & VisData::UNAVAILABLE)
                {
                    continue;
                }
                
                size_t statA = baselines[idx].first;
                size_t statB = baselines[idx].second;
                if(statDone[statA] && !statDone[statB])
                {
                    statUVW[3 * statB] =
                        buffer->uvw[idx][tslot][0] - statUVW[3 * statA];
                    statUVW[3 * statB + 1] =
                        buffer->uvw[idx][tslot][1] - statUVW[3 * statA + 1];
                    statUVW[3 * statB + 2] =
                        buffer->uvw[idx][tslot][2] - statUVW[3 * statA + 2];
                    statDone[statB] = true;
                    itsUVWNodes[statB]->set(time, statUVW[3 * statB],
                        statUVW[3 * statB + 1], statUVW[3 * statB + 2]);
                    ++nDone;
                }
                else if(statDone[statB] && !statDone[statA])
                {
                    statUVW[3 * statA] =
                        statUVW[3 * statB] - buffer->uvw[idx][tslot][0];
                    statUVW[3 * statA + 1] =
                        statUVW[3 * statB + 1] - buffer->uvw[idx][tslot][1];
                    statUVW[3 * statA + 2] =
                        statUVW[3 * statB + 2] - buffer->uvw[idx][tslot][2];
                    statDone[statA] = true;
                    itsUVWNodes[statA]->set(time, statUVW[3 * statA],
                        statUVW[3 * statA + 1], statUVW[3 * statA + 2]);
                    ++nDone;
                }
            }
        }
        while(nDone > 0);
        
        for(size_t i = 0; i < statDone.size(); ++i)
        {
            ASSERTSTR(statDone[i], "UVW of station " << i << " could not be"
                " determined for timeslot " << tslot);
        }
    }
}
*/    
    

/*
void Model::makeBeamNodes(const ModelConfig &config,
    LOFAR::ParmDB::ParmDB *db, ParmGroup &group,
    vector<vector<JonesExpr> > &result) const
{
    ASSERT(config.beamConfig);
    ASSERT(config.beamConfig->type() == "HamakerDipole"
        || config.beamConfig->type() == "YatawattaDipole");
    
    // Create AzEl node for each source-station combination.
    vector<Expr> azel(itsSourceNodes.size());
    for(size_t i = 0; i < itsSourceNodes.size(); ++i)
    {
        // TODO: This code uses station 0 as reference position on earth
        // (see global_model.py in EJones_HBA). Is this accurate enough?
        azel[i] =
            new AzEl(*(itsSourceNodes[i]), *(itsStationNodes[0].get()));
    }

    if(config.beamConfig->type() == "HamakerDipole")
    {
        HamakerDipoleConfig::ConstPointer beamConfig =
            dynamic_pointer_cast<const HamakerDipoleConfig>(config.beamConfig);
        ASSERT(beamConfig);
        
        // Read beam model coefficients.
        BeamCoeff coeff = readBeamCoeffFile(beamConfig->coeffFile);

        // Create a beam node for each source-station combination.
        result.resize(itsStationNodes.size());
        for(size_t i = 0; i < itsStationNodes.size(); ++i)
        {
            // Get dipole orientation.
            Expr orientation(ParmFunklet::create("orientation:"
                + itsStationNodes[i]->getName(), group, db));

            result[i].resize(itsSourceNodes.size());
            for(size_t j = 0; j < itsSourceNodes.size(); ++j)
            {
                result[i][j] = new NumericalDipoleBeam(coeff, azel[j],
                    orientation);
            }
        }
    }
    else if(config.beamConfig->type() == "YatawattaDipole")
    {
        YatawattaDipoleConfig::ConstPointer beamConfig =
            dynamic_pointer_cast<const YatawattaDipoleConfig>
                (config.beamConfig);
        ASSERT(beamConfig);

        // TODO: Where is this scale factor coming from (see global_model.py
        // in EJones_droopy_comp and EJones_HBA)?
//        const double scaleFactor = options[DIPOLE_BEAM_LBA] ? 1.0 / 88.0
//            : 1.0 / 600.0;

        // Create a beam node for each source-station combination.
        result.resize(itsStationNodes.size());
        for(size_t i = 0; i < itsStationNodes.size(); ++i)
        {
            // Get dipole orientation.
            Expr orientation(ParmFunklet::create("orientation:"
                + itsStationNodes[i]->getName(), group, db));

            result[i].resize(itsSourceNodes.size());
            for(size_t j = 0; j < itsSourceNodes.size(); ++j)
            {
//                result[i][j] = new DipoleBeam(azel[j], 1.706, 1.38,
//                    casa::C::pi / 4.001, -casa::C::pi_4);
                result[i][j] =
                    new DipoleBeamExternal(beamConfig->moduleTheta,
                        beamConfig->modulePhi, azel[j], orientation, 1.0);
            }
        }
    }
    else
    {
        ASSERT(false);
    }
}
*/    

void Model::makeGainNodes(vector<JonesExpr> &result, const ModelConfig &config,
    bool inverse)
{
    string elem1(config.usePhasors ? "Ampl"  : "Real");
    string elem2(config.usePhasors ? "Phase" : "Imag");

    const size_t nStations = itsInstrument.stations.size();
    result.resize(nStations);

    for(size_t i = 0; i < nStations; ++i)
    {
        Expr J11, J12, J21, J22;
        
        string suffix(itsInstrument.stations[i].name);

        Expr J11_elem1 =
            makeExprParm(INSTRUMENT, "Gain:11:" + elem1 + ":" + suffix);
        Expr J11_elem2 =
            makeExprParm(INSTRUMENT, "Gain:11:" + elem2 + ":" + suffix);
        Expr J12_elem1 =
            makeExprParm(INSTRUMENT, "Gain:12:" + elem1 + ":" + suffix);
        Expr J12_elem2 =
            makeExprParm(INSTRUMENT, "Gain:12:" + elem2 + ":" + suffix);
        Expr J21_elem1 =
            makeExprParm(INSTRUMENT, "Gain:21:" + elem1 + ":" + suffix);
        Expr J21_elem2 =
            makeExprParm(INSTRUMENT, "Gain:21:" + elem2 + ":" + suffix);
        Expr J22_elem1 =
            makeExprParm(INSTRUMENT, "Gain:22:" + elem1 + ":" + suffix);
        Expr J22_elem2 =
            makeExprParm(INSTRUMENT, "Gain:22:" + elem2 + ":" + suffix);

        if(config.usePhasors)
        {
            J11 = new ExprAPToComplex(J11_elem1, J11_elem2);
            J12 = new ExprAPToComplex(J12_elem1, J12_elem2);
            J21 = new ExprAPToComplex(J21_elem1, J21_elem2);
            J22 = new ExprAPToComplex(J22_elem1, J22_elem2);
        }
        else
        {
            J11 = new ExprToComplex(J11_elem1, J11_elem2);
            J12 = new ExprToComplex(J12_elem1, J12_elem2);
            J21 = new ExprToComplex(J21_elem1, J21_elem2);
            J22 = new ExprToComplex(J22_elem1, J22_elem2);
        }

        result[i] = new JonesNode(J11, J12, J21, J22);
        if(inverse)
        {
            result[i] = new JonesInvert(result[i]);
        }
    }
}

void Model::makeDirectionalGainNodes(boost::multi_array<JonesExpr, 2> &result,
    const ModelConfig &config, const vector<Source::Pointer> &sources,
    bool inverse)
{
    string elem1(config.usePhasors ? "Ampl"  : "Real");
    string elem2(config.usePhasors ? "Phase" : "Imag");

    const size_t nStations = itsInstrument.stations.size();
    const size_t nSources = sources.size();
    result.resize(boost::extents[nStations][nSources]);

    for(size_t i = 0; i < nStations; ++i)
    {
        for(size_t j = 0; j < nSources; ++j)
        {
            Expr J11, J12, J21, J22;
            
            string suffix(itsInstrument.stations[i].name + ":"
                + sources[j]->getName());

            Expr J11_elem1 =
                makeExprParm(INSTRUMENT, "Gain:11:" + elem1 + ":" + suffix);
            Expr J11_elem2 =
                makeExprParm(INSTRUMENT, "Gain:11:" + elem2 + ":" + suffix);
            Expr J12_elem1 =
                makeExprParm(INSTRUMENT, "Gain:12:" + elem1 + ":" + suffix);
            Expr J12_elem2 =
                makeExprParm(INSTRUMENT, "Gain:12:" + elem2 + ":" + suffix);
            Expr J21_elem1 =
                makeExprParm(INSTRUMENT, "Gain:21:" + elem1 + ":" + suffix);
            Expr J21_elem2 =
                makeExprParm(INSTRUMENT, "Gain:21:" + elem2 + ":" + suffix);
            Expr J22_elem1 =
                makeExprParm(INSTRUMENT, "Gain:22:" + elem1 + ":" + suffix);
            Expr J22_elem2 =
                makeExprParm(INSTRUMENT, "Gain:22:" + elem2 + ":" + suffix);

            if(config.usePhasors)
            {
                J11 = new ExprAPToComplex(J11_elem1, J11_elem2);
                J12 = new ExprAPToComplex(J12_elem1, J12_elem2);
                J21 = new ExprAPToComplex(J21_elem1, J21_elem2);
                J22 = new ExprAPToComplex(J22_elem1, J22_elem2);
            }
            else
            {
                J11 = new ExprToComplex(J11_elem1, J11_elem2);
                J12 = new ExprToComplex(J12_elem1, J12_elem2);
                J21 = new ExprToComplex(J21_elem1, J21_elem2);
                J22 = new ExprToComplex(J22_elem1, J22_elem2);
            }

            result[i][j] = new JonesNode(J11, J12, J21, J22);
            if(inverse)
            {
                result[i][j] = new JonesInvert(result[i][j]);
            }   
        }
    }
}

/*
BeamCoeff Model::readBeamCoeffFile(const string &filename) const
{
    LOG_DEBUG_STR("Reading beam coefficients...");

    // Open file.
    ifstream in(filename.c_str());
    if(!in)
    {
        THROW(BBSKernelException, "Unable to open file: " << filename << ".");
    }

    // Read file header.
    string header, token0, token1, token2, token3, token4, token5;
    getline(in, header);
    
    size_t nElements, nHarmonics, nPowerTime, nPowerFreq;
    double freqAvg, freqRange;
    
    istringstream iss(header);
    iss >> token0 >> nElements >> token1 >> nHarmonics >> token2 >> nPowerTime
        >> token3 >> nPowerFreq >> token4 >> freqAvg >> token5 >> freqRange;

    if(!in || !iss || token0 != "d" || token1 != "k" || token2 != "pwrT"
        || token3 != "pwrF" || token4 != "freqAvg" || token5 != "freqRange")
    {
        THROW(BBSKernelException, "Unable to parse header");
    }

    if(nElements * nHarmonics * nPowerTime * nPowerFreq == 0)
    {
        THROW(BBSKernelException, "The number of coefficients should be larger"
            " than zero.");
    }

    LOG_DEBUG_STR("nElements: " << nElements << " nHarmonics: " << nHarmonics
        << " nPowerTime: " << nPowerTime << " nPowerFreq: " << nPowerFreq);

    // Allocate coefficient matrix.
    shared_ptr<boost::multi_array<dcomplex, 4> > coeff
        (new boost::multi_array<dcomplex, 4>(boost::extents[nElements]
            [nHarmonics][nPowerTime][nPowerFreq]));

    size_t nCoeff = 0;
    while(in.good())
    {
        // Read line from file.
        string line;
        getline(in, line);

        // Skip lines that contain only whitespace.
        if(line.find_last_not_of(" ") == string::npos)
        {
            continue;
        }

        // Parse line. 
        size_t element, harmonic, powerTime, powerFreq;
        double re, im;
       
        iss.str(line);
        iss >> element >> harmonic >> powerTime >> powerFreq >> re >> im;
        
        if(!iss || element >= nElements || harmonic >= nHarmonics
            || powerTime >= nPowerTime || powerFreq >= nPowerFreq)
        {
            THROW(BBSKernelException, "Error reading file.");
        }
        
        // Store coefficient.
        (*coeff)[element][harmonic][powerTime][powerFreq] =
            makedcomplex(re, im);

        // Update coefficient counter.
        ++nCoeff;
    }

    if(!in.eof())
    {
        THROW(BBSKernelException, "Error reading file.");
    }

    if(nCoeff != nElements * nHarmonics * nPowerTime * nPowerFreq)
    {
        THROW(BBSKernelException, "Number of coefficients in header does not"
            " match number of coefficients in file.");
    }

    return BeamCoeff(freqAvg, freqRange, coeff);
}
*/    

Expr Model::makeExprParm(uint category, const string &name)
{
    ParmProxy::ConstPointer proxy(ParmManager::instance().get(category, name));

    pair<map<uint, Expr>::const_iterator, bool> status =
        itsParms.insert(make_pair(proxy->getId(), Expr(new ExprParm(proxy))));

    return status.first->second;
}

void Model::makeStationUvw()
{
    const vector<Station> &stations = itsInstrument.stations;

    itsStationUvw.resize(stations.size());
    for(size_t i = 0; i < stations.size(); ++i)
    {
        itsStationUvw[i] = StatUVW::ConstPointer(new StatUVW(stations[i].name,
            stations[i].position, itsInstrument.position, itsPhaseRef));
    }
}

/*
void Model::makeSources(vector<Source::Pointer> &result, ParmGroup &group,
    vector<string> names) const
{
    result.clear();

    if(names.empty())
    {
        // By default include all available sources.
        SourceList::ContainerT::const_iterator it = itsSourceList.begin();
        while(it != itsSourceList.end())
        {
            try
            {
                ParmGroup tmp;
                result.push_back(makeSource(*it, tmp));
                copy(tmp.begin(), tmp.end(), std::inserter(group,
                    group.begin()));
            }
            catch(Exception &ex)
            {
                LOG_WARN_STR("Unable to initialize source: " << it->name
                    << " (" << ex.what() << "); ignored.");
            }

            ++it;
        }
    }
    else
    {
        // Only consider unique names.
        sort(names.begin(), names.end());
        vector<string>::const_iterator nameEnd = unique(names.begin(),
            names.end());
        vector<string>::const_iterator nameIt = names.begin();
        
        while(nameIt != nameEnd)
        {
            SourceList::ContainerT::const_iterator srcIt =
                itsSourceList.find(*nameIt);

            if(srcIt != itsSourceList.end())
            {
                try
                {
                    ParmGroup tmp;
                    result.push_back(makeSource(*srcIt, tmp));
                    copy(tmp.begin(), tmp.end(), std::inserter(group,
                        group.begin()));
                }
                catch(Exception &ex)
                {
                    LOG_WARN_STR("Unable to initialize source: " << *nameIt
                        << " (" << ex.what() << "); ignored.");
                }
            }
            else
            {
                LOG_WARN_STR("Unknown source: " << *nameIt << "; ignored.");
                continue;
            }
            
            ++nameIt;
        }
    }
}
*/

Source::Pointer Model::makeSource(const SourceDescriptor &source)
{
    cout << "Creating source: " << source.name << endl;
    switch(source.type)
    {
    case SourceDescriptor::POINT:
        {
            Expr ra(makeExprParm(SKY, "Ra:" + source.name));
            Expr dec(makeExprParm(SKY, "Dec:" + source.name));
            Expr i(makeExprParm(SKY, "I:" + source.name));
            Expr q(makeExprParm(SKY, "Q:" + source.name));
            Expr u(makeExprParm(SKY, "U:" + source.name));
            Expr v(makeExprParm(SKY, "V:" + source.name));

            return PointSource::Pointer(new PointSource(source.name,
                ra, dec, i, q, u, v));
        }
        break;
    /*
    case Source::GAUSSIAN:
        {
            Expr ra(new ExprParm(ParmManager::instance().get(SKY, "Ra:"
                + source.name, group)));
            Expr dec(new ExprParm(ParmManager::instance().get(SKY, "Dec:"
                + source.name, group)));
            Expr i(new ExprParm(ParmManager::instance().get(SKY, "I:"
                + source.name, group)));
            Expr q(new ExprParm(ParmManager::instance().get(SKY, "Q:"
                + source.name, group)));
            Expr u(new ExprParm(ParmManager::instance().get(SKY, "U:"
                + source.name, group)));
            Expr v(new ExprParm(ParmManager::instance().get(SKY, "V:"
                + source.name, group)));
            Expr maj(new ExprParm(ParmManager::instance().get(SKY, "Major:"
                + source.name, group)));
            Expr min(new ExprParm(ParmManager::instance().get(SKY, "Minor:"
                + source.name, group)));
            Expr phi(new ExprParm(ParmManager::instance().get(SKY, "Phi:"
                + source.name, group)));
         
            return 
                GaussianSource::Pointer(new GaussianSource(source.name,
                    ra, dec, i, q, u, v, maj, min, phi));
        }
        break;
    */        
    default:
        break;
    }
    THROW(BBSKernelException, "Unable to construct source: " << source.name);
}

} // namespace BBS
} // namespace LOFAR
