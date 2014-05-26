//# tBandPass.cc
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

#include <iostream> // TMP
#include <iomanip>

#include <Common/LofarLogger.h>
#include <CoInterface/fpequals.h>
#include <GPUProc/BandPass.h>

using LOFAR::Cobalt::BandPass::computeCorrectionFactors;
using LOFAR::Cobalt::fpEquals;

// Simple test with a priori known, assumed correct output,
// so we can see sudden breakage on changes.

int main() {
  int status = 0;

  INIT_LOGGER("tBandPass");

  const unsigned nrChannelsPerSubband = 4096;
  float *factors = new float[nrChannelsPerSubband];
  const double scale = 1.0 / 256;

  computeCorrectionFactors(factors, nrChannelsPerSubband, scale);

  // Only check a few values.
  const double epsilon(1e-9); // just allow BandPass.cc to internally work in single precision in case we want to switch back
  double refVal;


  refVal = 0.005477008875459432602;
  if (!fpEquals(refVal, (double)factors[0], epsilon)) {
    LOG_ERROR_STR("refVal=" << std::setprecision(17) << refVal << " factors[0]=" <<
                  factors[0] << " epsilon=" << epsilon);
    status = 1;
  }

  refVal = 0.005476939957588911057;
  if (!fpEquals(refVal, (double)factors[1], epsilon)) {
    LOG_ERROR_STR("refVal=" << std::setprecision(17) << refVal << " factors[1]=" <<
                  factors[0] << " epsilon=" << epsilon);
    status = 1;
  }

  refVal = 0.003918072674423456192;
  if (!fpEquals(refVal, (double)factors[2608], epsilon)) {
    LOG_ERROR_STR("refVal=" << std::setprecision(17) << refVal << " factors[2608]=" <<
                  factors[0] << " epsilon=" << epsilon);
    status = 1;
  }

  // symmetry
  if (!fpEquals((double)factors[1], (double)factors[nrChannelsPerSubband-1], epsilon)) {
    LOG_ERROR_STR("factors[1]=" << std::setprecision(17) << refVal << " factors[" <<
                  nrChannelsPerSubband-1 << "]=" << factors[0] << " epsilon=" << epsilon);
    status = 1;
  }


  delete[] factors;

  return status;
}

