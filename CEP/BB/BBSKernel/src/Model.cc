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

#include <BBSKernel/Measurement.h>

#include <BBSKernel/MNS/MeqPhaseRef.h>
#include <BBSKernel/MNS/MeqParmFunklet.h>
#include <BBSKernel/MNS/MeqParmSingle.h>
#include <BBSKernel/MNS/MeqDiag.h>
#include <BBSKernel/MNS/MeqJonesInvert.h>
#include <BBSKernel/MNS/MeqBaseDFTPS.h>
#include <BBSKernel/MNS/MeqBaseLinPS.h>
#include <BBSKernel/MNS/MeqJonesCMul3.h>
#include <BBSKernel/MNS/MeqJonesSum.h>
#include <BBSKernel/MNS/MeqJonesVisData.h>

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
{
    // Construct source list.
    itsSourceList.reset(new MeqSourceList(*skyDBase, parmGroup));

    // Make nodes for all stations.
    makeStationNodes(instrument, *phaseRef);
}


void Model::makeEquations(EquationType type, const vector<string> &components,
    const set<baseline_t> &baselines, const vector<string> &sources,
    MeqParmGroup &parmGroup, ParmDB *instrumentDBase, MeqPhaseRef *phaseRef,
    VisData::Pointer data)
{
    // Parse component names.
    vector<bool> mask(N_ModelComponent, false);
    for(vector<string>::const_iterator it = components.begin();
        it != components.end();
        ++it)
    {
        if(*it == "GAIN")
            mask[GAIN] = true;
        else if(*it == "DIRECTIONAL_GAIN")
            mask[DIRECTIONAL_GAIN] = true;
        else if(*it == "BANDPASS")
            mask[BANDPASS] = true;
        else if(*it == "PHASORS")
            mask[PHASORS] = true;
    }
/*
    if(mask{GAIN] && mask[DIRECTIONAL_GAIN])
        LOG_WARN("Model components GAIN and DIRECTIONAL_GAIN are mutually"
            " exclusive; using GAIN only.");
*/

    string part1("real:");
    string part2("imag:");
    if(mask[PHASORS])
    {
        part1 = "ampl:";
        part2 = "phase:";
    }

    // Clear all equations.
    itsEquations.clear();

    // Make nodes for all specified sources (use all if none specified).
    if(sources.empty())
        makeSourceNodes(itsSourceList->getSourceNames(), phaseRef);
    else
        makeSourceNodes(sources, phaseRef);

    // Make baseline equations.
    size_t nStations = itsStationNodes.size();
    size_t nSources = itsSourceNodes.size();

    vector<MeqExpr> dft(nStations * nSources);
    vector<MeqJonesExpr> gain, inv_gain, bandpass;

    if(mask[BANDPASS])
        bandpass.resize(nStations);

    if(mask[GAIN] || mask[DIRECTIONAL_GAIN])
    {
        inv_gain.resize(nStations);
        if(mask[GAIN])
            gain.resize(nStations);
        else
            gain.resize(nStations * nSources);
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

            bandpass[i] = new MeqDiag(MeqExpr(B11), MeqExpr(B22));
        }

        // Make a complex gain expression per station and possibly per source.
        if(mask[GAIN])
        {
            MeqExprRep *J11, *J12, *J21, *J22;
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

            if(mask[PHASORS])
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

            gain[i] = new MeqJonesNode(MeqExpr(J11), MeqExpr(J12),
                MeqExpr(J21), MeqExpr(J22));
            inv_gain[i] = new MeqJonesInvert(gain[i]);
        }
        else if(mask[DIRECTIONAL_GAIN])
        {
            // Make a J-jones expression per station per source. Eventually,
            // patches of several sources will be supported as well.
            for(size_t j = 0; j < nSources; ++j)
            {
                MeqExprRep *J11, *J12, *J21, *J22;
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

                if(mask[PHASORS])
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

                gain[i * nSources + j] = new MeqJonesNode(MeqExpr(J11),
                    MeqExpr(J12), MeqExpr(J21), MeqExpr(J22));

                // Gain correction is always performed with respect to the
                // direction of the first source (patch).
                if(j == 0)
                    inv_gain[i] = new MeqJonesInvert(gain[i * nSources]);
            }
        }
    }

    if(type == CORRECT)
    {
        for(set<baseline_t>::const_iterator it = baselines.begin();
            it != baselines.end();
            ++it)
        {
            const baseline_t &baseline = *it;
            itsEquations[baseline] = new MeqJonesCMul3(inv_gain[baseline.first],
                    new MeqJonesVisData(data, baseline),
                    inv_gain[baseline.second]);
        }
    }
    else
    {

        for(set<baseline_t>::const_iterator it = baselines.begin();
            it != baselines.end();
            ++it)
        {
            const baseline_t &baseline = *it;

            vector<MeqJonesExpr> terms;
            for(size_t j = 0; j < nSources; ++j)
            {
                MeqExpr base
                    (new MeqBaseDFTPS(dft[baseline.first * nSources + j],
                    dft[baseline.second * nSources + j], itsLMNNodes[j]));

                MeqPointSource *source =
                    dynamic_cast<MeqPointSource*>(itsSourceNodes[j]);

                MeqJonesExpr sourceTerm(new MeqBaseLinPS(base, source));

                if(mask[DIRECTIONAL_GAIN])
                {
                    terms.push_back
                        (new MeqJonesCMul3(gain[baseline.first * nSources + j],
                        sourceTerm, gain[baseline.second * nSources + j]));
                }
                else
                    terms.push_back(sourceTerm);
            }

            MeqJonesExpr sum;
            if(terms.size() == 1)
                sum = terms.front();
            else
                sum = MeqJonesExpr(new MeqJonesSum(terms));

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
#pragma omp for schedule(dynamic)
                for(size_t i=0; i < nodes.size(); ++i)
                    nodes[i]->precalculate(request);
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

    itsStationNodes.resize(instrument.getStationCount());
    itsUVWNodes.resize(instrument.getStationCount());
    for(size_t i = 0; i < instrument.getStationCount(); ++i)
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


void Model::setStationUVW(const Instrument &instrument, VisData::Pointer data)
{
    size_t nStations = instrument.getStationCount();
    vector<bool> statDone(nStations);
    vector<double> statUVW(3 * nStations);

    // Step through the MS by timeslot.
    for (size_t tslot = 0; tslot < data->time.size(); ++tslot)
    {
        double time = data->time(tslot);
        fill(statDone.begin(), statDone.end(), false);

        // Set UVW of first station used to 0 (UVW coordinates are relative!).
        size_t station0 = data->baselines.begin()->first.first;
        statUVW[3 * station0] = 0.0;
        statUVW[3 * station0 + 1] = 0.0;
        statUVW[3 * station0 + 2] = 0.0;
        itsUVWNodes[station0]->set(time, 0.0, 0.0, 0.0);
        statDone[station0] = true;

        size_t nDone;
        do
        {
            nDone = 0;
            for (map<baseline_t, size_t>::const_iterator it =
                    data->baselines.begin();
                it != data->baselines.end();
                ++it)
            {
                size_t blindex = it->second;
                if(data->tslot_flag[blindex][tslot])
                    continue;

                size_t statA = it->first.first;
                size_t statB = it->first.second;
                if (statDone[statA] && !statDone[statB])
                {
                    statUVW[3 * statB] =
                        data->uvw[blindex][tslot][0] - statUVW[3 * statA];
                    statUVW[3 * statB + 1] =
                        data->uvw[blindex][tslot][1] - statUVW[3 * statA + 1];
                    statUVW[3 * statB + 2] =
                        data->uvw[blindex][tslot][2] - statUVW[3 * statA + 2];
                    statDone[statB] = true;
                    itsUVWNodes[statB]->set(time, statUVW[3 * statB],
                        statUVW[3 * statB + 1], statUVW[3 * statB + 2]);
                    ++nDone;
                }
                else if (statDone[statB] && !statDone[statA])
                {
                    statUVW[3 * statA] =
                        statUVW[3 * statB] - data->uvw[blindex][tslot][0];
                    statUVW[3 * statA + 1] =
                        statUVW[3 * statB + 1] - data->uvw[blindex][tslot][1];
                    statUVW[3 * statA + 2] =
                        statUVW[3 * statB + 2] - data->uvw[blindex][tslot][2];
                    statDone[statA] = true;
                    itsUVWNodes[statA]->set(time, statUVW[3 * statA],
                        statUVW[3 * statA + 1], statUVW[3 * statA + 2]);
                    ++nDone;
                }
            }
        }
        while(nDone > 0);
    }
}

} //# namespace BBS
} //# namespace LOFAR
