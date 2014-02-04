//# SystemSetup.cc: Wraps system-related setup and tear-down routines
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

#include "SystemSetup.h"

#include <Common/LofarLogger.h>
#include <InputProc/OMPThread.h>

#include <omp.h>
#include <cstdlib>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>

#ifdef HAVE_LIBNUMA
#include <numa.h>
#include <numaif.h>
#endif

#include <boost/format.hpp>

#include "OpenMP_Lock.h"
#include "cpu_utils.h"
#include "Storage/SSH.h"

using namespace std;
using boost::format;

namespace LOFAR
{
  namespace Cobalt
  {
    SystemSetup::SystemSetup(struct ObservationSettings::Node *node)
    :
      gpus(init_hardware(node))
    {
      LOG_INFO("----- Initialising Environment");

      init_environment();

      LOG_INFO("----- Initialising Memory Locking");

      init_memlock();

      LOG_INFO("----- Initialising OpenMP");

      init_omp();

      LOG_INFO("----- Initialising SSH library");

      SSH_Init();
    }

    SystemSetup::~SystemSetup()
    {
      LOG_INFO("----- Closing SSH library");

      SSH_Finalize();
    }

    /*
     * Initialise system environment
     */
    void SystemSetup::init_environment()
    {
      // Ignore SIGPIPE, as we handle disconnects ourselves
      if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        THROW_SYSCALL("signal(SIGPIPE)");

      // Ignore SIGTSTP/SIGTTIN/SIGTTOU, we don't want to block on blocking tty
      if (signal(SIGTSTP, SIG_IGN) == SIG_ERR)
        THROW_SYSCALL("signal(SIGTSTP)");
      if (signal(SIGTTIN, SIG_IGN) == SIG_ERR)
        THROW_SYSCALL("signal(SIGTTIN)");
      if (signal(SIGTTOU, SIG_IGN) == SIG_ERR)
        THROW_SYSCALL("signal(SIGTTOU)");

      // Make sure all time is dealt with and reported in UTC
      if (setenv("TZ", "UTC", 1) < 0)
        THROW_SYSCALL("setenv(TZ)");
    }

    /*
     * Initialise NUMA (tie to a specific socket/cpu)
     */
    void SystemSetup::init_numa(unsigned cpuId)
    {
      setProcessorAffinity(cpuId);

#ifdef HAVE_LIBNUMA
      if (numa_available() != -1) {
        // force node + memory binding for future allocations
        struct bitmask *numa_node = numa_allocate_nodemask();
        numa_bitmask_clearall(numa_node);
        numa_bitmask_setbit(numa_node, cpuId);
        numa_bind(numa_node);
        numa_bitmask_free(numa_node);

        // only allow allocation on this node in case
        // the numa_alloc_* functions are used
        numa_set_strict(1);

        // retrieve and report memory binding
        numa_node = numa_get_membind();
        vector<string> nodestrs;
        for (size_t i = 0; i < numa_node->size; i++)
          if (numa_bitmask_isbitset(numa_node, i))
            nodestrs.push_back(str(format("%s") % i));

        // migrate currently used memory to our node
        numa_migrate_pages(0, numa_all_nodes_ptr, numa_node);

        numa_bitmask_free(numa_node);

        using LOFAR::operator<<; // Because C++ scoping is complex and head-ache inducing

        LOG_DEBUG_STR("Bound to memory on nodes " << nodestrs);
      } else {
        LOG_WARN("Cannot bind memory (libnuma says there is no numa available)");
      }
#else
      LOG_WARN("Cannot bind memory (no libnuma support)");
#endif
    }

    /*
     * Initialise GPUs.
     *
     * Returns a list of all GPUs found.
     */
    vector<gpu::Device> SystemSetup::init_gpus()
    {
      gpu::Platform platform;
      LOG_INFO_STR("GPU platform " << platform.getName());

      return platform.devices();
    }

    /*
     * Initialise GPUs.
     *
     * Returns a list of the selected GPUs.
     */
    vector<gpu::Device> SystemSetup::init_gpus(const vector<unsigned> &gpuIds)
    {
      vector<gpu::Device> allDevices(init_gpus());
      vector<gpu::Device> devices;

      for (size_t i = 0; i < gpuIds.size(); ++i) {
        ASSERTSTR(gpuIds[i] < allDevices.size(), "Request to use GPU #" << gpuIds[i] << ", but found only " << allDevices.size() << " GPUs");

        gpu::Device &d = allDevices[gpuIds[i]];

        devices.push_back(d);
      }

      return devices;
    }

    /*
     * Initialise NIC (if provided)
     */
    void SystemSetup::init_nic(const std::string &nic)
    {
      if (nic == "")
        return;

      LOG_DEBUG_STR("Binding to interface " << nic);

      if (setenv("OMPI_MCA_btl_openib_if_include", nic.c_str(), 1) < 0)
        THROW_SYSCALL("setenv(OMPI_MCA_btl_openib_if_include)");
    }

    /*
     * Initialise OpenMP
     */
    void SystemSetup::init_omp()
    {
      // Allow usage of nested omp calls
      omp_set_nested(true);

      // Allow OpenMP thread registration
      OMPThread::init();
    }

    /*
     * Lock application in memory,
     * and raise memory lock limits.
     */
    void SystemSetup::init_memlock()
    {
      // Remove limits on pinned (locked) memory
      struct rlimit unlimited = { RLIM_INFINITY, RLIM_INFINITY };
      if (setrlimit(RLIMIT_MEMLOCK, &unlimited) < 0)
        LOG_WARN("Cannot setrlimit(RLIMIT_MEMLOCK, unlimited)");

      // Lock everything in memory
      if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0)
        LOG_WARN("Cannot mlockall(MCL_CURRENT | MCL_FUTURE)");
    }

    vector<gpu::Device> SystemSetup::init_hardware(struct ObservationSettings::Node *node)
    {
      vector<gpu::Device> gpus;

      // If we are testing we do not want dependency on hardware specific cpu configuration
      if(node) {
        LOG_INFO_STR("I am " << node->name << " running on " << node->hostName);

        LOG_INFO("----- Initialising NUMA bindings");

        init_numa(node->cpu);

        LOG_INFO("----- Initialising GPUs");

        gpus = init_gpus(node->gpus);

        LOG_INFO("----- Initialising NIC");

        init_nic(node->nic);
      } else {
        LOG_INFO("----- Initialising GPUs");

        gpus = init_gpus();
      }

      // Report the GPUs we use
      for (size_t i = 0; i < gpus.size(); ++i)
        LOG_INFO_STR("Bound to GPU " << gpus[i].pciId() << ": " << gpus[i].getName() << ". Compute capability: " <<
                     gpus[i].getComputeCapabilityMajor() << "." <<
                     gpus[i].getComputeCapabilityMinor() <<
                     " global memory: " << (gpus[i].getTotalGlobalMem() / 1024 / 1024) << " Mbyte");


      return gpus;
    }
  }
}
