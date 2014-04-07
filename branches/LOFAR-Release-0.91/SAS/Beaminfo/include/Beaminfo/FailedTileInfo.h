//# FailedTleInfo.cc: Class to write failed tile info into an MS
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
//# $Id: FailedTileInfo.h 19920 2012-01-24 14:57:23Z diepen $

#ifndef LOFAR_FAILEDTILEINFO_H
#define LOFAR_FAILEDTILEINFO_H

#include <tables/Tables/Table.h>
#include <casa/Quanta/MVEpoch.h>

#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>
#include <Common/lofar_string.h>

namespace LOFAR {

  class FailedTileInfo
  {
  public:
    // Define a vector of FailedTileInfo objects.
    typedef vector<FailedTileInfo> VectorFailed;

    // Construct the object for a failed tile (RCU).
    FailedTileInfo (const string& station, const string& time,
                    const casa::MVEpoch& epoch, int rcu);

    // Read the broken hardware file giving a map of antenna name to a vector
    // of pairs of RCUnr and time.
    static map<string, VectorFailed> readFile
    (const string& fileName, double startTime, double endTime);

    // Convert the info per station name to info per antennaId.
    static vector<VectorFailed> antennaConvert
    (const casa::Table& ms, const map<string,VectorFailed>& broken);

    // Write the failed tile info.
    static void writeFailed (casa::Table& ms,
                             const vector<VectorFailed>& brokenBefore,
                             const vector<VectorFailed>& brokenDuring);

    // Read the failed tile info and write into MS as needed.
    static void failedTiles2MS (const string& msName, const string& beforeName,
                                const string& duringName);

  private:
    string        itsStation;
    string        itsTime;
    casa::MVEpoch itsEpoch;
    int           itsRcu;
  };

} //# end namespace

#endif
