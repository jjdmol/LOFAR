//# addbeaminfo.cc: add and update failed tiles info to the MeasurementSet 
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
#include <Common/Exception.h>     // THROW macro for exceptions

// SAS
#include <OTDB/OTDBconstants.h>
#include <OTDB/OTDBconnection.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/TreeValue.h>
#include <OTDB/ClassifConv.h>
#include <OTDB/Converter.h>
#include <OTDB/TreeTypeConv.h>

// C system headers
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Boost
#include <boost/bind.hpp>           // for searching vectors of structs
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
#include <measures/Measures.h>
#include <casa/Quanta/MVTime.h>
#include <casa/OS/Time.h>
#include <casa/Arrays/VectorIter.h>
#include <casa/Arrays/ArrayIter.h>

#include <Beaminfo/showinfo.h>    // showVector, showMap etc. debug functions

using namespace std;
using namespace LOFAR;
using namespace LOFAR::OTDB;
using namespace casa;

// MS reading functions
//boost::posix_time::ptime fromCasaTime (const MEpoch& epoch, double addDays);
string fromCasaTime (const MEpoch& epoch, double addDays);
MVEpoch toCasaTime(const string &time);
MVEpoch toCasaTime(const ptime &time);
bool checkTime(const ptime &starttime, const ptime &endtime);
bool checkTime(const string &starttimeString, const string &endtimeString);
bool checkTime(const MVEpoch &starttime, const MVEpoch &endtime);
string getLofarAntennaSet(const string &msName);
string getLofarAntennaSet(MeasurementSet &ms);

vector<string> readLofarStations(const MeasurementSet &ms);
void getLofarStationMap(const MeasurementSet &ms, map<string, int> &StationMap);
string determineStationType(const string &station);

Vector<MVEpoch> readObservationTimes(MeasurementSet &ms);
vector<string> readAntennas(const MeasurementSet &ms);
vector<string> readLofarStations(const MeasurementSet &ms);
//void readAntennas(const MeasurementSet &ms, vector<string> &antennas);
vector<unsigned int> getAntennaIds(const MeasurementSet &ms, const string &stationName);


string getAntennaConfiguration( MeasurementSet &ms, const string &station, 
                                const string &mode,
                                const string &AntennaConfFile);
vector<int> getAntennaConfigurationRCUs(const string &antennaConf, const string &station);


// RCUmap type definition to make life easier
typedef map<string, vector<unsigned int> > RCUmap;
// Define a type for failedTimes including antennaId, elementFlags and respective timestamps

// This is also defined in showinfo.h
#ifndef __FAILED_TILE__
#define __FAILED_TILE__
typedef struct
{
  unsigned int antennaId;
  vector<unsigned int> rcus;
  vector<ptime> timeStamps;
} failedTile;
#endif

// MS Antenna field functions
RCUmap getRCUs( const MeasurementSet &ms,
                const string &tableName="LOFAR_ANTENNA_FIELD",
                const string &elementColumnName="ELEMENT_FLAG");
void addFailedAntennaTiles( MeasurementSet &ms, 
                            const vector<failedTile> &failedTiles);
void addFailedAntennaTiles( MeasurementSet &ms, 
                            const vector<failedTile> &failedTiles,
                            const vector<bool> &flags);
void doAddFailedAntennaTile(Table &failedElementsTable, 
                            int antennaId, 
                            int element_index, 
                            const MVEpoch &timeStamp, 
                            bool flag=false);                            
bool failedAntennaElementExists(const Table &failedElementsTable, 
                                int antennaId, 
                                int element_index);

// SAS functions
vector<string> getBrokenHardware( OTDBconnection &conn, 
                                  const MVEpoch &timestamp=0);
map<string, ptime> getBrokenHardwareMap(OTDBconnection &conn,
                                        const MVEpoch &timestamp=0);
map<string, ptime> getFailedHardware( MeasurementSet &ms, 
                                      OTDBconnection &conn);

RCUmap getBrokenRCUs(const map<string, ptime> &brokenHardware);
RCUmap getBrokenRCUs( const map<string, ptime> &brokenHardware, 
                      const RCUmap &rcusMS);
RCUmap getBrokenRCUs( const vector<string> &brokenHardware, 
                      const RCUmap &rcusMS);
void extractRCUs(RCUmap &brokenRCUs, 
                 const map<string, ptime> &brokenHardware, 
                 const string &station);                      
/*
map<string, ptime> getFailedHardware( map<string, ptime> &brokenBegin, 
                                      map<string, ptime> &brokenEnd);
*/
map<string, ptime> getFailedHardware( const map<string, ptime> &brokenBegin, 
                                      const map<string, ptime> &brokenEnd);
void getFailedTiles(MeasurementSet &ms,
                    const map<string, ptime> &failedTilesSAS, 
                    map<unsigned int, vector<unsigned int> > &brokenElements,                      
                    vector<ptime> &failureTimes);
//vector<failedTile> getFailedTiles(const map<string, ptime> &failedTilesSAS);
RCUmap getFailedTiles(const map<string, ptime> &failedTilesSAS);
vector<failedTile> getFailedTilesAntennaId(MeasurementSet &ms,
                                           const map<string, ptime> &failedTilesSAS);
// don't need this anymore: now use find_fi and boost::bind
//vector<failedTile>::iterator getFailedTileAntennaId(vector<failedTile> &failedTiles,
//                                                    unsigned int antennaId);
//vector<failedTile>::const_iterator getFailedTileAntennaId(const vector<failedTile> &failedTiles,
//                                                          unsigned int antennaId);
vector<failedTile> getBrokenTiles(MeasurementSet &ms, const RCUmap &brokenRCus);
vector<failedTile> getFailedTiles(MeasurementSet &ms, 
                                  const RCUmap &failedRCUs, 
                                  const map<string, ptime> &failedHardware);
//map<unsigned int, unsigned int> getAntennaFieldId(MeasurementSet &ms,
//                                                  const string &station, 
//                                                  unsigned int RCUnum);
unsigned int getAntennaFieldId( MeasurementSet &ms,
                                const string &station, 
                                unsigned int RCUnum,
                                unsigned int &elementIndex);
void padTo(std::string &str, const size_t num, const char paddingChar);




// MS Table writing functions TODO
//void updateAntennaFieldTable( MeasurementSet &ms, 
//                              const map<string, vector<unsigned int> > &brokenRCUs);
void updateAntennaFieldTable( MeasurementSet &ms, const vector<failedTile> &brokenTiles);
void updateElementFlags(Table &table, unsigned int antennaId, unsigned int elementIndex);

// File I/O
void writeBrokenHardwareFile(const string &filename, const vector<string> &brokenHardware, bool strip=true);
void writeBrokenHardwareFile(const string &filename, const map<string, ptime> &brokenHardware, bool strip=true);
map<string, ptime> readBrokenHardwareFile(const string &filename);
string stripRCUString(const string &brokenHardware);
void getSASFailureTimes();
void writeFailedElementsFile( const string &filename,
                              const vector<string> &brokenHardware,
                              const vector<MVEpoch> &timestamps,
                              bool strip=true);                              
map<string, MVEpoch> readFailedElementsFile(const string &filename);


void usage(char *programname)
{
  cout << "Usage: " << programname << "<options>" << endl;
  cout << "-d             run in debug mode" << endl;
  cout << "-q             query SAS database for broken tiles ifnromation" << endl;
  cout << "-f             read broken hardware information from file" << endl;
  cout << "-m <msname>    MeasurementSet to add beaminfo to" << endl;
  cout << "-p <filename>  read parset (instead of default)" << endl;
  cout << "-s <time>      start time of observation in MS"<< endl;
  cout << "-e <time>      end time of observation in MS"<< endl;
  cout << "-v             turn on verbose mode" << endl;
  cout << "-h             show this help info" << endl;

  exit(0);
}

// These two flags are global so that every function can act accordingly
bool debug=false;                           // debug mode
bool verbose=false;                         // verbose mode

int main (int argc, char* argv[])
{
  int opt=0;                                // argument parsing, current option
  string optString="dqfm:p:h";              // allowed options
  bool file=false, query=false;             // read from file, query database
  bool update=false;                        // update MS with beaminfo (node mode)
  vector<MEpoch> failingTimes;

  string msArgName="";                      // name of MS to update as command argument
  string parsetName="addbeaminfo.parset";   // parset location (default)
  string starttimeString, endtimeString;    // strings to get start and end time

  map<string, ptime> brokenHardware;        // map of broken hardware with timestamps
  map<string, ptime> brokenHardwareAfter;   // hardware that failed duing obs
  map<string, ptime> failedHardware;        // hardware that failed during the observation
  RCUmap brokenRCUs;                        // broken RCUs (before obs)
  // Vector<MVEpoch> obsTimes;              // observation times of MS (for debugging mode only!)
  MVEpoch startTime, endTime;               // starttime and endtime of observation

  //---------------------------------------------
  // Init logger
  string progName = basename(argv[0]);
  INIT_LOGGER(progName);

  // Parse command line arguments TODO!
  //opt = getopt( argc, argv, optString.c_str());
  while(opt != -1) 
  {
    opt = getopt( argc, argv, "dqfm:p:s:e::uvh");  //optString.c_str()
    switch(opt) 
    { 
      case 'd':
        debug=true;
        break;
      case 'q':         // query database
        query=true;
        update=false;
        break;
      case 'f':         // read from files
        file=true;
        break;
      case 'm':         // other MS (overwriting parset MS entry)
        msArgName=optarg;
        break;
      case 'p':         // location of parset file
        parsetName=optarg;
        break;
      case 's':         // start time
        starttimeString=optarg;
      case 'e':         // end time
        endtimeString=optarg;
      case 'u':         // update MeasurementSet
        update=true;
        break;
      case 'v':         // turn on verbose display of messages
        verbose=true;
        break;
      case 'h':
        cout << "option -h" << endl;
        usage(argv[0]);
        break;
      case ':':
        cout << "Option " << opt << " is missing an argument" << endl;
        usage(argv[0]);
        break;
      default:
        break;
    }
  }
  
  // Check command arguments consistency
  if(query==true && update==true)
  {
    LOG_DEBUG_STR(argv[0] << ": options -q and -u are mutually exclusive. Exiting.");
    usage(argv[0]);
    exit(0);
  }
  
  // Parse parset entries
  try
  {
    if(verbose)
      LOG_INFO_STR("Reading parset: " << parsetName);

    //---------------------------------------------------------------------
    ParameterSet parset(parsetName);
    //string host        = parset.getString("host", "sas.control.lofar.eu");  // production
    string host        = parset.getString("host", "RS005.astron.nl");         // DEBUG
    string db          = parset.getString("db", "TESTLOFAR_3");
    string user        = parset.getString("user", "paulus");
    string password    = parset.getString("password", "boskabouter");
    string brokenfilename = parset.getString("brokenTilesFile", "/opt/lofar/share/brokenTiles.txt");
    string brokenAfterfilename = parset.getString("brokenTilesAfterFile", "/opt/lofar/share/brokenTilesAfter.txt");
    string failedfilename = parset.getString("failedTilesFile", "/opt/lofar/share/failedTiles.txt");
    // Optional parameters that give outside access of internal format handling
    // this should not be necessary to be changed
    string elementTable = parset.getString("elementTable", "LOFAR_ANTENNA_FIELD");                                          
    string elementColumn = parset.getString("elementColumn", "ELEMENT_FLAG");

    //---------------------------------------------------------------------
    // Handle observation starttime and endtime
    if(starttimeString=="")   // if we didn't get the start time from the command arguments
    {
      starttimeString = parset.getString("StartTime", "");
    }
    if(endtimeString=="")     // if we didn't get the end time from the command arguments
    {
      endtimeString = parset.getString("EndTime", "");
    }
    startTime=toCasaTime(starttimeString);
    endTime=toCasaTime(endtimeString);
    if(checkTime(startTime, endTime) != true)
    {
      THROW(Exception, "starttime >= endtime: " << starttimeString << " >= " << endtimeString);    
    }

    //---------------------------------------------------------------------
    // Connect to SAS: Query mode
    if(query)
    {   
      LOG_INFO_STR("Getting SAS antenna health information");
      OTDBconnection conn(user, password, db, host); 
      LOG_INFO("Trying to connect to the database");
      ASSERTSTR(conn.connect(), "Connnection failed");
      LOG_INFO_STR("Connection succesful: " << conn);

      // Get broken hardware strings from SAS
      brokenHardware=getBrokenHardwareMap(conn, startTime);
      writeBrokenHardwareFile(brokenfilename, brokenHardware);
      
      if(debug)
        showMap(brokenHardware);   // DEBUG     

      // Get hardware strings that was broken after observation
      brokenHardwareAfter=getBrokenHardwareMap(conn, endTime);          
      writeBrokenHardwareFile(brokenAfterfilename, brokenHardwareAfter);
      
      failedHardware=getFailedHardware(brokenHardware, brokenHardwareAfter);
      writeBrokenHardwareFile(failedfilename, failedHardware);

      if(debug)
      {
        cout << "failedHardware:" << endl;
        showMap(failedHardware);    // DEBUG
      }
    }

    //---------------------------------------------------------------------
    // Read broken hardware information from file: File mode
    //
    if(file)
    {
      // TEST: reading broken hardware from a file
      LOG_INFO_STR("reading brokenHardware from file:" << brokenfilename);
      brokenHardware=readBrokenHardwareFile(brokenfilename);
      brokenHardwareAfter=readBrokenHardwareFile(brokenAfterfilename);
    
      failedHardware=getFailedHardware(brokenHardware, brokenHardwareAfter);
    }

    //---------------------------------------------------------------------
    // Update MS with broken tiles and failed tiles info: Update mode
    //
    if(update)
    {
      // This is the "second" call, when information stored in the brokenTiles.txt
      // and failedTiles.txt is used to update the MS
      string msName;     
      if(msArgName=="")   // if no MS filename was supplied as command argument
      {
        msName = parset.getString("ms");   
      }
      else
      {
        msName=msArgName;
      }
      if(msName=="")
      {
        THROW(Exception, "No MS filename given.");
      }

      MeasurementSet ms(msName, Table::Update);     // open MeasurementSet

      //----------------------------------------------------------------------------    
      LOG_INFO_STR("reading brokenHardware from file:" << brokenfilename);
      brokenHardware=readBrokenHardwareFile(brokenfilename);
      brokenHardwareAfter=readBrokenHardwareFile(brokenAfterfilename);
      

      brokenRCUs=getBrokenRCUs(brokenHardware);   // get all broken RCUs into a RCUmap
      if(debug)
        showMap(brokenRCUs);
      
      vector<failedTile> brokenTiles=getBrokenTiles(ms, brokenRCUs); // Convert stations to antennaIds
      if(debug)
        showFailedTiles(brokenTiles);
      updateAntennaFieldTable(ms, brokenTiles);    // TODO: Update LOFAR_ANTENNA_FIELD table
    
      //----------------------------------------------------------------------------
      // Get Tiles that failed during observation
      //
      failedHardware=getFailedHardware(brokenHardware, brokenHardwareAfter);
//      RCUmap failedRCUs=getFailedTiles(failedHardware);        
      vector<failedTile> failedTiles=getFailedTilesAntennaId(ms, failedHardware);

      if(debug)    
        showFailedTiles(failedTiles);
        
      addFailedAntennaTiles(ms, failedTiles);
    }
  
  }
  catch (std::exception& x)
  {
    LOG_DEBUG_STR("Unexpected exception: " << x.what());
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
  return t.getTime().ISODate();
}

/*!
  \brief Convert a time string time YYYY-Mon-DD TT:MM:SS.ss to a CASA MVEpoch
  \param time   string in time format
  \return MVEpoch time format
*/
MVEpoch toCasaTime(const string &time)
{
  // e.g. 2011-Mar-19 21:17:06.514000  // use slashes instead spaces
  Double casaTime;        // casa MVEpoch time to be returned
  MVTime casaMVTime;      // need this for conversion
//  MVEpoch casaMVTime;      // need this for conversion
  String copyTime=time;   // make a temporary copy

  Quantity result(casaTime, "s");   // set quantity unit to seconds
  
  if(time.empty())
  {
    THROW(Exception, "toCasaTime() string time is empty"); 
  }
  else
  {
    copyTime.gsub(" ", "/");      // replace spaces with slashes for casa conversion   
    casaMVTime.read(result, copyTime);
  }
  
  return result;
}


/*!
  \brief Convert a ptime to a CASA MVEpoch
  \param time   ptime format
  \return MVEpoch time format
*/
MVEpoch toCasaTime(const ptime &time)
{
  MVEpoch casaTime;
  string timeString;

  timeString=to_simple_string(time);
  casaTime=toCasaTime(timeString);

  return casaTime;
}


/*!
  \brief Check that Starttime < Endtime
  \param starttime    start time of observation
  \param endtime      end time of observation
  \return valid       vailid if starttime < endtime
*/
bool checkTime(const ptime &starttime, const ptime &endtime)
{
  bool valid=false;
  
  if(starttime < endtime)
  {
    valid=true;
  }
  return valid;
}

bool checkTime(const string &starttimeString, const string &endtimeString)
{
  bool valid=false;

  MVEpoch starttimeCasa, endtimeCasa;
  
  cout << "starttimeString: " << starttimeString << endl;
  cout << "endtimeString: " << endtimeString << endl;
  
  starttimeCasa=toCasaTime(starttimeString);
  starttimeCasa=toCasaTime(endtimeString);
  
  ptime starttime=time_from_string(fromCasaTime(starttimeCasa));
  ptime endtime=time_from_string(fromCasaTime(endtimeCasa));
  
  valid=checkTime(starttime, endtime);
  return valid;
}

bool checkTime(const MVEpoch &starttimeCasa, const MVEpoch &endtimeCasa)
{
  bool valid=False;

  ptime starttime=time_from_string(fromCasaTime(starttimeCasa));
  ptime endtime=time_from_string(fromCasaTime(endtimeCasa));
  
  valid=checkTime(starttime, endtime);
  return valid;
}


/*!
  \brief  Update the element flags in the MeasurementSet with failed tiles info
  \param  ms            MeasurementSet to update ELEMENT_FLAGs
  \param  brokenTiles   vector containing failed tiles against antennaId
  \param  overwrite     overwrite existing entries (default=true)
*/
void updateAntennaFieldTable(MeasurementSet &ms,  const vector<failedTile> &brokenTiles)
{
  LOG_INFO_STR("Updating ELEMENT_FLAG in MS");

  cout << "brokenTiles.size() = " << brokenTiles.size() << endl;
  if(brokenTiles.size()==0)     // if there were no broken tiles...
  {
    return;                     // just return
  }

  // Open MS/LOFAR_ANTENNA_FIELD table
  Table antennaFieldTable(ms.rwKeywordSet().asTable("LOFAR_ANTENNA_FIELD"));
  ArrayColumn<Bool> elementFlagCol(antennaFieldTable, "ELEMENT_FLAG");

  // loop over ANTENNA_FIELD_TABLE
  uInt nrows=antennaFieldTable.nrow();
  map<unsigned int, unsigned int> indices;
  for(unsigned int antennaId=0; antennaId<nrows; antennaId++)
  {
    
    for(unsigned int i=0; i<brokenTiles.size(); i++)
    {
      if(brokenTiles[i].antennaId==antennaId)
      {
        indices[antennaId]=i;
      }
    }
  }
   
  for(unsigned int antennaId=0; antennaId<nrows; antennaId++)
  {
      map<unsigned int, unsigned int>::iterator it;
      
      if(indices.find(antennaId) != indices.end())
      {
        unsigned int index=indices[antennaId];  
        for(unsigned int rcuIndex=0; rcuIndex < brokenTiles[index].rcus.size(); rcuIndex++)
        {
          unsigned int rcuNum=brokenTiles[index].rcus[rcuIndex];
          
          // Update ELEMENT_FLAGS column in LOFAR_ANTENNA_FIELD table
          TableLocker locker(antennaFieldTable, FileLocker::Write);
          updateElementFlags(antennaFieldTable, antennaId, rcuNum);    
        }
      }
  }
}


/*!
  \brief Function that performs the update of the array entries in a ELEMENT_FLAG column
  \param ms             MeasurementSet to update
  \param antennaId      antenna id (=rownr) in LOFAR_ANTENNA_FIELD table
  \param elementIndex   index into ELEMENT_FLAG column array of RCUs
*/
void updateElementFlags(Table &table, unsigned int antennaId, unsigned int rcu)
{
  ArrayColumn<Bool> elementFlagCol(table, "ELEMENT_FLAG");  
  unsigned int nrows=elementFlagCol.nrow();
  
  if(antennaId > nrows-1)
  {
    THROW(Exception, "updateElementFlags() antennaId " << antennaId << " out of range");
  }
  
  // get ELEMENT_FLAG array for antennaId (row=antennaId)
  Matrix<Bool> elementFlags=elementFlagCol(antennaId);  
  unsigned int elementIndex=rcu / 2;

  cout << "updateELementFlags() antennaId = " << antennaId << "\t";     // DEBUG
  cout << "rcu = " << rcu << "\telementIndex = " << elementIndex << endl;  // DEBUG

  elementFlags(0, elementIndex)=true;          // Update ELEMENT_FLAGS at elementIndex
  elementFlags(1, elementIndex)=true;          // Update ELEMENT_FLAGS at elementIndex

  elementFlagCol.put(antennaId, elementFlags);  // write modified array to column
}


/*!
  \brief Function that performs the update of the array entries in a ELEMENT_FLAG column
  \param ms             MeasurementSet to update
  \param antennaId      antenna id (=rownr) in LOFAR_ANTENNA_FIELD table
  \param elementIndex   index into ELEMENT_FLAG column array of RCUs
*/
void updateElementFlags(Table &table, unsigned int antennaId, vector<unsigned int> rcus)
{
  ArrayColumn<Bool> elementFlagCol(table, "ELEMENT_FLAG");  
  Matrix<Bool> elementFlags=elementFlagCol(antennaId);

  unsigned int nrows=elementFlagCol.nrow();

  if(antennaId > nrows-1)
  {
    THROW(Exception, "updateElementFlags() antennaId " << antennaId << " out of range");
  }
  
  //uInt ncolumn=elementFlags.ncolumn();
//  cout << "updateELementFlags() shape: " << shape << endl;              // DEBUG
//  cout << "updateELementFlags() antennaId = " << antennaId << endl;     // DEBUG

  unsigned int nrcus=rcus.size();
  for(unsigned int i=0; i<nrcus; i++)
  {
    unsigned int elementIndex=rcus[i] % 2;
    elementFlags(0, elementIndex)=true;          // Update ELEMENT_FLAGS at elementIndex
    elementFlags(1, elementIndex)=true;          // Update ELEMENT_FLAGS at elementIndex
  }

  elementFlagCol.put(antennaId, elementFlags);
}


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

  ScalarColumn<String> nameCol(AntennaTable,"NAME");
  antennaVec=nameCol.getColumn();
  
  // convert to std::vector
  unsigned int n=antennaVec.size();
  for(unsigned int i=0; i<n; i++)
  {
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
  vector<unsigned int> antennaIds;          // antennaIds for a particular station
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
    for(vector<unsigned int>::iterator idIt=antennaIds.begin(); idIt!=antennaIds.end(); ++idIt)
    {
      unsigned int row=*idIt;                // idIt is the index into the row of LOFAR_ANTENNA_FIELD
      string name=nameCol(row);              // ANTENNA_FIELD name (e.g. LBA_INNER, HBA0, HBA1)
      
      // Handle ELEMENT_FLAG array
      Matrix<Bool> elementFlags = elementFlagCol(row);   // Read ELEMENT_FLAG column from row
      IPosition shape=elementFlags.shape();              // get shape of elements array

      // number of elements = number of RCUs
      uInt ncols=elementFlags.ncolumn(); 
      uInt nrows=elementFlags.nrow();
      // Loop over RCU indices and pick those which are 0/false (i.e. NOT FLAGGED)
      for(unsigned int i=0; i<ncols; i++)
      {
        for(unsigned int j=0; j<nrows; j++)
        {
          unsigned int rcuNum=i*2+j;    // formula to get RCU number from array row=i and col=j   
          
          if(elementFlags(i, 0)==0 && elementFlags(i, 1)==0)  // if neither of the dipoles failed
          {
            rcus[station].push_back(rcuNum);
          }
        }
      }
    }
  }  
  return(rcus);
}


/*
  \brief Get the antennaId (i.e. row number in the ANTENNA table) for a named antenna
  \param stationName    name of station antenna to look for
  \return               vector of antennaIds for this station
*/
vector<unsigned int> getAntennaIds(const MeasurementSet &ms, const string &stationName)
{
  vector<unsigned int> antennaIds;   // to hold 1 or 2 antenna ids indexing into LOFAR_ANTENNA_FIELD

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
  return antennaIds;
}


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
    cout << "writeBrokenHardwareFile(): Unable to open file " << filename << " for reading." << endl;
}


void writeBrokenHardwareFile(const string &filename, const map<string, ptime> &brokenHardware, bool strip)
{
  fstream outfile;
  outfile.open(filename.c_str(), ios::out);   // this shows the correct behaviour of overwriting the file
  string datetime;          // string to format date-time string before writing to file

  if (outfile.is_open())
  {
    LOG_INFO_STR("Writing SAS broken RCUs to file: " << filename);
  
    for(map<string, ptime>::const_iterator it=brokenHardware.begin(); it!=brokenHardware.end() ; ++it)
    {
      if(it->first.find("RCU")!=string::npos)   // Only write lines that contain RCU
      {
        // make datetime: YYYY-MM-DD-HH-MM-SS.ssss
        datetime=to_simple_string(it->second);
        //datetime.replace(11, 1,"-");    // replace space with "-"

        // optionally strip off unnecessary information from string        
        if(strip)
        {
          outfile << stripRCUString(it->first) << "\t" << datetime << endl;
        }
        else
        {
          outfile << it->first << "\t" << datetime << endl;
        }
      }
    }
    outfile.close();
  }
  else
    cout << "writeBrokenHardwareFile(): Unable to open file " << filename << " for reading." << endl;
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
  
//  cout << tokens[3] << "\t" << tokens[7] << endl;     // DEBUG
  stripped=tokens[3].append(".").append(tokens[7]).append(".");
   
  return stripped;
}


/*!
  \brief  Read broken dipoles from temporary file (this is quicker to work on than
          querying the database), lines starting with # are ignored
  \param filename          name of temporary file
  \return brokenHardware   map containing SAS output of broken hardware read from file
*/
map<string, ptime> readBrokenHardwareFile(const string &filename)//, vector<string> &brokenHardware)
{
  map<string, ptime> brokenHardware;
  string name, date, time, datetime;
  fstream infile;
  infile.open(filename.c_str(), ios::in);

  if(infile.is_open())
  {
    while(infile.good())
    {
      datetime="";
      infile >> name >> date >> time;
      datetime=date.append(" ").append(time);     // YYYY-MM-DD HH-MM-SS.ssss        
      brokenHardware.insert(std::make_pair(name, time_from_string(datetime)));
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
  string::size_type pos1=string::npos, pos2=string::npos;    // positions for line split
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
        
//        cout << "readFailedElementsFile() name = "  << name << endl;            // DEBUG
//        cout << "readFailedElementsFile() timestamp = " << timestamp << endl;   // DEBUG
        
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
  \return brokenHardware vector of broken hardware
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


vector<string> getBrokenHardware(OTDBconnection &conn)
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


/*!
  \brief Get broken hardware including timestamp of time of failure
  \param connection       OTDB connection to SAS
  \param timestamp        timestamp to check for broken hardware at
  \return brokenHardware  map of broken hardware with timestamp of failure
*/
map<string, ptime> getBrokenHardwareMap( OTDBconnection &conn,
                                         const MVEpoch &timestamp)
{
  TreeTypeConv TTconv(&conn);     // TreeType converter object
  ClassifConv CTconv(&conn);      // converter I don't know
  vector<OTDBvalue> valueList;    // OTDB value list for the previous month
  
  map<string, ptime> brokenHardware;  // map of name and time of the broken hardware (all)
  
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

  // Query SAS for broken hardware
  if(timestamp==0)
  {
     valueList = tv.getBrokenHardware();
  }
  else
  {
    valueList = tv.getBrokenHardware((time_from_string(fromCasaTime(timestamp))));
  }

  for(unsigned int i=0; i < valueList.size(); i++)
  {
    brokenHardware.insert(make_pair(valueList[i].name, valueList[i].time));
  }
  
  return brokenHardware;
}


/*
  \brief Convert SAS information broken hardware to a RCU number map
  \param map          of broken hardware with associated time stamps
  \return brokenRCUs  map of broken rcus
*/
RCUmap getBrokenRCUs(const map<string, ptime> &brokenHardware)
{
  RCUmap brokenRCUs;           // map containing broken rcus
  map<string, ptime>::const_iterator brokenIt;
  string station;              // name of station found in broken hardware
  vector<string> stations;     // list of stations extracted from broken hardware
  
  LOG_INFO_STR("Converting broken hardware info into map of broken RCUs");

  // Loop over broken hardware to extract station names
  for(brokenIt=brokenHardware.begin(); brokenIt!=brokenHardware.end(); ++brokenIt)
  {
    station=brokenIt->first.substr(0, 5);   // station name extracted from broken hardware
    // Must check if it is already listed in the vector of stations
    vector<string>::iterator found;
    if( (found=find(stations.begin(), stations.end(), station)) == stations.end() )
    {
      stations.push_back(station);   // get first 5 character station name
    }
  }
  
  vector<string>::iterator stationsIt;
  for(stationsIt=stations.begin(); stationsIt!=stations.end(); ++stationsIt)
  {
    extractRCUs(brokenRCUs, brokenHardware, *stationsIt);  
  }
  
  return brokenRCUs;
}


/*!
  \brief Get a map of broken RCUs, reduced to those being present in the MS
  \param brokenHardware     map of broken hardware string against time stamp
  \param rcusMS             RCUmap of RCUs present in the MS
  \return map of broken RCUs, <station name, vector<int> >
*/
RCUmap getBrokenRCUs( const map<string, ptime> &brokenHardware,
                      const RCUmap &rcusMS)
{
  RCUmap brokenRCUs;
  string rcuSAS;            // substring from SAS of the form "RCU<num>"

  RCUmap::const_iterator rcusMSIt;
  string::iterator foundIt;

  LOG_INFO("getting brokenRCUs for this MS");

  // Loop over vector with all broken hardware
  for(rcusMSIt = rcusMS.begin(); rcusMSIt != rcusMS.end(); ++rcusMSIt)
  {
    string station=rcusMSIt->first;
    extractRCUs(brokenRCUs, brokenHardware, station);
  }
  return brokenRCUs;
}


/*!
  \brief Extract RCU numbers for a station in broken hardware map
  \param brokenRCUs       map of broken RCUs, will be updated
  \param brokenHardware   map of broken hardware with timestamps
  \param station          station name to look for in broken hardware
*/
void extractRCUs(RCUmap &brokenRCUs, 
                 const map<string, ptime> &brokenHardware, 
                 const string &station)
{
//  string stationMS = rcusMSIt->first;             // name of station in MS
  map<string, ptime>::const_iterator brokenIt;    // iterator over broken Hardware string vector
  vector<unsigned int> rcuNumbers;   // vector containing rcu numbers of brokenHardware, will be put into map
  string rcuSAS;                                // rcu string  

  // Find all occurences of this station in the list of broken hardware
  for(brokenIt=brokenHardware.begin(); brokenIt != brokenHardware.end(); ++brokenIt)
  {      
    // Check if broken RCU's station is present in MS
    if(brokenIt->first.find(station) != string::npos)
    {
      string::size_type pos1=brokenIt->first.find("RCU");
      pos1+=3;                          // skip "RCU"
      string::size_type pos2=brokenIt->first.find(".", pos1);
  
      if(pos2!=string::npos)            // if we find a "."
      {
        rcuSAS=brokenIt->first.substr(pos1, pos2-pos1);
      }
      else
      {
        LOG_DEBUG_STR("SAS broken hardware string corrupt: " << brokenIt->first);
      }
      
      unsigned int rcuSASNum = boost::lexical_cast<unsigned int>(rcuSAS);
      RCUmap::iterator brokenRCUsIt;
      // If stationMS already exists in rcuMAP
      if( (brokenRCUsIt = brokenRCUs.find(station)) != brokenRCUs.end() ) 
      {
        brokenRCUsIt->second.push_back(rcuSASNum);
      }
      else    // else, it doesn't exist, yet, make a new pair
      {
        rcuNumbers.push_back(rcuSASNum);
        brokenRCUs.insert(std::make_pair(station, rcuNumbers));
      }
    }
  }
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
  vector<unsigned int> rcuNumbers;   // vector containing rcu numbers of brokenHardware, will be put into map
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
      // Check if broken RCU's station is present in MS
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
  \return map       map of failed hardware and timestamp
*/
map<string, ptime> getFailedHardware( MeasurementSet &ms, 
                                      OTDBconnection &conn)
{
  map<string, ptime> failedHardware;      // hardware that failed during observation
  map<string, ptime> brokenHardwareStart;   // list of broken hardware at the start of observation 
  map<string, ptime> brokenHardwareEnd;     // list of broken hardware at the end of observation
  Vector<MVEpoch> obsTimes;                 // vector with observation times from the MS
  MVEpoch timeStart, timeEnd;               // single timestamp


  obsTimes=readObservationTimes(ms);        // get observation times from ms
  timeStart=obsTimes[0];                    // broken hardware at the beginning of observation
  timeEnd=obsTimes[obsTimes.size()-1];      // broken hardware at the last timestamp
  
  brokenHardwareStart=getBrokenHardwareMap(conn, timeStart);
  brokenHardwareEnd=getBrokenHardwareMap(conn, timeEnd);  // get broken hardware for last timestamp

  // throw out all broken hardware that was already broken at the beginning
  //vector<string>::iterator hwStartIt=brokenHardwareStart.begin();
  map<string, ptime>::iterator hwEndIt=brokenHardwareEnd.begin();
  for(; hwEndIt != brokenHardwareEnd.end(); ++hwEndIt)
  {
    // find broken hardware after observation in that before observation?
    map<string, ptime>::const_iterator found=find(brokenHardwareStart.begin(),
                                              brokenHardwareStart.end(), *hwEndIt);
    // If we DID NOT find the particular RCU in the broken hardware at the start...
    if(found == brokenHardwareStart.end())
    {
      failedHardware.insert(*hwEndIt);      // ...copy it over to the failed hardware vector
    }  
  }

  return failedHardware;
}


/*!
  \brief Get failed hardware without SAS, but two different timestamp brokenHardware
        maps
  \param brokenBegin      brokenHardware at the begin of the observation
  \param brokenEnd        brokenHardware at the end of the observation
  \return failedHardware  map of failed hardware with timestamp of failure
*/
/*
map<string, ptime> getFailedHardware( map<string, ptime> &brokenBegin, 
                                      map<string, ptime> &brokenEnd)
{
  map<string, ptime> failedHardware;  // hardware that failed during observation

  // throw out all broken hardware that was already broken at the beginning
  //vector<string>::iterator hwStartIt=brokenHardwareStart.begin();
  map<string, ptime>::iterator hwEndIt=brokenEnd.begin();
  for(; hwEndIt != brokenEnd.end(); ++hwEndIt)
  {
    map<string, ptime>::const_iterator found=find( brokenBegin.begin(),
                                                   brokenBegin.end(), *hwEndIt);
    // If we DID NOT find the particular RCU in the broken hardware at the start...
    if(found == brokenBegin.end())
    {
        failedHardware.insert(*hwEndIt);
    }  
  }
  
  return failedHardware;
}
*/

/*!
  \brief Get tiles that failed during the observation without SAS connection
  \param  brokenBegin     map of broken hardware beginning the observation
  \param  brokenEnd       map of broken hardware end the observation
  \return failedHardware  map of hardware that failed during the observation
*/
map<string, ptime> getFailedHardware( const map<string, ptime> &brokenBegin, 
                                      const map<string, ptime> &brokenEnd)
{
  map<string, ptime> failedHardware;      // map of hardware that failed during observation
 
  map<string, ptime>::const_iterator brokenEndIt=brokenEnd.begin();
  for(; brokenEndIt != brokenEnd.end(); ++brokenEndIt)
  {
    // try to find brokenHardware
    map<string, ptime>::const_iterator found=brokenBegin.find(brokenEndIt->first);

    if(brokenEndIt->second < (--brokenBegin.end())->second) 
    {
      failedHardware.insert(*brokenEndIt);
    }
  }

  return failedHardware;
}


/*!
  \brief Get the failed tiles including failure times
  \param MeasurementSet     measurement set to get
  \param failureTiles SAS   individual tiles that failed during observation failed hardware info
  \return vector<failedTile>    vector of failed tiles structs with antennaId, element index, failure times
*/
//void getFailedTiles(MeasurementSet &ms,
//                    const map<string, ptime> &failedTilesSAS, 
//                    map<unsigned int, vector<unsigned int> > &brokenElements,                      
//                    vector<ptime> &failureTimes)
//void getFailedTiles(MeasurementSet &ms,
//                    const map<string, ptime> &failedTilesSAS, 
//                    vector<failedTile> &failedTiles)
RCUmap getFailedTiles(const map<string, ptime> &failedTilesSAS)
{
  if(verbose)
    LOG_INFO_STR("Determining failed tiles from failed hardware SAS information");

  string station;                      // station name extracted from one string
  unsigned int rcuNum=0;               // RCU number converted from string
  vector<unsigned int> rcuNumbers;     // RCU numbers to extract
  vector<unsigned int> antennaIds;     // ANTENNA_FIELD_ids for a particular station
  unsigned int antennaId=0, elementIndex=0;
  vector<unsigned int> elementFlags;    // ELEMENT_FLAGS to set for a particular ANTENNA_FIELD_id
  vector<ptime> timeStamps;             // need local vector to create a new failedTile struct
  RCUmap failedTiles;                   // vector to return containing failed tiles

  //--------------------------------------------------------------------

  // First get a list of antennaIds from failedTilesSAS information
  map<string, ptime>::const_iterator failedTilesSASIt=failedTilesSAS.begin();
  for(; failedTilesSASIt != failedTilesSAS.end(); ++failedTilesSASIt)
  {
    station=failedTilesSASIt->first.substr(0, 5);   // station name extracted from broken hardware
    rcuNum=boost::lexical_cast<unsigned int>(failedTilesSASIt->first.substr(9, 2));
    ptime timestamp=failedTilesSASIt->second;

    // Determine ANTENNA_FIELD_ID and ELEMENT_FLAG index this RCU belongs to
    /*
    antennaId=getAntennaFieldId(ms, station);

//    cout << failedTilesSASIt->first << endl;                // DEBUG
//    cout << "antennaId: " << antennaId << endl;             // DEBUG
//    cout << "elementIndex: " << elementIndex << endl;       // DEBUG

    // We need to find the failed tile corresponding to this antennaId
    // if it already exists
    
    vector<failedTile>::iterator failedTileIt;
    failedTileIt=getFailedTileAntennaId(failedTiles, antennaId);
    if( failedTileIt != failedTiles.end() )
    {
      failedTileIt->rcus.push_back(rcuNum);
      failedTileIt->timeStamps.push_back(timestamp);
    }
    else  // ... otherwise create a new failed tile struct and insert into the vector
    {
      elementFlags.push_back(rcuNum);
      timeStamps.push_back(timestamp);
      failedTile newTile={antennaId, elementFlags, timeStamps};  
      failedTiles.push_back(newTile);
    }
  */
  }
  showMap(failedTiles);     // DEBUG
  return failedTiles;
}

/*!
  \brief Convert a RCUmap (station -> rcu) to a failedTile struct (antennaId -> rcu)
  \param ms               MeasurementSet containing tiles
  \param failedTilesSAS   failed SAS hardware strings containing map of station to rcu number,
                          and timestamps
  \return failedTiles vector of failedTile structs with antennaId -> rcu number
*/
vector<failedTile> getFailedTilesAntennaId( MeasurementSet &ms,
                                            const map<string, ptime> &failedTilesSAS)
{
  vector<failedTile> failedTiles;   // vector of failed Tiles (anntennaId -> rcu)
  vector<unsigned int> rcus;        // local vector with rcus for newTile creation

  // Open ANTENNA table
  Table antennaTable(ms.keywordSet().asTable("ANTENNA"));    // open ANTENNA table as read
  ScalarColumn<String> nameCol(antennaTable, "NAME");        // create nameCol scalarColumn

  uInt nrows=antennaTable.nrow();
  for(uInt antennaId=0; antennaId<nrows; antennaId++)        // Loop over ANTENNA table
  {
    string name=nameCol(antennaId).substr(0,5);  // get ANTENNA table station name from name column
  
    // Loop over failedHardware
    map<string, ptime>::const_iterator failedTilesSASIt=failedTilesSAS.begin();
    for(; failedTilesSASIt != failedTilesSAS.end(); ++failedTilesSASIt)
    {
      string station=failedTilesSASIt->first.substr(0,5);
      string rcuNumString=failedTilesSASIt->first.substr(9,2);
      unsigned int rcu=boost::lexical_cast<unsigned int>(rcuNumString);
      ptime timestamp=failedTilesSASIt->second;
      vector<unsigned int> antennaIds;
      
      /*
      cout << "name = " << name << "\t";                  // DEBUG
      cout << "station = " << station << "\t";            // DEBUG
      cout << "rcuNumString = " << rcuNumString << "\t";  // DEBUG
      cout << "rcu = " << rcu << "\t";                    // DEBUG    
      cout << "ptime = " << timestamp << endl;            // DEBUG
      */
      
      if(name==station)     // if SAS station name is that of ANTENNA
      {
        // Check if this antennaId is already in failedTiles vector
        vector<failedTile>::iterator failedTileIt=std::find_if(failedTiles.begin(),
        failedTiles.end(), boost::bind(&failedTile::antennaId, _1) == antennaId);
        if( failedTileIt != failedTiles.end() )
        {
          failedTileIt->rcus.push_back(rcu);
          failedTileIt->timeStamps.push_back(timestamp);
        }
        else  // ... otherwise create a new failed tile struct and insert into the vector
        {
          failedTile newTile;
          newTile.antennaId=antennaId;
          newTile.rcus.push_back(rcu);
          newTile.timeStamps.push_back(timestamp);
          failedTiles.push_back(newTile);     // put tile into failedTiles vector
        }
      }
    }
  }
  
  return failedTiles;
}


/*!
  \brief Get antenna Ids for a particular station name
  \param  station     name of station to get antenna Ids for
  \return antennaIds  vector (possibly more than one) of antenna Ids
*/
vector<unsigned int> getAntennaIds(Table &antennaTable, const string &station)
{
  vector<unsigned int> antennaIds;

  ScalarColumn<String> nameCol(antennaTable, "NAME");        // create nameCol scalarColumn  
  
  // Loop over ANTENNA table and find station
  uInt nrows=antennaTable.nrow();
  for(uInt i=0; i<nrows; i++)
  {
    unsigned int antennaId=i;
    string name=nameCol(i).substr(0,4);     // get station name from nameCol
    
    if(name == station)                     // if name matches the station in question
    {
      antennaIds.push_back(antennaId);
    }
  }
  
  return antennaIds;
}


// Ger's approach, looping over the ANTENNA table to determine
// ANTENNA_IDs in MS and referencing them to RCU numbers
//
/*!
  \brief Convert a RCUmap of broken RCUs to a list of broken tiles with antennaIds
  \param ms           Measurementset to read ANTENNA table from
  \param brokenRCUs   map of broken RCUs against station name
  \return vector      vector with failed RCUs against antennaId
*/
vector<failedTile> getBrokenTiles(MeasurementSet &ms, const RCUmap &brokenRCUs)
{
  vector<failedTile> brokenTiles;

  Table antennaTable(ms.keywordSet().asTable("ANTENNA"));
  uInt nrows=antennaTable.nrow();

  ROScalarColumn<String> nameCol(antennaTable, "NAME");         // get Name col

  // Loop over ANTENNA table
  for(uInt antennaId=0; antennaId<nrows; antennaId++)
  {
    String station=nameCol(antennaId).substr(0, 5);       // get station name part of nameCol
    
    //cout << "antennaId = " << antennaId << " station = " << station << endl;
    
    RCUmap::const_iterator rcuStation=brokenRCUs.find(station);
    if(rcuStation != brokenRCUs.end())        // if that station in the map of broken RCUs
    {
      failedTile newTile;                     // new tile struct
      newTile.antennaId=antennaId;
      for(unsigned int i=0; i<rcuStation->second.size(); i++)
      {
        newTile.rcus.push_back(rcuStation->second[i]);
      }
      
      brokenTiles.push_back(newTile);         // store it in vector of all brokenTiles
    }
  }
  
  return brokenTiles;
}


/*!
  \brief Create antenna table with failed antenna tiles and their times of failure
         after the observation
  \param ms           MeasurementSet to add failed tiles information to
  \param failedTiles  vector of struct containing all failed tiles information
*/
void addFailedAntennaTiles( MeasurementSet &ms, 
                            const vector<failedTile> &failedTiles)
{
  Table failedElementsTable(ms.rwKeywordSet().asTable("LOFAR_ELEMENT_FAILURE"));

  //TODO
  unsigned int length=failedTiles.size();
  if(length==0)     // if there are no failed tiles....
  {
    LOG_INFO_STR("No failed tiles to be added to table");
    return;         // ...do nothing
  }
  else              // Loop over map of failed RCUs for this MS
  {
    for(unsigned int i=0; i<length; i++)  
    {
      // Check if number of element_flags is equal to number of timestamps
      ASSERTSTR(failedTiles[i].rcus.size() == failedTiles[i].timeStamps.size(),
      "addFailedAntennaTiles() number of element_flags != number of timestamps for antennaId="
      << failedTiles[i].antennaId);

      // Loop over multiple element_flags per antennaId
      for(unsigned int j=0; j<failedTiles[i].rcus.size(); j++)
      {
        TableLocker locker(failedElementsTable, FileLocker::Write);

        MVEpoch timestamp= toCasaTime(failedTiles[i].timeStamps[j]).getDay() + 
                        toCasaTime(failedTiles[i].timeStamps[j]).getDayFraction();

        doAddFailedAntennaTile( failedElementsTable, 
                                failedTiles[i].antennaId,      
                                failedTiles[i].rcus[j] % 2,
                                timestamp);
      }
    }
  }
}

/*!
  \brief (Overloaded function) Create antenna table with failed antenna tiles and their 
          times of failure after the observation
  \param ms           MeasurementSet to add failed tiles information to
  \param failedTiles  vector of struct containing all failed tiles information
  \param flags        set them immediately as flagged? (default=false)
*/
void addFailedAntennaTiles( MeasurementSet &ms, 
                            const vector<failedTile> &failedTiles,
                            const vector<bool> &flags)
{
  Table failedElementsTable(ms.rwKeywordSet().asTable("LOFAR_ELEMENT_FAILURE"));

  //TODO
  unsigned int length=failedTiles.size();
  if(length==0)     // if there are no failed tiles....
  {
    LOG_INFO_STR("No failed tiles to be added to table");
    return;         // ...do nothing
  }
  else              // Loop over map of failed RCUs for this MS
  {
    for(unsigned int i=0; i<length; i++)  
    {
      // Check if number of element_flags is equal to number of timestamps
      ASSERTSTR(failedTiles[i].rcus.size() == failedTiles[i].timeStamps.size(),
      "addFailedAntennaTiles() number of element_flags != number of timestamps for antennaId="
      << failedTiles[i].antennaId);

      // Loop over multiple element_flags per antennaId
      // TODO: use vector update function instead!
      for(unsigned int j=0; j<failedTiles[i].rcus.size(); j++)
      {
        TableLocker locker(failedElementsTable, FileLocker::Write);

        MVEpoch timestamp=toCasaTime(failedTiles[i].timeStamps[j]).getDay() + 
                          toCasaTime(failedTiles[i].timeStamps[j]).getDayFraction();

        doAddFailedAntennaTile( failedElementsTable, 
                                failedTiles[i].antennaId,      
                                failedTiles[i].rcus[j] % 2,
                                timestamp,
                                flags[i]);
      }
    }
  }
}


/*!
  \brief Do add a row to the Failed Antenna Tiles table
  \param failedElementsTable    table to write failed tiles into
  \param antennaId              antenna Id of the field with failed tiles
  \param element_index          index into ELEMENT_FLAGS array
  \param timeStamp              time of failure in seconds
  \param flag                   flag the rows? (default=false)
*/
void doAddFailedAntennaTile(Table &failedElementsTable, 
                            int antennaId, 
                            int element_index, 
                            const MVEpoch &timeStamp, 
                            bool flag)
{
  // Create a new table according to TableDesc matching MS2.0.7 ICD
  // ANTENNA_FIELD_ID, ELEMENT_INDEX, TIME, FLAG_ROW
  
  ScalarColumn<Int> antennaFieldIdCol(failedElementsTable, "ANTENNA_FIELD_ID");
  ScalarColumn<Int> elementIndexCol(failedElementsTable, "ELEMENT_INDEX");
  ScalarMeasColumn<MEpoch> timeStampCol(failedElementsTable, "TIME");

  // First check if that failed tile isn't already present in the table
  // we don't want to have double entries of failures
  
  if(!failedAntennaElementExists(failedElementsTable, antennaId, element_index))
  {
    uint rownr = failedElementsTable.nrow();
    failedElementsTable.addRow();
  
    antennaFieldIdCol.put(rownr, antennaId);
    elementIndexCol.put(rownr, element_index);
    timeStampCol.put(rownr, timeStamp.getTime(Unit("s")));
   
    // If there is a FLAG_ROW column, add the flags
    if(failedElementsTable.tableDesc().isColumn("FLAG_ROW"))
    {
      ScalarColumn<Bool> flagCol(failedElementsTable, "FLAG_ROW");
      flagCol.put(rownr, flag);
    }
  }
}


/*!
  \brief Check if an entry for that failed antenna element already exists
  \param failedElementsTable    table containing LOFAR_FAILED_ELEMENT entries
  \param antennaId              antenna field Id
  \param element_index          ELEMENT_FLAGS index of RCU
  \param timeStamp              time of failure
  \return bool    if it already exists
*/
bool failedAntennaElementExists(const Table &failedElementsTable, 
                                int antennaId, 
                                int element_index) 
{
  bool exists=False;
  uInt nrows=failedElementsTable.nrow();
  
  ROScalarColumn<Int> antennaFieldIdCol(failedElementsTable, "ANTENNA_FIELD_ID");
  ROScalarColumn<Int> elementIndexCol(failedElementsTable, "ELEMENT_INDEX");

  // loop over table rows and check ANTENNA_FIELD_ID and ELEMENT_INDEX, TIME
  for(uInt i=0; i < nrows; i++)
  {
    if( antennaFieldIdCol(i)==antennaId && elementIndexCol(i)==element_index)
    {
      exists=true;
    }
  }
  
  return exists;
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
