//# tDelayAndBandpass.cc: test delay and bandpass CUDA kernel
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
//# $Id: tDelayAndBandPass.cc 25699 2013-07-18 07:18:02Z mol $

#include <lofar_config.h>

#include <cstdlib>
#include <cmath> 
#include <string>
#include <sstream>
#include <typeinfo>
#include <vector>

#include <Common/Exception.h>
#include <Common/LofarLogger.h>

#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <UnitTest++.h>

#include <sched.h>

#include "TestUtil.h"
#include <hwloc.h>

using namespace std;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;

// 
float * runTest()
{
  // Set up environment
  try {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " CUDA devices" << endl;
  } catch (gpu::CUDAException& e) {
    cerr << e.what() << endl;
    exit(3);
  }
  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);
  Stream cuStream(ctx);
  std::stringstream tostrstream("");

  return NULL;
}

TEST(BandPass)
{
  // The input samples are all ones
  // After correction, multiply with 2.
  // The first and the last complex values are retrieved. They should be scaled with the bandPassFactor == 2
  runTest();
  CHECK(true);
}



int main()
{
  INIT_LOGGER("tProcessorAffinity");
  cout << "We are here." << endl;
  
  cpu_set_t mask;
  //                                    pid, size of cpu set, mask to place data in    
  bool sched_return = sched_getaffinity(0, sizeof(cpu_set_t), &mask);
  cout << "Number of cpu's in the mask: " << CPU_COUNT(&mask) << endl;
  unsigned numCPU = sysconf( _SC_NPROCESSORS_ONLN );
  cout << "Number of cpu's: " << numCPU << endl;
  cout << "cpu mask: "<< endl;
  for (unsigned idx_cpu =0; idx_cpu <numCPU ; ++ idx_cpu)
  {
    cout << CPU_ISSET(idx_cpu, &mask) << ",";
  cout << endl;
  }
  CPU_ZERO(&mask) ;
    for (unsigned idx_cpu =0; idx_cpu < (unsigned)2; ++ idx_cpu)
  {
    CPU_SET(idx_cpu, &mask);
  }
    cout << "cpu mask: "<< endl;
   for (unsigned idx_cpu =0; idx_cpu < numCPU; ++ idx_cpu)
   {
     cout << CPU_ISSET(idx_cpu, &mask) << ",";
   }
   sched_return = sched_setaffinity(0, sizeof(cpu_set_t), &mask);
   cout << "result of set affinity command (should be zer0): " << sched_return << endl;
   
  cout << "Current process is running on processor id: " << sched_getcpu() << endl;
  cout << "resetting the cpu mask" << endl;
  CPU_ZERO(&mask) ;
  for (unsigned idx_cpu =14; idx_cpu < (unsigned)15; ++ idx_cpu)
  {
    CPU_SET(idx_cpu, &mask);
  }
     sched_return = sched_setaffinity(0, sizeof(cpu_set_t), &mask);
   cout << "result of set affinity command (should be zer0): " << sched_return << endl;
   cout << "Current process is running on processor id: " << sched_getcpu() << endl;
  
  cout << "**********************************************" << endl;
  
#     pragma omp parallel sections
  {
# pragma omp section
    {
  
#   pragma omp parallel for num_threads(2)
      for (size_t i = 0; i < 2; ++i) 
      {
          cout << "Current process is running on processor id: " << sched_getcpu() << endl;
      }
    }
  }
  
  int depth;
unsigned i, n;
unsigned long size;
int levels;
char string[128];
int topodepth;
hwloc_topology_t topology;
hwloc_cpuset_t cpuset;
hwloc_obj_t obj;
/* Allocate and initialize topology object. */
hwloc_topology_init(&topology);
  return 0;
  //return UnitTest::RunAllTests() > 0;
    
}

// /* Example hwloc API program.
// *
// * Copyright © 2009-2010 inria. All rights reserved.
// * Copyright © 2009-2011 Université Bordeaux 1
// * Copyright © 2009-2010 Cisco Systems, Inc. All rights reserved.
// * See COPYING in top-level directory.
// *
// * hwloc-hello.c
// */
// #include <hwloc.h>
// #include <errno.h>
// #include <stdio.h>
// #include <string.h>
// static void print_children(hwloc_topology_t topology,
// hwloc_obj_t obj,
// int depth)
// {
// char string[128];
// unsigned i;
// hwloc_obj_snprintf(string, sizeof(string), topology, obj, "#", 0);
// printf("%*s%snn", 2*depth, "", string);
// for (i = 0; i < obj->arity; i++) {
// print_children(topology, obj->children[i], depth + 1);
// }
// }
// int main(void)
// {
// int depth;
// unsigned i, n;
// unsigned long size;
// int levels;
// char string[128];
// int topodepth;
// hwloc_topology_t topology;
// hwloc_cpuset_t cpuset;
// hwloc_obj_t obj;
// /* Allocate and initialize topology object. */
// hwloc_topology_init(&topology);
// /* ... Optionally, put detection configuration here to ignore
// some objects types, define a synthetic topology, etc....
// The default is to detect all the objects of the machine that
// the caller is allowed to access. See Configure Topology
// Detection. */
// /* Perform the topology detection. */
// hwloc_topology_load(topology);
// /* Optionally, get some additional topology information
// in case we need the topology depth later. */
// topodepth = hwloc_topology_get_depth(topology);
// /*****************************************************************
// * First example:
// * Walk the topology with an array style, from level 0 (always
// * the system level) to the lowest level (always the proc level).
// *****************************************************************/
// for (depth = 0; depth < topodepth; depth++) {
// printf("*** Objects at level %dnn", depth);
// for (i = 0; i < hwloc_get_nbobjs_by_depth(topology, depth);
// i++) {
// hwloc_obj_snprintf(string, sizeof(string), topology,
// hwloc_get_obj_by_depth(topology, depth, i),
// "#", 0);
// printf("Index %u: %snn", i, string);
// }
// }
// /*****************************************************************
// * Second example:
// * Walk the topology with a tree style.
// *****************************************************************/
// printf("*** Printing overall treenn");
// print_children(topology, hwloc_get_root_obj(topology), 0);
// /*****************************************************************
// * Third example:
// * Print the number of sockets.
// *****************************************************************/
// depth = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);
// if (depth == HWLOC_TYPE_DEPTH_UNKNOWN) {
// printf("*** The number of sockets is unknownnn");
// } else {
// printf("*** %u socket(s)nn",
// hwloc_get_nbobjs_by_depth(topology, depth));
// }
// /*****************************************************************
// * Fourth example:
// * Compute the amount of cache that the first logical processor
// * has above it.
// *****************************************************************/
// levels = 0;
// size = 0;
// for (obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_PU, 0);
// obj;
// obj = obj->parent)
// if (obj->type == HWLOC_OBJ_CACHE) {
// levels++;
// size += obj->attr->cache.size;
// }
// printf("*** Logical processor 0 has %d caches totaling %luKBnn",
// levels, size / 1024);
// /*****************************************************************
// * Fifth example:
// * Bind to only one thread of the last core of the machine.
// *
// * First find out where cores are, or else smaller sets of CPUs if
// * the OS doesn’t have the notion of a "core".
// *****************************************************************/
// depth = hwloc_get_type_or_below_depth(topology,
// HWLOC_OBJ_CORE);
// /* Get last core. */
// obj = hwloc_get_obj_by_depth(topology, depth,
// hwloc_get_nbobjs_by_depth(topology, depth) - 1);
// if (obj) {
// /* Get a copy of its cpuset that we may modify. */
// cpuset = hwloc_bitmap_dup(obj->cpuset);
// /* Get only one logical processor (in case the core is
// SMT/hyperthreaded). */
// hwloc_bitmap_singlify(cpuset);
// /* And try to bind ourself there. */
// if (hwloc_set_cpubind(topology, cpuset, 0)) {
// char *str;
// int error = errno;
// hwloc_bitmap_asprintf(&str, obj->cpuset);
// printf("Couldn’t bind to cpuset %s: %snn", str, strerror(error));
// free(str);
// }
// /* Free our cpuset copy */
// hwloc_bitmap_free(cpuset);
// }
// /*****************************************************************
// * Sixth example:
// * Allocate some memory on the last NUMA node, bind some existing
// * memory to the last NUMA node.
// *****************************************************************/
// /* Get last node. */
// n = hwloc_get_nbobjs_by_type(topology,
// HWLOC_OBJ_NODE);
// if (n) {
// void *m;
// size = 1024*1024;
// obj = hwloc_get_obj_by_type(topology,
// HWLOC_OBJ_NODE, n - 1);
// m = hwloc_alloc_membind_nodeset(topology, size, obj->
// nodeset,
// HWLOC_MEMBIND_DEFAULT, 0);
// hwloc_free(topology, m, size);
// m = malloc(size);
// hwloc_set_area_membind_nodeset(topology, m, size, obj->
// nodeset,
// HWLOC_MEMBIND_DEFAULT, 0);
// free(m);
// }
// /* Destroy topology object. */
// hwloc_topology_destroy(topology);
// return 0;
// }