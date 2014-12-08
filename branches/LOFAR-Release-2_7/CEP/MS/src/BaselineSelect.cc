//# BaselineSelect.cc: Convert MSSelection baseline string to a Matrix
//#
//# Copyright (C) 2010
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
//#  $Id$

//# Includes
#include <MS/BaselineSelect.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>

using namespace casa;

namespace LOFAR {

  Matrix<bool> BaselineSelect::convert (const string& msName,
                                        const string& baselineSelection)
  {
    // Find the unique baselines in the MS.
    // Do not use unique sort, because that is slow for a large MS.
    // Simply go through all baselines.
    Table bltab;
    {
      Table tab(msName);
      Vector<Int> a1 = ROScalarColumn<Int>(tab, "ANTENNA1").getColumn();
      Vector<Int> a2 = ROScalarColumn<Int>(tab, "ANTENNA2").getColumn();
      int nant = 1 + std::max(max(a1), max(a2));
      Matrix<bool> bl(nant, nant, false);
      vector<uInt> rows;
      rows.reserve (nant*nant);
      for (uint i=0; i<a1.size(); ++i) {
        if (! bl(a1[i], a2[i])) {
          rows.push_back (i);
          bl(a1[i], a2[i]) = true;
        }
      }
      bltab = tab(Vector<uInt>(rows));
    }
    MeasurementSet ms(bltab);
    MSSelection select;
    // Set given selection strings.
    select.setAntennaExpr (baselineSelection);
    // Create a table expression over a MS representing the selection
    TableExprNode node = select.toTableExprNode (&ms);
    Table seltab = ms(node);
    // Get the antenna numbers.
    Vector<Int> a1 = ROScalarColumn<Int>(seltab, "ANTENNA1").getColumn();
    Vector<Int> a2 = ROScalarColumn<Int>(seltab, "ANTENNA2").getColumn();
    int nant = ms.antenna().nrow();
    Matrix<bool> bl(nant, nant, false);
    for (uint i=0; i<a1.size(); ++i) {
      bl(a1[i], a2[i]) = true;
      bl(a2[i], a1[i]) = true;
    }
    return bl;
  }

} // end namespace
