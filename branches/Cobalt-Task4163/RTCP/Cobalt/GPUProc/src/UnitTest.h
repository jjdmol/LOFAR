//# UnitTest.h
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#ifndef LOFAR_GPUPROC_UNITTEST_H
#define LOFAR_GPUPROC_UNITTEST_H

#include <vector>
#include <iostream>
#include <complex>

#include <CoInterface/Parset.h>
#include "PerformanceCounter.h"
#include "opencl-incl.h"

namespace LOFAR
{
  namespace Cobalt
  {
    class UnitTest
    {
    protected:
      UnitTest(const Parset &ps, const char *programName = 0);

      template <typename T>
      void check(T actual, T expected)
      {
        if (expected != actual) {
          std::cerr << "Test FAILED: expected " << expected << ", computed " << actual << std::endl;
          exit(1);
        } else {
          std::cout << "Test OK" << std::endl;
        }
      }

      cl::Context context;
      std::vector<cl::Device> devices;
      cl::Program program;
      cl::CommandQueue queue;

      PerformanceCounter counter;


      /**
       * Fuzzy compare of floating point value val with ref.
       * Use absolute error around 0.0 (ref within epsilon), otherwise use relative error.
       *
       * \param[in] val          value to test
       * \param[in] ref          reference value to test against
       * \param[in] epsilon      max absolute difference. Must be positive.
       *
       * A good epsilon depends on the computation, but pick something reasonable for our single precision tests and allow override.
       *
       * Return true if val is close enough to ref, false otherwise (including if val or ref is NaN).
       */
      bool fpEquals(double val, double ref, double epsilon = 1.0e-5) const;

      /**
       * See fpEquals(), but for complex values.
       */
      bool cfpEquals(std::complex<double> val, std::complex<double> ref, double epsilon = 1.0e-5) const;
    };
  }
}

#endif

