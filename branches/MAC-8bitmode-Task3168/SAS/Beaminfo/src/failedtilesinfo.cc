//# failedtilesinfo.cc: extract failed tiles info from the SAS database
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
//# $Id: failedtilesinfo.cc 18832 2011-09-19 17:22:32Z duscha $
//#
//# @author Sven Duscha

#include <lofar_config.h>

// LOFAR
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>    // needed for basename
#include <Common/Exception.h>     // THROW macro for exceptions
#include <ApplCommon/LofarDirs.h>

// SAS
#include <OTDB/OTDBconstants.h>
#include <OTDB/OTDBconnection.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/TreeValue.h>
#include <OTDB/ClassifConv.h>
#include <OTDB/Converter.h>
#include <OTDB/TreeTypeConv.h>

// STL
#include <iostream>

// Casacore
#include <measures/Measures/MEpoch.h>
#include <casa/Quanta/MVTime.h>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::OTDB;
using namespace casa;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

// Time converter helper functions
string fromCasaTime (const MEpoch& epoch, double addDays);
MVEpoch toCasaTime(const string &time);
bool checkTime(const MVEpoch &starttime, const MVEpoch &endtime);

void getFailedTilesInfo(OTDBconnection &conn, 
                        const string &filename,
                        const MVEpoch &timeStart,
                        const MVEpoch &timeEnd=0);
string stripRCUString(const string &brokenHardware);

//----------------------------------------------------------------------------------
void usage(char *programname)
{
  cout << "Usage: " << programname << " <options>" << endl;
  cout << "-d             run in debug mode" << endl;
  cout << "-p <filename>  read parset (instead of default: failedtilesinfo.parset)" << endl;
  cout << "-s <time>      start time of observation in MS like 3-Mar-2011/13:54:23"<< endl;
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
  vector<MEpoch> failingTimes;

  string parsetName="failedtilesinfo.parset";   // parset location (default)
  string starttimeString, endtimeString;    // strings to get start and end time
  MVEpoch startTime, endTime;               // starttime and endtime of observation

  //---------------------------------------------
  // Init logger
  string progName = LOFAR::basename(argv[0]);
  INIT_LOGGER(progName);

  // Parse command line arguments TODO!
  while(opt != -1) 
  {
    opt = getopt( argc, argv, "dp:s:e:vh");
    switch(opt) 
    { 
      case 'd':
        debug=true;
        break;
      case 'p':         // location of parset file
        parsetName=optarg;
        break;
      case 's':         // start time
        starttimeString=optarg;
      case 'e':         // end time
        endtimeString=optarg;
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
  
  // Parse parset entries
  try
  {
    if(verbose)
    {
      LOG_INFO_STR("Reading parset: " << parsetName);
    }

    //---------------------------------------------------------------------
    ParameterSet parset(parsetName);
    //string host        = parset.getString("host", "sas.control.lofar.eu");  // production
    string host        = parset.getString("host", "RS005.astron.nl");         // DEBUG
    string db          = parset.getString("db", "TESTLOFAR_3");
    string user        = parset.getString("user", "paulus");
    string password    = parset.getString("password", "boskabouter");
    string port        = parset.getString("port", "5432");
    // Locations to save SAS hardware strings of broken and failed tiles to
    string failedfilename = parset.getString("failedTilesFile", LOFAR_SHARE_LOCATION "/failedTiles.txt");
    string brokenfilename = parset.getString("brokenTilesFile", LOFAR_SHARE_LOCATION "/brokenTiles.txt");

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

    LOG_INFO_STR("Getting SAS antenna health information");
    OTDBconnection conn(user, password, db, host, port); 
    LOG_INFO("Trying to connect to the database");
    ASSERTSTR(conn.connect(), "Connnection failed");
    LOG_INFO_STR("Connection succesful: " << conn);

    // Get broken hardware strings from SAS
    getFailedTilesInfo(conn, brokenfilename, startTime);
    getFailedTilesInfo(conn, failedfilename, startTime, endTime);
  }
  catch (Exception& x)
  {
    LOG_FATAL_STR("Unexpected exception: " << x);
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

// Convert a time string time YYYY-Mon-DD TT:MM:SS.ss to a CASA MVEpoch
MVEpoch toCasaTime(const string &time)
{
  // e.g. 2011-Mar-19 21:17:06.514000
  Double casaTime;                  // casa MVEpoch time to be returned
  Quantity result(casaTime, "s");   // set quantity unit to seconds

  ASSERT(!time.empty());
  MVTime::read(result, time);

  return result;
}

bool checkTime(const MVEpoch &starttimeCasa, const MVEpoch &endtimeCasa)
{
  return(starttimeCasa.get() < endtimeCasa.get());
}

// Get information about broken tiles from SAS database and store it in 
// an ASCII text file
//
void getFailedTilesInfo(OTDBconnection &conn, 
                        const string &filename,
                        const MVEpoch &timeStart,
                        const MVEpoch &timeEnd)
{

  ASSERT(!filename.empty());

  TreeTypeConv TTconv(&conn);     // TreeType converter object
  ClassifConv CTconv(&conn);      // converter I don't know
  vector<OTDBvalue> valueList;    // OTDB value list for the previous month
 
  fstream outfile;
  outfile.open(filename.c_str(), ios::out);   // this shows the correct behaviour of overwriting the file

  // Get list of all broken hardware from SAS for timestamp
  LOG_INFO("Searching for a Hardware tree");
  vector<OTDBtree>    treeList = conn.getTreeList(TTconv.get("hardware"), CTconv.get("operational"));
  //  showTreeList(treeList);
  ASSERTSTR(treeList.size(),"No hardware tree found, run tPICtree first");  
  treeIDType  treeID = treeList[treeList.size()-1].treeID();
  LOG_INFO_STR ("Using tree " << treeID << " for the tests");
  OTDBtree    treeInfo = conn.getTreeInfo(treeID);
  LOG_INFO_STR(treeInfo);
  LOG_INFO("Trying to construct a TreeValue object");
  TreeValue   tv(&conn, treeID);

  if(timeEnd==0)    // getting tiles broken at beginning
  {
    valueList = tv.getBrokenHardware(time_from_string(fromCasaTime(timeStart)));
  }
  else              // getting tiles failed during observation
  {
    LOG_INFO_STR("Getting failed hardware from " << MVTime(timeStart.get()).getTime().ISODate() 
                 << " to " << MVTime(timeEnd.get()).getTime().ISODate());
    valueList = tv.getFailedHardware(time_from_string(fromCasaTime(timeStart)), 
                                     time_from_string(fromCasaTime(timeEnd)));
  }

  if(valueList.empty())
  {
    LOG_INFO_STR("No failed hardware found.");
  }
  else
  {
    // Now write entry in valuelist with broken hardware to file
    for(unsigned int i=0; i<valueList.size(); i++)
    {
      if(valueList[i].name.find("RCU")!=string::npos)   // Only write lines that contain RCU
      {
        outfile << stripRCUString(valueList[i].name) << "\t" << valueList[i].time << endl;
      }  
    }  
  }
  outfile.close();
}

// Strip the RCU string in broken hardware of unnecessary information
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
