//# FFT_Plan.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

#include "FFT_Plan.h"
#include <GPUProc/gpu_wrapper.h>

#include <map>
#include <utility>      // std::pair, std::make_pair
#include <Common/Thread/Mutex.h>

namespace LOFAR
{
  namespace Cobalt
  {

    Mutex planCacheMutex;
    std::map<pair<unsigned, unsigned>, pair<unsigned, cufftHandle> > planCache;

    FFT_Plan::FFT_Plan(gpu::Context &context, unsigned fftSize, unsigned nrFFTs)
      :
      context(context),
      itsfftSize(fftSize),
      itsnrFFTs(nrFFTs)
    {
      gpu::ScopedCurrentContext scc(context);

      cufftResult error;

      // Use the fftplan parameters to create a key to find a possible 
      // cached fftplan (memory issues if all classes own their own)
      
      ScopedLock sl(planCacheMutex);
      if (planCache.find(make_pair(itsfftSize, itsnrFFTs)) != planCache.end())  // Test if it exists
      {
        // get from cache and assign
        plan = planCache[make_pair(itsfftSize, itsnrFFTs)].second;
        // Record that we have an aditional entry with these settings
        planCache[make_pair(itsfftSize, itsnrFFTs)].first++;
      }
      else
      {
        // Create the itsfftSize
        error = cufftPlan1d(&plan, itsfftSize, CUFFT_C2C, itsnrFFTs);

        if (error != CUFFT_SUCCESS)
          THROW(gpu::CUDAException, "cufftPlan1d: " << gpu::cufftErrorMessage(error));
        // add to cache
        planCache[make_pair(itsfftSize, itsnrFFTs)] = make_pair(1, plan);
       }
    }

    FFT_Plan::~FFT_Plan()
    {
      gpu::ScopedCurrentContext scc(context);
      ScopedLock sl(planCacheMutex);

      // test if the amount fftplans with these settings is 1
      if (planCache[make_pair(itsfftSize, itsnrFFTs)].first == 1)
      { 
        // last one. desctruct the plan
        cufftDestroy(planCache[make_pair(itsfftSize, itsnrFFTs)].second);

        // remove the entry from the map, no need to decrease counter first
        planCache.erase(make_pair(itsfftSize, itsnrFFTs));

      }
      else
        planCache[make_pair(itsfftSize, itsnrFFTs)].first--;

    }

    void FFT_Plan::setStream(const gpu::Stream &stream) const
    {
      cufftResult error;

      gpu::ScopedCurrentContext scc(context);

      error = cufftSetStream(plan, stream.get());

      if (error != CUFFT_SUCCESS)
        THROW(gpu::CUDAException, "cufftSetStream: " << gpu::cufftErrorMessage(error));
    }
  }
}

