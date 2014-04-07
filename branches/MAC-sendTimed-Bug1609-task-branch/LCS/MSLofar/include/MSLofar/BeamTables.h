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

#ifndef MSLOFAR_BEAMTABLES_H
#define MSLOFAR_BEAMTABLES_H

#include <ApplCommon/AntField.h>
#include <ApplCommon/AntennaSets.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>

//# Forward Declarations.
namespace casa {
  class Table;
  template<typename T> class Array;
  template<typename T> class Vector;
  class String;
}


namespace LOFAR {

  //# Forward Declarations.
  class MSAntennaFieldColumns;
  class MSStation;
  class MSStationColumns;


  // This class contains some functions to fill the antenna field tables
  // from the AntennaField files.

  class BeamTables
  {
  public:

    // Create the subtables and attach them to the MS.
    static void create (casa::Table& ms,
                        bool overwrite = false);

    // Fill the subtables. They should be empty.
    // <src>mustExist</src> tells if the AntennaField and iHBADelta file on
    // an antenna must exist.
    static void fill (casa::Table& ms,
                      const string& antennaSet,
                      const string& antennaSetFileName,
                      const string& antennaFieldDir,
                      const string& iHBADeltaDir,
                      bool mustExist=false);

    // Write an AntennaField entry in the given row.
    static void writeAntField (MSAntennaFieldColumns& columns, int rownr,
                               int antennaId, const string& stationName,
                               const AntField& antField,
                               const string& antFieldName,
                               const AntField::AFArray& hbaOffsets,
                               int firstHBAOffset);

    // Write the possible AntennaField elements.
    // The elements in the configuration are given in the bitset
    // starting at the given bit (there is a bit for X and one for Y).
    // The number of skipped elements has to be added for HBA1.
    static void writeElements (MSAntennaFieldColumns& columns,
                               int rownr,
                               const AntField::AFArray& elemOffsets,
                               const vector<int16>& elemPresent,
                               bool addSkip,
                               const AntField::AFArray& stationCenter,
                               const AntField::AFArray& fieldCenter);

    // Write the antenna set name into all rows of the LOFAR_ANTENNA_SET
    // column of the OBSERVATION table.
    // The column is added if not defined yet.
    static void writeObservation (casa::Table& obsTable,
                                  const string& antennaSet);

    // Write the LOFAR_STATION table.
    // If needed, define clock-ids for stations (starting at 1).
    static void writeStation (MSStation&, MSStationColumns&,
                              const vector<string>& stationNames,
                              int nstations);

    // Write the LOFAR_STATION_ID column in the ANTENNA table.
    // The column is added if not existing.
    static void writeAntenna (casa::Table& antTable,
                              const casa::Vector<casa::String>& antNames,
                              const map<string,int>& stationIdMap);

    // Convert an AFArray to a casacore Array object.
    static casa::Array<double> array2Casa (const AntField::AFArray& barray);

    // Read the HBA dipole offsets.
    static void getHBADeltas (const string& filename, AntField::AFArray&,
                              bool mustExist);
  };

} //# end namespace

#endif

