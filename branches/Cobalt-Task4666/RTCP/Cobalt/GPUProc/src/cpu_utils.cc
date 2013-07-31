#include <lofar_config.h>

#include <sched.h>
#include <CoInterface/Parset.h>


namespace LOFAR
{
  namespace Cobalt
  {
    // Set the correct processer affinity based on the parset entry
    // settings.nodes[rank].cpu
    void setProcessorAffinity(const LOFAR::Cobalt::Parset &ps, int rank)
    {
      int cpuId = ps.settings.nodes[rank].cpu;

      // Get the number of cpu's (32)
      unsigned numCPU = sysconf(_SC_NPROCESSORS_ONLN );

      // Get a valid cpu set
      // sched_getaffinity(pid, size of cpu set, mask to place data in) //pid 0 = current thread
      cpu_set_t mask;  
      bool sched_return = sched_getaffinity(0, sizeof(cpu_set_t), &mask);

      //// zero the mask 
      CPU_ZERO(&mask) ; 

      // Set the mask, beginning with cpuId and set the corresponding cpus
      // use a step of two, this is cobalt specific. The Cobalt procs 
      // are even/odd for each physical core
      for (unsigned idx_cpu = cpuId; idx_cpu < numCPU; idx_cpu += 2)  
        CPU_SET(idx_cpu, &mask);

      // now assign the mask and set the affinity
      sched_return = sched_setaffinity(0, sizeof(cpu_set_t), &mask);

      if (sched_return != 0)
        LOG_FATAL_STR("Failed setting processor affinity."); 
    }
  }
}
