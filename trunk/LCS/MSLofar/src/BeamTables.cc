//# BeamTables.cc: Class to fill the LOFAR antenna field tables
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
//# $Id$
//#
//# @author Ger van Diepen

#include <lofar_config.h>

#include <MSLofar/BeamTables.h>
#include <MSLofar/MSStation.h>
#include <MSLofar/MSStationColumns.h>
#include <MSLofar/MSAntennaField.h>
#include <MSLofar/MSAntennaFieldColumns.h>
#include <MSLofar/MSElementFailure.h>
#include <Common/LofarLogger.h>
#include <ApplCommon/AntennaSets.h>
#include <ApplCommon/StationInfo.h>

#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ScaColDesc.h>
#include <casa/Arrays/Vector.h>

using namespace LOFAR;
using namespace casa;

void BeamTables::create (Table& ms,
                         const string& antennaSetName,
                         const string& antennaSetFileName,
                         const string& antennaFieldDir,
                         const string& iHBADeltaDir,
                         bool overwrite)
{
  // Open the AntennaSets file.
  AntennaSets antennaSet(antennaSetFileName);
  // If needed, append a trailing slash to the directory names.
  string antFieldPath (antennaFieldDir);
  if (!antFieldPath.empty()  &&  antFieldPath[antFieldPath.size()-1] != '/') {
    antFieldPath.append ("/");
  }
  string hbaDeltaPath (iHBADeltaDir);
  if (!hbaDeltaPath.empty()  &&  hbaDeltaPath[hbaDeltaPath.size()-1] != '/') {
    hbaDeltaPath.append ("/");
  }

  // If no overwrite, check if the subtables already exist.
  if (!overwrite) {
    ASSERTSTR (!ms.keywordSet().isDefined("LOFAR_ANTENNA_FIELD"),
               "LOFAR_ANTENNA_FIELD subtable of MS " << ms.tableName()
               << " already exists");
  }
  // Create the LOFAR antenna field tables.
  SetupNewTable statNew(ms.tableName() + "/LOFAR_STATION",
                        MSStation::requiredTableDesc(), Table::New);
  SetupNewTable antfNew(ms.tableName() + "/LOFAR_ANTENNA_FIELD",
                        MSAntennaField::requiredTableDesc(), Table::New);
  SetupNewTable failNew(ms.tableName() + "/LOFAR_ELEMENT_FAILURE",
                        MSElementFailure::requiredTableDesc(), Table::New);
  MSStation        statTab(statNew, 0, False);
  MSAntennaField   antfTab(antfNew, 0, False);
  MSElementFailure failTab(failNew, 0, False);
  // Define the keywords for them.
  ms.rwKeywordSet().defineTable ("LOFAR_STATION", statTab);
  ms.rwKeywordSet().defineTable ("LOFAR_ANTENNA_FIELD", antfTab);
  ms.rwKeywordSet().defineTable ("LOFAR_ELEMENT_FAILURE", failTab);
  // Create the column objects of those tables.
  MSStationColumns      statCols (statTab);
  MSAntennaFieldColumns antfCols (antfTab);

  // Read the station names from the MS ANTENNA subtable.
  Table antTab (ms.keywordSet().asTable("ANTENNA"));
  ROScalarColumn<String> antNameCol(antTab, "NAME");
  Vector<String> antNames(antNameCol.getColumn());

  // Fill the LOFAR_ANTENNA_FIELD table for each entry in the ANTENNA table.
  // An HBA station can have 2 ears resulting in 2 rows.
  antfTab.addRow (antNames.size());
  int rownr = 0;
  // Keep a list of station names; keep it in order.
  // Also keep a map of unique station names and map them to clockId.
  vector<string> stationNames;
  map<string,int> stationIdMap;
  stationNames.reserve (antNames.size());

  // Now write the info for each entry in the MS ANTENNA table.
  for (uint i=0; i<antNames.size(); ++i) {
    // The MS antenna name consists of antenna field name and type.
    int    stationType  = stationTypeValue (antNames[i]);
    string stationName  = antNames[i].substr (0, 5);
    string antFieldName = antNames[i].substr (5, 4);
    string antFieldType = antFieldName.substr (0, 3);
    stationNames.push_back (stationName); // possibly non-unique names
    // Define id for a new station, otherwise get the id.
    if (stationIdMap.find(stationName) == stationIdMap.end()) {
      int stationId = stationIdMap.size();
      stationIdMap[stationName] = stationId;
    }
    AntennaField antField(antFieldPath + stationName +
                          "-AntennaField.conf");
    // Get the station type from the station name (using StationInfo.h).
    // Use it to get the bitset telling which elements are present for
    // the given antennaSet.

    // HBA stations have to be treated a bit special.
    blitz::Array<double,2> hbaOffsets;   // offsets of HBA dipoles in a tile
    bool done = false;
    int firstHbaOffset = 0;
    if (antFieldType == "HBA") {
      // Get the offsets of HBA dipoles w.r.t. tile center.
      hbaOffsets.reference (getHBADeltas (hbaDeltaPath + stationName +
                                          "-iHBADeltas.conf"));
    }
    if (antFieldName == "HBA") {
      // HBA can be split into HBA0 and HBA1.
      // They have to be written separately.
      if (antFieldName == "HBA") {
        uint nelem0 = antField.nrAnts("HBA0");
        uint nelem1 = antField.nrAnts("HBA1");
        if (nelem0 > 0  &&  nelem1 > 0) {
          // An extra row is needed.
          antfTab.addRow();
          // The HBA offsets can be the same for HBA0 and HBA1 (16 values)
          // or different (32 values).
          if (hbaOffsets.shape()[0] == 16) {
            writeAntField (antfCols, rownr, i, stationName,
                           antField, "HBA0", hbaOffsets, 0);
            writeAntField (antfCols, rownr+1, i, stationName,
                           antField, "HBA1", hbaOffsets, 0);
          } else {
            writeAntField (antfCols, rownr, i, stationName,
                           antField, "HBA0", hbaOffsets, 0);
            writeAntField (antfCols, rownr+1, i, stationName,
                           antField, "HBA1", hbaOffsets, 16);
          }
          // Write all elements.
          writeElements (antfCols, rownr, antField.AntPos("HBA"),
                         antennaSet.positionIndex ("HBA_ZERO", stationType),
                         antField.Centre("HBA"),
                         antField.Centre("HBA0"));
          writeElements (antfCols, rownr+1, antField.AntPos("HBA"),
                         antennaSet.positionIndex ("HBA_ONE", stationType),
                         antField.Centre("HBA"),
                         antField.Centre("HBA1"));
          rownr += 2;
          done = true;
        }
      }
    }
    // In all other cases write a single row.
    if (!done) {
      if (hbaOffsets.shape()[0] == 32  &&  antFieldName == "HBA1") {
        firstHbaOffset = 16;
      }
      string setName(antennaSetName);
      if (antFieldName == "HBA0") {
        setName = "HBA_ZERO";
      } else if (antFieldName == "HBA1") {
        setName = "HBA_ONE";
      }        
      writeAntField (antfCols, rownr, i, stationName,
                     antField, antFieldName, hbaOffsets, firstHbaOffset);
      writeElements (antfCols, rownr, antField.AntPos(antFieldType),
                     antennaSet.positionIndex (setName, stationType),
                     antField.Centre(antFieldType),
                     antField.Centre(antFieldName));
      rownr++;
    }
    LOG_INFO_STR ("Wrote " << rownr << " station field rows");
  }

  // Write the LOFAR_STATION subtable.
  writeStation (statTab, statCols, stationNames, stationIdMap.size());
  // Write the STATION_ID in the ANTENNA subtable.
  writeAntenna (antTab, antNames, stationIdMap);
  // Write the AntennaSet name into the OBSERVATION subtable.
  if (ms.keywordSet().isDefined ("OBSERVATION")) {
    Table obsTable (ms.keywordSet().asTable("OBSERVATION"));
    writeObservation (obsTable, antennaSetName);
  }
}

void BeamTables::writeAntField (MSAntennaFieldColumns& columns, int rownr,
                                int antennaId, const string& stationName,
                                const AntennaField& antField,
                                const string& antFieldName,
                                const blitz::Array<double,2>& hbaOffsets,
                                int sthba)
{
  string type = antFieldName.substr(0,3);
  columns.antennaId().put (rownr, antennaId);
  columns.name().put      (rownr, antFieldName);
  columns.position().put  (rownr, blitz2Casa(antField.Centre(antFieldName)));
  blitz::Array<double,2> rotMat = antField.rotationMatrix(antFieldName);
  ASSERTSTR (rotMat.shape()[0]==3 && rotMat.shape()[1]==3,
             "No valid rotation matrix for station " << stationName);
  columns.coordinateAxes().put (rownr, blitz2Casa
                                (rotMat.transpose(blitz::secondDim,
                                                  blitz::firstDim)));
  if (type == "HBA") {
    columns.tileRotation().put (rownr, 0.);
    blitz::Array<double,2> arr = hbaOffsets(blitz::Range(sthba,sthba+15),
                                            blitz::Range::all());
    columns.tileElementOffset().put (rownr, blitz2Casa(arr));
  }
}

void BeamTables::writeElements (MSAntennaFieldColumns& columns,
                                int rownr,
                                const blitz::Array<double,3>& elemOffsets,
                                const vector<int16>& elemPresent,
                                const blitz::Array<double,1>& stationCenter,
                                const blitz::Array<double,1>& fieldCenter)
{
  double off0 = stationCenter(0) - fieldCenter(0);
  double off1 = stationCenter(1) - fieldCenter(1);
  double off2 = stationCenter(2) - fieldCenter(2);
  int nelem = elemOffsets.shape()[0];
  Matrix<Double> offset(3,nelem);
  Matrix<Bool>   flag(2,nelem, True);
  for (int i=0; i<nelem; ++i) {
    // The element offsets are given as [nelem,npol,xyz].
    // Offsets are the same for the X and Y polarisation.
    offset(0,i) = elemOffsets(i,0,0) + off0;
    offset(1,i) = elemOffsets(i,0,1) + off1;
    offset(2,i) = elemOffsets(i,0,2) + off2;
  }
  // Clear flag for the present dipoles.
  Bool* flagPtr = flag.data();
  int nskip = 0;
  for (vector<int16>::const_iterator iter=elemPresent.begin();
       iter!=elemPresent.end(); ++iter) {
    if (*iter < 0) {
      nskip++;
    } else {
      int index = *iter + nskip;
      ASSERT (index < int16(flag.size()));
      flagPtr[index] = False;
    }
  }
  columns.elementOffset().put (rownr, offset);
  columns.elementFlag().put   (rownr, flag);
}

void BeamTables::writeObservation (Table& obsTable,
                                   const string& antennaSet)
{
  if (! obsTable.tableDesc().isColumn ("LOFAR_ANTENNA_SET")) {
    obsTable.addColumn (ScalarColumnDesc<String> ("LOFAR_ANTENNA_SET"));
  }
  ScalarColumn<String> antCol (obsTable, "LOFAR_ANTENNA_SET");
  for (uInt i=0; i<obsTable.nrow(); ++i) {
    antCol.put (i, antennaSet);
  }
}

void BeamTables::writeStation (MSStation& tab, MSStationColumns& columns,
                               const vector<string>& stationNames,
                               int nstations)
{
  // The vector gives the station names, but can contain duplicates.
  // If a station is written, its clock-id is set.
  tab.addRow (nstations);
  // Use a map to ensure that only unique station names are written.
  map<string,int> clockIdMap;
  int clockId = 1;
  int rownr = 0;
  for (vector<string>::const_iterator iter=stationNames.begin();
       iter!=stationNames.end(); ++iter) {
    // Only write if not written yet.
    if (clockIdMap.find(*iter) == clockIdMap.end()) {
      // All core stations have the same clock-id (0).
      int id = clockId;
      if (iter->substr(0,2) == "CS") {
        id = 0;
      } else {
        clockId++;
      }
      clockIdMap[*iter] = id;
      columns.name().put    (rownr, *iter);
      columns.clockId().put (rownr, id);
      columns.flagRow().put (rownr, False);
      rownr++;
    }
  }
  ASSERT (rownr == nstations);
}

void BeamTables::writeAntenna (Table& antTable,
                               const Vector<String>& antNames,
                               const map<string,int>& stationIdMap)
{
  ASSERT (antTable.nrow() == antNames.size());
  // Add column if not existing yet.
  if (! antTable.tableDesc().isColumn ("LOFAR_STATION_ID")) {
    antTable.addColumn (ScalarColumnDesc<Int> ("LOFAR_STATION_ID"));
  }
  // Write for each antenna the id in the LOFAR_STATION table.
  ScalarColumn<Int> idCol (antTable, "LOFAR_STATION_ID");
  for (uInt i=0; i<antTable.nrow(); ++i) {
    string stationName = antNames[i].substr(0,5);
    idCol.put (i, stationIdMap.find(stationName)->second);
  }
}

blitz::Array<double,2> BeamTables::getHBADeltas (const string& fileName)
{
  ifstream file(fileName.c_str());
  ASSERTSTR(file.good(), "Can not open file " << fileName);
  // The file may have comment lines at the top, starting with '#'
  // These must be skipped
  string line;
  getline (file, line);
  while (line[0] == '#') {
    getline (file, line);
  }
  // The array is stored after the name line which has just been read.
  blitz::Array<double,2> deltas;
  file >> deltas;
  ASSERTSTR (deltas.shape()[1] == 3  &&
             (deltas.shape()[0] == 16  ||  deltas.shape()[0] == 32),
             "Incorrect array shape " << deltas.shape() << " in " <<fileName);
  return deltas;
} 

template<typename T, int NDIM>
Array<T> BeamTables::blitz2Casa (const blitz::Array<T,NDIM>& barray)
{
  // Create the shape (axes must be reversed).
  IPosition shape(NDIM);
  for (uint i=0; i<NDIM; ++i) {
    shape[i] = barray.shape()[NDIM-i-1];
  }
  // Make a copy directly into a casacore Array object.
  Array<T> carr(shape);
  blitz::Array<T,NDIM> barr(carr.data(), barray.shape(),
                            blitz::neverDeleteData);
  barr = barray;
  return carr;
}
