//# FinalMetaDataGatherer.cc:
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
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

// LOFAR
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>    // needed for basename
#include <Common/StringUtil.h>    // needed for split
#include <Common/Exception.h>     // THROW macro for exceptions
#include <Common/CasaLogSink.h>
#include <Common/Exceptions.h>
#include <Common/NewHandler.h>
#include <Interface/Exceptions.h>
#include <Interface/Parset.h>
#include <Interface/Stream.h>
#include <Interface/FinalMetaData.h>
#include <Stream/PortBroker.h>

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
#include <libgen.h>

// boost
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// Casacore
#include <measures/Measures/MEpoch.h>
#include <casa/Quanta/MVTime.h>

// install a new handler to produce backtraces for bad_alloc
LOFAR::NewHandler h(LOFAR::BadAllocException::newHandler);

using namespace LOFAR;
using namespace LOFAR::RTCP;
using namespace LOFAR::OTDB;
using namespace std;
using namespace casa;
using namespace boost::posix_time;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

string logPrefix;

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

vector<OTDBvalue> getHardwareTree(OTDBconnection &conn, const string &timeStart, const string &timeEnd = string())
{
  // Get OTDB info.
  TreeTypeConv TTconv(&conn);     // TreeType converter object
  ClassifConv CTconv(&conn);      // converter
  vector<OTDBvalue> valueList;    // OTDB value list
  vector<OTDBtree> treeList = conn.getTreeList(TTconv.get("hardware"),
                                               CTconv.get("operational"));
  ASSERTSTR(treeList.size(), "No hardware tree found, run tPICtree first");  

  treeIDType treeID = treeList[treeList.size()-1].treeID();
  LOG_DEBUG_STR (logPrefix << "Using tree " << treeID);

  OTDBtree treeInfo = conn.getTreeInfo(treeID);
  LOG_DEBUG_STR(logPrefix << treeInfo);

  LOG_DEBUG_STR(logPrefix << "Constructing a TreeValue object");
  TreeValue tv(&conn, treeID);

  // Get list of all broken hardware from SAS for timestamp
  LOG_DEBUG_STR(logPrefix << "Searching for a Hardware tree");

  if (timeEnd == "")
    valueList = tv.getBrokenHardware(time_from_string(timeStart));
  else
    valueList = tv.getBrokenHardware(time_from_string(timeStart), time_from_string(timeEnd)); 

  return valueList;
}

// Get information about broken tiles from SAS database
void parseBrokenHardware (const vector<OTDBvalue> &hardware, vector<struct FinalMetaData::BrokenRCU> &brokenrcus)
{
  // Don't mess up our counts below
  ASSERT(brokenrcus.empty());

  // Write entry in valuelist with broken hardware
  // A broken antenna element/tile entry must contain .status_state

  for (size_t i = 0; i < hardware.size(); i++) {
    try {
      if (hardware[i].name.find(".status_state") != string::npos) {
        LOG_DEBUG_STR(logPrefix << "Hardware status line '" << hardware[i].name << "'");

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

        if (type != "") {
          struct FinalMetaData::BrokenRCU info;

          info.station = station;
          info.type    = type;
          info.seqnr   = seqnr;
          info.time    = to_simple_string(hardware[i].time);

          brokenrcus.push_back(info);

          LOG_DEBUG_STR(logPrefix << "Found broken " << info.station << " " << info.type << " antenna " << seqnr << " at " << info.time);
        }
      }
    } catch(std::out_of_range &ex) {
      LOG_ERROR_STR(logPrefix << "Error parsing name '" << hardware[i].name << "' time '" << hardware[i].time << "': " << ex.what());
    }
  }

  LOG_INFO_STR(logPrefix << "Found " << brokenrcus.size() << " broken rcus/tiles");
}

char stdoutbuf[1024], stderrbuf[1024];

int main(int argc, char *argv[])
{
  INIT_LOGGER("FinalMetaDataGatherer");

  CasaLogSink::attach();

  try {
    if (argc != 2)
      throw StorageException(str(boost::format("usage: %s obsid") % argv[0]), THROW_ARGS);

    setvbuf(stdout, stdoutbuf, _IOLBF, sizeof stdoutbuf);
    setvbuf(stderr, stderrbuf, _IOLBF, sizeof stderrbuf);

    LOG_DEBUG_STR("Started: " << argv[0] << ' ' << argv[1]);

    int observationID = boost::lexical_cast<int>(argv[1]);

    PortBroker::createInstance(storageBrokerPort(observationID));

    // retrieve the parset
    string resource = getStorageControlDescription(observationID, -1);
    PortBroker::ServerStream controlStream(resource);

    Parset parset(&controlStream);
    logPrefix = str(boost::format("[FinalMetaDataGatherer obs %u] ") % parset.observationID());

    string host;
    if (parset.isDefined("Cobalt.FinalMetaDataGatherer.database.host"))
      host = parset.getString("Cobalt.FinalMetaDataGatherer.database.host");
    else // TODO: remove last OLAP key when BG/P is gone
      host = parset.getString("OLAP.FinalMetaDataGatherer.database.host");
    string db       = "LOFAR_4";
    string user     = "paulus";
    string password = "boskabouter";
    string port     = "5432";

    // TODO: use actual run times
    string timeStart = parset.getString("Observation.startTime");
    string timeEnd   = parset.getString("Observation.stopTime");

    FinalMetaData finalMetaData;

    LOG_INFO_STR (logPrefix << "Connecting to SAS database " << db << " on " << host);

    OTDBconnection conn(user, password, db, host, port); 
    bool connected = conn.connect();
    ASSERTSTR(connected, "FinalMetaDataGatherer: Connnection failed");

    LOG_INFO_STR (logPrefix << "Retrieving hardware broken at observation start");
    vector<OTDBvalue> hardwareBrokenAtBegin = getHardwareTree(conn, timeStart);
    parseBrokenHardware(hardwareBrokenAtBegin, finalMetaData.brokenRCUsAtBegin);

    LOG_INFO_STR (logPrefix << "Retrieving hardware broken during the observation");
    vector<OTDBvalue> hardwareBrokenDuring  = getHardwareTree(conn, timeStart, timeEnd);
    parseBrokenHardware(hardwareBrokenDuring, finalMetaData.brokenRCUsDuring);

    LOG_INFO_STR (logPrefix << "Uploading all information");
    finalMetaData.write(controlStream);

  } catch (Exception &ex) {
    LOG_FATAL_STR("[FinalMetaDataGatherer obs unknown] Caught Exception: " << ex);
    return 1;
  }

  LOG_INFO_STR("[FinalMetaDataGatherer obs unknown] Program end");
  return 0;
}

