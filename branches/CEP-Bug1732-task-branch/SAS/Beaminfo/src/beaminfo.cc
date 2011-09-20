//# msFailedTilesTable.cc: add and update failed tiles info to the MeasurementSet 
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
//# $Id: addbeaminfo.cc 18832 2011-09-19 17:22:32Z duscha $
//#
//# @author Sven Duscha

#include <lofar_config.h>

//#include <MSLofar/BeamTables.h>
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>   // for ASSERT and ASSERTSTR?

#include <OTDB/OTDBconstants.h>
#include <OTDB/OTDBconnection.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/TreeValue.h>
#include <boost/date_time.hpp>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/Quanta/MVTime.h>
#include <casa/OS/Time.h>

using namespace LOFAR;
using namespace LOFAR::OTDB;
using namespace casa;


boost::posix_time::ptime fromCasaTime (const MVEpoch& epoch, double addDays);
void getSASInfo (const string& antSet,
                 const MVEpoch& beginTime, const MVEpoch& endTime);

int main (int argc, char* argv[])
{
  try {
    string parsetName = "msFailedTiles.parset";
    if (argc > 1) {
      parsetName = argv[1];
    }
    ParameterSet parset(parsetName);
    string msName      = parset.getString("ms");
    string antSet      = parset.getString("antennaset", "");
    string antSetFile  = parset.getString("antennasetfile",
                                          "/opt/cep/lofar/share/AntennaSets.conf");
    string antFieldDir = parset.getString("antennafielddir",
                                          "/opt/cep/lofar/share/AntennaFields");
    string hbaDeltaDir = parset.getString("ihbadeltadir",
                                          "/opt/cep/lofar/share/iHBADeltas");
    bool   overwrite   = parset.getBool  ("overwrite", true);
    MeasurementSet ms(msName, Table::Update);
    // If needed, try to get the AntennaSet name from the Observation table.
    if (antSet.empty()) {
      if (ms.observation().tableDesc().isColumn ("ANTENNA_SET")) {
        ROScalarColumn<String> antSetCol(ms.observation(), "ANTENNA_SET");
        antSet = antSetCol(0);
      }
    }
    ASSERTSTR (!antSet.empty(), "No ANTENNASET found in Observation table of "
               << msName << " or in keyword 'antennaset' in ParSet file");
    
    MSObservationColumns obsColumns(ms.observation());
    Vector<MEpoch> obsTimes (obsColumns.timeRangeMeas()(0));
//    BeamTables::create (ms, overwrite);
//    BeamTables::fill   (ms, antSet, antSetFile, antFieldDir, hbaDeltaDir, true);
    getSASInfo (antSet, obsTimes[0].getValue(), obsTimes[1].getValue());
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

boost::posix_time::ptime fromCasaTime (const MVEpoch& epoch, double addDays=0)
{
  MVTime t (epoch.get() + addDays);
  return boost::posix_time::from_iso_string (t.getTime().ISODate());
}

void getSASInfo (const string& antSet,
                 const MVEpoch& beginTime, const MVEpoch& endTime)
{
  // Make connection to database.
  cout << "making connection ..." << endl;
  OTDBconnection connection("diepen", "", "TESTLOFAR_2", "RS005.astron.nl");
  if (! connection.connect()) {
    cout << connection.errorMsg() << endl;
    return;
  }
///OTDBconnection connection("postgres", "", "LOFAR_2", "sas.control.lofar.eu");
  // Get the tree for the operational hardware. Its id will be used.
  // There should be one tree only.
  cout << "getting tree list ..." << endl;
  vector<OTDBtree> trees = connection.getTreeList (TThardware, TCoperational);
  ASSERT (trees.size() == 1);
  TreeValue treeVal (&connection, trees[0].treeID());
  // Get the nodeId for the LOFAR.PIC tree.
  // There should be only one node.
  cout << "getting treeMaintenance ..." << endl;
  TreeMaintenance treeMaintenance (&connection);
  cout << "getting node list ..." << endl;
  vector<OTDBnode> nodes = treeMaintenance.getItemList (trees[0].treeID(),
                                                        "LOFAR.PIC");
  ASSERT (nodes.size() == 1);
  // Find the most recent entries in the month before the start of the
  // observation. 
  cout << "find values ..." << endl;
  vector<OTDBvalue> values = treeVal.searchInPeriod
    (nodes[0].nodeID(), 7,
     fromCasaTime(beginTime, -31), fromCasaTime(beginTime),
     true);
  cout << "list size = " << values.size() << endl;
  // Find all entries during the observation.
  // Only use the elements that broke during the observation.
  // A name looks like:
  //    LOFAR.PIC.Core.CS002.Cabinet1.Subrack2.RSPBoard8.RCU68.status_state
  // So at least 8 dots need to be present.
  for (vector<OTDBvalue>::const_iterator iter=values.begin();
         iter != values.end(); ++iter) {
    const string& name = iter->name;
  }
}
