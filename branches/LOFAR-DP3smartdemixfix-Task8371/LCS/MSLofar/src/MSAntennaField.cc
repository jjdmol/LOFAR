//# MSAntennaField.cc: LOFAR MS ANTENNA_FIELD subtable
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
#include <MSLofar/MSAntennaField.h>

#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>

using namespace casa;

namespace LOFAR {

  MSAntennaField::MSAntennaField()
  {}

  MSAntennaField::MSAntennaField (const String& tableName,
                                  Table::TableOption option) 
    : MSLofarTable (tableName, option)
  {}

  MSAntennaField::MSAntennaField (SetupNewTable& newTab, uInt nrrow,
                                  Bool initialize)
    : MSLofarTable (newTab, nrrow, initialize)
  {}

  MSAntennaField::MSAntennaField (const Table& table)
    : MSLofarTable (table)
  {}

  MSAntennaField::MSAntennaField (const MSAntennaField& that)
    : MSLofarTable (that)
  {}

  MSAntennaField::~MSAntennaField()
  {}

  MSAntennaField& MSAntennaField::operator= (const MSAntennaField& that)
  {
    MSLofarTable::operator= (that);
    return *this;
  }

  TableDesc MSAntennaField::requiredTableDesc()
  {
    TableDesc td;
    addColumn (td, "ANTENNA_ID", TpInt, "Antenna ID in ANTENNA table");
    addColumn (td, "NAME", TpString, "Antenna field name");
    addColumn (td, "POSITION", TpArrayDouble,
               "Center of light of antenna field",
               "m", "POSITION", MPosition::ITRF, 1, IPosition(1,3),
               ColumnDesc::FixedShape + ColumnDesc::Direct);
    addColumn (td, "COORDINATE_AXES", TpArrayDouble,
               "Local field coordinates as P,Q,R (cartesian) direction vectors",
               "m", "DIRECTION", MDirection::ITRF, 2, IPosition(2,3,3),
               ColumnDesc::FixedShape + ColumnDesc::Direct);
    addColumn (td, "ELEMENT_OFFSET", TpArrayDouble,
               "Position offsets of elements w.r.t. center of station field",
               "m", "POSITION", MPosition::ITRF, 2);
    addColumn (td, "ELEMENT_RCU", TpArrayInt,
               "RCU used for the elements (for X and Y)",
               String(), String(), 0, 2);
    addColumn (td, "ELEMENT_FLAG", TpArrayBool,
               "T for inactive element receptors (for X and Y)",
               String(), String(), 0, 2);
    addColumn (td, "TILE_ROTATION", TpDouble,
               "Rotation of HBA tile w.r.t. station coordinates",
               "rad");
    addColumn (td, "TILE_ELEMENT_OFFSET", TpArrayDouble,
               "Position offsets of dual dipole elements inside HBA tile"
               "w.r.t. tile center",
               "m", "POSITION", MPosition::ITRF);
    return td;
  }

} //# end namespace
