//# t_complex.cc
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

#if 1
#include <GPUProc/complex.h>
using namespace LOFAR::Cobalt;
#else
#include <complex>
namespace gpu = std;
#endif

int main()
{
  gpu::complex<float> cf(3.14f, 2.72f);
  cf *= 3;
  gpu::complex<double> cd(1.618, 0.577);
  cd *= 3;
  gpu::complex<long double> cl(1.41421356237L, 1.73205080757L);
  cl *= 3;
  std::cout << cf << std::endl;
  std::cout << cd << std::endl;
  std::cout << cl << std::endl;
  cd *= gpu::complex<double>(cf);
  cf *= gpu::complex<float>(cd);
  cl *= cd;
  cf *= cl;
  std::cout << cf << std::endl;
  std::cout << cd << std::endl;
  std::cout << cl << std::endl;
  return 0;
}
