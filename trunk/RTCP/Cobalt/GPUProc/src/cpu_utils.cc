#include <lofar_config.h>

#include <fstream>
#include <sched.h>
#include <boost/format.hpp>
#include <CoInterface/Parset.h>
#include <CoInterface/Exceptions.h>

#include <Common/SystemCallException.h>
#include <CoInterface/PrintVector.h>

namespace LOFAR
{
  namespace Cobalt
  {
    // Set the correct processer affinity
    void setProcessorAffinity(unsigned cpuId)
    {
      // Get the number of cores (32)
      unsigned numCores = sysconf(_SC_NPROCESSORS_ONLN);


      // Determine the cores local to the specified cpuId
      vector<unsigned> localCores;

      for (unsigned core = 0; core < numCores; ++core) {
        // The file below contains an integer indicating the physical CPU
        // hosting this core.
        std::ifstream fs(str(boost::format("/sys/devices/system/cpu/cpu%u/topology/physical_package_id") % core).c_str());

        unsigned physical_cpu;
        fs >> physical_cpu;

        if (!fs.good())
          continue;

        // Add this core to the mask if it matches the requested CPU
        if (physical_cpu == cpuId)
          localCores.push_back(core);
      }

      if (localCores.empty())
        THROW(GPUProcException, "Request to bind to non-existing CPU: " << cpuId);

      LOG_DEBUG_STR("Binding to CPU " << cpuId << ": cores " << localCores);

      // put localCores in a cpu_set
      cpu_set_t mask;  

      CPU_ZERO(&mask); 

      for (vector<unsigned>::const_iterator i = localCores.begin(); i != localCores.end(); ++i)
        CPU_SET(*i, &mask);

      // now assign the mask and set the affinity
      int sched_return = sched_setaffinity(0, sizeof(cpu_set_t), &mask);
      int localerrno = errno;
      if (sched_return != 0)
        throw SystemCallException("sched_setaffinity", localerrno, THROW_ARGS);
    }
  }
}
