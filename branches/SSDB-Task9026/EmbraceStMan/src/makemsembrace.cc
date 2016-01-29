//# makemsembrace.cc: Make an MS for an EMBRACE observation
//# Copyright (C) 2012
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
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
#include <EmbraceStMan/EmbraceStMan.h>
#include <EmbraceStMan/Package__Version.h>
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>

#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableCopy.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Containers/BlockIO.h>
#include <casa/IO/AipsIO.h>
#include <casa/OS/File.h>
#include <casa/OS/Path.h>

using namespace EMBRACE;
using namespace LOFAR;
using namespace casa;
using namespace std;

int main (int argc, char* argv[])
{
  TEST_SHOW_VERSION (argc, argv, EmbraceStMan);
  try {
    // Get the default or given parset name.
    string psName("embracems.cfg");
    if (argc > 1) {
      psName = argv[1];
    }
    // Get name of MS and data file and endianness from the parset.
    ParameterSet parset(psName);
    String msName   (parset.getString ("MSName"));
    String fileName (parset.getString ("FileName"));
    Bool bigEndian  (parset.getBool ("BigEndian"));
    Vector<Int> vant1, vant2;
    Block<Int> ant1, ant2;
    double startTime, interval;
    uInt npol, nchan;
    // Turn the data file name into an absolute path.
    // Check if it exists and is regular.
    {
      File file(fileName);
      ASSERTSTR (file.isRegular(False),
                 "Embrace data file " + fileName + " is not a regular file");
      fileName = file.path().absoluteName();
    }
    // Make the MS.
    system (("makems " + psName).c_str());
    // Make EmbraceStMan known to the table system.
    EmbraceStMan::registerClass();
    {
      // Open the MS just created.
      Table msOld (msName);
      // Check it is not empty.
      ASSERTSTR (msOld.nrow() > 0, "Nr of timeslots should be > 0");
      // Iterate over time to get info from the first time slot.
      TableIterator tabIter(msOld, "TIME");
      const Table& tab = tabIter.table();
      startTime = ROScalarColumn<Double> (tab, "TIME")(0);
      interval  = ROScalarColumn<Double> (tab, "INTERVAL")(0);
      startTime -= interval*0.5;
      IPosition shp = ROArrayColumn<Complex> (tab, "DATA").shape(0);
      npol  = shp[0];
      nchan = shp[1];
      vant1 = ROScalarColumn<Int> (tab, "ANTENNA1").getColumn();
      vant2 = ROScalarColumn<Int> (tab, "ANTENNA2").getColumn();
      vant1.toBlock (ant1);
      vant2.toBlock (ant2);
      // Create an empty MS with EmbraceStMan as storage manager.
      SetupNewTable newtab (msName + "_new", msOld.tableDesc(), Table::New);
      EmbraceStMan sm1;
      newtab.bindAll (sm1);
      Table msNew(newtab, 0, False, Table::LocalEndian);
      TableCopy::copySubTables (msNew, msOld);
      msNew.flush();
    }
    {
      // Create the meta file.
      AipsIO aio(msName + "_new/table.f0meta", ByteIO::New);
      aio.putstart ("EmbraceStMan", 1);
      aio << ant1 << ant2
          << startTime << interval << nchan
          << npol << bigEndian << fileName;
      aio.putend();
      // Rename the output MS to the original msname.
    }
    {
      Table ms(msName + "_new", Table::Update);
      ms.rename (msName, Table::New);
    }

  } catch (std::exception& x) {
    cout << "Unexpected exception in " << argv[0] << ": " << x.what() << endl;
    return 1;
  }
  return 0;
}
