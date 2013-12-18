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
#include <BBSKernel/MeasurementExprLOFARGen.h>
#include <BBSKernel/MeasurementExprLOFARUtil.h>
#include <BBSKernel/Correlation.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/Measurement.h>
#include <BBSKernel/Expr/ExprVisData.h>
#include <BBSKernel/Expr/Request.h>
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

#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>

namespace LOFAR
{
namespace BBS
{

MeasurementExprLOFAR::MeasurementExprLOFAR(SourceDB &sourceDB,
    const BufferMap &buffers, const ModelConfig &config,
    const Instrument::ConstPtr &instrument, const BaselineSeq &baselines,
    double refFreq, const casa::MDirection &refPhase,
    const casa::MDirection &refDelay, const casa::MDirection &refTile,
    bool circular)
    :   itsBaselines(baselines),
        itsCachePolicy(new DefaultCachePolicy())
{
    setCorrelations(circular);
    makeForwardExpr(sourceDB, buffers, config, instrument, refFreq, refPhase,
        refDelay, refTile, circular);
}

MeasurementExprLOFAR::MeasurementExprLOFAR(SourceDB &sourceDB,
    const BufferMap &buffers, const ModelConfig &config,
    const VisBuffer::Ptr &buffer, const BaselineMask &mask, bool inverse,
    bool useMMSE, double sigmaMMSE)
    :   itsBaselines(filter(buffer->baselines(), mask)),
        itsCachePolicy(new DefaultCachePolicy())
{
    const bool circular = buffer->isCircular();
    ASSERTSTR(circular != buffer->isLinear(), "The correlations in the"
        " measurement should be either all linear or all circular.");
    setCorrelations(circular);

    if(inverse)
    {
        makeInverseExpr(sourceDB, buffers, config, buffer, useMMSE, sigmaMMSE);
    }
    else
    {
        makeForwardExpr(sourceDB, buffers, config, buffer->instrument(),
            buffer->getReferenceFreq(), buffer->getPhaseReference(),
            buffer->getDelayReference(), buffer->getTileReference(), circular);
    }
}

void MeasurementExprLOFAR::solvablesChanged()
{
    itsCache.clear(Cache::VOLATILE);
    itsCache.clearStats();
}

void MeasurementExprLOFAR::makeForwardExpr(SourceDB &sourceDB,
    const BufferMap &buffers, const ModelConfig &config,
    const Instrument::ConstPtr &instrument, double refFreq,
    const casa::MDirection &refPhase, const casa::MDirection &refDelay,
    const casa::MDirection &refTile, bool circular)
{
    NSTimer timer;
    timer.start();

    LOG_DEBUG_STR("Building expression tree...");

    if(config.useFlagger())
    {
        LOG_WARN("Condition number flagging is only implemented for the inverse"
            " model.");
    }

    // Make a list of patches matching the selection criteria specified by the
    // user.
    vector<Source::Ptr> sources = makeSourceList(sourceDB, buffers,
        config.sources());
    if(sources.empty())
    {
        THROW(BBSKernelException, "No sources found matching selection.");
    }

    itsExpr = makeMeasurementExpr(itsScope, sources, instrument, itsBaselines,
        config, refFreq, refPhase, refDelay, refTile, circular);

    // Set caching policy.
    itsCachePolicy = CachePolicy::Ptr(new DefaultCachePolicy());
    if(config.useCache())
    {
        itsCachePolicy = CachePolicy::Ptr(new ExperimentalCachePolicy());
    }

    itsCachePolicy->apply(itsExpr.begin(), itsExpr.end());

    timer.stop();
    LOG_DEBUG_STR("Building expression tree... done.");
    LOG_DEBUG_STR("" << timer);
}

void MeasurementExprLOFAR::makeInverseExpr(SourceDB &sourceDB,
    const BufferMap &buffers, const ModelConfig &config,
    const VisBuffer::Ptr &buffer, bool useMMSE, double sigmaMMSE)
{
    NSTimer timer;
    timer.start();

    LOG_DEBUG_STR("Building expression tree...");

    vector<Expr<JonesMatrix>::Ptr> stationExpr;
    if(config.sources().empty())
    {
        stationExpr = makeStationExpr(itsScope, buffer->getPhaseReference(),
            buffer->instrument(), config, buffer->getReferenceFreq(),
            buffer->getDelayReference(), buffer->getTileReference(), true,
            useMMSE, sigmaMMSE);
    }
    else
    {
        // Make a list of patches matching the selection criteria specified by
        // the user.
        vector<Source::Ptr> sources = makeSourceList(sourceDB, buffers,
            config.sources());

        stationExpr = makeStationExpr(itsScope, sources, buffer->instrument(),
            config, buffer->getReferenceFreq(), buffer->getDelayReference(),
            buffer->getTileReference(), true, useMMSE, sigmaMMSE);
    }

    itsExpr = vector<Expr<JonesMatrix>::Ptr>(itsBaselines.size());
    for(size_t i = 0; i < itsBaselines.size(); ++i)
    {
        const baseline_t &baseline = itsBaselines[i];
        Expr<JonesMatrix>::Ptr coherence(new ExprVisData(buffer, baseline));

        itsExpr[i] = apply(stationExpr[baseline.first], coherence,
            stationExpr[baseline.second]);
    }

    // Set caching policy.
    itsCachePolicy = CachePolicy::Ptr(new DefaultCachePolicy());
    if(config.useCache())
    {
        itsCachePolicy = CachePolicy::Ptr(new ExperimentalCachePolicy());
    }

    itsCachePolicy->apply(itsExpr.begin(), itsExpr.end());

    timer.stop();
    LOG_DEBUG_STR("Building expression tree... done.");
    LOG_DEBUG_STR("" << timer);
}

size_t MeasurementExprLOFAR::size() const
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

size_t MeasurementExprLOFAR::nParms() const
{
    return itsScope.size();
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
        if(parm_it == itsScope.end())
        {
            clearSolvables();
            THROW(BBSKernelException, "Model does not depend on parameter: "
                << ParmManager::instance().get(*sol_it)->getName());
        }

        parm_it->second->setPValueFlag();
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

    EXPR_TIMER_START_NAMED("MeasurementExprLOFAR::evaluate()");

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

    EXPR_TIMER_STOP_NAMED("MeasurementExprLOFAR::evaluate()");

    return result;
}

void MeasurementExprLOFAR::setCorrelations(bool circular)
{
    itsCorrelations.clear();

    if(circular)
    {
        LOG_DEBUG_STR("Visibilities will be simulated using circular-RL"
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

vector<Source::Ptr> MeasurementExprLOFAR::makeSourceList(SourceDB &sourceDB,
    const BufferMap &buffers, const vector<string> &patterns)
{
    // Find all matching sources.
    vector<string> names;
    if(patterns.empty())
    {
        names = findSources(sourceDB, "*");
    }
    else
    {
        for(vector<string>::const_iterator it = patterns.begin(),
            end = patterns.end(); it != end; ++it)
        {
            if(it->empty())
            {
                continue;
            }

            if((*it)[0] == '@')
            {
                names.push_back(*it);
            }
            else
            {
                vector<string> match = findSources(sourceDB, *it);
                names.insert(names.end(), match.begin(), match.end());
            }
        }
    }

    // Remove duplicates.
    sort(names.begin(), names.end());
    vector<string>::const_iterator end = unique(names.begin(), names.end());

    // Create Source instances.
    vector<Source::Ptr> sources;
    for(vector<string>::const_iterator it = names.begin(); it != end; ++it)
    {
        if(it->empty())
        {
            continue;
        }

        if((*it)[0] == '@')
        {
            BufferMap::const_iterator itBuffer = buffers.find(*it);
            ASSERT(itBuffer != buffers.end());
            sources.push_back(Source::create(*it, itBuffer->second));
        }
        else
        {
            sources.push_back(Source::create(sourceDB.getSource(*it),
                itsScope));
        }
    }

    return sources;
}

vector<string> MeasurementExprLOFAR::findSources(SourceDB &sourceDB,
    const string &pattern)
{
    vector<SourceInfo> match = sourceDB.getSources(pattern);

    vector<string> result;
    result.reserve(match.size());
    for(vector<SourceInfo>::const_iterator it = match.begin(),
        end = match.end(); it != end; ++it)
    {
        result.push_back(it->getName());
    }
    return result;
}

} // namespace BBS
} // namespace LOFAR
