//# tcreateProgram.cc: test CUDA kernel runtime compilation from src file
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

#include <string>
#include <vector>

#include <CoInterface/Parset.h>
#include <GPUProc/createProgram.h>

using namespace std;
using namespace LOFAR::Cobalt;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " <L12345.parset> <kernel.cu>" << endl;
    return 1;
  }

  Parset ps(argv[1]);
  gpu::Device device(0);
  gpu::Context ctx(device);
  vector<string> targets; // unused atm, so can be empty
  string srcFilename(argv[2]);

  try {
    gpu::Module module(createProgram(ps, ctx, targets, srcFilename));
    cout << "Succesfully compiled '" << srcFilename << "'" << endl;

  } catch (gpu::Error& exc) {
    cerr << "CuError: " << exc << endl;
    return 1;
  }

  return 0;
}

