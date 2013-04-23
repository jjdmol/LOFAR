//# opencl-wrapper.h
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

#ifndef LOFAR_GPUPROC_OPENCL_OPENCL_WRAPPER_H
#define LOFAR_GPUPROC_OPENCL_OPENCL_WRAPPER_H

#include "opencl-incl.h"

namespace LOFAR {
namespace Cobalt {

  // CUDA is our primary GPU API, so adapt the OpenCL names (except for Buffer).
  // (This is a bit silly given OpenCL's raison d'etre...)

  using cl::Context;

  class Module : cl::Program
  {
    // TODO: add constructors to classes here
  };

  class Function : cl::Kernel
  {
  };

  // Buffer maps to host + device memory objects, so can only really adapt the other way around.
  using cl::Buffer;

  class GPUProcException : cl::Error
  {
  };

  using cl::Device;

  using cl::Event;

  class Stream : cl::CommandQueue
  {
  };

} // namespace Cobalt
} // namespace LOFAR

#endif

