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
#include <Common/SystemUtil.h>    // needed for basename

#include <OTDB/OTDBconstants.h>
#include <OTDB/OTDBconnection.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/TreeValue.h>
#include <OTDB/ClassifConv.h>
//#include <OTDB/OTDBtypes.h>
#include <OTDB/Converter.h>
#include <OTDB/TreeTypeConv.h>

#include <boost/date_time.hpp>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/Quanta/MVTime.h>
#include <casa/OS/Time.h>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::OTDB;
using namespace casa;

boost::posix_time::ptime fromCasaTime (const MVEpoch& epoch, double addDays);
void readStations(const string &MSname, vector<string> &stations);

// DEBUG output functions
void showTreeList(const vector<OTDBtree>&	trees);
void showNodeList(const vector<OTDBnode>&	nodes);
void showValueList(const vector<OTDBvalue>&	items);

void getBrokenHardware( OTDBconnection &conn, 
                        vector<string> &brokenHardware,
                        const MVEpoch &timestamp=0);

/*void getSASInfo (const string& antSet,
                 const MVEpoch& beginTime, 
                 const MVEpoch& endTime);
void getSASInfo (const string& antSet,
                 const MVEpoch& beginTime, 
                 const MVEpoch& endTime,
                 vector<string> &dAntennaTiles);
*/
void getSASFailureTimes();

int main (int argc, char* argv[])
{
  vector<string> antennaTiles;
  vector<MVEpoch> failingTimes;

  // Init logger
  string progName = basename(argv[0]);
  INIT_LOGGER(progName);

  // Parse parset entries
  try {
    string parsetName = "msFailedTiles.parset";
    if (argc > 1) {
      parsetName = argv[1];
    }

    LOG_INFO_STR("Reading parset: " << parsetName);

    ParameterSet parset(parsetName);
    string msName      = parset.getString("ms");

    string antSet      = parset.getString("antennaset", "");
    //string host        = parset.getString("host", "sas.control.lofar.eu");
    string host        = parset.getString("host", "RS005.astron.nl");
    string db          = parset.getString("db", "TESTLOFAR_3");
    string user        = parset.getString("user", "paulus");
    string password    = parset.getString("password", "boskabouter");
/*
    string antSetFile  = parset.getString("antennasetfile",
                                          "/opt/cep/lofar/share/AntennaSets.conf");
    string antFieldDir = parset.getString("antennafielddir",
                                          "/opt/cep/lofar/share/AntennaFields");
//    string hbaDeltaDir = parset.getString("ihbadeltadir",
//                                          "/opt/cep/lofar/share/iHBADeltas");
    bool   overwrite   = parset.getBool  ("overwrite", true);
    */
    
    /*
    LOG_INFO_STR("Updating MeasurementSet: " << msName);
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
    
    LOG_INFO_STR("Reading observation times from MS");
    MSObservationColumns obsColumns(ms.observation());
    Vector<MEpoch> obsTimes (obsColumns.timeRangeMeas()(0));
    */

//    BeamTables::create (ms, overwrite);
//    BeamTables::fill   (ms, antSet, antSetFile, antFieldDir, hbaDeltaDir, true);

    LOG_INFO_STR("Getting SAS antenna health information");
    OTDBconnection conn(user, password, db, host);
   
    LOG_INFO("Trying to connect to the database");
    ASSERTSTR(conn.connect(), "Connnection failed");
    LOG_INFO_STR("Connection succesful: " << conn);

    vector<string> brokenDipoles;
    getBrokenHardware(conn, brokenDipoles);
   
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  
  
  LOG_INFO_STR ("Terminated succesfully: " << argv[0]);
  
  return 0;
}

/*!
  \brief Convert casa epoch to posix time
  \param epoch      casa epoch
  \param addDays    add days (default=0)
*/
boost::posix_time::ptime fromCasaTime (const MVEpoch& epoch, double addDays=0)
{
  MVTime t (epoch.get() + addDays);
  return boost::posix_time::from_iso_string (t.getTime().ISODate());
}

/*!
  \brief Read stations from MS ANTENNA table
  \param MSname     name of MeasurementSet
  \param stations   LOFAR stations in ANTENNA table
*/
void readStations(const string &MSname, vector<string> &stations)
{
  // Open MS ANTENNA table
  
  // get entries and put them into vector
}


/*!
  \brief Get a list of all RCUs corresponding to this station
  \param connection OTDB connection to SAS
  \param mode       configuration mode: LBA_Inner, LBA_Outer, HBA0, HBA1, HBA_DUAL, HBA_Combined?
*/
void getRCUs( const OTDBconnection &connection, 
              const string &station ,
              const string &mode, 
              vector<string> &rcus)
{

}

/*!
  \brief Get a complete list of all RCUs taking part in the observation using stations
  \param stations       list of LOFAR stations
  \param stationrcus    list of 
*/
void getObservationDipoles( const OTDBconnection &conn,
                            const vector<string> &stations, 
                            const string &mode, 
                            vector<string> &stationdipoles)
{
  // Loop over stations
    
    // getRCUs depending on mode
    
  // Loop over stationdipole vector
  for (vector<string>::iterator it = stationdipoles.begin(); it!=stationdipoles.end(); ++it) 
  {
    // check in SAS if they were broken
  }



}

/*!
  \brief Get all broken hardware from SAS with startTime and endTime
  \param connection     OTDB connection to SAS
  \param timestamp      timestamp to check for broken hardware at
  \param brokenHardware list of broken hardware
*/
void getBrokenHardware( OTDBconnection &conn, 
                        vector<string> &brokenHardware,
                        const MVEpoch &timestamp)
{
  TreeTypeConv TTconv(&conn);     // TreeType converter object
  ClassifConv CTconv(&conn);      // converter I don't know
  vector<OTDBvalue> valueList;    // OTDB value list
  
  // Get list of all broken hardware from SAS for timestamp
  LOG_INFO("Searching for a Hardware tree");
  vector<OTDBtree>    treeList = conn.getTreeList(TTconv.get("hardware"), CTconv.get("operational"));
  showTreeList(treeList);
  ASSERTSTR(treeList.size(),"No hardware tree found, run tPICtree first");
  
  treeIDType  treeID = treeList[treeList.size()-1].treeID();
  LOG_INFO_STR ("Using tree " << treeID << " for the tests");
  OTDBtree    treeInfo = conn.getTreeInfo(treeID);
  LOG_INFO_STR(treeInfo);
  
  LOG_INFO("Trying to construct a TreeValue object");
  TreeValue   tv(&conn, treeID);
  
  LOG_INFO_STR("Getting broken hardware (now)");
  valueList = tv.getBrokenHardware();
  showValueList(valueList);
                  
  LOG_INFO_STR("Getting broken hardware at " << timestamp);
  valueList = tv.getBrokenHardware(time_from_string("2010-05-26 07:30:00"));  // DEBUG

  showValueList(valueList);     // DEBUG output
}

/*!
  \brief Create LOFAR_ANTENNA_FIELD table
  \param MSname         name of MS to create antenna
  \param RCUs           list of RCUs used in this observation
  \param failureTimes   time an antenna tile failed (optional)
  \param overwrite      overwrite existing table (default=True)
*/
void createAntennaFieldTable( const std::string &MSname, 
                              const vector<string> &rcus, 
                              bool overwrite=true)
{


}

/*!
  \brief Create antenna table with failed antenna tiles and their times of failure
*/
void createFailedAntennaTilesTable()
{

}



//
// showTreeList
//
void showTreeList(const vector<OTDBtree>&	trees)
{


	cout << "treeID|Classif|Creator   |Creationdate        |Type|Campaign|Starttime" << endl;
	cout << "------+-------+----------+--------------------+----+--------+------------------" << endl;
	for (uint32	i = 0; i < trees.size(); ++i) {
		string row(formatString("%6d|%7d|%-10.10s|%-20.20s|%4d|%-8.8s|%s",
			trees[i].treeID(),
			trees[i].classification,
			trees[i].creator.c_str(),
			to_simple_string(trees[i].creationDate).c_str(),
			trees[i].type,
			trees[i].campaign.c_str(),
			to_simple_string(trees[i].starttime).c_str()));
		cout << row << endl;
	}

	cout << trees.size() << " records" << endl << endl;
}

//
// showNodeList
//
void showNodeList(const vector<OTDBnode>&	nodes)
{


	cout << "treeID|nodeID|parent|par.ID|index|leaf|name" << endl;
	cout << "------+------+------+------+-----+----+--------------------------------------" << endl;
	for (uint32	i = 0; i < nodes.size(); ++i) {
		string row(formatString("%6d|%6d|%6d|%6d|%5d|%s|%s",
			nodes[i].treeID(),
			nodes[i].nodeID(),
			nodes[i].parentID(),
			nodes[i].paramDefID(),
			nodes[i].index,
			nodes[i].leaf ? " T  " : " F  ",
			nodes[i].name.c_str()));
		cout << row << endl;
	}

	cout << nodes.size() << " records" << endl << endl;
}

//
// show the resulting list of Values
//
void showValueList(const vector<OTDBvalue>&	items) 
{


	cout << "name                                         |value |time" << endl;
	cout << "---------------------------------------------+------+--------------------" << endl;
	for (uint32	i = 0; i < items.size(); ++i) {
		string row(formatString("%-45.45s|%-7.7s|%s",
			items[i].name.c_str(),
			items[i].value.c_str(),
			to_simple_string(items[i].time).c_str()));
		cout << row << endl;
	}

	cout << items.size() << " records" << endl << endl;
}




/*! 
  \brief Get the list of dipoles participating in this observation
  \param antSet     location of the antenna set file
  \param beginTime  begin of the observation
  \param endTime    end of the observation
*/
/*
void getSASInfo (const string& antSet,
                 const MVEpoch& beginTime, 
                 const MVEpoch& endTime)
{
  // Make connection to database.
  LOG_INFO_STR("Making connection ...");
  OTDBconnection connection("paulus", "boskabouter", "TESTLOFAR_3", "RS005.astron.nl");
  if (! connection.connect()) {
    LOG_DEBUG_STR("Connection failed: " << connection.errorMsg() << endl);
    return;
  }
///OTDBconnection connection("postgres", "", "LOFAR_2", "sas.control.lofar.eu");

  // Get the tree for the operational hardware. Its id will be used.
  // There should be one tree only.
  LOG_DEBUG_STR("Getting tree list ...");
  vector<OTDBtree> trees = connection.getTreeList (TThardware, TCoperational);
  ASSERT (trees.size() == 1);
  TreeValue treeVal (&connection, trees[0].treeID());
  // Get the nodeId for the LOFAR.PIC tree.
  // There should be only one node.

  LOG_DEBUG_STR("Getting treeMaintenance ...");
  TreeMaintenance treeMaintenance (&connection);
  
  LOG_DEBUG_STR("Getting node list ...");
  vector<OTDBnode> nodes = treeMaintenance.getItemList (trees[0].treeID(),
                                                        "LOFAR.PIC");
  ASSERT (nodes.size() == 1);
  // Find the most recent entries in the month before the start of the
  // observation. 
  
  cout << "Start " << beginTime << "fromCasa: " << fromCasaTime(beginTime, -31) << endl;
  cout << "EndTime " << endTime << "fromCasa:" << fromCasaTime(beginTime) << endl;
  cout.flush();
 
  LOG_INFO_STR("Find values ...");
  vector<OTDBvalue> values = treeVal.searchInPeriod
    (nodes[0].nodeID(), 7,
     fromCasaTime(beginTime, -31), fromCasaTime(beginTime),
     true);
  
  LOG_INFO_STR("List size = " << values.size());
  // Find all entries during the observation.
  // Only use the elements that broke during the observation.
  // A name looks like:
  //    LOFAR.PIC.Core.CS002.Cabinet1.Subrack2.RSPBoard8.RCU68.status_state
  // So at least 8 dots need to be present.
  for (vector<OTDBvalue>::const_iterator iter=values.begin();
         iter != values.end(); ++iter) {
    const string& name = iter->name;
    
    cout << "name = " << name << endl;// DEBUG


  }
}
*/