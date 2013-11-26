
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
#include <cstdlib>
#include <omp.h>
#include <UnitTest++.h>
#include <boost/format.hpp>

#include <CoInterface/DataFactory.h>
#include <CoInterface/CorrelatedData.h>
#include <CoInterface/Stream.h>
#include <OutputProc/SubbandWriter.h>
#include <Stream/PortBroker.h>

#include <MSLofar/FailedTileInfo.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Quanta/MVTime.h>

using namespace std;
using namespace LOFAR;
using namespace Cobalt;
using namespace casa;
using boost::format;

SUITE(SubbandWriter)
{
  struct OneBeam {
    Parset ps;

    OneBeam() {
      ps.add("Observation.VirtualInstrument.stationList",            "[CS001]");
      ps.add("Observation.Dataslots.CS001LBA.RSPBoardList",          "[0]");
      ps.add("Observation.Dataslots.CS001LBA.DataslotList",          "[0]");
      ps.add("PIC.Core.CS001LBA.position",                           "[0,0,0]");
      ps.add("PIC.Core.CS001LBA.phaseCenter",                        "[0,0,0]");

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

    ~OneBeam() {
      int dummy = system("rm -rf tWriter.out_raw");

      (void)dummy; // satisfy compiler
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
      data->read(&f, true, 512);

      for (size_t i = 0; i < data->visibilities.num_elements(); ++i) {
        CHECK_EQUAL(complex<float>(i, 2*i), *(data->visibilities.origin() + i));
      }
    }
  }

  TEST_FIXTURE(OneBeam, FinalMetaData)
  {
    SubbandWriter w(ps, 0, "");

    // process
#   pragma omp parallel sections
    {
#     pragma omp section
      {
        w.process();
      }

#     pragma omp section
      {
        /* connect & disconnect */
        string sendDesc = getStreamDescriptorBetweenIONandStorage(ps, CORRELATED_DATA, 0);
        SmartPtr<Stream> inputStream = createStream(sendDesc, false, 0);
      }
    }

    // add final meta data
    FinalMetaData finalMetaData;
    const int nonBrokenAntenna    = 1;
    const int brokenAntennaBefore = 2;
    const int brokenAntennaDuring = 3;

    finalMetaData.brokenRCUsAtBegin.push_back( FinalMetaData::BrokenRCU("CS001", "LBA", brokenAntennaBefore, "2012-01-01 12:34") );
    finalMetaData.brokenRCUsDuring.push_back( FinalMetaData::BrokenRCU("CS001", "LBA", brokenAntennaDuring, "2013-01-01 00:30") );

    w.augment(finalMetaData);

    // list failures BEFORE obs
    {
      Table tab("tWriter.out_raw/LOFAR_ANTENNA_FIELD");
      ROArrayColumn<Bool> flagCol(tab, "ELEMENT_FLAG");

      if (flagCol.nrow() == 0) {
        // no final meta data added, because AntennaSets.conf could not be
        // found? Bail.
        cout << "WARNING: Could not check writing failed tile info, because no tile info was written in the first place. Most likely, AntennaSets.conf could not be found." << endl;
        return;
      }

      // print antenna flag array
      cout << flagCol(0);

      // brokenAntennaBefore should be broken
      CHECK_EQUAL(true, flagCol(0)(IPosition(2, 0, brokenAntennaBefore)));
      CHECK_EQUAL(true, flagCol(0)(IPosition(2, 1, brokenAntennaBefore)));

      // nonBrokenAntenna should NOT be broken
      CHECK_EQUAL(false, flagCol(0)(IPosition(2, 0, nonBrokenAntenna)));
      CHECK_EQUAL(false, flagCol(0)(IPosition(2, 1, nonBrokenAntenna)));

      // brokenAntennaDuring should NOT be broken
      CHECK_EQUAL(false, flagCol(0)(IPosition(2, 0, brokenAntennaDuring)));
      CHECK_EQUAL(false, flagCol(0)(IPosition(2, 1, brokenAntennaDuring)));
    }

    // list failures DURING obs
    {
      Table tab("tWriter.out_raw/LOFAR_ELEMENT_FAILURE");
      ROTableRow row(tab);


      // print failures
      cout << "nr element failure rows: " << tab.nrow() << endl;
      for (uint i=0; i<tab.nrow(); ++i) {
        row.get(i).print (cout, -1, "    ");
      }

      // should have ONE antenna failure
      CHECK_EQUAL(1UL, tab.nrow());

      // should be our antenna
      CHECK_EQUAL(0,                   row.get(0).asInt("ANTENNA_FIELD_ID"));
      CHECK_EQUAL(brokenAntennaDuring, row.get(0).asInt("ELEMENT_INDEX"));
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

