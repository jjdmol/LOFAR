//# FailedTileInfo.cc: Class to write failed tile info into the MS
//# Copyright (C) 2012
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
//# $Id: FailedTileInfo.cc 19920 2012-01-24 14:57:23Z diepen $

#include <lofar_config.h>
#include <Beaminfo/FailedTileInfo.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <measures/TableMeasures/ScalarQuantColumn.h>
#include <measures/TableMeasures/ArrayQuantColumn.h>
#include <casa/Quanta/MVEpoch.h>
#include <casa/Quanta/MVTime.h>

#include <Common/StringUtil.h>
#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>
#include <Common/lofar_iostream.h>

using namespace casa;

namespace LOFAR {

  FailedTileInfo::FailedTileInfo (const string& station, const string& time,
                                  const MVEpoch& epoch, int rcu)
    : itsStation(station), itsTime(time), itsEpoch(epoch), itsRcu(rcu)
  {}

  // Read the broken hardware file giving a map of antenna name to a vector
  // of pairs of RCUnr and time.
  map<string, FailedTileInfo::VectorFailed> FailedTileInfo::readFile
  (const string& fileName, double startTime, double endTime)
  {
    map<string, FailedTileInfo::VectorFailed> brokenHardware;
    fstream infile (fileName.c_str());
    if (! infile) {
      LOG_WARN_STR ("Unable to open file " << fileName);
      return brokenHardware;
    }
    int linenumber=0;
    int rcu;
    string name, date, time;
    Quantity q;
    while (infile) {
      linenumber++;
      bool valid = false;
      infile >> name >> date >> time;
      if (!(name.empty() || date.empty() || time.empty())) {
        if (MVTime::read (q, date+'/'+time)) {
          // Split name into station name and RCU name.
          vector<string> parts = StringUtil::split(name, '.');
          if (parts.size() > 1  &&  parts[1].substr(0,3) == "RCU") {
            valid = true;
            // Only use the entry if the time is within the limits.
            MVEpoch epoch(q);
            if (epoch.get() >= startTime  &&  epoch.get() < endTime) {
              istringstream istr(parts[1].substr(3));
              istr >> rcu;
              // Add the the vector for this station.
              // Note that the vector is created automatically if not existing.
              brokenHardware[parts[0]].push_back
                (FailedTileInfo(parts[0], date+'/'+time.substr(0,8),
                                epoch, rcu));
            }
          }
        }
      }
      if (!valid) {
        LOG_WARN_STR("line " << linenumber << " in file " << fileName
                     << " is invalid. Skipping...");
      }
    }
    return brokenHardware;
  }

  // Convert the info per station name to info per antennaId.
  vector<FailedTileInfo::VectorFailed> FailedTileInfo::antennaConvert
  (const Table& ms, const map<string,FailedTileInfo::VectorFailed>& broken)
  {
    // Open ANTENNA table columns.
    Table antTab (ms.keywordSet().asTable("ANTENNA"));
    ROScalarColumn<String> nameCol(antTab, "NAME");
    // Size the vector which creates empty elements.
    vector<FailedTileInfo::VectorFailed> brokenAnt (antTab.nrow());
    for (uInt row=0; row<antTab.nrow(); ++row) {
      // Get first 5 characters of the station name.
      string name = nameCol(row).substr(0,5);
      map<string, FailedTileInfo::VectorFailed>::const_iterator iter =
        broken.find(name);
      if (iter != broken.end()) {
        // Insert the info into the vector at the given row (=antId).
        brokenAnt[row] = iter->second;
      }
    }
    return brokenAnt;
  }

  // Write the failed tile info.
  void FailedTileInfo::writeFailed
  (Table& ms,
   const vector<FailedTileInfo::VectorFailed>& brokenBefore,
   const vector<FailedTileInfo::VectorFailed>& brokenDuring)
  {
    ASSERT (brokenBefore.size() == brokenDuring.size());
    // Open ANTENNA_FIELD table.
    Table antFieldTab (ms.keywordSet().asTable("LOFAR_ANTENNA_FIELD"));
    ROScalarColumn<Int> antIdCol(antFieldTab, "ANTENNA_ID");
    ArrayColumn<Bool>   flagCol (antFieldTab, "ELEMENT_FLAG");
    vector<Double> times;
    vector<Int>    rcus;
    vector<Int>    antFldIds;
    vector<string> stations;
    vector<string> timestrs;
    // Loop through all rows in the AntennaField table.
    for (uInt row=0; row<antFieldTab.nrow(); ++row) {
      uInt antId = antIdCol(row);
      ASSERT (antId < brokenBefore.size());
      // Process if the matching entry in the antenna info vector is not empty.
      if (!brokenBefore[antId].empty()  ||  !brokenDuring[antId].empty()) {
        Matrix<Bool> flags = flagCol(row);
        bool changed = false;
        // Loop through all rcus in the vector.
        for (FailedTileInfo::VectorFailed::const_iterator
               iter = brokenBefore[antId].begin();
             iter != brokenBefore[antId].end(); ++iter) {
          uInt elem = iter->itsRcu / 2;
          ASSERT (elem < flags.ncolumn());
          if (!flags(0,elem)) {
            // Only set and report if not set yet.
            flags(0,elem) = flags(1,elem) = True;
            changed = true;
            LOG_INFO_STR ("Flagged element " << elem
                          << " for antenna field " << row
                          << " (RCU " << iter->itsRcu
                          << " on " << iter->itsStation
                          << " failed at " << iter->itsTime << ')');
          }
        }
        if (changed) {
          flagCol.put (row, flags);
        }
        // Now loop through all entries for the tiles broken during the obs.
        // Add to a vector if its flag is not set yet.
        for (FailedTileInfo::VectorFailed::const_iterator
               iter = brokenDuring[antId].begin();
             iter != brokenDuring[antId].end(); ++iter) {
          int elem = iter->itsRcu / 2;
          if (!flags(0,elem)) {
            rcus.push_back (iter->itsRcu);
            times.push_back (iter->itsEpoch.get());
            antFldIds.push_back (row);
            stations.push_back (iter->itsStation);
            timestrs.push_back (iter->itsTime);
          }
        }
        // Now write the elements that failed during the observation.
        // First sort the vectors on time, antfldid, index.
        if (! times.empty()) {
          Sort sort;
          sort.sortKey (&(times[0]), TpDouble);
          sort.sortKey (&(antFldIds[0]), TpInt);
          sort.sortKey (&(rcus[0]), TpInt);
          Vector<uInt> index;
          sort.sort (index, times.size());
          // Now write the sorted data into the ELEMENT_FAILURE table.
          Table failedTab (ms.keywordSet().asTable("LOFAR_ELEMENT_FAILURE"));
          ScalarColumn<Int>   antFldIdCol (failedTab, "ANTENNA_FIELD_ID");
          ScalarColumn<Int>     elemIdCol (failedTab, "ELEMENT_INDEX");
          ScalarQuantColumn<Double> timeCol (failedTab, "TIME");
          failedTab.addRow (index.size());
          for (uInt i=0; i<index.size(); ++i) {
            int inx = index[i];
            antFldIdCol.put (i, antFldIds[inx]);
            int elem = rcus[inx] / 2;
            elemIdCol.put (i, elem);
            timeCol.put (i, Quantity(times[inx], "d"));  // MVEpoch gives days
            LOG_INFO_STR ("Added failed element " << elem
                          << " for antenna field " << antFldIds[inx]
                          << " (RCU " << rcus[inx]
                          << " on " << stations[inx]
                          << " failed at " << timestrs[inx] << ')');
          }
        }
      }
    }
  }

  void FailedTileInfo::failedTiles2MS
  (const string& msName, const string& beforeName, const string& duringName)
  {
    ASSERT (!(msName.empty() || beforeName.empty() || duringName.empty()));
    // Open the measurementset table.
    Table ms(msName, Table::Update);
    // Get the start and end time from the Observation subtable.
    Table obsTab (ms.keywordSet().asTable ("OBSERVATION"));
    ROArrayQuantColumn<Double> timeCol(obsTab, "TIME_RANGE");
    ASSERT (obsTab.nrow() > 0);
    Vector<Quantity> times = timeCol(0);
    ASSERT (times.size() > 1);
    MVEpoch startTime (times[0]);
    MVEpoch   endTime (times[1]);
    // Read the files containing the elements broken before and during the obs.
    // Turn the results into a vector per station.
    vector<FailedTileInfo::VectorFailed> before
      (antennaConvert(ms, readFile(beforeName, 0., startTime.get())));
    vector<FailedTileInfo::VectorFailed> during
      (antennaConvert(ms, readFile(duringName, startTime.get(),
                                   endTime.get())));
    // Write the broken info into the MS.
    writeFailed (ms, before, during);
  }

} // end namespace
