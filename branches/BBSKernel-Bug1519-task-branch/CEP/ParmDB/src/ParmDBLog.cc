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
#include <Common/StringUtil.h>         // contains toString() functions for LofarTypes.h
#include <Common/LofarLogger.h>        // needed to write log messages
#include <ParmDB/ParmDBLog.h>
#include <ParmDB/ParmDBLogLevel.h>
#include <BBSKernel/ParmManager.h>     // needed to access ParmDB database

#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/TableLocker.h>
#include <casa/Arrays/Vector.h>

#include <vector>

using namespace casa;
using namespace std;

namespace LOFAR {
namespace BBS {

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
    itsMaxIter.attach (itsTable, "MAXITER");
    itsLastIter.attach (itsTable, "LASTITER");
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
  
  
  // Public function to add ParmDB parameter keywords
  void ParmDBLog::addParmKeywords (const CoeffIndex &coeffMap)
  {
     TableLocker locker(itsTable, FileLocker::Write);      
     doAddParmKeywords(coeffMap);
  }
  
  
  // Public function to add initial solver parameter values
  void ParmDBLog::addSolverKeywords (double EpsValue, double EpsDerivative, 
                                     size_t MaxIter, double ColFactor, double LMFactor)
  {
     TableLocker locker(itsTable, FileLocker::Write);      
     doAddSolverKeywords(EpsValue, EpsDerivative, MaxIter, ColFactor, LMFactor);
  }
   
  
  // Add the ParmDB parameter keywords to the table keywords
  void ParmDBLog::doAddParmKeywords ( const CoeffIndex &coeffMap )
  {     
     // Get rw-keywordset from table
     TableRecord &keywords = itsTable.rwKeywordSet();
    
     // Iterate over coeffMap and write Parm name and corresponding coefficients
     // to casa table        
     for(CoeffIndex::const_iterator coeff_it = coeffMap.begin(),
           coeff_end = coeffMap.end(); coeff_it != coeff_end; ++coeff_it)
     {        
        // Writing a casa::Array<Int> does not work, yet
        // Write start and end indices of coeffIndices to table
        casa::Vector<uint32> coeffIndices(2);
        coeffIndices[0]=coeff_it->second.start;
        coeffIndices[1]=coeff_it->second.start + coeff_it->second.length-1;
        
        keywords.define(coeff_it->first, coeffIndices);        
     } 
  }
  
  
  // Add initial solver parameter values to the table keywords 
  void ParmDBLog::addSolverKeywords (const SolverOptions &options)
  {
     TableLocker locker(itsTable, FileLocker::Write);   
     
     doAddSolverKeywords(options.epsValue, options.epsDerivative, options.maxIter,
         options.colFactor, options.lmFactor);
  }

  
  void ParmDBLog::doAddSolverKeywords (double EpsValue, double EpsDerivative, 
                                        unsigned int MaxIter, double ColFactor, double LMFactor)
  {  
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

     keywords.define("EpsValue", EpsValue);    
     keywords.define("EpsDerivative", EpsDerivative);
     keywords.define("MaxIter", MaxIter);
     keywords.define("EpsValue", EpsValue);
     keywords.define("ColFactor", ColFactor);
     keywords.define("LMFactor", LMFactor);       
  }  

  
  /*
  void ParmDBLog::createKeywords (const string& parsetFilename, casa::Map<String, Vector<size_t> > &coeffMap )
  {
     // Get rw-keywordset from table
     TableRecord &keywords = itsTable.rwKeywordSet();
     keywords.define("Parset", parsetFilename);  // Write Parset filename to keywords
     
     casa::Array<unsigned int> coeffs;  // casa array needed since we can not pass on a vector to table keywords
     
     casa::ConstMapIter<casa::String, casa::Vector<size_t> > it=coeffMap.getIter();
     for(it.toStart(); !it.atEnd(); ++it) 
     {     
        unsigned int i=0;                       // index variable into casa array
        coeffs.resize(it.getVal().shape());     // need to resize array to length of vector
        
        for(casa::Vector<size_t>::const_iterator at = it.getVal().begin(); at!=it.getVal().end(); ++at)
        {
           coeffs[i]=*at;                       // write vector entry to casa array coefficients 
           i++;
        }
     
        keywords.description();                 // DEBUG
        keywords.define(it.getKey(), coeffs);   // write keyword and its coefficients to the table keywords
     }
  }
  */
  

  void ParmDBLog::add (double startFreq, double endFreq,
                       double startTime, double endTime,
                       uint iter, uint maxIter, Bool lastIter,
                       uint rank, uint rankDeficiency,
                       double chiSquare, double lmFactor,
                       const vector<double>& solution, const string& message)
  {
    TableLocker locker(itsTable, FileLocker::Write);
    doAdd (startFreq, endFreq, startTime, endTime,
           iter, maxIter, lastIter, rank, rankDeficiency,
           chiSquare, lmFactor, solution, message);
  }

  void ParmDBLog::add (double startFreq, double endFreq,
                       double startTime, double endTime,
                       uint iter, uint maxIter, Bool lastIter,
                       uint rank, uint rankDeficiency,
                       double chiSquare, double lmFactor,
                       const vector<double>& solution, const string& message,
                       const casa::Array<double>& correlationMatrix)
  {
    TableLocker locker(itsTable, FileLocker::Write);
    doAdd (startFreq, endFreq, startTime, endTime,
           iter, maxIter, lastIter, rank, rankDeficiency,
           chiSquare, lmFactor, solution, message);
    itsCorrMat.put (itsTable.nrow()-1, correlationMatrix);
  }

  void ParmDBLog::doAdd (double startFreq, double endFreq,
                         double startTime, double endTime,
                         uint iter, uint maxIter, bool lastIter,
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
