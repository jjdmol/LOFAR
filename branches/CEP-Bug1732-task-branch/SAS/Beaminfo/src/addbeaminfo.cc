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
map<string, ptime> getBrokenHardwareMap(OTDBconnection &conn,
                                        const MVEpoch &timestamp=0);
RCUmap getBrokenRCUs(const map<string, ptime> &brokenHardware);
void extractRCUs(RCUmap &brokenRCUs, 
                 const map<string, ptime> &brokenHardware, 
                 const string &station);                      
map<string, ptime> getFailedHardware( const map<string, ptime> &brokenBegin, 
                                      const map<string, ptime> &brokenEnd);
vector<failedTile> getFailedTilesAntennaId(MeasurementSet &ms,
                                           const map<string, ptime> &failedTilesSAS);
vector<failedTile> getBrokenTiles(MeasurementSet &ms, const RCUmap &brokenRCus);
vector<failedTile> getFailedTiles(MeasurementSet &ms, 
                                  const RCUmap &failedRCUs, 
                                  const map<string, ptime> &failedHardware);
bool failedElementInObservation(const Table &antennaFieldTable,
                                const Table &failedElementsTable,
                                unsigned int antennaId,
                                unsigned int element_index);                                
void padTo(std::string &str, const size_t num, const char paddingChar);

// MS Table writing functions TODO
void updateAntennaFieldTable( MeasurementSet &ms, const vector<failedTile> &brokenTiles);
void updateElementFlags(Table &table, unsigned int antennaId, unsigned int elementIndex);
void updateElementFlags(Table &table, 
                        unsigned int antennaId, 
                        const vector<unsigned int> &elementIndex);

// File I/O
void writeBrokenHardwareFile(const string &filename, const vector<string> &brokenHardware, bool strip=true);
void writeBrokenHardwareFile(const string &filename, const map<string, ptime> &brokenHardware, bool strip=true);
map<string, ptime> readBrokenHardwareFile(const string &filename);
string stripRCUString(const string &brokenHardware);
void writeFailedElementsFile( const string &filename,
                              const map<string, ptime> &failedHardware);
map<string, ptime> readFailedElementsFile(const string &filename);


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
      LOG_INFO_STR("reading brokenHardware after from file:" << brokenAfterfilename);      
      brokenHardwareAfter=readBrokenHardwareFile(brokenAfterfilename);

//      showMap(brokenHardware);        // DEBUG
//      showMap(brokenHardwareAfter);   // DEBUG
    
      failedHardware=getFailedHardware(brokenHardware, brokenHardwareAfter);
      if(debug)
        showMap(failedHardware);
        
      writeFailedElementsFile(failedfilename, failedHardware);  // write failed tiles to disk

      cout << "Read failed tiles from file:" << endl;  // DEBUG
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
      updateAntennaFieldTable(ms, brokenTiles);
      
      //----------------------------------------------------------------------------
      // Get Tiles that failed during observation
      //
      failedHardware=getFailedHardware(brokenHardware, brokenHardwareAfter);
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
  String copyTime=time;   // make a temporary copy
  Quantity result(casaTime, "s");   // set quantity unit to seconds
  
  if(time.empty())
  {
    THROW(Exception, "toCasaTime() string time is empty"); 
  }
  else
  {
    copyTime.gsub(" ", "/");      // replace spaces with slashes for casa conversion   
    MVTime::read(result, copyTime);
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
        /*
        for(unsigned int rcuIndex=0; rcuIndex < brokenTiles[i].rcus.size(); rcuIndex++)
        {
          unsigned int rcuNum=brokenTiles[i].rcus[rcuIndex];
          updateElementFlags(antennaFieldTable, antennaId, rcuNum);    
        }
        */
        
        updateElementFlags(antennaFieldTable, antennaId, brokenTiles[i].rcus);
        break;
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
void updateElementFlags(Table &table, unsigned int antennaId, const vector<unsigned int> &rcus)
{
  ArrayColumn<Bool> elementFlagCol(table, "ELEMENT_FLAG");  
  Matrix<Bool> elementFlags=elementFlagCol(antennaId);

  unsigned int nrows=elementFlagCol.nrow();

  if(antennaId > nrows-1)
  {
    THROW(Exception, "updateElementFlags() antennaId " << antennaId << " out of range");
  }
  if(rcus.size()==0)    // if there are no rcus to be updated for this antennaId....
  {
    return;             // ... just return
  }
  
  //uInt ncolumn=elementFlags.ncolumn();
//  cout << "updateELementFlags() shape: " << shape << endl;              // DEBUG
//  cout << "updateELementFlags() antennaId = " << antennaId << endl;     // DEBUG

  unsigned int nrcus=rcus.size();
  for(unsigned int i=0; i<nrcus; i++)
  {
    unsigned int elementIndex=rcus[i] / 2;
    elementFlags(0, elementIndex)=true;          // Update ELEMENT_FLAGS at elementIndex
    elementFlags(1, elementIndex)=true;          // Update ELEMENT_FLAGS at elementIndex
  }

  elementFlagCol.put(antennaId, elementFlags);
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
    while(infile.good())
    {
      string datetime="";
      infile >> name >> date >> time;
      datetime=date.append(" ").append(time);     // YYYY-MM-DD HH-MM-SS.ssss        
      failedElements.insert(std::make_pair(name, time_from_string(datetime)));
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
  ROScalarColumn<String> nameCol(antennaTable, "NAME");   // get Name col

  for(uInt antennaId=0; antennaId<nrows; antennaId++)     // Loop over ANTENNA table
  {
    String station=nameCol(antennaId).substr(0, 5);       // get station name part of nameCol
    
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
                                failedTiles[i].rcus[j] / 2,
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
      << failedTiles[i].antennaId);

      // Loop over multiple element_flags per antennaId
      for(unsigned int j=0; j<failedTiles[i].rcus.size(); j++)
      {
        unsigned int antennaId=failedTiles[i].antennaId;
        unsigned int elementIndex=failedTiles[i].rcus[j] / 2;
      
        // First check if the ELEMENT_FLAG is true, i.e. only if the RCU is used
        // in this observation mode
        if(failedElementInObservation(antennaFieldTable, failedElementsTable,
                                      antennaId, elementIndex) == true)
        {
            return;   // we don't add it to the LOFAR_FAILED_ELEMENT table
        }
        else
        {
            MVEpoch timestamp=toCasaTime(failedTiles[i].timeStamps[j]).getDay() + 
                              toCasaTime(failedTiles[i].timeStamps[j]).getDayFraction();
    
            doAddFailedAntennaTile( failedElementsTable, 
                                    antennaId,      
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
 
  // Check if that failed tile isn't already present in the table
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
  \brief Particular element takes part in observation
  \param antennaFieldTable  LOFAR_ANTENNA_FIELD table of MS
  \param antennaId          antenna Id within LOFAR_ANTENNA_FIELD table
  \param element_index      element_index into ELEMENT_FLAG table
*/
bool failedElementInObservation(const Table &antennaFieldTable,
                                const Table &failedElementsTable,
                                unsigned int antennaId,
                                unsigned int element_index)
{
  bool operational=False;
  
  ROScalarColumn<Int> antennaFieldIdCol(failedElementsTable, "ANTENNA_FIELD_ID");
  ROArrayColumn<Bool> elementFlagCol(antennaFieldTable, "ELEMENT_FLAG");

  Matrix<Bool> elementFlags=elementFlagCol(antennaId);   // ELEMENT_FLAG array for antennaId  
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
