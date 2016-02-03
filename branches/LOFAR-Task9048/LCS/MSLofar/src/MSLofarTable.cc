//# MSLofarTable.cc: Base class for LOFAR MS Tables
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

#include <MSLofar/MSLofarTable.h>
#include <Common/LofarLogger.h>

#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ColDescSet.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <ms/MeasurementSets/MSTableImpl.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableQuantumDesc.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>

using namespace casa;

namespace LOFAR {

  MSLofarTable::MSLofarTable()
  {}

  MSLofarTable::MSLofarTable (const String& tableName,
                              Table::TableOption option) 
    : Table (tableName, option)
  {}

  MSLofarTable::MSLofarTable (SetupNewTable& newTab, uInt nrrow,
                              Bool initialize)
    : Table (newTab, nrrow, initialize)
  {}

  MSLofarTable::MSLofarTable (const Table& table)
    : Table (table)
  {}

  MSLofarTable::MSLofarTable (const MSLofarTable& that)
    : Table (that)
  {}

  MSLofarTable::~MSLofarTable()
  {}

  MSLofarTable& MSLofarTable::operator= (const MSLofarTable& that)
  {
    if (this != &that) {
      Table::operator= (that);
    }
    return *this;
  }

  void MSLofarTable::addColumn (TableDesc& td, const casa::String& colName,
                                DataType dtype,
                                const casa::String& comment,
                                const casa::String& unit,
                                const casa::String& measure, int measRefType,
                                int ndim, const IPosition& shape,
                                int option)
  {
    MSTableImpl::addColumnToDesc (td, colName, dtype, comment, unit, measure,
                                  ndim, shape, option, String());
    String meas = measure;
    meas.downcase();
    if (meas == "direction") {
      TableMeasValueDesc measVal(td, colName);
      TableMeasDesc<MDirection> measCol(measVal, TableMeasRefDesc(measRefType));
      measCol.write(td);
      setUnit (td, colName, unit, 2);
    } else if (meas == "position") {
      TableMeasValueDesc measVal(td, colName);
      TableMeasDesc<MPosition> measCol(measVal, TableMeasRefDesc(measRefType));
      measCol.write(td);
      setUnit (td, colName, unit, 3);
    } else if (meas == "epoch") {
      TableMeasValueDesc measVal(td, colName);
      TableMeasDesc<MEpoch> measCol(measVal, TableMeasRefDesc(measRefType));
      measCol.write(td);
      setUnit (td, colName, unit, 1);
    } else {
      ASSERT (measure.empty());
    }
  }

  void MSLofarTable::setUnit (TableDesc& td, const String& colName,
			      const String& unit, int nunit)
  {
    Vector<Unit> vu(nunit, Unit(unit));
    TableQuantumDesc tqd(td, colName, vu);
    tqd.write(td);
  }

} //# end namespace
