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
#include <BBSKernel/Expr/GaussianSource.h>

#include <BBSKernel/Expr/ExprParm.h>
#include <BBSKernel/Expr/PhaseRef.h>
#include <BBSKernel/Expr/PhaseShift.h>
#include <BBSKernel/Expr/PointCoherence.h>
#include <BBSKernel/Expr/JonesMul.h>
#include <BBSKernel/Expr/JonesMul2.h>
#include <BBSKernel/Expr/JonesVisData.h>
#include <BBSKernel/Expr/JonesSum.h>

#include <BBSKernel/Expr/LMN.h>
#include <BBSKernel/Expr/DFTPS.h>
#include <BBSKernel/Expr/JonesNode.h>
#include <BBSKernel/Expr/JonesInvert.h>
#include <BBSKernel/Expr/JonesCMul3.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/JonesExpr.h>

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
    const casa::MDirection &phaseRef)
    :   itsInstrument(instrument),
        itsSourceDb(sourceDb)
{
    // Store phase reference.
    itsPhaseRef = PhaseRef::ConstPointer(new PhaseRef(phaseRef));

    // Make UVW nodes for all stations.
    makeStationUvw();
}

bool Model::makeFwdExpressions(const ModelConfig &config,
    const vector<baseline_t> &baselines)
{
    ASSERTSTR(itsExpressions.empty(), "Model already initialized; call Model::"
        "clearExpressions() first");

    // Parse user supplied model component selection.
    vector<bool> mask(parseComponents(config.components));

    // Create source nodes for all selected sources.
    vector<Source::Pointer> sources;
    makeSources(sources, config.sources);
    if(sources.empty())
    {
        LOG_ERROR_STR("Source selection is empty.");
        return false;
    }

    const size_t nSources = sources.size();
    const size_t nStations = itsInstrument.stations.size();

//    cout << "nSources: " << nSources << endl;

    // Create coherence nodes for all point sources.
    map<size_t, JonesExpr> pointCoherences;
    for(size_t i = 0; i < nSources; ++i)
    {
        // A point source's coherence is independent of baseline UVW
        // coordinates. Therefore, they are constructed here and shared
        // between baselines.
        PointSource::ConstPointer pointSource =
            dynamic_pointer_cast<const PointSource>(sources[i]);

        if(pointSource)
        {
            // Create point source coherence.
            pointCoherences[i] = new PointCoherence(pointSource);
        }
    }

    // Direction independent (uv-plane) effects.
    bool uvEffects = mask[BANDPASS] || mask[GAIN];
    
    // Create a bandpass node per station.
    vector<JonesExpr> bandpass;
//    if(mask[BANDPASS])
//    {
//        makeBandpassNodes(bandpass, config);
//    }
    
    // Create a gain node per station.
    vector<JonesExpr> gain;
    if(mask[GAIN])
    {
        makeGainNodes(gain, config);
    }

    // Create a single JonesExpr per station that is the accumulation of all
    // uv-plane effects.
    vector<JonesExpr> uvJones;
    if(uvEffects)
    {
        uvJones.resize(nStations);
        for(size_t i = 0; i < nStations; ++i)
        {   
            if(mask[GAIN])
            {
                uvJones[i] = gain[i];
            }
                
//            if(mask[BANDPASS])
//            {
//                uvJones[i] = uvJones[i].isNull() ? bandpass[i]
//                    : JonesExpr(new JonesMul2(bandpass[i], uvJones[i]));
//            }

            // TODO: Add other uv-plane effects here.
        }
    }
    
    // Direction dependent (image-plane) effects.
    bool imgEffects = mask[DIRECTIONAL_GAIN] || mask[BEAM];
    
    // Create a station shift node per station-source combination.
    boost::multi_array<Expr, 2> stationShift;
    makeStationShiftNodes(stationShift, sources);

    // Create a directional gain node per station-source combination.
    boost::multi_array<JonesExpr, 2> gainDirectional;
    if(mask[DIRECTIONAL_GAIN])
    {
        makeDirectionalGainNodes(gainDirectional, config, sources);
    }
    
    // Create a single JonesExpr per station-source combination that is the
    // accumulation of all image-plane effects.
    // TODO: Incorporate station phase shift by splitting it per station-source.
    boost::multi_array<JonesExpr, 2> imgJones;
    if(imgEffects)
    {
        imgJones.resize(boost::extents[nStations][nSources]);

        for(size_t i = 0; i < nStations; ++i)
        {
            for(size_t j = 0; j < nSources; ++j)
            {
                if(mask[DIRECTIONAL_GAIN])
                {
                    imgJones[i][j] = gainDirectional[i][j];
                }
                
                // TODO: Add other image-plane effects here.
            }
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
            // TODO: Remove this assert after adding GaussianSource.
            ASSERT(!coherence.isNull());

            // Phase shift the source coherence.
            coherence = new JonesMul(coherence, shift);
            
            // Apply direction dependent (image-plane) effects.
            if(imgEffects)
            {
                coherence = new JonesCMul3(imgJones[baseline.first][j],
                    coherence, imgJones[baseline.second][j]);
            }

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

        // Apply direction independent (uv-plane) effects.
        if(uvEffects)
        {
            sum = new JonesCMul3(uvJones[baseline.first], sum,
                uvJones[baseline.second]);
        }

        itsExpressions[baseline] = sum;
    }

    return true;
}

bool Model::makeInvExpressions(const ModelConfig &config,
    const VisData::Pointer &chunk, const vector<baseline_t> &baselines)
{
    ASSERTSTR(itsExpressions.empty(), "Model already initialized; call Model::"
        "clearExpressions() first");

    const size_t nStations = itsInstrument.stations.size();

    // Parse user supplied model component selection.
    vector<bool> mask(parseComponents(config.components));

    // Direction independent (uv-plane) effects.
    bool uvEffects = mask[BANDPASS] || mask[GAIN];
    
    // Create a bandpass node per station.
    vector<JonesExpr> bandpass;
//    if(mask[BANDPASS])
//    {
//        makeBandpassNodes(bandpass, config, true);
//    }
    
    // Create a gain node per station.
    vector<JonesExpr> gain;
    if(mask[GAIN])
    {
        makeGainNodes(gain, config, true);
    }

    // Create a single JonesExpr per station-source combination that is the
    // accumulation of all uv-plane and image-plane effects.
    // Accumulate all uv-plane effects.
    boost::multi_array<JonesExpr, 1> jJones(boost::extents[nStations]);
    if(uvEffects)
    {
        for(size_t i = 0; i < nStations; ++i)
        {   
//            if(mask[BANDPASS])
//            {
//                jJones[i] = bandpass[i];
//            }
            
            if(mask[GAIN])
            {
                jJones[i] = jJones[i].isNull() ? gain[i]
                    : JonesExpr(new JonesMul2(gain[i], jJones[i]));
            }

            // TODO: Add other uv-plane effects here.
        }
    }
    
    // Direction dependent (image-plane) effects.
    bool imgEffects = mask[DIRECTIONAL_GAIN] || mask[BEAM];

    if(imgEffects)
    {
        // Create source nodes for all selected sources.
        vector<Source::Pointer> sources;
        makeSources(sources, config.sources);
        if(sources.size() != 1)
        {
            LOG_ERROR_STR("No direction, or more than one direction specified."
                " A correction can only be applied for a single direction on"
                " the sky");
            return false;
        }

        // Create a directional gain node per station-source combination.
        boost::multi_array<JonesExpr, 2> gainDirectional;
        if(mask[DIRECTIONAL_GAIN])
        {
            makeDirectionalGainNodes(gainDirectional, config, sources, true);
        }
        
        // Accumulate the image-plane effects.
        for(size_t i = 0; i < nStations; ++i)
        {
            if(mask[DIRECTIONAL_GAIN])
            {
                jJones[i] = jJones[i].isNull() ? gainDirectional[i][0]
                    : JonesExpr(new JonesMul2(gainDirectional[i][0], jJones[i]));
            }
            
            // TODO: Add other image-plane effects here.
        }
    }
        
    // Create an expression tree for each baseline.
    for(size_t i = 0; i < baselines.size(); ++i)
    {
        const baseline_t &baseline = baselines[i];
        
        JonesExpr vdata(new JonesVisData(chunk, baseline));
        
        if(uvEffects || imgEffects)
        {
            vdata = new JonesCMul3(jJones[baseline.first], vdata,
                jJones[baseline.second]);
        }
        
        itsExpressions[baseline] = vdata;
    }

    return true;
}

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
        else
        {
            LOG_WARN_STR("Ignored unsupported model component \"" << *it
                << "\".");
        }
    }

    return mask;
}

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

void Model::makeStationShiftNodes(boost::multi_array<Expr, 2> &result,
    const vector<Source::Pointer> &sources) const
{
    ASSERTSTR(itsStationUvw.size() == itsInstrument.stations.size(),
        "Model::makeStationUvw() should be called first.");

    result.resize(boost::extents[itsStationUvw.size()][sources.size()]);
    for(size_t j = 0; j < sources.size(); ++j)
    {
        Expr lmn = new LMN(sources[j], itsPhaseRef);
    
        for(size_t i = 0; i < itsStationUvw.size(); ++i)
        {
            result[i][j] = new DFTPS(itsStationUvw[i], lmn);
        }
    }
}

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

void Model::makeSources(vector<Source::Pointer> &result,
    vector<string> patterns)
{
    result.clear();
    
    if(patterns.empty())
    {
        vector<SourceInfo> sources(itsSourceDb.getSources("*"));
        for(size_t i = 0; i < sources.size(); ++i)
        {
            Source::Pointer source = makeSource(sources[i]);
            if(source)
            {
                result.push_back(source);
            }
        }
    }
    
    map<string, SourceInfo> sources;
    
    // Only consider unique patterns.
    sort(patterns.begin(), patterns.end());
    vector<string>::const_iterator patternItEnd = unique(patterns.begin(),
        patterns.end());
    vector<string>::const_iterator patternIt = patterns.begin();
    
    while(patternIt != patternItEnd)
    {
        vector<SourceInfo> tmp = itsSourceDb.getSources(*patternIt);
        for(size_t i = 0; i < tmp.size(); ++i)
        {
            sources.insert(make_pair(tmp[i].getName(), tmp[i]));
        }
        ++patternIt;
    }
    
    map<string, SourceInfo>::const_iterator srcIt = sources.begin();
    map<string, SourceInfo>::const_iterator srcItEnd = sources.end();
    while(srcIt != srcItEnd)
    {
        Source::Pointer source = makeSource(srcIt->second);
        if(source)
        {
            result.push_back(source);
        }
        ++srcIt;
    }
}

Source::Pointer Model::makeSource(const SourceInfo &source)
{
//    cout << "Creating source: " << source.getName() << endl;
    
    switch(source.getType())
    {
    case SourceInfo::POINT:
        {
            Expr ra(makeExprParm(SKY, "Ra:" + source.getName()));
            Expr dec(makeExprParm(SKY, "Dec:" + source.getName()));
            Expr i(makeExprParm(SKY, "I:" + source.getName()));
            Expr q(makeExprParm(SKY, "Q:" + source.getName()));
            Expr u(makeExprParm(SKY, "U:" + source.getName()));
            Expr v(makeExprParm(SKY, "V:" + source.getName()));

            return PointSource::Pointer(new PointSource(source.getName(),
                ra, dec, i, q, u, v));
        }
        break;
    /*
    case SourceInfo::GAUSSIAN:
        {
            Expr ra(makeExprParm(SKY, "Ra:" + source.getName()));
            Expr dec(makeExprParm(SKY, "Dec:" + source.getName()));
            Expr i(makeExprParm(SKY, "I:" + source.getName()));
            Expr q(makeExprParm(SKY, "Q:" + source.getName()));
            Expr u(makeExprParm(SKY, "U:" + source.getName()));
            Expr v(makeExprParm(SKY, "V:" + source.getName()));
            Expr maj(makeExprParm(SKY, "Major:" + source.getName()));
            Expr min(makeExprParm(SKY, "Minor:" + source.getName()));
            Expr phi(makeExprParm(SKY, "Phi:" + source.getName()));

            return GaussianSource::Pointer(new GaussianSource(source.getName(),
                ra, dec, i, q, u, v, maj, min, phi));
        }
        break;
    */        
    default:
        LOG_WARN_STR("Unable to construct source: " << source.getName());
        break;
    }
    
    return Source::Pointer();
}

} // namespace BBS
} // namespace LOFAR
