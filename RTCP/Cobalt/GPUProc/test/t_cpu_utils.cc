//# t_cpu_utils.cc: test cpu utilities
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

#include <GPUProc/cpu_utils.h>

#include <cstring>
#include <sched.h>
#include <mpi.h>
#include <string>
#include <iostream>

#include <Common/LofarLogger.h>
#include <Common/SystemCallException.h>
#include <Common/SystemUtil.h>
#include <CoInterface/Parset.h>

using namespace std;
using namespace LOFAR::Cobalt;

static int test(const unsigned nprocs, unsigned cpuId)
{
  int status = 0;

  setProcessorAffinity(cpuId);

  // Validate the correct setting of the affinity
  cpu_set_t mask;  
  if (sched_getaffinity(0, sizeof(cpu_set_t), &mask) != 0)
    THROW_SYSCALL("sched_getaffinity");

  // expect alternating on cbt nodes
  // (the original test code intended this, but was broken in many ways (still a poor idea to make it so specific))
  unsigned expect = !cpuId;
  for (unsigned i = 0; i < nprocs; i++) {
    if (CPU_ISSET(i, &mask) != expect) {
      LOG_FATAL_STR("cpuId=" << cpuId << " Found that core " << i << " is" << (!expect ? " " : " NOT ") <<
                    "in the set while it should" << (expect ? " " : " NOT ") << "be!");
      status = 1;
    }
    expect ^= 1;
  }

  return status;
}

int main()
{
  INIT_LOGGER("t_cpu_utils");

  string name(LOFAR::myHostname(false));
  if (strncmp(name.c_str(), "cbt", sizeof("cbt")-1))
  {
    cout << "Test is not running on cobalt hardware and therefore skipped: " << name << endl;
    return 0;
  }

  Parset ps("t_cpu_utils.in_parset");
  
  const unsigned nprocs = sysconf( _SC_NPROCESSORS_ONLN );

  int status = 0;
  status |= test(nprocs, 0);
  status |= test(nprocs, 1);
  return status;
}

