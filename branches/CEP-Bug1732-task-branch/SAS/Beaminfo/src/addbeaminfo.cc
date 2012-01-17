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
//#include <tables/Tables/TableLocker.h>
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

// RCUmap type definition to make life easier
typedef map<string, vector<unsigned int> > RCUmap;
// Define a type for failedTimes including antennaId, elementFlags and respective timestamps

// This is also defined in showinfo.h
#ifndef __FAILED_TILE__
#define __FAILED_TILE__
typedef struct
{
  unsigned int antennaFieldId;
  vector<unsigned int> rcus;
  vector<ptime> timeStamps;
} failedTile;
#endif

// MS Antenna field functions
void addFailedAntennaTiles( MeasurementSet &ms, 
                            const vector<failedTile> &failedTiles);
void addFailedAntennaTiles( MeasurementSet &ms, 
                            const vector<failedTile> &failedTiles,
                            const vector<bool> &flags);
void doAddFailedAntennaTile(Table &failedElementsTable, 
                            int antennaFieldId, 
                            int element_index, 
                            const MVEpoch &timeStamp, 
                            bool flag=false);    
int getAntennaFieldId(const MeasurementSet &ms, int antennaId, int rcu);
bool failedAntennaElementExists(Table &failedElementsTable, 
                                int antennaFieldId, 
                                int element_index);

// SAS functions
map<string, ptime> getBrokenHardwareMap(OTDBconnection &conn,
                                        const MVEpoch &timestamp=0);
map<string, ptime> getFailedHardwareMap( OTDBconnection &conn,
                                         const MVEpoch &timeStart,
                                         const MVEpoch &timeEnd);                                        
RCUmap getBrokenRCUs(const map<string, ptime> &brokenHardware);
void extractRCUs(RCUmap &brokenRCUs, 
                 const map<string, ptime> &brokenHardware, 
                 const string &station);                      
map<string, ptime> getFailedHardware( const map<string, ptime> &brokenBegin, 
                                      const map<string, ptime> &brokenEnd);
vector<failedTile> getBrokenTilesAntennaId( MeasurementSet &ms, 
                                            const map<string, ptime> &failedTilesSAS);
vector<failedTile> getFailedTilesAntennaId(MeasurementSet &ms,
                                           const map<string, ptime> &failedTilesSAS);

bool elementInObservation(const Table &antennaFieldTable,
                          unsigned int antennaId,
                          unsigned int element_index);                                
void sortFailedTilesByTime(vector<failedTile> &tiles);
void padTo(std::string &str, const size_t num, const char paddingChar);

// MS Table writing function
void updateBeamTables(MeasurementSet &ms, 
                      const vector<failedTile> &brokenTile, 
                      const vector<failedTile> &failedTile);
void updateAntennaFieldTable( MeasurementSet &ms, const vector<failedTile> &brokenTiles);
void updateElementFlags(Table &table, unsigned int antennaFieldId, unsigned int elementIndex);
void updateElementFlags(Table &table, 
                        unsigned int antennaId, 
                        const vector<unsigned int> &elementIndex);

// File I/O
void writeBrokenHardwareFile( const string &filename, 
                              const vector<string> &brokenHardware, 
                              bool strip=true);
void writeBrokenHardwareFile( const string &filename, const map<string, 
                              ptime> &brokenHardware, bool strip=true);
map<string, ptime> readBrokenHardwareFile(const string &filename);
string stripRCUString(const string &brokenHardware);
void writeFailedElementsFile( const string &filename,
                              const map<string, ptime> &failedHardware);
map<string, ptime> readFailedElementsFile(const string &filename);


//----------------------------------------------------------------------------------
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
  MVEpoch startTime, endTime;               // starttime and endtime of observation

  //---------------------------------------------
  // Init logger
  string progName = basename(argv[0]);
  INIT_LOGGER(progName);

  // Parse command line arguments TODO!
  while(opt != -1) 
  {
    opt = getopt( argc, argv, "dqfm:p:s:e::uvh");
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
    if(starttimeString.empty())   // if we didn't get the start time from the command arguments
    {
      starttimeString = parset.getString("StartTime", "");
    }
    if(endtimeString.empty())     // if we didn't get the end time from the command arguments
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
      {
        showMap(brokenHardware);   // DEBUG     
      }

      // Get hardware strings that was broken after observation
      brokenHardwareAfter=getBrokenHardwareMap(conn, endTime);          
      writeBrokenHardwareFile(brokenAfterfilename, brokenHardwareAfter);
      
      //failedHardware=getFailedHardware(brokenHardware, brokenHardwareAfter);  // use comparison
      failedHardware=getFailedHardwareMap(conn, startTime, endTime);

//      cout << "main() failedHardware:" << endl;           // DEBUG
//      showMap(failedHardware);                            // DEBUG

      writeBrokenHardwareFile(failedfilename, failedHardware);

      if(debug)
      {
        LOG_DEBUG_STR("failedHardware:");
        showMap(failedHardware);    // DEBUG
      }
    }

    //---------------------------------------------------------------------
    // Read broken hardware information from file: File mode (for debugging)
    //
    if(file)
    {
      // TEST: reading broken hardware from a file
      LOG_INFO_STR("reading brokenHardware from file:" << brokenfilename);
      brokenHardware=readBrokenHardwareFile(brokenfilename);
      LOG_INFO_STR("reading brokenHardware after from file:" << brokenAfterfilename);      
      brokenHardwareAfter=readBrokenHardwareFile(brokenAfterfilename);
    
//      failedHardware=getFailedHardware(brokenHardware, brokenHardwareAfter);
//      failedHardware=readFailedElementsFile(failedfilename);
      if(debug)
      {
        showMap(failedHardware);
      }
      
//      writeFailedElementsFile(failedfilename, failedHardware);  // write failed tiles to disk

      LOG_INFO_STR("Read failed tiles from file:");             // DEBUG
      failedHardware=readFailedElementsFile(failedfilename);    // DEBUG
      showMap(failedHardware);
    }

    //---------------------------------------------------------------------
    // Update MS with broken tiles and failed tiles info: Update mode
    //
    if(update)
    {
      // This is the "second" call, when information stored in the brokenTiles.txt
      // and failedTiles.txt is used to update the MS
      string msName;     
      if(msArgName.empty())           // if no MS filename was supplied as command argument
      {
        msName = parset.getString("ms");   
      }
      else
      {
        msName=msArgName;
      }
      if(msName.empty())
      {
        THROW(Exception, "No MS filename given.");
      }

      MeasurementSet ms(msName, Table::Update);     // open MeasurementSet

      //----------------------------------------------------------------------------    
      LOG_INFO_STR("reading brokenHardware from file:" << brokenfilename);
      brokenHardware=readBrokenHardwareFile(brokenfilename);
      LOG_INFO_STR("reading brokenHardware after from file:" << brokenfilename);
      brokenHardwareAfter=readBrokenHardwareFile(brokenAfterfilename);
      
      vector<failedTile> brokenTiles=getFailedTilesAntennaId(ms, brokenHardware);
      if(debug)
      {
        LOG_DEBUG_STR("Broken tiles:");
        showFailedTiles(brokenTiles);
      }
      updateAntennaFieldTable(ms, brokenTiles);
      
      //----------------------------------------------------------------------------
      // Get Tiles that failed during observation
      //
      //failedHardware=getFailedHardware(brokenHardware, brokenHardwareAfter);
      failedHardware=readFailedElementsFile(failedfilename);
      vector<failedTile> failedTiles=getFailedTilesAntennaId(ms, failedHardware);

      if(debug)
      {
        LOG_DEBUG_STR("Failed tiles:");
        showFailedTiles(failedTiles);       
      }
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
  // e.g. 2011-Mar-19 21:17:06.514000
  Double casaTime;                  // casa MVEpoch time to be returned
  Quantity result(casaTime, "s");   // set quantity unit to seconds
  
  if(time.empty())
  {
    THROW(Exception, "toCasaTime() string time is empty"); 
  }
  else
  {
    MVTime::read(result, time);
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
  return(starttime < endtime);
}

bool checkTime(const string &starttimeString, const string &endtimeString)
{
  ptime starttime=time_from_string(starttimeString);
  ptime endtime=time_from_string(endtimeString);

  return(checkTime(starttime, endtime));
}

bool checkTime(const MVEpoch &starttimeCasa, const MVEpoch &endtimeCasa)
{
  return(starttimeCasa.get() < endtimeCasa.get());
}

/*!
  \brief  High-level function that updates both ELEMENT_FLAG in LOFAR_ANTENNA_FIELD table and adds
          LOFAR_FAILED_ELEMENT entries
  \param ms           MeasurementSet to update
  \param brokenTiles  tiles with timestamp broken before observation
  \param failedTiles  tiles with timestamp that failed during observation
  \return void
*/
/*
void updateBeamTables(MeasurementSet &ms, 
                      const map<string, ptime> &brokenHardware, 
                      const map<string, ptime> &failedHardware)
*/
void updateBeamTables(MeasurementSet &ms, 
                      const vector<failedTile> &brokenTiles, 
                      const vector<failedTile> &failedTiles)
{
  // open ANTENNA table
  // open LOFAR_ANTENNA_FIELD table
  // open LOFAR_FAILED_ELEMENT table
  Table antennaTable(ms.keywordSet().asTable("ANTENNA"));
  Table antennaFieldTable(ms.keywordSet().asTable("LOFAR_ANTENNA_FIELD"));
  Table failedElementsTable(ms.keywordSet().asTable("LOFAR_FAILED_ELEMENT"));    
  ScalarColumn<String> nameCol(antennaTable, "NAME");        // create nameCol scalarColumn

  // Loop over LOFAR_ANTENNA_FIELD table
  unsigned int nrows=antennaFieldTable.nrow();
  for(unsigned int antennaFieldId=0; antennaFieldId<nrows; antennaFieldId++)
  {
    // index ANTENNA_ID into ANTENNA table
    ScalarColumn<String> nameCol(antennaTable, "NAME");         // get station name
    string name=nameCol(antennaFieldId);  

    // check if station is in brokenHardware
    vector<failedTile>::const_iterator brokenTileIt=std::find_if(brokenTiles.begin(),
        brokenTiles.end(), boost::bind(&failedTile::antennaFieldId, _1) == antennaFieldId);
    //map<string, ptime>::iterator brokenHardwareIt=std::find(brokenHardware.begin(),
    //    brokenHardware.end()), name);
    if(brokenTileIt!=brokenTiles.end())
    {
      // update ELEMENT_FLAG in LOFAR_ANTENNA_FIELD at loop-index, i.e. ANTENNA_FIELDID
      updateElementFlags(antennaFieldTable, antennaFieldId, brokenTileIt->rcus);  
    }
  
    // check if station is in failedHardware
    vector<failedTile>::const_iterator failedTileIt=std::find_if(failedTiles.begin(),
        failedTiles.end(), boost::bind(&failedTile::antennaFieldId, _1) == antennaFieldId);
    if(failedTileIt!=failedTiles.end())
    {
      // loop over RCUs from failedHardware
      for(unsigned int i=0; i<failedTileIt->rcus.size(); i++)
      {
        unsigned int elementIndex=failedTileIt->rcus[i]/2;
        MVEpoch timestamp = toCasaTime(failedTileIt->timeStamps[i]);
        doAddFailedAntennaTile(failedElementsTable, antennaFieldId, elementIndex, timestamp);
      }
    }
  }
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

  if(brokenTiles.size()==0)     // if there were no broken tiles...
  {
    return;                     // just return
  }

  // Open MS/LOFAR_ANTENNA_FIELD table
  Table antennaFieldTable(ms.rwKeywordSet().asTable("LOFAR_ANTENNA_FIELD"));
  ArrayColumn<Bool> elementFlagCol(antennaFieldTable, "ELEMENT_FLAG");

  // loop over ANTENNA_FIELD_TABLE
  uInt nrows=antennaFieldTable.nrow();
  for(unsigned int i=0; i<brokenTiles.size(); i++)    // loop over broken tiles map
  {
    if(brokenTiles[i].antennaFieldId < nrows-1)       // if the antenna id is present in ANTENNA table
    {
      updateElementFlags(antennaFieldTable, brokenTiles[i].antennaFieldId, brokenTiles[i].rcus);
    }
  }
}

/*!
  \brief Function that performs the update of the array entries in a ELEMENT_FLAG column
  \param ms             MeasurementSet to update
  \param antennaId      antenna id (=rownr) in LOFAR_ANTENNA_FIELD table
  \param elementIndex   index into ELEMENT_FLAG column array of RCUs
*/
void updateElementFlags(Table &table, unsigned int antennaFieldId, unsigned int rcu)
{
  ArrayColumn<Bool> elementFlagCol(table, "ELEMENT_FLAG");  
  unsigned int nrows=elementFlagCol.nrow();
  
  if(antennaFieldId > nrows-1)
  {
    THROW(Exception, "updateElementFlags() antennaFieldId " << antennaFieldId << " out of range");
  }
  
  // get ELEMENT_FLAG array for antennaId (row=antennaId)
  Matrix<Bool> elementFlags=elementFlagCol(antennaFieldId);  
  unsigned int elementIndex=rcu / 2;

  if(elementIndex > elementFlags.ncolumn()-1)        // if elementIndex is out of range
  {
    return;
  }
  else
  {
    elementFlags(0, elementIndex)=true;                 // Update ELEMENT_FLAGS at elementIndex
    elementFlags(1, elementIndex)=true;                 // Update ELEMENT_FLAGS at elementIndex
    elementFlagCol.put(antennaFieldId, elementFlags);   // write modified array to column
  }
}

/*!
  \brief Function that performs the update of the array entries in a ELEMENT_FLAG column
  \param ms             MeasurementSet to update
  \param antennaFieldId antenna id (=rownr) in LOFAR_ANTENNA_FIELD table
  \param elementIndex   index into ELEMENT_FLAG column array of RCUs
*/
void updateElementFlags(Table &table, unsigned int antennaFieldId, const vector<unsigned int> &rcus)
{
  ArrayColumn<Bool> elementFlagCol(table, "ELEMENT_FLAG");  
  Matrix<Bool> elementFlags=elementFlagCol(antennaFieldId);

  unsigned int nrows=elementFlagCol.nrow();

  if(rcus.size()==0)    // if there are no rcus to be updated for this antennaId....
  {
    return;             // ... just return
  }
  if(antennaFieldId > nrows-1)
  {
    THROW(Exception, "updateElementFlags() antennaId " << antennaFieldId << " out of range");
  }
  
  if(debug) // if debugging is switched on, show information on flag updates
  {
    LOG_DEBUG_STR("updateELementFlags() antennaFieldId = " << antennaFieldId);   // DEBUG
    LOG_DEBUG_STR("rcus: ");                                                     // DEBUG
    showVector(rcus);                                                            // DEBUG
  }
  for(unsigned int i=0; i<rcus.size(); i++)
  {
    unsigned int elementIndex=rcus[i] / 2;
    if(elementIndex > elementFlags.ncolumn()-1)       // if elementIndex is out of range
    {
      return;
    }
    else
    {
      elementFlags(0, elementIndex)=true;             // update array at elementIndex X polarization
      elementFlags(1, elementIndex)=true;             // update array at elementIndex Y polarization
      elementFlagCol.put(antennaFieldId, elementFlags);    // update ELEMENT_FLAGS column/row
    }
  }
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
  {
    THROW(Exception, "writeBrokenHardwareFile(): Unable to open file " << filename << 
          " for reading.");
  }
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
  {
    THROW(Exception, "writeBrokenHardwareFile(): Unable to open file " << filename << 
          " for reading.");
  }
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

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(".");
  tokenizer tok(brokenHardware, sep);

  for(tokenizer::iterator beg=tok.begin(); beg!=tok.end();++beg)
  {
    tokens.push_back(*beg);
  }
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
  fstream infile;
  infile.open(filename.c_str(), ios::in);

  if(infile.is_open())
  {
    unsigned int linenumber=0;      // keep a track of line numbers to report errors
    while(infile.good())
    {
      linenumber++;
      string name, date, time, datetime;

      infile >> name >> date >> time;
      datetime=date.append(" ").append(time);     // YYYY-MM-DD HH-MM-SS.ssss        
      if(name.empty() || date.empty() || time.empty())
      {
        LOG_WARN_STR("readFailedElementsFile() line " << linenumber << " is corrupt. Skipping...");      
        continue;
      }
      else
      {
        brokenHardware.insert(std::make_pair(name, time_from_string(datetime)));
      }
    } 
    infile.close();
  }
  else
  {
    THROW(Exception, "readFile(): Unable to open file" << filename << " for reading.");
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
                              const map<string, ptime> &failedElements)
{
  fstream outfile;
  outfile.open(filename.c_str(), ios::out);   // this shows the correct behaviour of overwriting the file

  if (outfile.is_open())
  {
    LOG_INFO_STR("Writing SAS failed elements RCUs with timestamps to file: " << filename);
  
    for(map<string, ptime>::const_iterator it=failedElements.begin(); it!=failedElements.end() ; ++it)
    {
      if(it->first.find("RCU")!=string::npos)   // Only write lines that contain RCU
      {
        outfile << it->first;                           // write Station.RCU<num>
        outfile << "\t" << it->second << endl;          // Write timestamp
      }
    }
    outfile.close();
  }
  else
  {
    THROW(Exception, "writeFailedElementsFile(): Unable to open file " << filename << 
          " for writing.");
  }
}

/*!
  \brief Read failed elements with timestamps from ASCII file
  \param filename       name of file to contain list of failed elements
  \param failedElements vector of failed element names
  \param timestamps     vector containing the associated timestamps for the failed elements
*/
map<string, ptime> readFailedElementsFile(const string &filename)
{
  string name, date, time;                        // name and timestamp to split into 
  map<string, ptime> failedElements;
  fstream infile;
  infile.open(filename.c_str(), ios::in);

  if(infile.is_open())
  {
    unsigned int linenumber=0;      // keep a track of line numbers to report errors
    while(infile.good())
    {
      linenumber++;
    
      string datetime="";
      infile >> name >> date >> time;
      datetime=date.append(" ").append(time);     // YYYY-MM-DD HH-MM-SS.ssss        
      if(name.empty() || date.empty() || date.empty())
      {
        LOG_WARN_STR("readFailedElementsFile() line " << linenumber << " is corrupt. Skipping...");      
        continue;
      }
      else
      {
        failedElements.insert(std::make_pair(name, time_from_string(datetime)));      
      }
    }
    infile.close();
  }
  else
  {
    THROW(Exception, "readFailedElementsFile(): Unable to open file" << filename << 
          " for reading.");
  }
  return failedElements;
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

  LOG_INFO_STR("Getting broken hardware at " << MVTime(timestamp.get()).getTime().ISODate());

  // Query SAS for broken hardware
  if(timestamp==0)
  {
     valueList = tv.getBrokenHardware();
  }
  else
  {
    valueList = tv.getBrokenHardware(time_from_string(fromCasaTime(timestamp)));
  }

  for(unsigned int i=0; i < valueList.size(); i++)
  {
    brokenHardware.insert(make_pair(valueList[i].name, valueList[i].time));
  }
  
  return brokenHardware;
}

/*!
  \brief  Get hardware that failed during time interval between timeStart and timeEnd
          including timestamp of time of failure
  \param connection       OTDB connection to SAS
  \param timeStart        timestamp to check for failed hardware from
  \param timeEnd          timestamp to check for failed hardware to
  \return failedHardware  map of failed hardware with timestamp of failure
*/
map<string, ptime> getFailedHardwareMap( OTDBconnection &conn,
                                         const MVEpoch &timeStart,
                                         const MVEpoch &timeEnd)
{
  TreeTypeConv TTconv(&conn);     // TreeType converter object
  ClassifConv CTconv(&conn);      // converter I don't know
  vector<OTDBvalue> valueList;    // OTDB value list for the previous month
  
  map<string, ptime> failedHardware;  // map of name and time of the broken hardware (all)
  
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

  LOG_INFO_STR("Getting failed hardware from " << MVTime(timeStart.get()).getTime().ISODate() 
               << " to " << MVTime(timeEnd.get()).getTime().ISODate());

  // Query SAS for broken hardware
  valueList = tv.getFailedHardware(time_from_string(fromCasaTime(timeStart)), 
                                   time_from_string(fromCasaTime(timeEnd)));
  for(unsigned int i=0; i < valueList.size(); i++)
  {
    failedHardware.insert(make_pair(valueList[i].name, valueList[i].time));
  }

  return failedHardware;
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
  \brief Extract RCU numbers for a station in broken hardware map
  \param brokenRCUs       map of broken RCUs, will be updated
  \param brokenHardware   map of broken hardware with timestamps
  \param station          station name to look for in broken hardware
*/
void extractRCUs(RCUmap &brokenRCUs, 
                 const map<string, ptime> &brokenHardware, 
                 const string &station)
{
  map<string, ptime>::const_iterator brokenIt;    // iterator over broken Hardware string vector
  vector<unsigned int> rcuNumbers;   // vector containing rcu numbers of brokenHardware
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
    map<string, ptime>::const_iterator found=brokenBegin.find(brokenEndIt->first);

    if(found == brokenBegin.end())        // if we didn't find it in broken hardware at the beginning
    {
      failedHardware.insert(*brokenEndIt);
    }
  } 
  return failedHardware;
}

/*!
  \brief  Convert failedTiles SAS maps (station.rcu-string + timestamp) to a failedTile 
          struct (antennaId -> rcu)
  \param ms               MeasurementSet containing tiles
  \param brokenTilesSAS   failed SAS hardware strings containing map of station to rcu number,
                          and timestamps
  \return failedTiles vector of failedTile structs with antennaId -> rcu number
*/
vector<failedTile> getBrokenTilesAntennaId( MeasurementSet &ms,
                                            const map<string, ptime> &brokenTilesSAS)
{
  vector<failedTile> brokenTiles;   // vector of failed Tiles (anntennaId -> rcu)
//  vector<unsigned int> rcus;        // local vector with rcus for newTile creation

  // Open ANTENNA table
  Table antennaTable(ms.keywordSet().asTable("ANTENNA"));    // open ANTENNA table as read
  Table antennaFieldTable(ms.keywordSet().asTable("LOFAR_ANTENNA_FIELD"));    // open ANTENNA table as read
  ScalarColumn<String> nameCol(antennaTable, "NAME");        // create nameCol scalarColumn

  uInt nrows=antennaTable.nrow();
  for(uInt antennaId=0; antennaId<nrows-1; antennaId++)        // Loop over ANTENNA table
  {
    string name=nameCol(antennaId).substr(0,5);  // get ANTENNA table station name from name column
  
    // Loop over failedHardware
    map<string, ptime>::const_iterator brokenTilesSASIt=brokenTilesSAS.begin();
    for(; brokenTilesSASIt != brokenTilesSAS.end(); ++brokenTilesSASIt)
    {
      string station=brokenTilesSASIt->first.substr(0,5);
      string rcuNumString=brokenTilesSASIt->first.substr(9,2);
      unsigned int rcu=boost::lexical_cast<unsigned int>(rcuNumString);
      ptime timestamp=brokenTilesSASIt->second;

      unsigned int elementIndex=rcu/2;
      int antennaFieldId=getAntennaFieldId(ms, antennaId, rcu);    // get antennaFieldId
      if(antennaFieldId==-1)
      {
        continue;
      }
      
      if(name==station)     // if SAS station name is that of ANTENNA
      {
        //cout << "rcu = " << rcu << " antennaFieldId = " << antennaFieldId << endl;    // DEBUG

        if(elementInObservation(antennaFieldTable, antennaFieldId, elementIndex) == false)
        {
          continue;
        }
        // Check if this antennaId is already in failedTiles vector
        vector<failedTile>::iterator brokenTileIt=std::find_if(brokenTiles.begin(),
        brokenTiles.end(), boost::bind(&failedTile::antennaFieldId, _1) == antennaFieldId);
        if( brokenTileIt != brokenTiles.end() )
        {
          brokenTileIt->rcus.push_back(rcu);
          brokenTileIt->timeStamps.push_back(timestamp);
        }
        else  // ... otherwise create a new failed tile struct and insert into the vector
        {
          failedTile newTile;
          newTile.antennaFieldId=antennaFieldId;
          newTile.rcus.push_back(rcu);
          newTile.timeStamps.push_back(timestamp);
          brokenTiles.push_back(newTile);     // put tile into failedTiles vector
        }
      }
    }
  }
  
  cout << "getBrokenTilesAntennaId() brokenTiles:" << endl;  // DEBUG
  showFailedTiles(brokenTiles);
  
  return brokenTiles;
}

/*!
  \brief  Convert failedTiles SAS maps (station.rcu-string + timestamp) to a failedTile 
          struct (antennaId -> rcu)
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
  Table antennaFieldTable(ms.keywordSet().asTable("LOFAR_ANTENNA_FIELD"));    // open ANTENNA table as read
  ScalarColumn<String> nameCol(antennaTable, "NAME");        // create nameCol scalarColumn

  uInt nrows=antennaTable.nrow();
  for(uInt antennaId=0; antennaId<nrows-1; antennaId++)        // Loop over ANTENNA table
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

      unsigned int elementIndex=rcu/2;
      int antennaFieldId=getAntennaFieldId(ms, antennaId, rcu);    // get antennaFieldId
      if(antennaFieldId==-1)
      {
        continue;
      }
      
      if(name==station)     // if SAS station name is that of ANTENNA
      {
        if(elementInObservation(antennaFieldTable, antennaFieldId, elementIndex) == false)
        {
          continue;
        }
        // Check if this antennaId is already in failedTiles vector
        vector<failedTile>::iterator failedTileIt=std::find_if(failedTiles.begin(),
        failedTiles.end(), boost::bind(&failedTile::antennaFieldId, _1) == antennaFieldId);
        if( failedTileIt != failedTiles.end() )
        {
          failedTileIt->rcus.push_back(rcu);
          failedTileIt->timeStamps.push_back(timestamp);
        }
        else  // ... otherwise create a new failed tile struct and insert into the vector
        {
          failedTile newTile;
          newTile.antennaFieldId=antennaFieldId;
          newTile.rcus.push_back(rcu);
          newTile.timeStamps.push_back(timestamp);
          failedTiles.push_back(newTile);     // put tile into failedTiles vector
        }
      }
    }
  }  
  sortFailedTilesByTime(failedTiles);
  return failedTiles;
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
  Table antennaFieldTable(ms.rwKeywordSet().asTable("LOFAR_ANTENNA_FIELD"));
  Table failedElementsTable(ms.rwKeywordSet().asTable("LOFAR_ELEMENT_FAILURE"));

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
      << failedTiles[i].antennaFieldId);

      // Loop over multiple element_flags per antennaId
      for(unsigned int j=0; j<failedTiles[i].rcus.size(); j++)
      {
        MVEpoch timestamp = toCasaTime(failedTiles[i].timeStamps[j]);
        unsigned int antennaFieldId=failedTiles[i].antennaFieldId;
        unsigned int elementIndex=failedTiles[i].rcus[j] / 2;

        if(elementInObservation(antennaFieldTable, antennaFieldId, elementIndex) == true)
        {
            doAddFailedAntennaTile( failedElementsTable, 
                                    antennaFieldId,      
                                    elementIndex,
                                    timestamp.get());
        }
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
  // needed to check in ELEMENT_FLAG column/Array if tile is taking part in observation
  Table antennaFieldTable(ms.rwKeywordSet().asTable("LOFAR_ANTENNA_FIELD"));
  
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
      << failedTiles[i].antennaFieldId);

      // Loop over multiple element_flags per antennaId
      for(unsigned int j=0; j<failedTiles[i].rcus.size(); j++)
      {
        unsigned int antennaFieldId=failedTiles[i].antennaFieldId;
        unsigned int elementIndex=failedTiles[i].rcus[j] / 2;
      
        // First check if the ELEMENT_FLAG is true, i.e. only if the RCU is used
        // in this observation mode
        if(elementInObservation(antennaFieldTable, antennaFieldId, elementIndex) == true)
        {
            MVEpoch timestamp=toCasaTime(failedTiles[i].timeStamps[j]).get(); 
            doAddFailedAntennaTile( failedElementsTable, 
                                    antennaFieldId,      
                                    elementIndex,
                                    timestamp,
                                    flags[i]);
        }
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
                            int antennaFieldId, 
                            int element_index, 
                            const MVEpoch &timeStamp, 
                            bool flag)
{
  // Create a new table according to TableDesc matching MS2.0.7 ICD
  // ANTENNA_FIELD_ID, ELEMENT_INDEX, TIME, FLAG_ROW  
  ScalarColumn<Int> antennaFieldIdCol(failedElementsTable, "ANTENNA_FIELD_ID");
  ScalarColumn<Int> elementIndexCol(failedElementsTable, "ELEMENT_INDEX");
  ScalarMeasColumn<MEpoch> timeStampCol(failedElementsTable, "TIME");
 
  // Check if that failed tile isn't already present in the table
  // we don't want to have double entries of failures
  if(!failedAntennaElementExists(failedElementsTable, antennaFieldId, element_index))
  {
    uint rownr = failedElementsTable.nrow();
    failedElementsTable.addRow();
  
    antennaFieldIdCol.put(rownr, antennaFieldId);
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
  \brief Get antennaFieldId for a station/RCU
  \param station          name of lofar station
  \param rcu              rcu number
  \return antennaFieldId  index into LOFAR_ANTENNA_FIELD table
*/
int getAntennaFieldId(const MeasurementSet &ms, int antennaId, int rcu)
{ 
  unsigned int antennaFieldId=-1;
  unsigned int elementIndex=rcu/2;

  Table antennaFieldTable(ms.keywordSet().asTable("LOFAR_ANTENNA_FIELD"));
  ROScalarColumn<Int> antennaIdCol(antennaFieldTable, "ANTENNA_ID");
  ROArrayColumn<Bool> elementFlagCol(antennaFieldTable, "ELEMENT_FLAG");
  
  uInt nrows=antennaFieldTable.nrow();
  for(antennaFieldId=0; antennaFieldId<nrows-1; antennaFieldId++)
  {
    const Matrix<Bool> elementFlags=elementFlagCol(antennaFieldId);   // ELEMENT_FLAG array for antennaId 

    if(elementIndex > elementFlags.ncolumn()-1)
    {
      return antennaFieldId;  // return error (-1)
    }
    if(antennaIdCol(antennaFieldId) == antennaId && 
       elementFlags(0, elementIndex)==false && elementFlags(1, elementIndex)==false)
    {
//      cout << "getAntennaFieldId() antennaId: " << antennaId << " rcu:  "<< rcu << " antennaFieldId: " 
//      << antennaFieldId << endl;      // DEBUG
      return antennaFieldId;
    }
  }
  
  return antennaFieldId;
}

/*!
  \brief Check if an entry for that failed antenna element already exists
  \param failedElementsTable    table containing LOFAR_FAILED_ELEMENT entries
  \param antennaId              antenna field Id
  \param element_index          ELEMENT_FLAGS index of RCU
  \param timeStamp              time of failure
  \return bool    if it already exists
*/
bool failedAntennaElementExists(Table &failedElementsTable, 
                                int antennaFieldId, 
                                int element_index) 
{
  bool exists=False;
  uInt nrows=failedElementsTable.nrow();
  
  ROScalarColumn<Int> antennaFieldIdCol(failedElementsTable, "ANTENNA_FIELD_ID");
  ROScalarColumn<Int> elementIndexCol(failedElementsTable, "ELEMENT_INDEX");

  // loop over table rows and check ANTENNA_FIELD_ID and ELEMENT_INDEX, TIME
  for(uInt i=0; i < nrows; i++)
  {
    if( antennaFieldIdCol(i)==antennaFieldId && elementIndexCol(i)==element_index)
    {
      exists=true;
    }
  }
  return exists;
}

/*!
  \brief Particular element takes part in observation
  \param antennaFieldTable  LOFAR_ANTENNA_FIELD table of MS
  \param antennaId          antenna Id within LOFAR_ANTENNA_FIELD table
  \param element_index      element_index into ELEMENT_FLAG table
*/
bool elementInObservation(const Table &antennaFieldTable,
                          unsigned int antennaFieldId,
                          unsigned int element_index)
{
  bool operational=False;
  ROArrayColumn<Bool> elementFlagCol(antennaFieldTable, "ELEMENT_FLAG");
  const Matrix<Bool> elementFlags=elementFlagCol(antennaFieldId);   // ELEMENT_FLAG array for antennaFieldId  

  if(element_index > elementFlags.ncolumn())
  {
    THROW(Exception, "element_index " << element_index << " out of range, ncolums = "
          << elementFlags.ncolumn());
  }
  if(elementFlags(0, element_index)==false && elementFlags(1, element_index)==false)
  {
    operational=true;
  }  
  return operational;
}

/*!
  \brief Sort failedTiles by timestamps of rcus
  \param tiles    vector of tiles to sort
*/
void sortFailedTilesByTime(vector<failedTile> &tiles)
{
  if(tiles.size()<1)
  {
    return;
  }
  for(unsigned int i=0; i<tiles.size(); i++)
  {
    // Sort rcus using bubblesort on timeStamps for all rcus
    for(unsigned int j=1; j<tiles[i].rcus.size()-1; j++)
    {
      if(tiles[i].timeStamps[j] > tiles[i].timeStamps[j+1])
      {
        unsigned int rcuTemp=tiles[i].rcus[j+1];
        ptime timeStampTemp=tiles[i].timeStamps[j+1];
        
        // swap rcu numbers
        tiles[i].rcus[j+1]=tiles[i].rcus[j];
        tiles[i].rcus[j]=rcuTemp;       
        // swap timestamps
        tiles[i].timeStamps[j+1]=tiles[i].timeStamps[j];
        tiles[i].timeStamps[j]=timeStampTemp;
      }
    }
  }
  // Sort failedTiles vector using bubble sort on last rcus timestamp
  for(unsigned int i=0; i<tiles.size()-1; i++)
  {
    unsigned int lastFirst=tiles[i].rcus.size();
    unsigned int lastSecond=tiles[i].rcus.size();
    if(tiles[i].rcus[lastFirst] > tiles[i+1].rcus[lastSecond])
    {
      failedTile tileTemp=tiles[i+1];
      tiles[i+1]=tiles[i];
      tiles[i]=tileTemp;
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
    {
        str.insert(0, num - str.size(), paddingChar);
    }
}
