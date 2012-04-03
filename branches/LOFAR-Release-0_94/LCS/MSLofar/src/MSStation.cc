//# MSStation.cc: LOFAR MS STATION subtable
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
#include <MSLofar/MSStation.h>

#include <measures/Measures/MPosition.h>

using namespace casa;

namespace LOFAR {

  MSStation::MSStation()
  {}

  MSStation::MSStation (const String& tableName,
                        Table::TableOption option) 
    : MSLofarTable (tableName, option)
  {}

  MSStation::MSStation (SetupNewTable& newTab, uInt nrrow,
                        Bool initialize)
    : MSLofarTable (newTab, nrrow, initialize)
  {}

  MSStation::MSStation (const Table& table)
    : MSLofarTable (table)
  {}

  MSStation::MSStation (const MSStation& that)
    : MSLofarTable (that)
  {}

  MSStation::~MSStation()
  {}

  MSStation& MSStation::operator= (const MSStation& that)
  {
    MSLofarTable::operator= (that);
    return *this;
  }

  TableDesc MSStation::requiredTableDesc()
  {
    TableDesc td;
    addColumn (td, "NAME", TpString, "Station name");
    addColumn (td, "CLOCK_ID", TpInt, "Index of shared clock");
    addColumn (td, "FLAG_ROW", TpBool, "Row flag");
    return td;
  }

} //# end namespace
