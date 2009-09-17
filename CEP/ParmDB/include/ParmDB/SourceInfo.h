//# SourceInfo.h: Info about a source
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
// @brief Info about a source
// @author Ger van Diepen (diepen AT astron nl)

#ifndef LOFAR_PARMDB_SOURCEINFO_H
#define LOFAR_PARMDB_SOURCEINFO_H

//# Includes
#include <Common/lofar_string.h>


namespace LOFAR {
namespace BBS {

  // @ingroup ParmDB
  // @{

  // @brief Info about a source
  class SourceInfo
  {
  public:
    // Define the source types.
    //# The values should never be changed.
    enum Type {POINT = 0,
               GAUSSIAN = 1,
               DISK = 2,
               SHAPELET = 3,
               SUN = 10,
               MOON = 11,
               JUPITER = 12,
               MARS = 13,
               VENUS = 14
    };


    // Create from source name and type.
    SourceInfo (const string& name, Type type)
      : itsName (name),
        itsType (type)
    {}

    // Get the name.
    const string& getName() const
      { return itsName; }

    // Get the type.
    Type getType() const
      { return itsType; }

  private:
    string itsName;
    Type   itsType;
  };

  // @}

} // namespace BBS
} // namespace LOFAR

#endif
