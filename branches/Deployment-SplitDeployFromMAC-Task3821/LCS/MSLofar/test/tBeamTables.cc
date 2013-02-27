//# tBeamTables.cc: Test program for class BeamTables
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
//#
//# @author Ger van Diepen

#include <lofar_config.h>

#include <MSLofar/BeamTables.h>
#include <MSLofar/FailedTileInfo.h>

#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ArrColDesc.h>
#include <measures/TableMeasures/TableQuantumDesc.h>
#include <measures/TableMeasures/ArrayQuantColumn.h>
#include <casa/Quanta/MVTime.h>
#include <casa/OS/Path.h>

using namespace LOFAR;
using namespace casa;

void showTab (const String& name)
{
  cout << endl << "  Table " << name << " ..." << endl;
  Table tab (name);
  ROTableRow row(tab);
  for (uint i=0; i<tab.nrow(); ++i) {
    row.get(i).print (cout, -1, "    ");
  }
}

void testBM (const string& antSet,
             const String& suffix1,
             const String& suffixa0 = String(),
             const String& suffixa1 = String(),
             bool makeObs = false)
{
  {
    cout << endl << "Test " << antSet << " ....." << endl;
    // Make a simple table tBeamTables_tmp.ms with ANTENNA subtable.
    // The ANTENNA table only needs columns NAME and DISH_DIAMETER.
    SetupNewTable stab1("tBeamTables_tmp.ms", TableDesc(), Table::New);
    Table tab1(stab1);
    TableDesc td;
    td.addColumn (ScalarColumnDesc<String>("NAME"));
    td.addColumn (ScalarColumnDesc<double>("DISH_DIAMETER"));
    SetupNewTable stab2("tBeamTables_tmp.ms/ANTENNA", td, Table::New);
    Table antTab(stab2, 3);
    // Write a core, remote, and international station into the table.
    // Add a possible suffix (e.g. HBA0).
    ScalarColumn<String> nameCol(antTab, "NAME");
    nameCol.put (0, "CS001"+suffix1+suffixa0);
    nameCol.put (1, "RS106"+suffix1);
    nameCol.put (2, "DE601"+suffix1);
    // Write core station more if a second suffix is given.
    if (! suffixa1.empty()) {
      antTab.addRow (1);
      nameCol.put (3, "CS001"+suffix1+suffixa1);
    }
    antTab.flush();
    tab1.rwKeywordSet().defineTable ("ANTENNA", antTab);
    // Optionally add OBSERVATION table.
    if (makeObs) {
      TableDesc td;
      td.addColumn (ArrayColumnDesc<Double> ("TIME_RANGE", IPosition(1,2)));
      TableQuantumDesc tq(td, "TIME_RANGE", Unit("s"));
      tq.write (td);
      SetupNewTable stab3("tBeamTables_tmp.ms/OBSERVATION",
                          td, Table::New);
      Table obsTab(stab3, 1);
      ArrayQuantColumn<double> timeCol (obsTab, "TIME_RANGE");
      Vector<Quantity> times(2);
      MVTime::read (times[0], "12-Oct-2012/12:00:00");
      MVTime::read (times[1], "12-Oct-2012/15:00:00");
      timeCol.put (0, times);
      tab1.rwKeywordSet().defineTable ("OBSERVATION", obsTab);
    }

    // Get current directory to get absolute path name.
    string pwd = Path(".").absoluteName();
    // Now add the beam table info.
    BeamTables::create (tab1, false);
    BeamTables::fill   (tab1, antSet, pwd+"/tBeamTables.in_antset",
                        pwd+"/tBeamTables.in_af", pwd+"/tBeamTables.in_hd",
                        true);
  }
  // Now print all the output (which is checked by assay).
  showTab ("tBeamTables_tmp.ms/ANTENNA");
  showTab ("tBeamTables_tmp.ms/LOFAR_ANTENNA_FIELD");
  showTab ("tBeamTables_tmp.ms/LOFAR_ELEMENT_FAILURE");
  showTab ("tBeamTables_tmp.ms/LOFAR_STATION");
  if (makeObs) {
    showTab ("tBeamTables_tmp.ms/OBSERVATION");
  }
  
  // Update for broken/failed tiles if an OBSERVATION table was written
  // (because FailedTileInfo needs it).
  // First with empty files; thereafter with some broken info.
  if (makeObs) {
    String name[2];
    name[0] = "_empty";
    name[1] = "_filled";
    for (int i=0; i<2; ++i) {
      FailedTileInfo::failedTiles2MS ("tBeamTables_tmp.ms",
                                      "tBeamTables.in_before" + name[i],
                                      "tBeamTables.in_during" + name[i]);
      // Show the resulting flags.
      Table tab("tBeamTables_tmp.ms/LOFAR_ANTENNA_FIELD");
      cout << endl << "ELEMENT_FLAG after FailedTileInfo "
           << name[i] << ':' << endl;
      ROArrayColumn<Bool> flagCol (tab, "ELEMENT_FLAG");
      for (uInt i=0; i<tab.nrow(); ++i) {
        cout << flagCol(i);
      }
      showTab ("tBeamTables_tmp.ms/LOFAR_ELEMENT_FAILURE");
    }
  }
}


int main()
{
  try {
    testBM ("LBA_OUTER", "LBA");
    testBM ("LBA_INNER", "LBA");
    testBM ("HBA_JOINED", "HBA");
    testBM ("HBA_JOINED_INNER", "HBA");
    testBM ("HBA_DUAL", "HBA", "0", "1");
    testBM ("HBA_DUAL_INNER", "HBA", "0", "1", True);
    testBM ("HBA_ZERO", "HBA", "0");
    testBM ("HBA_ZERO_INNER", "HBA", "0");
    testBM ("HBA_ONE",  "HBA", "1");
    testBM ("HBA_ONE_INNER",  "HBA", "1");
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
  }
  return 0;
}
