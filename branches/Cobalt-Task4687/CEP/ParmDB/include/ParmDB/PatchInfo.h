//# PatchInfo.h: Info about a patch
//#
//# Copyright (C) 2008
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

// @file
// @brief Info about a patch
// @author Ger van Diepen (diepen AT astron nl)

#ifndef LOFAR_PARMDB_PATCHINFO_H
#define LOFAR_PARMDB_PATCHINFO_H

//# Includes
#include <Common/lofar_string.h>
#include <casa/Arrays/Array.h>
#include <measures/Measures/MDirection.h>


namespace LOFAR {
namespace BBS {

  // @ingroup ParmDB
  // @{

  // @brief Info about a patch
  class PatchInfo
  {
  public:
    // Create from patch name, category, ra, dec, and apparent brightness (Jy).
    // Ra and dec must be in radians in J2000.
    PatchInfo (const string& name, double ra, double dec, int category,
               double apparentBrightness)
      : itsName          (name),
        itsRa            (ra),
        itsDec           (dec),
        itsCategory      (category),
        itsAppBrightness (apparentBrightness)
    {}

    // Get the patch name.
    const string& getName() const
      { return itsName; }

    // Get the right ascension in radians (J2000).
    double getRa() const
      { return itsRa; }

    // Get the declination in radians (J2000).
    double getDec() const
      { return itsDec; }

    // Get the category.
    int getCategory() const
      { return itsCategory; }

    // Get the apparent brightness of the patch (in Jy).
    double apparentBrightness() const
      { return itsAppBrightness; }

  private:
    string itsName;
    double itsRa;
    double itsDec;
    int    itsCategory;
    double itsAppBrightness;
  };


  // Show the contents of a PatchInfo object.
  inline std::ostream& operator<< (std::ostream& os, const PatchInfo& info)
  { os << "patch=" << info.getName() << " cat=" << info.getCategory()
       << " ra=" << info.getRa() << " dec=" << info.getDec()
       << " flux=" << info.apparentBrightness();
    return os;
  }

  // @}

} // namespace BBS
} // namespace LOFAR

#endif
