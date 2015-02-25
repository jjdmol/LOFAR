//# Copyright (C) 2012-2014
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
//# $Id: $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

// LOFAR
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>    // needed for split
#include <Common/Exception.h>     // THROW macro for exceptions
#include <Common/Exceptions.h>
#include <ApplCommon/StationInfo.h>

// SAS
#include <OTDB/OTDBconstants.h>
#include <OTDB/OTDBconnection.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/TreeValue.h>
#include <OTDB/ClassifConv.h>
#include <OTDB/Converter.h>
#include <OTDB/TreeTypeConv.h>

// STL / C
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdio>
#include <unistd.h>

// boost
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// pqxx
#include <pqxx/except>

// Casacore
#include <measures/Measures/MEpoch.h>
#include <casa/Quanta/MVTime.h>

#include "UpdateBrokenAntennaInfoToPVSS.h"

using namespace LOFAR;
// using namespace LOFAR::PVSS;
using namespace LOFAR::OTDB;
using namespace std;
using namespace casa;
using namespace boost::posix_time;
using boost::format;

namespace LOFAR {
  namespace PVSS {

    UpdateBrokenAntennaInfoToPVSS::UpdateBrokenAntennaInfoToPVSS()
    {
      LOG_TRACE_OBJ_STR ("creation");
    }

    UpdateBrokenAntennaInfoToPVSS::~UpdateBrokenAntennaInfoToPVSS()
    {
      LOG_TRACE_OBJ_STR ("destruction");
    }

    // Convert a casacore time string time YYYY-Mon-DD TT:MM:SS.ss to an MVEpoch
    MVEpoch toCasaTime (const string& time)
    {
      ASSERT(!time.empty());

      // e.g. 2011-Mar-19 21:17:06.514000
      Double casaTime = 0.0;            // casacore MVEpoch time to be returned
      Quantity result(casaTime, "s");   // set quantity unit to seconds
      MVTime::read(result, time);
      return result;
    }

    string fromCasaTime (const MVEpoch& epoch)
    {
      MVTime t (epoch.get());
      return t.getTime().ISODate();
    }

    vector<OTDBvalue> UpdateBrokenAntennaInfoToPVSS::getHardwareTree(OTDBconnection &conn, const string &timeNow)
    {
      // Get OTDB info.
      TreeTypeConv TTconv(&conn);     // TreeType converter object
      ClassifConv CTconv(&conn);      // converter
      vector<OTDBvalue> valueList;    // OTDB value list
      vector<OTDBtree> treeList = conn.getTreeList(TTconv.get("hardware"),
                                                   CTconv.get("operational"));
      ASSERTSTR(treeList.size(), "No hardware tree found, run tPICtree first");  

      treeIDType treeID = treeList[treeList.size()-1].treeID();
      LOG_DEBUG_STR ("Using tree " << treeID);

      OTDBtree treeInfo = conn.getTreeInfo(treeID);
      LOG_DEBUG_STR(treeInfo);

      LOG_DEBUG_STR("Constructing a TreeValue object");
      TreeValue tv(&conn, treeID);

      // Get list of all broken hardware from SAS for timestamp
      LOG_DEBUG_STR("Searching for a Hardware tree");
      LOG_DEBUG_STR("Using Time: " << from_iso_string(timeNow));
      valueList = tv.getBrokenHardware(from_iso_string(timeNow));

      return valueList;
    }


    // Get information about broken tiles from SAS database
    void UpdateBrokenAntennaInfoToPVSS::parseBrokenHardware (const vector<OTDBvalue> &hardware)
    {
      // Write entry in valuelist with broken hardware
      // A broken antenna element/tile entry must contain .status_state

      for (size_t i = 0; i < hardware.size(); i++) {
        try {
          if (hardware[i].name.find(".status_state") != string::npos) {
            LOG_DEBUG_STR("Hardware status line '" << hardware[i].name << "'");

            vector<string> parts = StringUtil::split (hardware[i].name, '.');

            // parts[3] is station name (f.e. CS001)
            string station = parts.size() > 3 ? parts[3] : "";

            // parts[4] is tile name/number (f.e. HBA1 or LBA3)
            string tile    = parts.size() > 4 ? parts[4] : "";

            // parts[7] is RCU name/number (f.e. RCU20)
            string rcu     = parts.size() > 7 ? parts[7] : "";

            string tiletype = tile.substr(0,3);
            string rcutype  = rcu.substr(0,3);

            string type = "";
            int seqnr = 0;

            if (tiletype == "LBA" || tiletype == "HBA") {
              // broken tile
              type = tiletype;
              seqnr = boost::lexical_cast<int>(tile.substr(3));
            } else if (rcutype == "RCU") {
              // broken rcu
              type = rcutype;
              seqnr = boost::lexical_cast<int>(rcu.substr(3));
            }

//             if (type != "") {
//               struct FinalMetaData::BrokenRCU info;

//               info.station = station;
//               info.type    = type;
//               info.seqnr   = seqnr;
//               info.time    = to_simple_string(hardware[i].time);
//               LOG_DEBUG_STR("Found broken " << info.station << " " << info.type << " antenna " << seqnr << " at " << info.time);
//            }
          }
        } catch(std::out_of_range &ex) {
          LOG_ERROR_STR("Error parsing name '" << hardware[i].name << "' time '" << hardware[i].time << "': " << ex.what());
        }
      }

      //      LOG_INFO_STR("Found " << brokenrcus.size() << " broken rcus/tiles");
    }

    
    vector<OTDBvalue> UpdateBrokenAntennaInfoToPVSS::getBrokenAntennaInfo()
    {
      string host = "sasdb";
      string db   = "LOFAR_4";
      string user     = "paulus";
      string password = "boskabouter";
      string port = "5432";

      LOG_INFO_STR("Connecting to SAS database " << db << " on " << host);

      OTDBconnection conn(user, password, db, host, port);

//      try {
        if (!conn.connect())
          THROW(Exception, "OTDBconnection failed to " << db << " on " << host);

        LOG_INFO_STR("Retrieving broken hardware");
 	string now = to_iso_string(second_clock::local_time());
        vector<OTDBvalue> hardwareBrokenAtBegin = getHardwareTree(conn, now);
        parseBrokenHardware(hardwareBrokenAtBegin);
        conn.disconnect();
//      } catch (pqxx::pqxx_exception &ex) {
//        THROW(Exception, "PostGreSQL error: " << ex.base().what());
//      }

      return hardwareBrokenAtBegin;
    }
  }
}

using namespace LOFAR::PVSS;

int main(int argc, char *argv[])
{
  INIT_LOGGER("UpdateBrokenAntennaInfoToPVSS");

  if (argc != 1) {
    cout << str(format("usage: %s ") % argv[0]) << endl;
    cout << endl;
    return 1;
  }

  UpdateBrokenAntennaInfoToPVSS* mainObject=new UpdateBrokenAntennaInfoToPVSS();

  // TODO: make this safe
  string thisUser = getenv("USER");

  // Substring to remove from returned hardware status strings
  string removePart = ".status_state";

  vector<OTDBvalue> itsBrokenAntennaInfo;
  itsBrokenAntennaInfo = mainObject->getBrokenAntennaInfo();

  // Extract setObjectState commands from returned list of broken antenna info
  for (vector<OTDBvalue>::iterator it = itsBrokenAntennaInfo.begin() ; it != itsBrokenAntennaInfo.end(); ++it)
  {
    OTDBvalue tmp = *it;
    // Find hardware object name and remove trailing ".status_state"
    string OTDBname = tmp.name;
    string::size_type i = OTDBname.find(removePart);
    if (i != string::npos)
       OTDBname.erase(i, removePart.length());
    // Convert OTDB name to PVSS name
    string PVSSname = SAS2PVSSname(OTDBname);
    // setObjectState requires different status numbers (0,1,2,3,4,5) than OTDB gives (0, 10, 20, .. , 50)
    int OTDBstatus = strToInt(tmp.value);
    int setObjectStatestatus = OTDBstatus/10;
    LOG_DEBUG_STR(OTDBname << ":" << OTDBstatus << "  " << PVSSname << ":" << setObjectStatestatus);
    // Construct setObjectState command to execute
    string PVSSUpdateCmd = "/opt/lofar/sbin/setObjectState " + thisUser+ ' ' + PVSSname + ' ' + toString(setObjectStatestatus);
    cout << "Will now execute: " << PVSSUpdateCmd << endl;
    // Run command, check return code 
    // TODO: try..catch or something
    int result = system("ls -1 >& /dev/null");
    // int result = system(PVSSUpdateCmd.c_str());
    if (result != 0) {
      cout << "Problem executing command; return code: " << result << endl;
    }
  }

  delete mainObject;
  return 0;

}


