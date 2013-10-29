
//# tSubbandWriter.cc
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

#include <lofar_config.h>

#include <string>
#include <omp.h>
#include <UnitTest++.h>

#include <CoInterface/DataFactory.h>
#include <CoInterface/CorrelatedData.h>
#include <CoInterface/Stream.h>
#include <OutputProc/Writer.h>
#include <Stream/PortBroker.h>

using namespace std;
using namespace LOFAR;
using namespace Cobalt;

SUITE(SubbandWriter)
{
  struct OneBeam {
    Parset ps;

    OneBeam() {
      ps.add("Observation.ObsID",                                    "0");
      ps.add("Observation.startTime",                                "2013-01-01 00:00");
      ps.add("Observation.stopTime",                                 "2013-01-01 01:00");

      ps.add("Observation.nrBeams",                                  "1");
      ps.add("Observation.Beam[0].subbandList",                      "[0]");
      ps.add("Observation.DataProducts.Output_Correlated.enabled",   "true");
      ps.add("Observation.DataProducts.Output_Correlated.filenames", "[tWriter.out_raw]");
      ps.add("Observation.DataProducts.Output_Correlated.locations", "[localhost:.]");
      ps.updateSettings();
    }
  };

  TEST_FIXTURE(OneBeam, Construction)
  {
    SubbandWriter w(ps, 0, "");
  }

  TEST_FIXTURE(OneBeam, IO)
  {
    SubbandWriter w(ps, 0, "");

    // process, and provide input
#   pragma omp parallel sections
    {
#     pragma omp section
      {
        w.process();
      }

#     pragma omp section
      {
        string sendDesc = getStreamDescriptorBetweenIONandStorage(ps, CORRELATED_DATA, 0);

        SmartPtr<Stream> inputStream = createStream(sendDesc, false, 0);

        SmartPtr<CorrelatedData> data = dynamic_cast<CorrelatedData*>(newStreamableData(ps, CORRELATED_DATA, 0));

        for (size_t i = 0; i < data->visibilities.num_elements(); ++i) {
          *(data->visibilities.origin() + i) = complex<float>(i, 2*i);
        }

        data->write(inputStream, true, 1);
      }
    }

    // verify output
    {
      FileStream f("tWriter.out_raw/table.f0data");

      SmartPtr<CorrelatedData> data = dynamic_cast<CorrelatedData*>(newStreamableData(ps, CORRELATED_DATA, 0));
      data->read(&f, true, 1);

      for (size_t i = 0; i < data->visibilities.num_elements(); ++i) {
        CHECK_EQUAL(complex<float>(i, 2*i), *(data->visibilities.origin() + i));
      }
    }
  }
}

int main()
{
  INIT_LOGGER("tWriter");

  omp_set_nested(true);

  PortBroker::createInstance(storageBrokerPort(0));

  return UnitTest::RunAllTests() > 0;
}

