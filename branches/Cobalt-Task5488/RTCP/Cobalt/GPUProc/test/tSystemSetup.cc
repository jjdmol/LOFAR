//# tSystemSetup.cc: test system initialisation
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

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>
#include <GPUProc/SystemSetup.h>
#include <UnitTest++.h>

#include <mpi.h>
#include <iostream>
#include <string>
#include <sched.h>

using namespace std;
using namespace LOFAR::Cobalt;

TEST(NoNodeAllocation)
{
  SystemSetup system(NULL);
}

TEST(BindToCPU0)
{
  ObservationSettings::Node node;
  
  node.name = "";
  node.hostName = "";
  node.cpu = 0;
  node.gpus = vector<unsigned>(0);
  node.nic = "";

  SystemSetup system(&node);
}

int main()
{
  INIT_LOGGER("tSystemSetup");

  return UnitTest::RunAllTests() > 0;
}

