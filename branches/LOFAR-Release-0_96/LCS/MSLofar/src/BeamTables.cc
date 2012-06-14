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
#include <Common/StreamUtil.h>

#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ScaColDesc.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/MatrixMath.h>

using namespace LOFAR;
using namespace casa;

void BeamTables::create (Table& ms, bool overwrite)
{
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
}

void BeamTables::fill (Table& ms,
                       const string& antennaSetName,
                       const string& antennaSetFileName,
                       const string& antennaFieldDir,
                       const string& iHBADeltaDir,
                       bool mustExist)
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

  // Open the subtables.
  MSStation      statTab(ms.keywordSet().asTable("LOFAR_STATION"));
  MSAntennaField antfTab(ms.keywordSet().asTable("LOFAR_ANTENNA_FIELD"));
  ASSERTSTR (statTab.nrow() == 0  &&  antfTab.nrow() == 0,
             "LOFAR_STATION and LOFAR_ANTENNA_FIELD subtables should be empty");
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
    // For test purposes we do not assume that the station name has 5 chars.
    int    stationType  = stationTypeValue (antNames[i]);
    string stationName  = antNames[i].substr (0, 5);
    string antFieldName;
    if (antNames[i].size() > 5) {
      antFieldName = antNames[i].substr (5, 4);
    }
    string antFieldType = antFieldName.substr (0, 3);
    stationNames.push_back (stationName); // possibly non-unique names
    // Define id for a new station, otherwise get the id.
    if (stationIdMap.find(stationName) == stationIdMap.end()) {
      int stationId = stationIdMap.size();
      stationIdMap[stationName] = stationId;
    }
    AntField antField(antFieldPath + stationName + "-AntennaField.conf",
                      mustExist);
    // Get the station type from the station name (using StationInfo.h).
    // Use it to get the bitset telling which elements are present for
    // the given antennaSet.

    // HBA stations have to be treated a bit special.
    AntField::AFArray hbaOffsets;   // offsets of HBA dipoles in a tile
    bool done = false;
    int firstHbaOffset = 0;
    if (antFieldType == "HBA") {
      // Get the offsets of HBA dipoles w.r.t. tile center.
      getHBADeltas (hbaDeltaPath + stationName + "-iHBADeltas.conf",
                    hbaOffsets, mustExist);
    } else if (antFieldType != "LBA") {
      // In RTCP test programs arbitrary station names are used, so test it.
      if (mustExist) {
        THROW (LOFAR::Exception,
               "AntennaFieldType of " << antNames[i] << " is LBA nor HBA");
      }
      // Set to a valid type.
      antFieldType = "LBA";
      antFieldName = "LBA";
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
          if (AntField::getShape(hbaOffsets)[0] == 16) {
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
                         false,
                         antField.Centre("HBA"),
                         antField.Centre("HBA0"));
          writeElements (antfCols, rownr+1, antField.AntPos("HBA"),
                         antennaSet.positionIndex ("HBA_ONE", stationType),
                         true,
                         antField.Centre("HBA"),
                         antField.Centre("HBA1"));
          rownr += 2;
          done = true;
        }
      }
    }
    // In all other cases write a single row.
    if (!done) {
      string setName(antennaSetName);
      if (antFieldName == "HBA0") {
        setName = "HBA_ZERO";
      } else if (antFieldName == "HBA1") {
        setName = "HBA_ONE";
        if (AntField::getShape(hbaOffsets)[0] == 32) {
          firstHbaOffset = 16;
        }
      }        
      writeAntField (antfCols, rownr, i, stationName,
                     antField, antFieldName, hbaOffsets, firstHbaOffset);
      writeElements (antfCols, rownr, antField.AntPos(antFieldType),
                     antennaSet.positionIndex (setName, stationType),
                     setName == "HBA_ONE",
                     antField.Centre(antFieldType),
                     antField.Centre(antFieldName));
      rownr++;
    }
    LOG_DEBUG_STR ("Wrote " << rownr << " station field rows");
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
                                const AntField& antField,
                                const string& antFieldName,
                                const AntField::AFArray& hbaOffsets,
                                int sthba)
{
  string type = antFieldName.substr(0,3);
  columns.antennaId().put (rownr, antennaId);
  columns.name().put      (rownr, antFieldName);
  columns.position().put  (rownr, array2Casa(antField.Centre(antFieldName)));
  const AntField::AFArray& rotMat = antField.rotationMatrix(antFieldName);
  ASSERTSTR (AntField::getShape(rotMat)[0]==3 &&
             AntField::getShape(rotMat)[1]==3,
             "No valid rotation matrix for station " << stationName);
  // Write transpose of rotation matrix (for BBS).
  columns.coordinateAxes().put (rownr,
                                transpose(Matrix<double>(array2Casa(rotMat))));
  if (type == "HBA") {
    columns.tileRotation().put (rownr, 0.);
    Array<double> arr = array2Casa(hbaOffsets);
    Array<double> subarr = arr(Slicer(IPosition(2, 0, sthba),
                                      IPosition(2, Slicer::MimicSource, 16)));
    columns.tileElementOffset().put (rownr, subarr);
  }
}

void BeamTables::writeElements (MSAntennaFieldColumns& columns,
                                int rownr,
                                const AntField::AFArray& elemOffsets,
                                const vector<int16>& elemPresent,
                                bool addSkip,
                                const AntField::AFArray& stationCenter,
                                const AntField::AFArray& fieldCenter)
{
  double off0 = (AntField::getData(stationCenter)[0] -
                 AntField::getData(fieldCenter)[0]);
  double off1 = (AntField::getData(stationCenter)[1] -
                 AntField::getData(fieldCenter)[1]);
  double off2 = (AntField::getData(stationCenter)[2] -
                 AntField::getData(fieldCenter)[2]);
  Cube<double> elemOff(array2Casa(elemOffsets));
  int nelem = elemOff.shape()[2];
  Matrix<Double> offset(3,nelem);
  Matrix<Bool>   flag(2,nelem, True);
  for (int i=0; i<nelem; ++i) {
    // The element offsets are given as [nelem,npol,xyz].
    // Offsets are the same for the X and Y polarisation.
    offset(0,i) = elemOff(0,0,i) + off0;
    offset(1,i) = elemOff(1,0,i) + off1;
    offset(2,i) = elemOff(2,0,i) + off2;
  }
  // Clear the flag for the dipoles that are present.
  // Normally the value in the elemPresent vector gives the element index,
  // but not for HBA1 in the core stations. For those the skipped number
  // of values has to be added.
  // Note that elemPresent defines which elements are used for a mode.
  // -1 means not used.
  // If >=0, the value is normally equal to the vector's index.
  // However, for HBA_ONE this is not the case. It has 48 -1 values, and
  // thereafter value 0..47. SAS/MAC/BeamServer needs it this way.
  // Therefore the code adds the nr of -1 values if value != index to get
  // the proper flag index (which is the RCU number).
  Bool* flagPtr = flag.data();
  int nskip = 0;
  for (uint i=0; i<elemPresent.size(); ++i) {
    if (elemPresent[i] < 0) {
      nskip++;
    } else {
      int index = elemPresent[i];
      if (addSkip) index += nskip;
      ASSERT (index < int(flag.size()));
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

void BeamTables::getHBADeltas (const string& fileName,
                               AntField::AFArray& deltas,
                               bool mustExist)
{
  ifstream file(fileName.c_str());
  if (mustExist) {
    ASSERTSTR(file.good(), "Cannot open file " << fileName);
  }
  if (file.good()) {
    // The file may have comment lines at the top, starting with '#'
    // These must be skipped
    string line;
    getline (file, line);
    while (line[0] == '#') {
      getline (file, line);
    }
    // The array is stored after the name line which has just been read.
    AntField::readBlitzArray<2> (deltas, file);
  } else {
    // File not found is acceptable; fill with zeroes.
    vector<size_t>& shape = AntField::getShape(deltas);
    vector<double>& data  = AntField::getData(deltas);
    shape.resize (2);
    shape[0] = 16;
    shape[1] = 3;
    data.resize (16*3);
    std::fill (data.begin(), data.end(), 0.);
  }
  ASSERTSTR (AntField::getShape(deltas)[1] == 3  &&
             (AntField::getShape(deltas)[0] == 16  ||
              AntField::getShape(deltas)[0] == 32),
             "Incorrect array shape " << AntField::getShape(deltas)
             << " in " << fileName);
} 

Array<double> BeamTables::array2Casa (const AntField::AFArray& barray)
{
  // Create the shape (axes must be reversed).
  int ndim = AntField::getShape(barray).size();
  IPosition shape(ndim);
  for (int i=0; i<ndim; ++i) {
    shape[i] = AntField::getShape(barray)[ndim-i-1];
  }
  // Create a casacore Array object referring to the data in AFArray.
  double* ptr = const_cast<double*>(&(AntField::getData(barray)[0]));
  return Array<double>(shape, ptr, SHARE);
}
