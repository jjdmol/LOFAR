//# tFFT.cc: test the FFT kernel
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

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>

#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <GPUProc/Kernels/FFT_Kernel.h>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::Cobalt;

size_t nrErrors = 0;

int main() {
  INIT_LOGGER("tFFT");

  gpu::Platform pf;
  gpu::Device device(0);
  gpu::Context ctx(device);

  gpu::Stream stream;

  const unsigned fftSize = 256;
  const unsigned nrFFTs = 1;

  const size_t size = 1024 * 1024;
  gpu::HostMemory inout(size  * sizeof(fcomplex));
  gpu::DeviceMemory d_inout(size  * sizeof(fcomplex));

  FFT_Kernel fftFwdKernel(fftSize, nrFFTs, true, d_inout);
  FFT_Kernel fftBwdKernel(fftSize, nrFFTs, false, d_inout);

  // First run two very basic tests, then a third test with many transforms with FFTW output as reference.
  // All tests run complex-to-complex float, in-place.

  // Test 1: Impulse at origin

  // init buffers
  for (size_t i = 0; i < size; i++) {
    inout.get<fcomplex>()[i] = fcomplex(0.0f, 0.0f);
  }

  inout.get<fcomplex>()[0] = fcomplex(1.0f, 0.0f);

  // Forward FFT: compute and I/O
  stream.writeBuffer(d_inout, inout);
  fftFwdKernel.enqueue(stream);
  stream.readBuffer(inout, d_inout, true);

  // verify output

  // Check for constant function in transfer domain. All real values 1.0 (like fftw, scaled). All imag must be 0.0.
  for (unsigned i = 0; i < fftSize; i++) {
    if (inout.get<fcomplex>()[i] != fcomplex(1.0f, 0.0f)) {
      if (++nrErrors < 100) { // limit spam
        cerr << "fwd: " << i << ':' << inout.get<fcomplex>()[i] << endl;
      }
    }
  }

  // Backward FFT: compute and I/O
  fftFwdKernel.enqueue(stream);
  stream.readBuffer(inout, d_inout, true);

  // See if we got only our scaled impuls back.
  if (inout.get<fcomplex>()[0] != fcomplex((float)fftSize, 0.0f)) {
    nrErrors += 1;
    cerr << "bwd: " << inout.get<fcomplex>()[0] << " at idx 0 should have been " << fcomplex((float)fftSize, 0.0f) << endl;
  }

  for (unsigned i = 1; i < fftSize; i++) {
    if (inout.get<fcomplex>()[i] != fcomplex(0.0f, 0.0f)) {
      if (++nrErrors < 100) {
        cerr << "bwd: " << i << ':' << inout.get<fcomplex>()[i] << endl;
      }
    }
  }

  if (nrErrors > 0)
  {
    cerr << "Error: " << nrErrors << " unexpected output values" << endl;
  }

  return nrErrors > 0;
}

