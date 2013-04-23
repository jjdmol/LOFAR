//# RTCP_UnitTest.cc
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
//# $Id: RTCP_UnitTest.cc 24553 2013-04-09 14:21:56Z mol $

#include <lofar_config.h>

#include <iostream>
#include <CoInterface/Parset.h>

#include <Common/LofarLogger.h>
#include <GPUProc/global_defines.h>
#include "FIR_FilterTest.h"
//

using namespace LOFAR;
using namespace LOFAR::Cobalt;

// Use our own terminate handler
//Exception::TerminateHandler t(OpenCL_Support::terminate);

int main(int argc, char **argv)
{

  INIT_LOGGER("RTCP");
  std::cout << "running ..." << std::endl;

  //if (argc < 2)
  //{
  //  std::cerr << "usage: " << argv[0] << " parset" << std::endl;
  //  return 1;
  //}

  LOFAR::Cobalt::Parset ps(argv[1]);

  //std::cout << "Obs ps: nSt=" << ps.nrStations() 
  //          << " nPol=" << NR_POLARIZATIONS
  //          << " nSampPerCh=" << ps.nrSamplesPerChannel() 
  //          << " nChPerSb="  << ps.nrChannelsPerSubband() << " nTaps=" << ps.nrPPFTaps()
  //          << " nBitsPerSamp=" << ps.nrBitsPerSample() << std::endl;

  (FIR_FilterTest)(ps);

  return 0;
}

