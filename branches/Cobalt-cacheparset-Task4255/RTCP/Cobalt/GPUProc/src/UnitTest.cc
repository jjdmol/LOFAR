/* UnitTest.cc
 * Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id: $
 */

#include <lofar_config.h>

#include "UnitTest.h"

#include "global_defines.h"
#include "OpenCL_Support.h"
#include "createProgram.h"

namespace LOFAR
{
  namespace Cobalt
  {

    UnitTest::UnitTest(const Parset &ps, const char *programName)
      :
      counter(programName != 0 ? programName : "test", profiling)
    {
      createContext(context, devices);
      queue = cl::CommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);

      if (programName != 0)
        program = createProgram(ps, context, devices, programName);
    }


    bool UnitTest::fpEquals(double val, double ref, double epsilon) const
    {
      double err = std::abs(val - ref);
      if (ref >= 1.0e-1) {
        err /= ref;         // prefer relative error cmp iff away from 0.0
      }
      return err < epsilon;
    }

    bool UnitTest::cfpEquals(std::complex<double> val, std::complex<double> ref, double epsilon) const
    {
      return fpEquals(val.real(), ref.real(), epsilon) &&
             fpEquals(val.imag(), ref.imag(), epsilon);
    }
  }
}

