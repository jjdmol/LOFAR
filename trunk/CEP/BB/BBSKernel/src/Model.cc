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

#include <BBSKernel/MNS/MeqPhaseRef.h>
#include <BBSKernel/MNS/MeqParmFunklet.h>
#include <BBSKernel/MNS/MeqParmSingle.h>
#include <BBSKernel/MNS/MeqDiag.h>
#include <BBSKernel/MNS/MeqJonesInvert.h>
#include <BBSKernel/MNS/MeqPhaseShift.h>
#include <BBSKernel/MNS/MeqPointCoherency.h>
#include <BBSKernel/MNS/MeqGaussianCoherency.h>
#include <BBSKernel/MNS/MeqJonesMul.h>
#include <BBSKernel/MNS/MeqJonesCMul3.h>
#include <BBSKernel/MNS/MeqJonesSum.h>
#include <BBSKernel/MNS/MeqJonesVisData.h>
#include <BBSKernel/MNS/MeqAzEl.h>
//include <BBSKernel/MNS/MeqDipoleBeam.h>
#include <BBSKernel/MNS/MeqDipoleBeamExternal.h>
#include <BBSKernel/MNS/MeqNumericalDipoleBeam.h>

#include <BBSKernel/MNS/MeqStation.h>
#include <BBSKernel/MNS/MeqStatUVW.h>
#include <BBSKernel/MNS/MeqLMN.h>
#include <BBSKernel/MNS/MeqDFTPS.h>
#include <BBSKernel/MNS/MeqJonesNode.h>
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
#include <Common/lofar_smartptr.h>

#include <measures/Measures/MDirection.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Arrays/Vector.h>

#include <ParmDB/ParmDB.h>

namespace LOFAR
{
namespace BBS 
{
using LOFAR::ParmDB::ParmDB;
using std::max;

Model::Model(const Instrument &instrument, MeqParmGroup &parmGroup,
    ParmDB *skyDBase, MeqPhaseRef *phaseRef)
    :   itsEquationType(UNSET)
{
    // Construct source list.
    itsSourceList.reset(new MeqSourceList(*skyDBase, parmGroup));

    // Make nodes for all stations.
    makeStationNodes(instrument, *phaseRef);
}


void Model::makeEquations(EquationType type, const ModelConfig &config,
    const vector<baseline_t> &baselines, MeqParmGroup &parmGroup,
    ParmDB *instrumentDBase, MeqPhaseRef *phaseRef, VisData::Pointer buffer)
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

    vector<MeqExpr> azel(nSources);
    vector<MeqExpr> dft(nStations * nSources);
    vector<MeqJonesExpr> bandpass, gain, dir_gain;
    vector<vector<MeqJonesExpr> > beam;

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
            dft[i * nSources + j] = MeqExpr(new MeqDFTPS(itsLMNNodes[j],
                itsUVWNodes[i].get()));

        // Make a bandpass expression per station.
        if(mask[BANDPASS])
        {
            string suffix = itsStationNodes[i]->getName();

            MeqExpr B11(MeqParmFunklet::create("bandpass:11:" + suffix,
                parmGroup, instrumentDBase));
            MeqExpr B22(MeqParmFunklet::create("bandpass:22:" + suffix,
                parmGroup, instrumentDBase));

            bandpass[i] = new MeqDiag(B11, B22);
        }

        // Make a complex gain expression per station and possibly per source.
        if(mask[GAIN])
        {
            MeqExpr J11, J12, J21, J22;
            string suffix = itsStationNodes[i]->getName();

            // Make a J-jones expression per station
            MeqExpr J11_part1(MeqParmFunklet::create("gain:11:" + part1
                + suffix, parmGroup, instrumentDBase));
            MeqExpr J11_part2(MeqParmFunklet::create("gain:11:" + part2
                + suffix, parmGroup, instrumentDBase));
            MeqExpr J12_part1(MeqParmFunklet::create("gain:12:" + part1
                + suffix, parmGroup, instrumentDBase));
            MeqExpr J12_part2(MeqParmFunklet::create("gain:12:" + part2
                + suffix, parmGroup, instrumentDBase));
            MeqExpr J21_part1(MeqParmFunklet::create("gain:21:" + part1
                + suffix, parmGroup, instrumentDBase));
            MeqExpr J21_part2(MeqParmFunklet::create("gain:21:" + part2
                + suffix, parmGroup, instrumentDBase));
            MeqExpr J22_part1(MeqParmFunklet::create("gain:22:" + part1
                + suffix, parmGroup, instrumentDBase));
            MeqExpr J22_part2(MeqParmFunklet::create("gain:22:" + part2
                + suffix, parmGroup, instrumentDBase));

            if(mask[config.usePhasors])
            {
                J11 = new MeqExprAPToComplex(J11_part1, J11_part2);
                J12 = new MeqExprAPToComplex(J12_part1, J12_part2);
                J21 = new MeqExprAPToComplex(J21_part1, J21_part2);
                J22 = new MeqExprAPToComplex(J22_part1, J22_part2);
            }
            else
            {
                J11 = new MeqExprToComplex(J11_part1, J11_part2);
                J12 = new MeqExprToComplex(J12_part1, J12_part2);
                J21 = new MeqExprToComplex(J21_part1, J21_part2);
                J22 = new MeqExprToComplex(J22_part1, J22_part2);
            }

            gain[i] = new MeqJonesNode(J11, J12, J21, J22);
//            inv_gain[i] = new MeqJonesInvert(gain[i]);
        }
        
        if(mask[DIRECTIONAL_GAIN])
        {
            // Make a J-jones expression per station per source. Eventually,
            // patches of several sources will be supported as well.
            for(size_t j = 0; j < nSources; ++j)
            {
                MeqExpr J11, J12, J21, J22;
                string suffix = itsStationNodes[i]->getName() + ":"
                    + itsSourceNodes[j]->getName();

                MeqExpr J11_part1(MeqParmFunklet::create("gain:11:" + part1
                    + suffix, parmGroup, instrumentDBase));
                MeqExpr J11_part2(MeqParmFunklet::create("gain:11:" + part2
                    + suffix, parmGroup, instrumentDBase));
                MeqExpr J12_part1(MeqParmFunklet::create("gain:12:" + part1
                    + suffix, parmGroup, instrumentDBase));
                MeqExpr J12_part2(MeqParmFunklet::create("gain:12:" + part2
                    + suffix, parmGroup, instrumentDBase));
                MeqExpr J21_part1(MeqParmFunklet::create("gain:21:" + part1
                    + suffix, parmGroup, instrumentDBase));
                MeqExpr J21_part2(MeqParmFunklet::create("gain:21:" + part2
                    + suffix, parmGroup, instrumentDBase));
                MeqExpr J22_part1(MeqParmFunklet::create("gain:22:" + part1
                    + suffix, parmGroup, instrumentDBase));
                MeqExpr J22_part2(MeqParmFunklet::create("gain:22:" + part2
                    + suffix, parmGroup, instrumentDBase));

                if(mask[config.usePhasors])
                {
                    J11 = new MeqExprAPToComplex(J11_part1, J11_part2);
                    J12 = new MeqExprAPToComplex(J12_part1, J12_part2);
                    J21 = new MeqExprAPToComplex(J21_part1, J21_part2);
                    J22 = new MeqExprAPToComplex(J22_part1, J22_part2);
                }
                else
                {
                    J11 = new MeqExprToComplex(J11_part1, J11_part2);
                    J12 = new MeqExprToComplex(J12_part1, J12_part2);
                    J21 = new MeqExprToComplex(J21_part1, J21_part2);
                    J22 = new MeqExprToComplex(J22_part1, J22_part2);
                }

                dir_gain[i * nSources + j] =
                    new MeqJonesNode(J11, J12, J21, J22);

                // Gain correction is always performed with respect to the
                // direction of the first source (patch).
//                if(j == 0)
//                    inv_gain[i] = new MeqJonesInvert(gain[i * nSources]);
            }
        }
    }

    if(type == CORRECT)
    {
        ASSERTSTR(mask[GAIN] || mask[DIRECTIONAL_GAIN] || mask[BANDPASS],
            "Need at least one of GAIN, DIRECTIONAL_GAIN, BANDPASS to"
            " correct for.");
            
        vector<MeqJonesExpr> inv_bandpass, inv_gain, inv_dir_gain;
        
        if(mask[BANDPASS])
        {
            inv_bandpass.resize(nStations);
            for(size_t i = 0; i < nStations; ++i)
            {
                inv_bandpass[i] = new MeqJonesInvert(bandpass[i]);
            }
        }

        if(mask[GAIN])
        {
            inv_gain.resize(nStations);
            for(size_t i = 0; i < nStations; ++i)
            {
                inv_gain[i] = new MeqJonesInvert(gain[i]);
            }
        }
        
        if(mask[DIRECTIONAL_GAIN])
        {
            inv_dir_gain.resize(nStations);
            for(size_t i = 0; i < nStations; ++i)
            {
                // Always correct for the first source (direction).
                inv_dir_gain[i] = new MeqJonesInvert(dir_gain[i * nSources]);
            }
        }

        for(vector<baseline_t>::const_iterator it = baselines.begin();
            it != baselines.end();
            ++it)
        {
            const baseline_t &baseline = *it;
            MeqJonesExpr vis = new MeqJonesVisData(buffer, baseline);
            
            if(mask[BANDPASS])
            {
                vis = new MeqJonesCMul3(inv_bandpass[baseline.first], vis,
                    inv_bandpass[baseline.second]);
            }

            if(mask[GAIN])
            {
                vis = new MeqJonesCMul3(inv_gain[baseline.first], vis,
                    inv_gain[baseline.second]);
            }
            
            if(mask[DIRECTIONAL_GAIN])
            {
                vis = new MeqJonesCMul3(inv_dir_gain[baseline.first], vis,
                    inv_dir_gain[baseline.second]);
            }

            itsEquations[baseline] = vis;
        }
    }
    else
    {
        for(vector<baseline_t>::const_iterator it = baselines.begin();
            it != baselines.end();
            ++it)
        {
            const baseline_t &baseline = *it;

            vector<MeqJonesExpr> terms;
            for(size_t j = 0; j < nSources; ++j)
            {
                // Phase shift (incorporates geometry and fringe stopping).
                MeqExpr shift
                    (new MeqPhaseShift(dft[baseline.first * nSources + j],
                    dft[baseline.second * nSources + j]));

                MeqJonesExpr sourceExpr;

                MeqPointSource *source =
                    dynamic_cast<MeqPointSource*>(itsSourceNodes[j]);

                if(source)
                {
                    // Point source.
                    sourceExpr = new MeqPointCoherency(source);
                }
                else
                {
                    MeqGaussianSource *source = dynamic_cast<MeqGaussianSource*>(itsSourceNodes[j]);
                    ASSERT(source);
                    sourceExpr = new MeqGaussianCoherency(source, itsUVWNodes[baseline.first].get(), itsUVWNodes[baseline.second].get());
                }

                // Phase shift the source coherency.
                sourceExpr = new MeqJonesMul(sourceExpr, shift);
                
                // Apply image plane effects if required.
                if(mask[BEAM])
                {
                    sourceExpr = new MeqJonesCMul3
                        (beam[baseline.first][j],
                        sourceExpr,
                        beam[baseline.second][j]);
                }

                if(mask[DIRECTIONAL_GAIN])
                {
                    sourceExpr = new MeqJonesCMul3
                        (dir_gain[baseline.first * nSources + j],
                        sourceExpr,
                        dir_gain[baseline.second * nSources + j]);
                }
                
                terms.push_back(sourceExpr);
            }

            MeqJonesExpr sum;
            if(terms.size() == 1)
                sum = terms.front();
            else
                sum = MeqJonesExpr(new MeqJonesSum(terms));

            // Apply UV-plane effects if required.
            if(mask[GAIN])
            {
                sum = new MeqJonesCMul3(gain[baseline.first], sum,
                    gain[baseline.second]);
            }

            if(mask[BANDPASS])
            {
                sum = new MeqJonesCMul3(bandpass[baseline.first], sum,
                    bandpass[baseline.second]);
            }
            
            itsEquations[baseline] = sum;
        }
    }
}


void Model::clearEquations()
{
    itsEquations.clear();
    itsEquationType = UNSET;
}


void Model::precalculate(const MeqRequest& request)
{
    if(itsEquations.empty())
        return;

    // First clear the levels of all nodes in the tree.
    for(map<baseline_t, MeqJonesExpr>::iterator it = itsEquations.begin();
        it != itsEquations.end();
        ++it)
    {
        MeqJonesExpr &expr = it->second;
        if(!expr.isNull())
            expr.clearDone();
    }

    // Now set the levels of all nodes in the tree.
    // The top nodes have level 0; lower nodes have 1, 2, etc..
    int nrLev = -1;
    for(map<baseline_t, MeqJonesExpr>::iterator it = itsEquations.begin();
        it != itsEquations.end();
        ++it)
    {
        MeqJonesExpr &expr = it->second;
        if(!expr.isNull())
            nrLev = max(nrLev, expr.setLevel(0));
    }
    nrLev++;
    ASSERT(nrLev > 0);

    // Find the nodes to be precalculated at each level.
    // That is not needed for the root nodes (the baselines).
    // The nodes used by the baselines are always precalculated (even if
    // having one parent).
    // It may happen that a station is used by only one baseline. Calculating
    // such a baseline is much more work if the station was not precalculated.
    vector<vector<MeqExprRep*> > precalcNodes(nrLev);
    for(size_t level = 1; level < nrLev; ++level)
    {
        vector<MeqExprRep*> &nodes = precalcNodes[level];
        nodes.resize(0);

        for(map<baseline_t, MeqJonesExpr>::iterator it = itsEquations.begin();
            it != itsEquations.end();
            ++it)
        {
            MeqJonesExpr &expr = it->second;
            if(!expr.isNull())
                expr.getCachingNodes(nodes, level, false);
        }
    }

/************ DEBUG DEBUG DEBUG ************/
    LOG_TRACE_FLOW_STR("#levels=" << nrLev);
    for(size_t i = 0; i < nrLev; ++i)
    {
        LOG_TRACE_FLOW_STR("#expr on level " << i << " is "
            << precalcNodes[i].size());
    }
/************ DEBUG DEBUG DEBUG ************/

#pragma omp parallel
    {
        // Loop through expressions to be precalculated.
        // At each level the expressions can be executed in parallel.
        // Level 0 is formed by itsExpr which are not calculated here.
        for(size_t level = precalcNodes.size(); --level > 0;)
        {
            vector<MeqExprRep*> &nodes = precalcNodes[level];
            if(!nodes.empty())
            {
//                ASSERT(nodes.size() <= numeric_limits<int>::max());
#pragma omp for schedule(dynamic)
                for(int i = 0; i < static_cast<int>(nodes.size()); ++i)
                {
                    nodes[i]->precalculate(request);
                }
            }
        }
    } // omp parallel
}


MeqJonesResult Model::evaluate(baseline_t baseline, const MeqRequest& request)
{
    map<baseline_t, MeqJonesExpr>::iterator it =
        itsEquations.find(baseline);
    ASSERTSTR(it != itsEquations.end(), "Result requested for unknown"
        " baseline " << baseline.first << " - " << baseline.second);

    return it->second.getResult(request);
}


void Model::makeStationNodes(const Instrument &instrument,
    const MeqPhaseRef &phaseRef)
{
    // Workaround to facilitate comparison with earlier versions of BBS
    // and with MeqTree.
    casa::Quantum<casa::Vector<casa::Double> > angles =
        phaseRef.direction().getAngle();
    casa::MDirection phaseRef2(casa::MVDirection(angles.getBaseValue()(0),
        angles.getBaseValue()(1)), casa::MDirection::J2000);
    LOG_WARN_STR("Superfluous conversion of phase center from/to"
        " casa::MDirection to allow comparison with earlier version of BBS and"
        " with MeqTree. Should be removed after validation.");

    itsStationNodes.resize(instrument.stations.size());
    itsUVWNodes.resize(instrument.stations.size());
    for(size_t i = 0; i < instrument.stations.size(); ++i)
    {
        const casa::MVPosition &position =
            instrument.stations[i].position.getValue();

        MeqParmSingle *x = new MeqParmSingle("position:x:" +
            instrument.stations[i].name, position(0));
        MeqParmSingle *y = new MeqParmSingle("position:y:" +
            instrument.stations[i].name, position(1));
        MeqParmSingle *z = new MeqParmSingle("position:z:" +
            instrument.stations[i].name, position(2));

        itsStationNodes[i].reset(new MeqStation(x, y, z,
            instrument.stations[i].name));

        itsUVWNodes[i].reset(new MeqStatUVW(itsStationNodes[i].get(),
            phaseRef2, instrument.position));
    }
}


void Model::makeSourceNodes(const vector<string> &names, MeqPhaseRef *phaseRef)
{
    itsSourceNodes.resize(names.size());
    itsLMNNodes.resize(names.size());
    for(size_t i = 0; i < names.size(); ++i)
    {
        MeqSource *source = itsSourceList->getSource(names[i]);
        itsSourceNodes[i] = source;

        // Create an LMN node for the source.
        itsLMNNodes[i] = new MeqLMN(source);
        itsLMNNodes[i]->setPhaseRef(phaseRef);
    }
}


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


void Model::makeBeamNodes(const ModelConfig &config,
    LOFAR::ParmDB::ParmDB *db, MeqParmGroup &group,
    vector<vector<MeqJonesExpr> > &result) const
{
    ASSERT(config.beamConfig);
    ASSERT(config.beamConfig->type() == "HamakerDipole"
        || config.beamConfig->type() == "YatawattaDipole");
    
    // Create AzEl node for each source-station combination.
    vector<MeqExpr> azel(itsSourceNodes.size());
    for(size_t i = 0; i < itsSourceNodes.size(); ++i)
    {
        // TODO: This code uses station 0 as reference position on earth
        // (see global_model.py in EJones_HBA). Is this accurate enough?
        azel[i] =
            new MeqAzEl(*(itsSourceNodes[i]), *(itsStationNodes[0].get()));
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
            MeqExpr orientation(MeqParmFunklet::create("orientation:"
                + itsStationNodes[i]->getName(), group, db));

            result[i].resize(itsSourceNodes.size());
            for(size_t j = 0; j < itsSourceNodes.size(); ++j)
            {
                result[i][j] = new MeqNumericalDipoleBeam(coeff, azel[j],
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
            MeqExpr orientation(MeqParmFunklet::create("orientation:"
                + itsStationNodes[i]->getName(), group, db));

            result[i].resize(itsSourceNodes.size());
            for(size_t j = 0; j < itsSourceNodes.size(); ++j)
            {
//                result[i][j] = new MeqDipoleBeam(azel[j], 1.706, 1.38,
//                    casa::C::pi / 4.001, -casa::C::pi_4);
                result[i][j] =
                    new MeqDipoleBeamExternal(beamConfig->moduleTheta,
                        beamConfig->modulePhi, azel[j], orientation, 1.0);
            }
        }
    }
    else
    {
        ASSERT(false);
    }
}


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


} // namespace BBS
} // namespace LOFAR
