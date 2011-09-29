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
#include <boost/lexical_cast.hpp>   // convert number to string
#include <iostream>
#include <fstream>
#include <map>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/Quanta/MVTime.h>
#include <casa/OS/Time.h>
#include <casa/Arrays/VectorIter.h>
#include <casa/Arrays/ArrayIter.h>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::OTDB;
using namespace casa;

// MS reading functions
boost::posix_time::ptime fromCasaTime (const MVEpoch& epoch, double addDays);
string getLofarAntennaSet(const string &msName);
string getLofarAntennaSet(MeasurementSet &ms);
void readObservationTimes(const string &msName, Vector<MEpoch> &obsTimes);
void readObservationTimes(MeasurementSet &ms, Vector<MEpoch> &obsTimes);
void readAntennaFields(const string &msName, vector<string> &antennas);
void readAntennaFields(MeasurementSet &ms, vector<string> &antennas);

// Antenna field functions
void getRCUs( const string &msName,
              map<string, vector<string> > &rcus,
              const string &tableName="LOFAR_ANTENNA_FIELD",
              const string &elementColumnName="ELEMENT_FLAG");
void getRCUs( const string &msName,
              map<string, vector<string> > &rcus,
              const vector<string> &antennaFields,
              const string &tableName="LOFAR_ANTENNA_FIELD",
              const string &elementColumName="ELEMENT_FLAG");
// SAS functions
void getBrokenHardware( OTDBconnection &conn, 
                        vector<string> &brokenHardware,
                        const MVEpoch &timestamp=0);

// DEBUG SAS output functions
void showTreeList(const vector<OTDBtree>&	trees);
void showNodeList(const vector<OTDBnode>&	nodes);
void showValueList(const vector<OTDBvalue>&	items);
void showVector(const vector<string> &v);
void showMap(const map<string, string> &m);
void showMap(const map<string, vector<string> > &m);
void padTo(std::string &str, const size_t num, const char paddingChar);

// MS Table writing functions TODO
void updateAntennaFieldTable( const std::string &msName, 
                              const vector<string> &rcus, 
                              bool overwrite=true);

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

  // Parse command line arguments TODO!

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
    //string host        = parset.getString("host", "sas.control.lofar.eu");  // production
    string host        = parset.getString("host", "RS005.astron.nl");         // DEBUG
    string db          = parset.getString("db", "TESTLOFAR_3");
    string user        = parset.getString("user", "paulus");
    string password    = parset.getString("password", "boskabouter");
    string elementTable = parset.getString("elementTable", "LOFAR_ANTENNA_FIELD");                                          
    string elementColumn = parset.getString("elementColumn", "ELEMENT_FLAG");
    
    if (antSet.empty())                   // if LOFAR_ANTENNA_SET was not provided in parset
    {
      antSet=getLofarAntennaSet(msName);  // get it from the MS
    }

    cout << "antSet = " << antSet << endl;

    // Read observation times from MS
    Vector<MEpoch> obsTimes;
    readObservationTimes(msName, obsTimes);

    // get antenna fields
    vector<string> antennaFields;
    readAntennaFields(msName, antennaFields);

    //string mode=getLofarAntennaSet(msName); // don't need the mode anymore

    //vector<string> rcus;
    //map<string, string> rcus;
    map<string, vector<string> > rcus;
    getRCUs(msName, rcus, antennaFields);

    showMap(rcus);          // DEBUG
    
    /*
    // Connect to SAS
    LOG_INFO_STR("Getting SAS antenna health information");
    OTDBconnection conn(user, password, db, host); 
    LOG_INFO("Trying to connect to the database");
    ASSERTSTR(conn.connect(), "Connnection failed");
    LOG_INFO_STR("Connection succesful: " << conn);

    vector<string> brokenDipoles;
    getBrokenHardware(conn, brokenDipoles);
    */
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
  \brief Write initial LOFAR_FAILED_ELEMENT table
*/
void writeFailedElementTable()
{
  // get OBSTIMES from MS
  
  // getBrokenTiles for timestamp
}


/*!
  \brief Update LOFAR_FAILED_ELEMENT table with tiles that broke during observation
*/
void updateFailedElementTable()
{
  // get OBSTIMES from MS

  // getBrokenTiles for timestamp
}

/*!
  \brief Get the LOFAR_ANTENNA_SET, i.e. the observing mode
  \param msName     name of MeasurementSet
  \return mode      LOFAR_ANTENNA_SET used in observation
*/

string getLofarAntennaSet(const string &msName)
{
  string antSet;

  LOG_INFO_STR("Updating MeasurementSet: " << msName);
  MeasurementSet ms(msName); //, Table::Update);
  // If needed, try to get the AntennaSet name from the Observation table.
  if (ms.observation().tableDesc().isColumn ("LOFAR_ANTENNA_SET"))
  {
      ROScalarColumn<String> antSetCol(ms.observation(), "LOFAR_ANTENNA_SET");
      antSet = antSetCol(0);
  }
  else
  {
    LOG_DEBUG_STR(msName << " is missing column LOFAR_ANTENNA_SET");
  }
  ASSERTSTR (!antSet.empty(), "No ANTENNASET found in Observation table of "
             << msName << " or in keyword 'antennaset' in ParSet file");

  return antSet;
}

string getLofarAntennaSet(MeasurementSet &ms)
{
  string antSet;

  // If needed, try to get the AntennaSet name from the Observation table.
  if (ms.observation().tableDesc().isColumn ("LOFAR_ANTENNA_SET"))
  {
      ROScalarColumn<String> antSetCol(ms.observation(), "LOFAR_ANTENNA_SET");
      antSet = antSetCol(0);
  }
  else
  {
    LOG_DEBUG_STR("Missing column LOFAR_ANTENNA_SET");
  }
  ASSERTSTR (!antSet.empty(), "No ANTENNASET found in Observation table in MS or in keyword 'antennaset' in ParSet file");

  return antSet;
}

/*!
  \brief Read observation times from MS
  \param msName     name of MeasurementSet
  \param obsTimes   observation times
*/
void readObservationTimes(const string &msName, Vector<MEpoch> &obsTimes)
{
  LOG_INFO_STR("Reading observation times from MS: " << msName);
  MeasurementSet ms(msName, Table::Update);
  MSObservationColumns obsColumns(ms.observation());
  obsTimes=obsColumns.timeRangeMeas()(0);

  ASSERTSTR(obsTimes.size() > 0, "No observation times found in MS " << msName);
}

void readObservationTimes(MeasurementSet &ms, Vector<MEpoch> &obsTimes)
{
  LOG_INFO_STR("Reading observation times from MS");
  MSObservationColumns obsColumns(ms.observation());
  obsTimes=obsColumns.timeRangeMeas()(0);

  ASSERTSTR(obsTimes.size() > 0, "No observation times found in MS");
}


/*!
  \brief Read antennas from MS ANTENNA table
  \param MSname     name of MeasurementSet
  \param stations   LOFAR stations in ANTENNA table
*/
void readAntennaFields(const string &msName, vector<string> &antennas)
{
  Vector<String> antennaVec;

  LOG_INFO_STR("Reading antenna fields from MS: " << msName);
  // Open MS ANTENNA table
  MeasurementSet ms(msName, Table::Update);
  MSAntennaColumns antColumns(ms.antenna());

  ScalarColumn<String> nameCol(antColumns.name());    // pick the name column from antenna columns
  antennaVec=nameCol.getColumn();                     // convert to a casa vector
  
  // convert to std::vector
  antennas.clear();
  unsigned int n=antennaVec.size();
  for(unsigned int i=0; i<n; i++)
  {
    antennas.push_back(antennaVec(i));     
  }
}

void readAntennaFields(MeasurementSet &ms, vector<string> &antennas)
{
  Vector<String> antennaVec;

  LOG_INFO_STR("Reading antenna fields from MS");
  MSAntennaColumns antColumns(ms.antenna());

  ScalarColumn<String> nameCol(antColumns.name());    // pick the name column from antenna columns
  antennaVec=nameCol.getColumn();                     // convert to a casa vector
  
  // convert to std::vector
  antennas.clear();
  unsigned int n=antennaVec.size();
  for(unsigned int i=0; i<n; i++)
  {
    antennas.push_back(antennaVec(i));     
  }
}

/*!
  \brief Read LOFAR Antenna field configuration file into buffer
  \param antFile    LOFAR antenna configuration file location
  \param buffer     map to read antenna configurations into
*/
/*
void readAntennaFieldConf(const string &antFile, map<string, string> &antennaConf)
{
  vector<string> configurations;

  LOG_INFO_STR("antennaFieldConf = " << antennaFieldConf);

  // Open AntennaFieldConf
  fstream antennaFieldConfFile (antennaFieldConf.c_str(), ios::in);
  if(antennaFieldConfFile.bad())
  {
    LOG_DEBUG_STR("readAntennaFieldConf() error opening " << antennaFieldConf);
  }

  antennaConf.clear();
  map<string, string>::iterator it=antennaConf.begin();
  
  while(antennaFieldConfFile.good())
  {
    unsigned long pos=0;
    string line;
    getline(antennaFieldConfFile, line);
    
    // look for station and mode in antennaConfiguration file
    if(line.find("#")==string::npos)
    {
      cout << "Found mode in at " << pos << " " << line << endl;    // DEBUG


      configurations.appemd();
    }
  }
  antennaFieldConfFile.close();
}
*/

/*!
  \brief Get a list of all RCUs corresponding to this station
  \param msName   name of Measurementset to look for LOFAR_ANTENNA_FIELD 
  \param rcus     Antennas and RCU id vector pairs read from table array indices
  \param
  \param
*/
void getRCUs( const string &msName,
              map<string, vector<string> > &rcus,
              const string &tableName,
              const string &elementColumnName) 
{
  LOG_INFO_STR("msName = " << msName);

  vector<string> antennaFields;

  // Open MS/LOFAR_ANTENNA_FIELD table
  MeasurementSet ms(msName, Table::Update);           // don't use: msName + "/" + tableName
  Table antennaFieldTable(ms.keywordSet().asTable(tableName));

  readAntennaFields(ms, antennaFields);         // get antennaFields from MS first
  getRCUs(msName, rcus, antennaFields, tableName, elementColumnName); // now call full function getRCUs
}

/*!
  \brief Get a list of all RCUs corresponding to this station
  \param msName     name of Measurementset to look for LOFAR_ANTENNA_FIELD
  \param rcus       Antennas and RCU id vector pairs read from table array indices
  \param fields     Antenna fields (if not given, it will be read from the MS)
  \param tableName  table name to look for (default=LOFAR_ANTENNA_FIELD)
  \param elementColumnName  name of column containing element flag array (default=ELEMENT_FLAG)
*/
void getRCUs( const string &msName,
              map<string, vector<string> > &rcus,
              const vector<string> &fields,
              const string &tableName,
              const string &elementColumnName)
{
  LOG_INFO_STR("msName = " << msName);

  // Open MS/LOFAR_ANTENNA_FIELD table
  Table table(msName, Table::Update);      // don't use: msName + "/" + tableName
  Table antennaFieldTable(table.keywordSet().asTable(tableName));
  
  ScalarColumn<Int> antennaIDCol(antennaFieldTable, "ANTENNA_ID");  
  ArrayColumn<Bool> elementFlagCol(antennaFieldTable, elementColumnName);

  // read each row and determine rcus through boolean element array
  // Loop through all rows in the table:
  // read ANTENNA_ID and ELEMENT_FLAG
  rcus.clear();                                       // preemptively clear the map
  uInt nrow = antennaFieldTable.nrow();
  for (uInt i=0; i<nrow; i++) 
  {
      string rcu;                                     // RCU basename for this field
    //  Int antennaID = antennaIDCol(i);              // Read ANTENNA_ID  from LOFAR_ANTENNA_FIELD
      string antennaName=fields[i];                   // get corresponding Field name
      std::stringstream convert;                      // use stringstream to convert number to string

      // Handle ELEMENT_FLAG array
      Matrix<Bool> elementFlags = elementFlagCol(i);   // Read ELEMENT_FLAG column.
      IPosition shape=elementFlags.shape();            // get shape of elements array
      uInt ncolumns=elementFlags.ncolumn();            // number of columns = number of RCUs

      // Loop over RCU indices and pick those which are 0/false (i.e. NOT FLAGGED)
      unsigned int j=0;
      while(j<ncolumns)
      {
        rcu="RCU";      // reset rcu string basename
        
        if(elementFlags(j, 0)==0 || elementFlags(j, 1)==0)  // if either of the dipoles failed
        {
          string rcuNumber=boost::lexical_cast<std::string>(j);
          padTo(rcuNumber, 2, '0');     // pad rcu number to 3 with zeros
          rcu.append(rcuNumber);        // create complete RCU string including number

          map<string, vector<string> >::iterator rcusIt;     // iterator to find existing antenna entries
          if((rcusIt=rcus.find(antennaName)) != rcus.end())  // if antennaName key exists, append rcu to vector
          {
            rcusIt->second.push_back(rcu);
          }
          else                              // if that antennaName is not present yet, create vector
          {
            vector<string> rcuv;            // since map->second is a vector, we need a dummy vector
            rcuv.push_back(rcu);
            rcus.insert(pair<string, vector<string> >(antennaName, rcuv));
          }
        }        
        j++;
      }  
      showMap(rcus);                                // DEBUG
  }
}


/*!
  \brief Join the two set of field RCUs into one station RCU set
  \param &ms          MeasurementSet
  \param fieldRCUs    vector containing RCUs of two fields
*/
/*
void joinFields(MS &ms, map<string, vector<string> > &fieldRCUs)
{
  // Check if map actually contains separate fields, i.e. HBA0 and HBA1 antenna fields
  if()
  {
    // Look for separate antenna fields in field names
    if((pos=it->first.find("HBA0") || it->first.find("HBA1")) != string::npos)
    {
    
    }
    else
    {
      LOG_DEBUG_STR();
    }
  }
  
  // get stations from MS
  
}
*/


/*!
  \brief Join the two set of field RCUs into one station RCU set
  \param fieldRCUs    vector containing RCUs of two fields
  \param stations     LOFAR stations to join fields into
*/
/*
void joinFields(map<string, vector<string> > &fieldRCUs, vector<string> &stations)
{
  // add +48 to all RCUs of HBA1 fields to get higher RCU numbers
}
*/


// This function might become obsolete, and is only commented for code reference
/*!
  \brief Get a complete list of all RCUs taking part in the observation using stations
  \param stations       list of LOFAR stations
  \param stationrcus    list of 
*/

/*
void getObservationDipoles( const OTDBconnection &conn,
                            vector<string> &obsDipoles)
{
  vector<string> stationDipoles;  // all the RCUs for all stations
  vector<string> brokenDipoles;   // broken RCUs from SAS
  vector<string> obsDipoles;      // resulting RCUs taking part in observation


  // Loop over stations
  // TODO do this through getRCUs...
  for(vector<string>::const_iterator stationsIt = stations.begin(); stationsIt != stations.end(); 
      ++stationsIt)
  {
  }

    //getRCUs(antennaFieldConf, *stationsIt, mode, rcus);       // getRCUs depending on mode

    // append these RCUs to others of observation
    //vector<string>::iterator obsRCUsIt=obsRCUs.end();
    obsRCUs.insert(obsRCUsIt, rcus.begin(), rcus.end());


  // Get list of all brokenHardware from SAS
  LOG_INFO_STR("Getting SAS antenna health information");
  OTDBconnection conn(user, password, db, host); 
  LOG_INFO("Trying to connect to the database");
  ASSERTSTR(conn.connect(), "Connnection failed");
  LOG_INFO_STR("Connection succesful: " << conn);
  getBrokenHardware(conn, brokenDipoles);

  // Loop over stationdipole vector
  for (vector<string>::iterator it = stationdipoles.begin(); it!=stationdipoles.end(); ++it) 
  {
    // check in SAS if they were broken
  }
}
*/

/*!
  \brief Determine LOFAR stationType from its name
  \param  station         name of LOFAR station
  \return stationType     type of LOFAR station: Core, Remote or European
*/
string determineStationType(const string &station)
{
  string stationType;
  
  if(station.find("CS")!=string::npos)        // core station
  {
    stationType="Core";
  }
  else if(station.find("RS")!=string::npos)   // NL remote station
  {
    stationType="Remote";
  }
  else                                        // all European stations
  {
    stationType="European";
  }
  
  return stationType;
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

  if(timestamp==0)
  {
    LOG_INFO_STR("Getting broken hardware (now)");
    valueList = tv.getBrokenHardware();
  }
  else
  {
    LOG_INFO_STR("Getting broken hardware at " << timestamp);
    valueList = tv.getBrokenHardware(time_from_string("2010-05-26 07:30:00"));  // DEBUG
  }
  //showValueList(valueList);     // DEBUG output
}


/*!
  \brief Create antenna table with failed antenna tiles and their times of failure
*/
void createFailedAntennaTilesTable(const string &msName)
{
  MeasurementSet ms(msName, Table::Update);
  
  //TODO
  // Create a new table according to TableDesc matching MS2.0.7 ICD
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
		string row(formatString("%-55.55s|%-7.7s|%s",
			items[i].name.c_str(),
			items[i].value.c_str(),
			to_simple_string(items[i].time).c_str()));
		cout << row << endl;
	}

	cout << items.size() << " records" << endl << endl;
}

//
// Show the content of a STL vector
//
void showVector(const vector<string> &v)
{
  for(vector<string>::const_iterator it=v.begin(); it!=v.end(); ++it)
  {
    cout << *it << endl;
  }
}


void showMap(const map<string, string> &m)
{
  for(map<string, string>::const_iterator it=m.begin(); it!=m.end(); ++it)
  {
    cout << (*it).first << "\t" << (*it).second << endl;
  }
}

void showMap(const map<string, vector<string> > &m)
{
  //map<string, vector<string> >::const_iterator it;
  for(map<string, vector<string> >::const_iterator it=m.begin(); it!=m.end(); ++it)
  {
    vector<string> v=it->second;
    
    cout << it->first << endl;
    for(vector<string>::const_iterator vit=v.begin(); vit!=v.end(); ++vit)
    {
      cout << (*vit) << "\t";
    }
    cout << endl;
  }
}


/*!
  \brief Left pad a string with a padding character
  \param str          string to pad
  \param num          number of characters to pad to
  \param paddingChar  character to pad with
*/
void padTo(std::string &str, const size_t num, const char paddingChar = ' ')
{
    if(num > str.size())
        str.insert(0, num - str.size(), paddingChar);
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