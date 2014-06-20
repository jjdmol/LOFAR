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
#include <ParmDB/ParmDBLogLevel.h>
#include <Common/StringUtil.h>         // contains toString() functions for LofarTypes.h
#include <Common/LofarLogger.h>        // needed to write log messages
#include <Common/lofar_vector.h>

#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/TableLocker.h>
#include <casa/Arrays/Vector.h>

using namespace casa;
using namespace std;

namespace LOFAR
{
namespace BBS
{

  ParmDBLog::ParmDBLog (const string& tableName, ParmDBLoglevel::LoggingLevel LogLevel, bool forceNew, bool wlock)
  {
    setLoggingLevel(LogLevel);

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
    itsLastIter.attach (itsTable, "LASTITER");
    itsRank.attach (itsTable, "RANK");
    itsRankDef.attach (itsTable, "RANKDEF");
    itsChiSqr.attach (itsTable, "CHISQR");
    itsLMFactor.attach (itsTable, "LMFACTOR");
    itsMessage.attach (itsTable, "MESSAGE");
    itsSolution.attach (itsTable, "SOLUTION");
    itsCorrMat.attach (itsTable, "CORRMATRIX");
  }

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
    td.addColumn (ScalarColumnDesc<Bool>  ("LASTITER"));
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
    tab.tableInfo().readmeAddLine ("BBS Solver logging");
  }

  void ParmDBLog::setCoeffIndex (const string &parm, unsigned int start,
                                 unsigned int end)
  {
    TableLocker locker(itsTable, FileLocker::Write);

    // Get rw-keywordset from table
    TableRecord &keywords = itsTable.rwKeywordSet();

    casa::Vector<uInt> range(2);
    range[0] = start;
    range[1] = end;

    keywords.define(parm, range);
  }

  // Public function to set the initial LSQ solver configuration.
  void ParmDBLog::setSolverKeywords (double epsValue, double epsDerivative,
                                     unsigned int maxIter, double colFactor,
                                     double lmFactor)
  {
     TableLocker locker(itsTable, FileLocker::Write);

     // Get rw-keywordset from table
     TableRecord &keywords = itsTable.rwKeywordSet();

     // Write logging level to table
     if (getLoggingLevel() == ParmDBLoglevel::PERSOLUTION)
        keywords.define("Logginglevel", "PERSOLUTION");
     else if (getLoggingLevel() == ParmDBLoglevel::PERITERATION)
        keywords.define("Logginglevel", "PERITERATION");
     else if (getLoggingLevel() == ParmDBLoglevel::PERSOLUTION_CORRMATRIX)
        keywords.define("Logginglevel", "PERSOLUTION_CORRMATRIX");
     else if (getLoggingLevel() == ParmDBLoglevel::PERITERATION_CORRMATRIX)
        keywords.define("Logginglevel", "PERITERATION_CORRMATRIX");

     keywords.define("EpsValue", epsValue);
     keywords.define("EpsDerivative", epsDerivative);
     keywords.define("MaxIter", maxIter);
     keywords.define("EpsValue", epsValue);
     keywords.define("ColFactor", colFactor);
     keywords.define("LMFactor", lmFactor);
  }

  void ParmDBLog::add (double startFreq, double endFreq,
                       double startTime, double endTime,
                       uint iter, Bool lastIter,
                       uint rank, uint rankDeficiency,
                       double chiSquare, double lmFactor,
                       const vector<double>& solution, const string& message)
  {
    TableLocker locker(itsTable, FileLocker::Write);
    doAdd (startFreq, endFreq, startTime, endTime,
           iter, lastIter, rank, rankDeficiency,
           chiSquare, lmFactor, solution, message);
  }

  void ParmDBLog::add (double startFreq, double endFreq,
                       double startTime, double endTime,
                       uint iter, Bool lastIter,
                       uint rank, uint rankDeficiency,
                       double chiSquare, double lmFactor,
                       const vector<double>& solution, const string& message,
                       const casa::Array<double>& correlationMatrix)
  {
    TableLocker locker(itsTable, FileLocker::Write);
    doAdd (startFreq, endFreq, startTime, endTime,
           iter, lastIter, rank, rankDeficiency,
           chiSquare, lmFactor, solution, message);
    itsCorrMat.put (itsTable.nrow()-1, correlationMatrix);
  }

  void ParmDBLog::doAdd (double startFreq, double endFreq,
                         double startTime, double endTime,
                         uint iter, bool lastIter,
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
    itsLastIter.put (rownr, lastIter);
    itsRank.put (rownr, rank);
    itsRankDef.put (rownr, rankDeficiency);
    itsChiSqr.put (rownr, chiSquare);
    itsLMFactor.put (rownr, lmFactor);
    itsMessage.put (rownr, message);
    itsSolution.put (rownr, Vector<double>(solution));
  }

} // namespace BBS
} // namespace LOFAR
