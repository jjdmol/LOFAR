//# ParmDBLog.h: Class to log results when solving parameters
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

// @file
// @brief Class to log results when solving parameters
// @author Ger van Diepen (diepen AT astron nl)

#ifndef LOFAR_PARMDB_PARMDBLOG_H
#define LOFAR_PARMDB_PARMDBLOG_H

//# Includes
#include <casa/Containers/Map.h>
#include <casa/Arrays/Array.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>

#include <Common/lofar_map.h>
#include <ParmDB/ParmDBLogLevel.h>

namespace LOFAR {
namespace BBS {

  // @ingroup ParmDB
  // @{

  // @brief Class to log results when solving parameters
  class ParmDBLog
  {
  public:
   // Setting which logging level is used
   /*
   enum LoggingLevel {
         NONE,
         PERSOLUTION,
         PERITERATION,
         PERSOLUTION_CORRMATRIX,
         PERITERATION_CORRMATRIX
         };
    */

    // Create the object.
    // The table is created if <src>forceNew=true</src> or if the table does
    // not exist yet.
    // If <src>lock=true</src> a write lock is acquired. In this way no
    // implcit locks have to be acquired on each access.
    // The default logging level is PERSOLUTION
    explicit ParmDBLog (const string &tableName, ParmDBLoglevel::LoggingLevel LogLevel=ParmDBLoglevel::PERSOLUTION, bool forceNew=true, bool lock=true);

    // Writelock and unlock the table.
    // It is not necessary to do this, but it can be useful if many
    // small accesses have to be done.
    // <group>
    void lock (bool lockForWrite=true);
    void unlock();
    // </group>

    // Add a solve entry (without correlation matrix).
    void add (double startFreq, double endFreq,
              double startTime, double endTime,
              uint iter, bool lastIter,
              uint rank, uint rankDeficiency,
              double chiSquare, double lmFactor,
              const vector<double>& solution, const string& message);

    // Add a solve entry (with correlation matrix).
    void add (double startFreq, double endFreq,
              double startTime, double endTime,
              uint iter, bool lastIter,
              uint rank, uint rankDeficiency,
              double chiSquare, double lmFactor,
              const vector<double>& solution, const string& message,
              const casa::Array<double>& correlationMatrix);

    // Get or set the logging level of solver parameters
    // <group>
    ParmDBLoglevel::LoggingLevel getLoggingLevel() const
      { return itsLoggingLevel; }
    void setLoggingLevel (ParmDBLoglevel::LoggingLevel level)
      { itsLoggingLevel = level; }
    // </group>

    // Set the index range corresponding to the specified parameter.
    void setCoeffIndex (const string &parm, unsigned int start,
                        unsigned int end);

    // Set the initial LSQ solver configuration.
    void setSolverKeywords (double epsValue, double epsDerivative,
                            unsigned int maxIter, double colFactor,
                            double lmFactor);

  private:
    // Create the tables.
    void createTables (const string& tableName);

    // Add a row and write the values.
    void doAdd (double startFreq, double endFreq,
                double startTime, double endTime,
                uint iter, bool lastIter,
                uint rank, uint rankDeficiency,
                double chiSquare, double lmFactor,
                const vector<double>& solution, const string& message);

    //# Data members
    ParmDBLoglevel::LoggingLevel itsLoggingLevel;
    casa::Table itsTable;
    casa::ScalarColumn<casa::Double> itsStartFreq;
    casa::ScalarColumn<casa::Double> itsEndFreq;
    casa::ScalarColumn<casa::Double> itsStartTime;
    casa::ScalarColumn<casa::Double> itsEndTime;
    casa::ScalarColumn<casa::uInt>   itsIter;
    casa::ScalarColumn<casa::Bool>   itsLastIter;
    casa::ScalarColumn<casa::uInt>   itsRank;
    casa::ScalarColumn<casa::uInt>   itsRankDef;
    casa::ScalarColumn<casa::Double> itsChiSqr;
    casa::ScalarColumn<casa::Double> itsLMFactor;
    casa::ScalarColumn<casa::String> itsMessage;
    casa::ArrayColumn<casa::Double>  itsSolution;
    casa::ArrayColumn<casa::Double>  itsCorrMat;
  };

  // @}

} // namespace BBS
} // namespace LOFAR

#endif
