//# MSLofarAntenna.cc: MS ANTENNA subtable with LOFAR extensions
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
#include <MSLofar/MSLofarAntenna.h>
#include <measures/Measures/MPosition.h>

using namespace casa;

namespace LOFAR {

  MSLofarAntenna::MSLofarAntenna()
  {}

  MSLofarAntenna::MSLofarAntenna (const String& tableName,
                                  Table::TableOption option) 
    : MSAntenna (tableName, option)
  {}

  MSLofarAntenna::MSLofarAntenna (SetupNewTable& newTab, uInt nrrow,
                                  Bool initialize)
    : MSAntenna (newTab, nrrow, initialize)
  {}

  MSLofarAntenna::MSLofarAntenna (const Table& table)
    : MSAntenna (table)
  {}

  MSLofarAntenna::MSLofarAntenna (const MSLofarAntenna& that)
    : MSAntenna (that)
  {}

  MSLofarAntenna::~MSLofarAntenna()
  {}

  MSLofarAntenna& MSLofarAntenna::operator= (const MSLofarAntenna& that)
  {
    MSAntenna::operator= (that);
    return *this;
  }

  TableDesc MSLofarAntenna::requiredTableDesc()
  {
    TableDesc td (MSAntenna::requiredTableDesc());
    MSLofarTable::addColumn (td, "LOFAR_STATION_ID", TpInt,
                             "ID in LOFAR_STATION table");
    MSLofarTable::addColumn (td, "LOFAR_PHASE_REFERENCE", TpArrayDouble,
                             "Beamformer phase reference position",
                             "m", "POSITION", MPosition::ITRF,
                             1, IPosition(1,3),
                             ColumnDesc::FixedShape + ColumnDesc::Direct);
    return td;
  }

} //# end namespace
