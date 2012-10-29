//# makebeamtables.cc: Program 
//# Copyright (C) 2011
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
//#
//# @author Ger van Diepen

#include <lofar_config.h>
#include <MSLofar/BeamTables.h>
#include <MsLofar/Package__Version.h>
#include <Common/InputParSet.h>
#include <Common/SystemUtil.h>
#include <Common/LofarLogger.h>
#include <tables/Tables/ScalarColumn.h>
#include <ms/MeasurementSets/MeasurementSet.h>

using namespace LOFAR;
using namespace casa;

int main (int argc, char* argv[])
{
  TEST_SHOW_VERSION (argc, argv, MSLofar);
  INIT_LOGGER(basename(string(argv[0])));
  try {
    InputParSet inputs;
    // define the input structure
    inputs.setVersion("2012Oct29-GvD");
    inputs.create ("ms", "",
		   "Name of MeasurementSet",
		   "string");
    inputs.create ("antennaset", "",
		   "Antenna set used (e.g. LBA_INNER)",
		   "string");
    inputs.create ("antennasetfile", "/opt/lofar/etc/AntennaSets.conf",
		   "Name of the AntennaSet file",
		   "string");
    inputs.create ("antennafielddir", "/opt/lofar/etc/StaticMetaData",
		   "Directory where the AntennaField.conf files reside",
		   "string");
    inputs.create ("ihbadeltadir", "/opt/lofar/etc/StaticMetaData",
		   "Directory where the iHBADelta.conf files reside",
		   "string");
    inputs.create ("overwrite", "false",
                   "Overwriting existing beam subtables?",
                   "bool");
    inputs.readArguments (argc, argv);
    String msName      = inputs.getString("ms");
    String antSet      = inputs.getString("antennaset");
    String antSetFile  = inputs.getString("antennasetfile");
    String antFieldDir = inputs.getString("antennafielddir");
    String hbaDeltaDir = inputs.getString("ihbadeltadir");
    Bool   overwrite   = inputs.getBool  ("overwrite");
    MeasurementSet ms(msName, Table::Update);
     // If needed, try to get the AntennaSet name from the Observation table.
    if (antSet.empty()) {
      if (ms.observation().tableDesc().isColumn ("LOFAR_ANTENNA_SET")) {
        ROScalarColumn<String> antSetCol(ms.observation(), "LOFAR_ANTENNA_SET");
        antSet = antSetCol(0);
        LOG_DEBUG_STR ("Using AntennaSet " << antSet
                       << " from OBSERVATION subtable");

      }
    }
    ASSERTSTR (!antSet.empty(), "No LOFAR_ANTENNA_SET found in OBSERVATION"
               " subtable of " << msName);
    BeamTables::create (ms, overwrite);
    BeamTables::fill   (ms, antSet, antSetFile, antFieldDir, hbaDeltaDir, true);
  } catch (Exception& x) {
    cerr << "Unexpected LOFAR exception: " << x << endl;
    return 1;
  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
