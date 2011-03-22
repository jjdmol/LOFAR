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

#include <ApplCommon/AntennaField.h>
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

    // Create and fill the subtables and attach them to the MS.
    static void create (casa::Table& ms,
                        const string& antennaSet,
                        const string& antennaSetFileName,
                        const string& antennaFieldDir,
                        const string& iHBADeltaDir,
                        bool overwrite = false);

    // Write an AntennaField entry in the given row.
    static void writeAntField (MSAntennaFieldColumns& columns, int rownr,
                               int antennaId, const string& stationName,
                               const AntennaField& antField,
                               const string& antFieldName,
                               const blitz::Array<double,2>& hbaOffsets,
                               int firstHBAOffset);

    // Write the possible AntennaField elements.
    // The elements in the configuration are given in the bitset
    // starting at the given bit (there is a bit for X and one for Y).
    static void writeElements (MSAntennaFieldColumns& columns,
                               int rownr,
                               const blitz::Array<double,3>& elemOffsets,
                               const vector<int16>& elemPresent,
                               const blitz::Array<double,1>& stationCenter,
                               const blitz::Array<double,1>& fieldCenter);

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

    // Convert a blitz array to a casacore Array object.
    template<typename T, int NDIM>
    static casa::Array<T> blitz2Casa (const blitz::Array<T,NDIM>& barray);

    // Read the HBA dipole offsets.
    static blitz::Array<double,2> getHBADeltas (const string& filename);
  };

} //# end namespace

#endif

