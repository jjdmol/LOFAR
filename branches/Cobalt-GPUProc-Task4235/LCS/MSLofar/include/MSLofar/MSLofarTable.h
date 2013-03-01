//# MSLofarTable.h: Base class for LOFAR MS Tables
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

#ifndef MSLOFAR_MSLOFARTABLE_H
#define MSLOFAR_MSLOFARTABLE_H

#include <tables/Tables/Table.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Utilities/DataType.h>

//# Forward Declarations.
namespace casa {
  class TableDesc;
  class SetupNewTable;
  class String;
}

namespace LOFAR {

  // This class is the base class for a LOFAR MeasurementSet table.
  // It is modeled after the casacore MS classes.

  class MSLofarTable: public casa::Table
  {
  public:
    // Default constructor creates an unusable object.
    MSLofarTable();

    // Create from an existing table.
    MSLofarTable (const casa::String& tableName, casa::Table::TableOption);

    // Create a new table.
    MSLofarTable (casa::SetupNewTable& newTab, casa::uInt nrrow,
                  casa::Bool initialize);

    // Create from an existing Table object.
    MSLofarTable (const casa::Table& table);

    // Copy constructor (reference semnatics).
    MSLofarTable (const MSLofarTable& that);

    // The destructor flushes the table if not done yet.
    ~MSLofarTable();

    // Assignment (reference semantics).
    MSLofarTable& operator= (const MSLofarTable& that);

    // Add a column to the table description.
    static void addColumn (casa::TableDesc& td,
                           const casa::String& colName,
                           casa::DataType dtype,
                           const casa::String& comment,
                           const casa::String& unit = casa::String(),
                           const casa::String& measure = casa::String(),
                           int measRefType = 0,
                           int ndim = -1,
                           const casa::IPosition& shape = casa::IPosition(),
                           int option = 0);

    // Set the units of the column.
    static void setUnit (casa::TableDesc& td, const casa::String& colName,
			 const casa::String& unit, int nunit);
  };

} //# end namespace

#endif
