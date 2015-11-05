//# SystemSetup.h: Wraps system-related setup and tear-down routines
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
//
#ifndef LOFAR_GPUPROC_SYSTEMSETUP_H
#define LOFAR_GPUPROC_SYSTEMSETUP_H

#include <CoInterface/Parset.h>
#include <GPUProc/gpu_wrapper.h>

#include <vector>
#include <string>

namespace LOFAR
{
  namespace Cobalt
  {
    class SystemSetup
    {
    public:
      const std::vector<gpu::Device> gpus;

      /*
       * Initialise the system, binding to `node' if provided,
       * or the whole machine otherwise.
       *
       * Initialised are:
       *   - Environment variables & signal handlers
       *   - CUDA (Selected GPUs are set in this->gpus)
       *   - NUMA (and other socket bindings)
       *   - SSH library
       */
      SystemSetup(struct ObservationSettings::Node *node = NULL);
      ~SystemSetup();

    private:
      void init_environment();

      vector<gpu::Device> init_hardware(struct ObservationSettings::Node *node);

      void init_numa(unsigned cpuId);
      std::vector<gpu::Device> init_gpus(const std::vector<unsigned> &gpuIds);
      std::vector<gpu::Device> init_gpus();
      void init_nic(const std::string &nic = "");

      void init_memlock();
      void init_omp();
    };
  }
}

#endif
