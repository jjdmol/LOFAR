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

// LOFAR
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>   // for ASSERT and ASSERTSTR?
#include <Common/SystemUtil.h>    // needed for basename

// SAS
#include <OTDB/OTDBconstants.h>
#include <OTDB/OTDBconnection.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/TreeValue.h>
#include <OTDB/ClassifConv.h>
//#include <OTDB/OTDBtypes.h>
#include <OTDB/Converter.h>
#include <OTDB/TreeTypeConv.h>

// Boost
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>   // convert string to number
#include <boost/tokenizer.hpp>

// STL
#include <iostream>
#include <fstream>
#include <map>

// Casacore
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/TableLocker.h>
#include <casa/Quanta/MVTime.h>
#include <casa/OS/Time.h>
#include <casa/Arrays/VectorIter.h>
#include <casa/Arrays/ArrayIter.h>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::OTDB;
using namespace casa;

// MS reading functions
//boost::posix_time::ptime fromCasaTime (const MEpoch& epoch, double addDays);
string fromCasaTime (const MEpoch& epoch, double addDays);
string getLofarAntennaSet(const string &msName);
string getLofarAntennaSet(MeasurementSet &ms);

vector<string> readLofarStations(const MeasurementSet &ms);
void getLofarStationMap(const MeasurementSet &ms, map<string, int> &StationMap);
string determineStationType(const string &station);

Vector<MVEpoch> readObservationTimes(MeasurementSet &ms);
vector<string> readAntennas(const MeasurementSet &ms);
vector<string> readLofarStations(const MeasurementSet &ms);
//void readAntennas(const MeasurementSet &ms, vector<string> &antennas);
vector<int> getAntennaIds(const MeasurementSet &ms, const string &stationName);


string getAntennaConfiguration( MeasurementSet &ms, const string &station, 
                                const string &mode,
                                const string &AntennaConfFile);
vector<int> getAntennaConfigurationRCUs(const string &antennaConf, const string &station);


// RCUmap type definition to make life easier
typedef map<string, vector<int> > RCUmap;

// MS Antenna field functions
RCUmap getRCUs( const MeasurementSet &ms,
                const string &tableName="LOFAR_ANTENNA_FIELD",
                const string &elementColumnName="ELEMENT_FLAG");

void addFailedAntennaTiles( MeasurementSet &ms, 
                            RCUmap &rcusMS);

// SAS functions
vector<string> getBrokenHardware( OTDBconnection &conn, 
                                  const MVEpoch &timestamp=0);
RCUmap getBrokenRCUs( const vector<string> &brokenHardware, 
                      const RCUmap &rcusMS);
RCUmap getFailedAntennaTiles( MeasurementSet &ms, 
                              OTDBconnection &conn);

// DEBUG SAS output functions
void showTreeList(const vector<OTDBtree>&	trees);
void showNodeList(const vector<OTDBnode>&	nodes);
void showValueList(const vector<OTDBvalue>&	items);

void showVector(const vector<string> &v, const string &key="");
void showMap(const map<string, string> &m, const string &key="");
void showMap(const map<string, vector<string> > &m, const string &key="");

void showVector(const vector<int> &v);
void showMap(const map<string, int> &m, const string &key="");
void showMap(const map<string, vector<int> > &m, const string &key="");

void padTo(std::string &str, const size_t num, const char paddingChar);


// MS Table writing functions TODO
void updateAntennaFieldTable( const MeasurementSet &ms, 
                              const map<string, vector<double> > &brokenRCUs);

// File I/O
void writeBrokenHardwareFile(const string &filename, const vector<string> &brokenHardware, bool strip=true);
vector<string> readBrokenHardwareFile(const string &filename);
string stripRCUString(const string &brokenHardware);
void getSASFailureTimes();
void writeFailedElementsFile( const string &filename,
                              const vector<string> &brokenHardware,
                              const vector<MVEpoch> &timestamps,
                              bool strip=true);                              
//void readFailedElementsFile(const string &filename,
//                            vector<string> &brokenHardware,
//                            vector<MVEpoch> &timestamps);
map<string, MVEpoch> readFailedElementsFile(const string &filename);


int main (int argc, char* argv[])
{
  vector<string> antennaTiles;
  vector<MEpoch> failingTimes;

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
    string brokenfilename = parset.getString("brokenrcusfile", "/opt/lofar/share/brokenrcus.txt");
    // Optional parameters that give outside access of internal format handling
    // this should not be necessary to be changed
    string elementTable = parset.getString("elementTable", "LOFAR_ANTENNA_FIELD");                                          
    string elementColumn = parset.getString("elementColumn", "ELEMENT_FLAG");
   
    if (antSet.empty())                   // if LOFAR_ANTENNA_SET was not provided in parset
    {
      antSet=getLofarAntennaSet(msName);  // get it from the MS
    }

    MeasurementSet ms(msName, Table::Update);     // open Measurementset

    // Read observation times from MS
    Vector<MVEpoch> obsTimes=readObservationTimes(ms);

//    cout << "obsTimes(0): " << obsTimes(0) << endl;   // DEBUG

    RCUmap rcus=getRCUs(ms);  //, rcus, antennas);   

    showMap(rcus);
//    showMap(rcus, "CS501HBA0");          // DEBUG
//    showMap(rcus, "CS501HBA1");          // DEBUG   

  
    // Connect to SAS
    LOG_INFO_STR("Getting SAS antenna health information");
    OTDBconnection conn(user, password, db, host); 
    LOG_INFO("Trying to connect to the database");
    ASSERTSTR(conn.connect(), "Connnection failed");
    LOG_INFO_STR("Connection succesful: " << conn);

    // Get broken hardware strings from SAS
    vector<string> brokenHardware;
    brokenHardware=getBrokenHardware(conn, obsTimes(0));

  // TEST: write broken hardware (raw vector) to file
    writeBrokenHardwareFile(brokenfilename, brokenHardware);  // DEBUG
//    showVector(brokenHardware);   // DEBUG


  // TEST: reading broken hardware from a file
//    cout << "reading brokenHardware from file:" << endl;      // DEBUG
//    readBrokenHardwareFile(brokenfilename, brokenHardware);
//    showVector(brokenHardware);

    // TEST: get broken RCUs for this MS
    RCUmap brokenRCUs;
    brokenRCUs=getBrokenRCUs(brokenHardware, rcus);
  
    showMap(brokenRCUs);
  
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
  \return dateTime  string with date and time in the format ("YYYY-MM-DD HH:MM:SS")
*/
//boost::posix_time::ptime fromCasaTime (const MVEpoch& epoch, double addDays=0)
string fromCasaTime (const MVEpoch& epoch, double addDays=0)
{
  MVTime t (epoch.get() + addDays);

//  return boost::posix_time::from_iso_string (t.getTime().ISODate());  // used to return ptime
  return t.getTime().ISODate();
}


/*!
  \brief  Update the element flags in the MeasurementSet with failed tiles info
  \param  ms            MeasurementSet to update ELEMENT_FLAGs
  \param  rcus          map containing failed RCUs for this MS
  \param  overwrite     overwrite existing entries (default=true)
*/
void updateAntennaFieldTable( const MeasurementSet &ms, 
                              const map<string, vector<double> > &brokenRCUs)
{
  LOG_INFO_STR("Updating failed RCUs in MS");

  // Open MS/LOFAR_ANTENNA_FIELD table
  Table antennaFieldTable(ms.keywordSet().asTable("LOFAR_ANTENNA_FIELD"));
  ArrayColumn<Bool> elementFlagCol(antennaFieldTable, "ELEMENT_FLAG");

  vector<string> stations=readLofarStations(ms);           // get LOFAR_STATIONS

  /*
  // Looping over stations (appearing as HBA_DUAL or HBA_JOINED) in ANTENNA table
  for(vector<string>::iterator stationIt=stations.begin(); stationIt!=stations.end(); ++stationIt)
  */

/*
  // Loop over map of failed rcus
  for(RCUmap::iterator rcusIt=rcus.begin(); rcusIt!=rcus.end(); ++rcusIt)
  {  
    const string station=rcusIt->first;   // better handle of station name
    vector<int> rcuNums=rcusIt->second;   // better handle of failed RCU numbers
    
    // get corresponding ANTENNA_ID index into LOFAR_ANTENNA_FIELD
    antennaIds=getAntennaIds(ms, station);           // this can be 1 or 2 (or more in the future?)

    // Depending on the RCU number < 48 we are talking about HBA0 or number >= 48 HBA1
    for(vector<int>::iterator idIt=antennaIds.begin(); idIt!=antennaIds.end(); ++idIt)
    {
      unsigned int row=*idIt;              // idIt is the index into the row of LOFAR_ANTENNA_FIELD
    
      // Handle ELEMENT_FLAG array
      Matrix<Bool> elementFlags = elementFlagCol(row);  // Read ELEMENT_FLAG column from row
      IPosition shape=elementFlags.shape();             // get shape of elements array
      uInt nelements=elementFlags.ncolumn();            // number of elements = number of RCUs

      // Check for NAME column: HBA0 0..47, HBA1 48..95

      // Loop over RCU indices of array and update those present in failed RCUmap
      for(unsigned int rcuIndx=0; rcuIndx<2*nelements; rcuIndx++)
      {
        if()
        {
        
        }
        
      }  
    }
  }
*/

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
  \brief  Get the antenna configuration from station name and mode
          based on entries in the AntennaConf file
  \param station        name of station to deduce CS, RS or intenational
  \param mode           mode the observation was taken in, e.h. LBA_Inner etc.
  \return Returns the string of the antenna configuration in AntennaConf file
*/
string getAntennaConfiguration( MeasurementSet &ms, const string &station, 
                                const string &AntennaConfFile)
{
//  vector<bool> stationTypes(3);                         // CS?,RS?,Int station?
//  const vector<string> stations=getLofarStations(ms);   // station names
  const string mode=getLofarAntennaSet(ms);
  string type=determineStationType(station);
  string antennaConfiguration;                      // antenna configuration to be returned
  
  antennaConfiguration="";
  
  
  return antennaConfiguration;
}


/*!
  \brief Get the corresponding RCU numbers for an Antenna Configuration and station (CS,RS, Int)
  \param  antennaConf   string denoting atenna configuration
  \param  station       name of station 
  \return RCUs          list of RCU numbers of this antenna configuration station combination
*/
vector<int> getAntennaConfigurationRCUs(const string &antennaConf, const string &configuration)
{
  vector<int> RCUs;

  // look for compound line in config file
  
  // interprete pattern

  return RCUs;
}


/*!
  \brief Read observation times from MS
  \param  ms        MeasurementSet to read observation times from
  \param  obsTimes  vector to hold observation times
*/
Vector<MVEpoch> readObservationTimes(MeasurementSet &ms)
{
  Vector<MVEpoch> obsTimes;
  Vector<MEpoch> obsTimesME;

  LOG_INFO_STR("Reading observation times from MS");
  MSObservationColumns obsColumns(ms.observation());
  obsTimesME=obsColumns.timeRangeMeas()(0);

  obsTimes.resize(obsTimesME.shape()(0));
  for(uInt i=0; i<obsTimesME.size(); i++)   // need loop for conversion since we have vectors
  {
    obsTimes[i]=obsTimesME[i].getValue();   // and getValue() works only on an individual MEpoch
  }

  ASSERTSTR(obsTimes.size() > 0, "No observation times found in MS");

  return obsTimes;
}


/*!
  \brief Get a map from LOFAR ANTENNA to LOFAR_ANTENNA_FIELD
  \param msName       name of MeasurementSet
  \param StationMap   map from string LOFAR STATION to rowIds LOFAR_ANTENNA_FIELD
*/
/*
void getLofarStationMap(const string &msName, map<string, int> &StationMap)
{
  MeasurementSet ms(msName, Table::Update);
  Table LofarStationTable(ms.keywordSet().asTable("LOFAR_STATION"));

  getLofarStationMap(ms, StationMap);   // call appropriate daughter function
}
*/

/*!
  \brief Get a map from LOFAR ANTENNA to LOFAR_ANTENNA_ID
  \param ms     MeasurementSet
  \param StationMap   map from string LOFAR STATION to rowIds LOFAR_ANTENNA_FIELD
*/
/*
void getLofarStationMap(MeasurementSet &ms, map<string, int> &StationMap)
{
  Vector<String> stationVec;        // vector containing station names
  Vector<Int> lofarStationIdVec;    // containing LOFAR_STATION ids = row number

  // open LOFAR_STATION subtable
  Table LofarStationTable(ms.keywordSet().asTable("LOFAR_STATION"));
  ROScalarColumn<String> nameCol(LofarStationTable, "NAME");      // pick the name column from antenna columns
  stationVec=nameCol.getColumn();                                 // convert to a casa vector
  
  uInt nRows=LofarStationTable.nrow();
  IPosition length=stationVec.shape();
  ASSERT(nRows == length(0));         // sanity check for length of names vector/nrows
  
  for(uInt i=0; i<nRows; i++)         // Loop over table rows
  {
    StationMap[stationVec[i]]=i;      // i=row=ID
  }
//  showMap(StationMap);    // DEBUG
}
*/


/*!
  \brief Read antennas from MS ANTENNA table
  \param ms         MeasurementSet
  \param antennas   LOFAR antenna fields in ANTENNA table
*/
//void readAntennas(const MeasurementSet &ms, vector<string> &antennas)
vector<string> readAntennas(const MeasurementSet &ms)
{
  vector<string> antennas;
  Vector<String> antennaVec;

  LOG_INFO_STR("Reading antenna fields from MS");
//  MSAntennaColumns antColumns(ms.antenna());

  Table AntennaTable(ms.keywordSet().asTable("ANTENNA"));

//  ScalarColumn<String> nameCol(antColumns.name());    // pick the name column from antenna columns
//antennaVec=nameCol.getColumn();                     // convert to a casa vector
  ScalarColumn<String> nameCol(AntennaTable,"NAME");
  antennaVec=nameCol.getColumn();
  
  // convert to std::vector
  //antennas.clear();
  unsigned int n=antennaVec.size();
  for(unsigned int i=0; i<n; i++)
  {
//    cout << "antennaVec(" << i << ") = " << antennaVec(i) << endl;
    antennas.push_back(antennaVec(i));     
  }
  
  return antennas;
}


/*!
  \brief Read vector of LOFAR STATIONS from MS
*/
vector<string> readLofarStations(const MeasurementSet &ms)
{
  vector<string> stations;
  Vector<String> stationVec;

  LOG_INFO_STR("Reading LOFAR stations from MS");

  Table AntennaTable(ms.keywordSet().asTable("LOFAR_STATION"));

  ScalarColumn<String> nameCol(AntennaTable,"NAME");
  stationVec=nameCol.getColumn();
  
  // convert to std::vector
  unsigned int n=stationVec.size();
  for(unsigned int i=0; i<n; i++)
  {
//    cout << "stationVec(" << i << ") = " << stationVec(i) << endl;    // DEBUG
    stations.push_back(stationVec(i));     
  }
  
  return stations;
}


/*!
  \brief Get a map of all RCUs corresponding for all stations
  \param ms                 MeasurementSet of observation
  \param tableName          name of subtable where to find ANTENNA_ELEMENTS 
                            (default=LOFAR_ANTENNA_FIELD)
  \param elementColumnName  column name of array of elements (default=ELEMENT_FLAG)
  \return RCUmap            map<string, vector<int> > of RCUs per station
*/
RCUmap getRCUs( const MeasurementSet &ms,
                const string &tableName,
                const string &elementColumnName)
{
  vector<string> stations;                  // LOFAR Stations
  vector<int> antennaIds;                   // antennaIds for a particular station
  RCUmap rcus;                              // RCUmap map<string, vector<int> > to return
  
  LOG_INFO_STR("Reading RCUs from MS");

  // Open MS/LOFAR_ANTENNA_FIELD table
  Table antennaFieldTable(ms.keywordSet().asTable(tableName));
  ScalarColumn<String> nameCol(antennaFieldTable, "NAME");
  ArrayColumn<Bool> elementFlagCol(antennaFieldTable, elementColumnName);

  stations=readLofarStations(ms);           // get LOFAR_STATIONS
  // Looping over stations (appearing as HBA_DUAL or HBA_JOINED) in ANTENNA table
  for(vector<string>::iterator stationIt=stations.begin(); stationIt!=stations.end(); ++stationIt)
  {  
    const string station=*stationIt;
    
    // get corresponding ANTENNA_ID index into LOFAR_ANTENNA_FIELD
    antennaIds=getAntennaIds(ms, station);           // this can be 1 or 2 (or more in the future?)
    for(vector<int>::iterator idIt=antennaIds.begin(); idIt!=antennaIds.end(); ++idIt)
    {
      unsigned int row=*idIt;                // idIt is the index into the row of LOFAR_ANTENNA_FIELD
      string name=nameCol(row);              // ANTENNA_FIELD name (e.g. LBA_INNER, HBA0, HBA1)
      
      cout << "getRCUs() name = " << name << endl;      // DEBUG
      
      // Handle ELEMENT_FLAG array
      Matrix<Bool> elementFlags = elementFlagCol(row);   // Read ELEMENT_FLAG column from row
      IPosition shape=elementFlags.shape();              // get shape of elements array
      uInt nelements=elementFlags.ncolumn();            // number of elements = number of RCUs

      cout << "nelements = " << nelements << endl;      // DEBUG

      // Loop over RCU indices and pick those which are 0/false (i.e. NOT FLAGGED)
      for(unsigned int i=0; i<nelements; i++)
      {
        unsigned int rcuNum=i;
        
//        rcuNum=i+nelements;
        
        // RCUs for HBA1 and LBA_OUTER have RCU numbers from 48 to 95
        if(name=="HBA1" || name=="LBA_OUTER")
        {
          rcuNum=i+nelements;
          cout << rcuNum << "\t";
        }

        if(elementFlags(i, 0)==0 && elementFlags(i, 1)==0)  // if neither of the dipoles failed
        {
          cout << rcuNum << "\t";
          rcus[station].push_back(rcuNum);
        }        
      }  
    }
  }
  
  return(rcus);
}


/*
getElementFlags(int)
{
    // Handle ELEMENT_FLAG array
    Matrix<Bool> elementFlags = elementFlagCol(i);   // Read ELEMENT_FLAG column.
    IPosition shape=elementFlags.shape();            // get shape of elements array
    uInt ncolumns=elementFlags.ncolumn();            // number of columns = number of RCUs

    // Loop over RCU indices and pick those which are 0/false (i.e. NOT FLAGGED)
    for(unsigned int rcu=0; rcu<2*ncolumns; rcu++)
    {
      if(elementFlags(rcu, 0)==0 && elementFlags(rcu, 1)==0)  // if neither of the dipoles failed
      {
        rcus[antennaName].push_back(rcu);
      }        
    }  
}
*/

/*
  \brief Get the antennaId (i.e. row number in the ANTENNA table) for a named antenna
  \param stationName    name of station antenna to look for
  \return               vector of antennaIds for this station
*/
vector<int> getAntennaIds(const MeasurementSet &ms, const string &stationName)
{
  vector<int> antennaIds;   // to hold 1 or 2 antenna ids indexing into LOFAR_ANTENNA_FIELD

  Table antennaTable(ms.keywordSet().asTable("ANTENNA"));
  ScalarColumn<String> nameCol(antennaTable, "NAME");

  uInt nrow = antennaTable.nrow();
  for(uInt i=0; i<nrow; i++)
  {
    if(nameCol(i).find(stationName)!=string::npos)
    {
      antennaIds.push_back(i);
    }
  } 
//  cout << "getAntennaIds() stationName = " << stationName << endl;    // DEBUG
//  showVector(antennaIds);  // DEBUG
  return antennaIds;
}


/*!
  \brief  From station name and RCU number give back LOFAR_ANTENNA_FIELD index 
          and ELEMENT_FLAG index map
  \param  station        LOFAR station name
  \param  RCU            (failed) RCU number
  \return RowElement     map with LOFAR_ANTENNA_FIELD row index and ELEMENT_FLAG array index
*/
/*
map<uInt, uInt> getAntennaIdFromMap(const string &station, int RCU)
{
  // TODO: Do we really need this? Sounds convenient, but we are looping sequentially over
  //       stations and antenna ids anyway...

}
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
  \brief  Write broken dipoles to temporary file (this is quicker to work on than
          querying the database)
  \param filename         name of temporary file
  \param brokenHardware   vector containing SAS output of broken hardware
  \param strip            strip SAS broken hardware string down to station.RCU. (default=true)
*/
void writeBrokenHardwareFile(const string &filename, const vector<string> &brokenHardware, bool strip)
{
  fstream outfile;
//  outfile.open(filename.c_str(), ios::trunc);
  outfile.open(filename.c_str(), ios::out);   // this shows the correct behaviour of overwriting the file

  if (outfile.is_open())
  {
    LOG_INFO_STR("Writing SAS broken RCUs to file: " << filename);
  
    for(vector<string>::const_iterator it=brokenHardware.begin(); it!=brokenHardware.end() ; ++it)
    {
      if(it->find("RCU")!=string::npos)   // Only write lines that contain RCU
      {
        // optionally strip off unnecessary information from string
        if(strip)
          outfile << stripRCUString(*it) << endl;
        else
          outfile << *it << endl;
      }
    }
    outfile.close();
  }
  else
    cout << "writeFile(): Unable to open file " << filename << " for reading." << endl;
}


/*!
  \brief Strip the RCU string in broken hardware of unnecessary information
  \param brokenHardware     broken hardware string to strip off
  \return stripped string
*/
string stripRCUString(const string &brokenHardware)
{
  string stripped;          // stripped broken hardware line
  vector<string> tokens;

  // TODO: will this really increase parsing speed of the broken hardware strings?
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(".");
  tokenizer tok(brokenHardware, sep);

  for(tokenizer::iterator beg=tok.begin(); beg!=tok.end();++beg)
  {
    tokens.push_back(*beg);
  }
  
    cout << tokens[3] << "\t" << tokens[7] << endl;
   stripped=tokens[3].append(".").append(tokens[7]).append(".");
   
   return stripped;
}


/*!
  \brief  Read broken dipoles from temporary file (this is quicker to work on than
          querying the database), lines starting with # are ignored
  \param filename         name of temporary file
  \param brokenHardware   vector containing SAS output of broken hardware read from file
*/
vector<string> readBrokenHardwareFile(const string &filename)//, vector<string> &brokenHardware)
{
  vector<string> brokenHardware;
  string line;
  fstream infile;
  infile.open(filename.c_str(), ios::in);

  if(infile.is_open())
  {
    while(infile.good())
    {
      getline (infile,line);      
      if(line.find("#") != string::npos)          // Ignore comment lines "#"
      {
        // do nothing
      }
      else
      {
        brokenHardware.push_back(line);
      }
    }   
    infile.close();
  }
  else
  {
    cout << "readFile(): Unable to open file" << filename << " for reading." << endl;
  }
  
  return brokenHardware;
}


/*!
  \brief Write failed elements with timestamps to ASCII file
  \param filename       name of file to contain list of failed elements
  \param failedElements vector containing strings with failed elements between end - start of MS
  \param timestamps     vector containing the associated timestamps for the failed elements
*/
void writeFailedElementsFile( const string &filename,
                              const vector<string> &failedElements,
                              const vector<MVEpoch> &timestamps,
                              bool strip)
{
  fstream outfile;
//  outfile.open(filename.c_str(), ios::trunc);
  outfile.open(filename.c_str(), ios::out);   // this shows the correct behaviour of overwriting the file

  ASSERTSTR(failedElements.size() == timestamps.size(), 
            "writeFailedElementsFile() sizes of failedElements and timestamps differ");

  if (outfile.is_open())
  {
    LOG_INFO_STR("Writing SAS failed elements RCUs with timestamps to file: " << filename);
  
    vector<MVEpoch>::const_iterator timestampsIt=timestamps.begin();
    for(vector<string>::const_iterator it=failedElements.begin(); it!=failedElements.end() ; ++it)
    {
      if(it->find("RCU")!=string::npos)   // Only write lines that contain RCU
      {
        // optionally strip off unnecessary information from string
        if(strip)
          outfile << stripRCUString(*it) << endl;
        else
          outfile << *it << endl;
          
        outfile << "\t" << *timestampsIt << endl;          // Write timestamp
        ++timestampsIt;
      }
    }
    outfile.close();
  }
  else
    cout << "writeFailedElementsFile(): Unable to open file " << filename << " for reading." << endl;
}


/*!
  \brief Read failed elements with timestamps from ASCII file
  \param filename       name of file to contain list of failed elements
  \param failedElements vector of failed element names
  \param timestamps     vector containing the associated timestamps for the failed elements
*/
//void readFailedElementsFile(const string &filename,
//                            vector<string> &failedElements,
//                            vector<MVEpoch> &timestamps)
map<string, MVEpoch> readFailedElementsFile(const string &filename)
{
  string line;                                    // line read from file
  string name, timestamp;                         // name and timestamp to split into 
  size_t pos1=string::npos, pos2=string::npos;    // positions for line split
  map<string, MVEpoch> failedElements;
  
  fstream infile;
  infile.open(filename.c_str(), ios::in);

  if(infile.is_open())
  {
    while(infile.good())
    {
      getline (infile,line);      
      if(line.find("#") != string::npos)          // Ignore comment lines "#"
      {
        // do nothing
      }
      else
      {
        // read first and second column of line
        if((pos1=line.find(" "))!=string::npos)
        {
          name=line.substr(0, pos1);        // read up to first space
          while(pos2!=string::npos)         // read all successive spaces
          {
            pos2=line.find(" ");            // find next space
          }
          pos2++;                           // skip space
          if(pos2!=string::npos)
          {
            timestamp=line.substr(pos2, pos2-pos1);  // read timestamp
          }
          else
          {
            LOG_DEBUG_STR("readFailedElementsFile() " << line << " lacks timestamp entry.");            
          }
        }
        else
        {
          LOG_DEBUG_STR("readFailedElementsFile() " << line << " contains an error near pos = " 
          << pos1);
        }
        
        cout << "readFailedElementsFile() name = "  << name << endl;            // DEBUG
        cout << "readFailedElementsFile() timestamp = " << timestamp << endl;   // DEBUG
        
        // convert timestamp to a casa epoch
        MVEpoch timestamp(Quantity(50237.29, "d"));
        
        failedElements[name]=timestamp;   // add to map
      }
    }   
    infile.close();
  }
  else
    cout << "readFailedElementsFile(): Unable to open file" << filename << " for reading." << endl;

  return failedElements;
}


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
vector<string> getBrokenHardware( OTDBconnection &conn, 
                                  const MVEpoch &timestamp)
{
  TreeTypeConv TTconv(&conn);     // TreeType converter object
  ClassifConv CTconv(&conn);      // converter I don't know
  vector<OTDBvalue> valueList;    // OTDB value list for the previous month
  
  vector<string> brokenHardware;  // vector of just the name of the broken hardware (all)
  
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

  LOG_INFO_STR("Getting broken hardware at " << timestamp);
  valueList = tv.getBrokenHardware((time_from_string(fromCasaTime(timestamp))));

  for(unsigned int i=0; i < valueList.size(); i++)
  {
    brokenHardware.push_back(valueList[i].name);
  }
  
  return brokenHardware;
}


vector<string> getBrokenHardware( OTDBconnection &conn)
{
  TreeTypeConv TTconv(&conn);     // TreeType converter object
  ClassifConv CTconv(&conn);      // converter I don't know
  vector<OTDBvalue> valueList;    // OTDB value list
  
  vector<string> brokenHardware;  // vector of just the name of the broken hardware (all)
  
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

  for(unsigned int i=0; i < valueList.size(); i++)
  {
    brokenHardware.push_back(valueList[i].name);
  }
  
  return brokenHardware;
}


/*
  \brief Get a list of broken RCUs for this observation
  \param brokenHardware   string of broken hardware names from SAS
  \param rcusMS           RCUmap of RCUs present in the MS
*/
RCUmap getBrokenRCUs( const vector<string> &brokenHardware,
                      const RCUmap &rcusMS)
{
  RCUmap brokenRCUs;
  string rcuSAS;            // substring from SAS of the form "RCU<num>"
  vector<int> rcuNumbers;   // vector containing rcu numbers of brokenHardware, will be put into map
  vector<string>::const_iterator brokenIt;
  RCUmap::const_iterator rcusMSIt;
  string::iterator foundIt;

  LOG_INFO("getting brokenRCUs for this MS");

  // Loop over vector with all broken hardware
  for(rcusMSIt = rcusMS.begin(); rcusMSIt != rcusMS.end(); ++rcusMSIt)
  {
    string stationMS = rcusMSIt->first;         // name of station in MS
    vector<string>::const_iterator brokenIt;    // iterator over broken Hardware string vector
 
    rcuNumbers.clear();                         // for each station clear the RCU numbers vector
    // Find all occurences of this station in the list of broken hardware
    for(brokenIt=brokenHardware.begin(); brokenIt != brokenHardware.end(); ++brokenIt)
    {
      if(brokenIt->find(stationMS) != string::npos)
      {
        string::size_type pos1=brokenIt->find("RCU");
        pos1+=3;                          // skip "RCU"
        string::size_type pos2=brokenIt->find(".", pos1);

        if(pos2!=string::npos)            // if we find a "."
        {
          rcuSAS=brokenIt->substr(pos1, pos2-pos1);
        }
        else
        {
          LOG_DEBUG_STR("SAS broken hardware string corrupt: " << *brokenIt);
        }
        
//        string rcuSAS=brokenIt->substr(pos+3, brokenIt->find(".", pos)-(pos+3));
        unsigned int rcuSASNum = boost::lexical_cast<unsigned int>(rcuSAS);
        RCUmap::iterator brokenRCUsIt;
        // If stationMS already exists in rcuMAP
        if( (brokenRCUsIt = brokenRCUs.find(stationMS)) != brokenRCUs.end() ) 
        {
          brokenRCUsIt->second.push_back(rcuSASNum);
        }
        else    // else, it doesn't exist, yet, make a new pair
        {
          rcuNumbers.push_back(rcuSASNum);
          brokenRCUs.insert(std::make_pair(stationMS, rcuNumbers));
        }
      }
    }
  }
  return brokenRCUs;
}


/*!
  \brief Get from SAS the antenna tiles that failed during observation
  \param ms         MeasurementSet
  \param conn       OTDBconnection to SAS
  \param timestamp  timestamp to get failed tines for (default last time in MS)
*/
RCUmap getFailedAntennaTiles( MeasurementSet &ms, 
                              OTDBconnection &conn)
{
  vector<string> failedHardware;
  RCUmap failedTiles;                   // map of station and RCUs that failed after timestamp
  vector<string> brokenHardwareStart;   // list of broken hardware at the start of observation 
  vector<string> brokenHardwareEnd;     // list of broken hardware at the end of observation
  Vector<MVEpoch> obsTimes;             // vector with observation times from the MS
  MVEpoch timeStart, timeEnd;           // single timestamp


  obsTimes=readObservationTimes(ms);        // get observation times from ms
  timeStart=obsTimes[0];                    // broken hardware at the beginning of observation
  timeEnd=obsTimes[obsTimes.size()-1];      // broken hardware at the last timestamp
  
  brokenHardwareStart=getBrokenHardware(conn, timeStart);
  brokenHardwareEnd=getBrokenHardware(conn, timeEnd);  // get broken hardware for last timestamp

  // throw out all broken hardware that was already broken at the beginning
  //vector<string>::iterator hwStartIt=brokenHardwareStart.begin();
  vector<string>::iterator hwEndIt=brokenHardwareEnd.begin();
  for(; hwEndIt != brokenHardwareEnd.end(); ++hwEndIt)
  {
    // find
    vector<string>::const_iterator found=find(brokenHardwareStart.begin(),
                                              brokenHardwareStart.end(), *hwEndIt);
    // If we DID NOT find the particular RCU in the broken hardware at the start...
    if(found == brokenHardwareStart.end())
    {
      failedHardware.push_back(*hwEndIt);      // ...copy it over to the failed hardware vector
    }  
  }

//getBrokenRCUs( const vector<string> &brokenHardware,
//                      const RCUmap &rcusMS)

  failedTiles=getBrokenRCUs();    // use this function to sort the reduced broken hardware vector into a map

  return failedTiles;
}


/*!
  \brief Create antenna table with failed antenna tiles and their times of failure
         after the observation
  \param
  \param
*/
void addFailedAntennaTiles( MeasurementSet &ms, 
                            RCUmap &rcusMS)
{

  RCUmap failedRCUs;                  // RCUs that failed during observation
  MVEpoch timestamp;                  // timestamp variable to write to table row

  Table FailedElementsTable(ms.rwKeywordSet().asTable("LOFAR_ELEMENT_FAILURE"));
  rcusMS=getRCUs(ms);

  // get timestamp for end of MS
  
  //TODO
  // get all hardware that failed within time end - start
//  brokenHardware=getBrokenHardware(conn, timestamp);
//  failedRCUs=getBrokenRCUs(brokenHardware, rcusMS);
  
  // Loop over map of failed RCUs for this MS
  
  // Create a new table according to TableDesc matching MS2.0.7 ICD
  // ANTENNA_FIELD_ID, ELEMENT_INDEX, TIME, FLAG_ROW
  TableLocker locker(FailedElementsTable, FileLocker::Write);
//  doAddFailedAntennaTile();
}


/*!
  \brief Do add a row to the Failed Antenna Tiles table
*/
void doAddFailedAntennaTile(Table &failedElementsTable, 
                          int fieldId, 
                          int element_index, 
                          MVEpoch timeStamp, 
                          bool flag)
{
  uint rownr = failedElementsTable.nrow();
  failedElementsTable.addRow();

  ScalarColumn<Int> antennaFieldIdCol;
  antennaFieldIdCol.put(rownr, fieldId);
  ScalarColumn<Int> elementIndexCol;
  elementIndexCol.put(rownr, element_index);
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
void showVector(const vector<string> &v, const string &key)
{
  for(vector<string>::const_iterator it=v.begin(); it!=v.end(); ++it)
  {
    if(key!="")
    {
      if(*it==key)
        cout << *it << endl;
    }
    else
    {
      cout << *it << endl;
    }
  }
}


void showMap(const map<string, string> &m, const string &key)
{
  for(map<string, string>::const_iterator it=m.begin(); it!=m.end(); ++it)
  {
    if(key!="")
    {
      if(it->first==key)
        cout << (*it).first << "\t" << (*it).second << endl;
    }
    else
    {
      cout << (*it).first << "\t" << (*it).second << endl;    
    }
  }
}

void showMap(const map<string, vector<string> > &m, const string &key)
{
  //map<string, vector<string> >::const_iterator it;
  for(map<string, vector<string> >::const_iterator it=m.begin(); it!=m.end(); ++it)
  {
    vector<string> v=it->second;
    
    if(key!="")
    {
      if(it->first==key)
      {
        cout << it->first << endl;
        for(vector<string>::const_iterator vit=v.begin(); vit!=v.end(); ++vit)
        {
          cout << (*vit) << "\t";
        }
        cout << endl;
      }
    }
    else
    {
      cout << it->first << endl;
      for(vector<string>::const_iterator vit=v.begin(); vit!=v.end(); ++vit)
      {
        cout << (*vit) << "\t";
      }
      cout << endl;
    }
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

void showVector(const vector<int> &v)
{
  for(vector<int>::const_iterator it=v.begin(); it!=v.end(); ++it)
  {
    cout << *it << endl;
  }
}

void showMap(const map<string, int> &m, const string &key)
{
  for(map<string, int>::const_iterator it=m.begin(); it!=m.end(); ++it)
  {
    if(key!="")
    {
      if(it->first==key)
        cout << (*it).first << "\t" << (*it).second << endl;
    }
    else
    {
      cout << (*it).first << "\t" << (*it).second << endl;    
    }
  }
}

void showMap(const map<string, vector<int> > &m, const string &key)
{
  for(map<string, vector<int> >::const_iterator it=m.begin(); it!=m.end(); ++it)
  {
    vector<int> v=it->second;
    
    if(key!="")
    {
      if(it->first==key)
      {
        cout << it->first << endl;
        for(vector<int>::const_iterator vit=v.begin(); vit!=v.end(); ++vit)
        {
          cout << (*vit) << "\t";
        }
        cout << endl;
      }
    }
    else
    {
      cout << it->first << endl;
      for(vector<int>::const_iterator vit=v.begin(); vit!=v.end(); ++vit)
      {
        cout << (*vit) << "\t";
      }
      cout << endl;
    }
  }
}


/*
StringSplit(string str, string delim, vector<string> results)
{
  int cutAt=0;
  while( (cutAt = str.find_first_of(delim)) != string::npos )
  {
    if(cutAt != string::npos)
    {
      results.push_back(str.substr(0,cutAt));
    }
    str = str.substr(cutAt+1);
  }
  if(str.length() > 0)
  {
    results.push_back(str);
  }
}
*/