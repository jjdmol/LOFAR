//# ParmDBLog.cc: Class to log results when solving parameters
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
//# $Id$

#include <lofar_config.h>
#include <ParmDB/ParmDBLog.h>

#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/TableLocker.h>
#include <casa/Arrays/Vector.h>

using namespace casa;
using namespace std;

namespace LOFAR {
namespace BBS {

  ParmDBLog::ParmDBLog (const string& tableName, bool forceNew, bool wlock)
  {
    // Create the table if needed or if it does not exist yet.
    if (forceNew  ||  !Table::isReadable (tableName)) {
      createTables (tableName);
    }
    // Open the table.
    itsTable = Table(tableName, TableLock::UserLocking, Table::Update);
    // Lock if needed
    if (wlock) {
      lock (true);
    }
    // Create the column objects.
    itsStartFreq.attach (itsTable, "STARTFREQ");
    itsEndFreq.attach (itsTable, "ENDFREQ");
    itsStartTime.attach (itsTable, "STARTTIME");
    itsEndTime.attach (itsTable, "ENDTIME");
    itsIter.attach (itsTable, "ITER");
    itsMaxIter.attach (itsTable, "MAXITER");
    itsRank.attach (itsTable, "RANK");
    itsRankDef.attach (itsTable, "RANKDEF");
    itsChiSqr.attach (itsTable, "CHISQR");
    itsLMFactor.attach (itsTable, "LMFACTOR");
    itsMessage.attach (itsTable, "MESSAGE");
    itsSolution.attach (itsTable, "SOLUTION");
    itsCorrMat.attach (itsTable, "CORRMATRIX");
  }

  ParmDBLog::~ParmDBLog()
  {}

  void ParmDBLog::lock (bool lockForWrite)
  {
    itsTable.lock (lockForWrite);
  }

  void ParmDBLog::unlock()
  {
    itsTable.unlock();
  }

  void ParmDBLog::createTables (const string& tableName)
  {
    TableDesc td("Solve log table", TableDesc::Scratch);
    td.comment() = String("Table containing results of BBS solving");
    td.addColumn (ScalarColumnDesc<Double>("STARTFREQ"));
    td.addColumn (ScalarColumnDesc<Double>("ENDFREQ"));
    td.addColumn (ScalarColumnDesc<Double>("STARTTIME"));
    td.addColumn (ScalarColumnDesc<Double>("ENDTIME"));
    td.addColumn (ScalarColumnDesc<uInt>  ("ITER"));
    td.addColumn (ScalarColumnDesc<uInt>  ("MAXITER"));
    td.addColumn (ScalarColumnDesc<uInt>  ("RANK"));
    td.addColumn (ScalarColumnDesc<uInt>  ("RANKDEF"));
    td.addColumn (ScalarColumnDesc<Double>("CHISQR"));
    td.addColumn (ScalarColumnDesc<Double>("LMFACTOR"));
    td.addColumn (ScalarColumnDesc<String>("MESSAGE"));
    td.addColumn (ArrayColumnDesc<Double> ("SOLUTION"));
    td.addColumn (ArrayColumnDesc<Double> ("CORRMATRIX"));

    SetupNewTable newtab(tableName, td, Table::New);
    Table tab(newtab);
    tab.tableInfo().setType ("BBSLog");
    tab.tableInfo().readmeAddLine ("BBS Solve logging");
  }

  void ParmDBLog::add (double startFreq, double endFreq,
                       double startTime, double endTime,
                       uint iter, uint maxIter,
                       uint rank, uint rankDeficiency,
                       double chiSquare, double lmFactor,
                       const vector<double>& solution, const string& message)
  {
    TableLocker locker(itsTable, FileLocker::Write);
    doAdd (startFreq, endFreq, startTime, endTime,
           iter, maxIter, rank, rankDeficiency,
           chiSquare, lmFactor, solution, message);
  }

  void ParmDBLog::add (double startFreq, double endFreq,
                       double startTime, double endTime,
                       uint iter, uint maxIter,
                       uint rank, uint rankDeficiency,
                       double chiSquare, double lmFactor,
                       const vector<double>& solution, const string& message,
                       const casa::Array<double>& correlationMatrix)
  {
    TableLocker locker(itsTable, FileLocker::Write);
    doAdd (startFreq, endFreq, startTime, endTime,
           iter, maxIter, rank, rankDeficiency,
           chiSquare, lmFactor, solution, message);
    itsCorrMat.put (itsTable.nrow()-1, correlationMatrix);
  }

  void ParmDBLog::doAdd (double startFreq, double endFreq,
                         double startTime, double endTime,
                         uint iter, uint maxIter,
                         uint rank, uint rankDeficiency,
                         double chiSquare, double lmFactor,
                         const vector<double>& solution, const string& message)
  {
    uint rownr = itsTable.nrow();
    itsTable.addRow();
    itsStartFreq.put (rownr, startFreq);
    itsEndFreq.put (rownr, endFreq);
    itsStartTime.put (rownr, startTime);
    itsEndTime.put (rownr, endTime);
    itsIter.put (rownr, iter);
    itsMaxIter.put (rownr, maxIter);
    itsRank.put (rownr, rank);
    itsRankDef.put (rownr, rankDeficiency);
    itsChiSqr.put (rownr, chiSquare);
    itsLMFactor.put (rownr, lmFactor);
    itsMessage.put (rownr, message);
    itsSolution.put (rownr, Vector<double>(solution));
  }


} // namespace BBS
} // namespace LOFAR
