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
#include <utility>      // std::pair
#include <Common/Thread/Mutex.h>

namespace LOFAR
{
  namespace Cobalt
  {

    Mutex planCacheMutex;
    std::map<std::pair< CUstream, std::pair<unsigned, unsigned > >, pair<unsigned, cufftHandle> > planCache;

    FFT_Plan::FFT_Plan(gpu::Context &context,  const gpu::Stream &stream, 
                       unsigned fftSize, unsigned nrFFTs)
      :
      context(context),
      itsfftSize(fftSize),
      itsnrFFTs(nrFFTs),
      itsStream(stream)

    {
      gpu::ScopedCurrentContext scc(context);

      cufftResult error;

      // Use the fftplan parameters to create a key to find a possible 
      // cached fftplan (memory issues if all classes own their own)
      
      ScopedLock sl(planCacheMutex);

      std::pair< CUstream, std::pair<unsigned, unsigned > >key = make_pair(
        itsStream.get(), make_pair(itsfftSize, itsnrFFTs));
      if (planCache.find(key) != planCache.end())  // Test if it exists
      {
        // get from cache and assign
        plan = planCache[key].second;
        // Record that we have an aditional entry with these settings
        planCache[key].first++;
      }
      else
      {
        // Create the itsfftSize
        error = cufftPlan1d(&plan, itsfftSize, CUFFT_C2C, itsnrFFTs);

        if (error != CUFFT_SUCCESS)
          THROW(gpu::CUDAException, "cufftPlan1d: " << gpu::cufftErrorMessage(error));
        // add to cache
        planCache[key] = make_pair(1, plan);
       }

      setStream();
    }

    FFT_Plan::~FFT_Plan()
    {
      gpu::ScopedCurrentContext scc(context);
      ScopedLock sl(planCacheMutex);

      // test if the amount fftplans with these settings is 1

      std::pair< CUstream, std::pair<unsigned, unsigned > >key = make_pair(
        itsStream.get(), make_pair(itsfftSize, itsnrFFTs));


      if (planCache[key].first == 1)
      { 
        // last one. desctruct the plan
        cufftDestroy(planCache[key].second);

        // remove the entry from the map, no need to decrease counter first
        planCache.erase(key);

      }
      else
        planCache[key].first--;

    }

    void FFT_Plan::setStream() const
    {
      cufftResult error;

      gpu::ScopedCurrentContext scc(context);

      error = cufftSetStream(plan, itsStream.get());

      if (error != CUFFT_SUCCESS)
        THROW(gpu::CUDAException, "cufftSetStream: " << gpu::cufftErrorMessage(error));
    }
  }
}

