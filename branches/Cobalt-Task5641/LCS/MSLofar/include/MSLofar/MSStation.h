//# MSStation.h: LOFAR MS STATION subtable
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

#ifndef MSLOFAR_MSSTATION_H
#define MSLOFAR_MSSTATION_H

#include <MSLofar/MSLofarTable.h>

namespace LOFAR {

  class MSStation: public MSLofarTable
  {
  public:

    // Default constructor creates an unusable object.
    MSStation();

    // Create from an existing table.
    MSStation (const casa::String& tableName,
                    casa::Table::TableOption option);

    // Create a new table.
    MSStation (casa::SetupNewTable& newTab, casa::uInt nrrow,
                    casa::Bool initialize);

    // Create from an existing Table object.
    MSStation (const casa::Table& table);

    // Copy constructor (reference semnatics).
    MSStation (const MSStation& that);

    // The destructor flushes the table if not done yet.
    ~MSStation();

    // Assignment (reference semantics).
    MSStation& operator= (const MSStation& that);
  
    // Create the table description containing the required columns. 
    static casa::TableDesc requiredTableDesc();
  };

} //# end namespace

#endif
