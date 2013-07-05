//# cuda_utils.cc
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include "cuda_utils.h"

#include <boost/format.hpp>

namespace LOFAR
{
  namespace Cobalt
  {
    namespace cuda
    {
      using namespace std;
      using boost::format;

      CUjit_target computeTarget(const gpu::Device &device)
      {
        unsigned major = device.getComputeCapabilityMajor();
        unsigned minor = device.getComputeCapabilityMinor();

        switch (major) {
          case 0:
            return CU_TARGET_COMPUTE_10;

          case 1:
            switch (minor) {
              case 0:
                return CU_TARGET_COMPUTE_10;
              case 1:
                return CU_TARGET_COMPUTE_11;
              case 2:
                return CU_TARGET_COMPUTE_12;
              case 3:
                return CU_TARGET_COMPUTE_13;
              default:
                return CU_TARGET_COMPUTE_13;
            }

          case 2:
            switch (minor) {
              case 0:
                return CU_TARGET_COMPUTE_20;
              case 1:
                return CU_TARGET_COMPUTE_21;
              default:
                return CU_TARGET_COMPUTE_21;
            }

#if CUDA_VERSION >= 5000
          case 3:
            if (minor < 5) {
              return CU_TARGET_COMPUTE_30;
            } else {
              return CU_TARGET_COMPUTE_35;
            }

          default:
            return CU_TARGET_COMPUTE_35;
#else
          default:
            return CU_TARGET_COMPUTE_30;
#endif

        }
      }


      CUjit_target computeTarget(const vector<gpu::Device> &devices)
      {
#if CUDA_VERSION >= 5000
        CUjit_target minTarget = CU_TARGET_COMPUTE_35;
#else
        CUjit_target minTarget = CU_TARGET_COMPUTE_30;
#endif

        for (vector<gpu::Device>::const_iterator i = devices.begin(); 
             i != devices.end(); ++i) {
          CUjit_target target = computeTarget(*i);

          if (target < minTarget)
            minTarget = target;
        }

        return minTarget;
      }


      string get_virtarch(CUjit_target target)
      {
        switch (target) {
        default:
          return "compute_unknown";

        case CU_TARGET_COMPUTE_10:
          return "compute_10";

        case CU_TARGET_COMPUTE_11:
          return "compute_11";

        case CU_TARGET_COMPUTE_12:
          return "compute_12";

        case CU_TARGET_COMPUTE_13:
          return "compute_13";

        case CU_TARGET_COMPUTE_20:
        case CU_TARGET_COMPUTE_21:
          // 21 not allowed for nvcc --gpu-architecture option value
          return "compute_20";

        case CU_TARGET_COMPUTE_30:
          return "compute_30";

#if CUDA_VERSION >= 5000
        case CU_TARGET_COMPUTE_35:
          return "compute_35";
#endif
        }
      }


      string get_gpuarch(CUjit_target target)
      {
        switch (target) {
        default:
          return "sm_unknown";

        case CU_TARGET_COMPUTE_10:
          return "sm_10";

        case CU_TARGET_COMPUTE_11:
          return "sm_11";

        case CU_TARGET_COMPUTE_12:
          return "sm_12";

        case CU_TARGET_COMPUTE_13:
          return "sm_13";

        case CU_TARGET_COMPUTE_20:
          return "sm_20";

        case CU_TARGET_COMPUTE_21:
          return "sm_21";

        case CU_TARGET_COMPUTE_30:
          return "sm_30";

#if CUDA_VERSION >= 5000
        case CU_TARGET_COMPUTE_35:
          return "sm_35";
#endif
        }
      }


    } // namespace cuda

  } // namespace Cobalt

} // namespace LOFAR

