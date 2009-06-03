//# Evaluator.cc: Evaluate a model and assign the result to or subtract it from
//# the visibility data in the chunk.
//#
//# Copyright (C) 2008
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
#include <BBSKernel/Evaluator.h>
#include <BBSKernel/Exceptions.h>

namespace LOFAR
{
namespace BBS 
{

Evaluator::Evaluator(const VisData::Pointer &chunk, const Model::Pointer &model,
    uint nThreads)
    :   itsChunk(chunk),
        itsModel(model),
        itsThreadCount(nThreads)
{
    ASSERT(itsChunk);
    ASSERT(itsModel);

#ifndef _OPENMP
    // Ignore thread count specified in constructor.
    itsThreadCount = 1;
#endif    
    
    // By default, select all the baselines and polarizations products
    // available.
    const VisDimensions &dims = itsChunk->getDimensions();    
    setSelection(dims.getBaselines(), dims.getPolarizations());
}

Evaluator::~Evaluator()
{
}

void Evaluator::setSelection(const vector<baseline_t> &baselines,
        const vector<string> &products)
{
    itsBaselines = baselines;
    
    // Determine product mask.
    const VisDimensions &dims = itsChunk->getDimensions();

    fill(&(itsProductMask[0]), &(itsProductMask[4]), -1);

    if(dims.hasPolarization("XX")
        && find(products.begin(), products.end(), "XX") != products.end())
    {
        itsProductMask[0] = dims.getPolarizationIndex("XX");
    }

    if(dims.hasPolarization("XY")
        && find(products.begin(), products.end(), "XY") != products.end())
    {
        itsProductMask[1] = dims.getPolarizationIndex("XY");
    }

    if(dims.hasPolarization("YX")
        && find(products.begin(), products.end(), "YX") != products.end())
    {
        itsProductMask[2] = dims.getPolarizationIndex("YX");
    }

    if(dims.hasPolarization("YY")
        && find(products.begin(), products.end(), "YY") != products.end())
    {
        itsProductMask[3] = dims.getPolarizationIndex("YY");
    }
}

void Evaluator::process(Mode mode)
{
    BlProcessor blProcessor = 0;
    switch(mode)
    {
    case ASSIGN:
        blProcessor = &Evaluator::blAssign;
        break;
    case SUBTRACT:        
        blProcessor = &Evaluator::blSubtract;
        break;
    case ADD:        
        blProcessor = &Evaluator::blAdd;
        break;
    default:
        THROW(BBSKernelException, "Invalid mode specified.");
    }            

    // Create a request.
    Request request(itsChunk->getDimensions().getGrid());
    
    // Precompute.
    LOG_DEBUG_STR("Precomputing...");
    itsTimers[PRECOMPUTE].reset();
    itsTimers[PRECOMPUTE].start();
    itsModel->precalculate(request);
    itsTimers[PRECOMPUTE].stop();
    LOG_DEBUG_STR("Precomputing... done");

    itsTimers[COMPUTE].reset();
    itsTimers[COMPUTE].start();
#pragma omp parallel for num_threads(itsThreadCount) schedule(dynamic)
    for(int i = 0; i < static_cast<int>(itsBaselines.size()); ++i)
    {
#ifdef _OPENMP
        (this->*blProcessor)(omp_get_thread_num(), itsBaselines[i], request);
#else
        (this->*blProcessor)(0, itsBaselines[i], request);
#endif
    }
    itsTimers[COMPUTE].stop();

    LOG_DEBUG("Timings:");
    LOG_DEBUG_STR("> Precomputation: " << itsTimers[PRECOMPUTE].getElapsed()
        * 1000.0 << " ms");
    LOG_DEBUG_STR("> Computation: " << itsTimers[COMPUTE].getElapsed()
        * 1000.0 << " ms");
}

void Evaluator::blAssign(uint, const baseline_t &baseline,
    const Request &request)
{
    // Find baseline index.
    const VisDimensions &dims = itsChunk->getDimensions();    
    const uint blIndex = dims.getBaselineIndex(baseline);

    // Evaluate the model.
    JonesResult jresult = itsModel->evaluate(baseline, request);

    // Put the results into a single array for easier handling.
    const Result *modelJRes[4];
    modelJRes[0] = &(jresult.getResult11());
    modelJRes[1] = &(jresult.getResult12());
    modelJRes[2] = &(jresult.getResult21());
    modelJRes[3] = &(jresult.getResult22());

    const uint nChannels = dims.getChannelCount();
    const uint nTimeslots = dims.getTimeslotCount();

    for(size_t i = 0; i < 4; ++i)
    {
        if(itsProductMask[i] == -1)
        {
            continue;
        }
        
        // Get a view on the relevant slice of the chunk.
        typedef boost::multi_array<sample_t, 4>::index_range Range;
        typedef boost::multi_array<sample_t, 4>::array_view<2>::type View;
        
        View vdata(itsChunk->vis_data[boost::indices[blIndex][Range()][Range()]
            [itsProductMask[i]]]);

        // Get pointers to the real and imaginary values.
        const double *re = 0, *im = 0;
        modelJRes[i]->getValue().dcomplexStorage(re, im);

        // Copy visibilities.
        for(size_t ts = 0; ts < nTimeslots; ++ts)
        {
            for(size_t ch = 0; ch < nChannels; ++ch)
            {
                vdata[ts][ch] = sample_t(*re, *im);
                ++re;
                ++im;
            }
        }
    }
}

void Evaluator::blSubtract(uint, const baseline_t &baseline,
    const Request &request)
{
    // Find baseline index.
    const VisDimensions &dims = itsChunk->getDimensions();    
    const uint blIndex = dims.getBaselineIndex(baseline);

    // Evaluate the model.
    JonesResult jresult = itsModel->evaluate(baseline, request);

    // Put the results into a single array for easier handling.
    const Result *modelJRes[4];
    modelJRes[0] = &(jresult.getResult11());
    modelJRes[1] = &(jresult.getResult12());
    modelJRes[2] = &(jresult.getResult21());
    modelJRes[3] = &(jresult.getResult22());

    const uint nChannels = dims.getChannelCount();
    const uint nTimeslots = dims.getTimeslotCount();

    for(size_t i = 0; i < 4; ++i)
    {
        if(itsProductMask[i] == -1)
        {
            continue;
        }
        
        // Get a view on the relevant slice of the chunk.
        typedef boost::multi_array<sample_t, 4>::index_range Range;
        typedef boost::multi_array<sample_t, 4>::array_view<2>::type View;
        
        View vdata(itsChunk->vis_data[boost::indices[blIndex][Range()][Range()]
            [itsProductMask[i]]]);

        // Get pointers to the real and imaginary values.
        const double *re = 0, *im = 0;
        modelJRes[i]->getValue().dcomplexStorage(re, im);

        // Subtract from visibilities.
        for(size_t ts = 0; ts < nTimeslots; ++ts)
        {
            for(size_t ch = 0; ch < nChannels; ++ch)
            {
                vdata[ts][ch] -= sample_t(*re, *im);
                ++re;
                ++im;
            }
        }
    }
}

void Evaluator::blAdd(uint, const baseline_t &baseline,
    const Request &request)
{
    // Find baseline index.
    const VisDimensions &dims = itsChunk->getDimensions();    
    const uint blIndex = dims.getBaselineIndex(baseline);

    // Evaluate the model.
    JonesResult jresult = itsModel->evaluate(baseline, request);

    // Put the results into a single array for easier handling.
    const Result *modelJRes[4];
    modelJRes[0] = &(jresult.getResult11());
    modelJRes[1] = &(jresult.getResult12());
    modelJRes[2] = &(jresult.getResult21());
    modelJRes[3] = &(jresult.getResult22());

    const uint nChannels = dims.getChannelCount();
    const uint nTimeslots = dims.getTimeslotCount();

    for(size_t i = 0; i < 4; ++i)
    {
        if(itsProductMask[i] == -1)
        {
            continue;
        }
        
        // Get a view on the relevant slice of the chunk.
        typedef boost::multi_array<sample_t, 4>::index_range Range;
        typedef boost::multi_array<sample_t, 4>::array_view<2>::type View;
        
        View vdata(itsChunk->vis_data[boost::indices[blIndex][Range()][Range()]
            [itsProductMask[i]]]);

        // Get pointers to the real and imaginary values.
        const double *re = 0, *im = 0;
        modelJRes[i]->getValue().dcomplexStorage(re, im);

        // Add from visibilities.
        for(size_t ts = 0; ts < nTimeslots; ++ts)
        {
            for(size_t ch = 0; ch < nChannels; ++ch)
            {
                vdata[ts][ch] += sample_t(*re, *im);
                ++re;
                ++im;
            }
        }
    }
}

} //# namespace BBS
} //# namespace LOFAR
