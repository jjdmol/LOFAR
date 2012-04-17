//# MSElementFailure.cc: LOFAR MS ELEMENT_FAILURE subtable
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
#include <MSLofar/MSElementFailure.h>

#include <measures/Measures/MEpoch.h>

using namespace casa;

namespace LOFAR {

  MSElementFailure::MSElementFailure()
  {}

  MSElementFailure::MSElementFailure (const String& tableName,
                                      Table::TableOption option) 
    : MSLofarTable (tableName, option)
  {}

  MSElementFailure::MSElementFailure (SetupNewTable& newTab, uInt nrrow,
                                      Bool initialize)
    : MSLofarTable (newTab, nrrow, initialize)
  {}

  MSElementFailure::MSElementFailure (const Table& table)
    : MSLofarTable (table)
  {}

  MSElementFailure::MSElementFailure (const MSElementFailure& that)
    : MSLofarTable (that)
  {}

  MSElementFailure::~MSElementFailure()
  {}

  MSElementFailure& MSElementFailure::operator= (const MSElementFailure& that)
  {
    MSLofarTable::operator= (that);
    return *this;
  }

  TableDesc MSElementFailure::requiredTableDesc()
  {
    TableDesc td;
    addColumn (td, "ANTENNA_FIELD_ID", TpInt,
               "Reference to ANTENNA_FIELD");
    addColumn (td, "ELEMENT_INDEX", TpInt,
               "Element index in ELEMENT_OFFSET array");
    addColumn (td, "TIME", TpDouble,
               "Time of element failure",
               "s", "EPOCH", MEpoch::UTC);
    return td;
  }

} //# end namespace
