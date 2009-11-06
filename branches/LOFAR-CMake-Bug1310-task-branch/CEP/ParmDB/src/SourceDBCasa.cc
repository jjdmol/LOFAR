//# SourceDBCasa.cc: Class for a Casa table holding sources and their parameters
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <ParmDB/SourceDBCasa.h>
#include <ParmDB/ParmMap.h>

#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ExprNodeSet.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableLocker.h>
#include <tables/Tables/TableIter.h>
#include <casa/Containers/RecordField.h>
using namespace casa;

namespace LOFAR {
namespace BBS {

  SourceDBCasa::SourceDBCasa (const ParmDBMeta& pdm, bool forceNew)
    : SourceDBRep (pdm, forceNew)
  {
    string tableName = pdm.getTableName() + "/SOURCES";
    // Create the table if needed or if it does not exist yet.
    if (forceNew  ||  !Table::isReadable (tableName)) {
      createTables (pdm.getTableName());
    }
    // Open the main table.
    itsSourceTable = Table(tableName, TableLock::UserLocking);
    // Open the names table.
    itsPatchTable = itsSourceTable.keywordSet().asTable ("PATCHES");
  }

  SourceDBCasa::~SourceDBCasa()
  {}

  void SourceDBCasa::lock (bool lockForWrite)
  {
    itsSourceTable.lock (lockForWrite);
    itsPatchTable.lock (lockForWrite);
  }

  void SourceDBCasa::unlock()
  {
    itsSourceTable.unlock();
    itsPatchTable.unlock();
  }

  void SourceDBCasa::createTables (const string& tableName)
  {
    // Create description of SOURCES.
    TableDesc td("Local Sky Model Sources", TableDesc::Scratch);
    td.comment() = String("Table containing the sources in the Local Sky Model");
    td.addColumn (ScalarColumnDesc<String>("SOURCENAME"));
    td.addColumn (ScalarColumnDesc<uint>  ("PATCHID"));
    td.addColumn (ScalarColumnDesc<int>   ("SOURCETYPE"));
    // Create description of PATCHES.
    TableDesc tdpat("Local Sky Model patches", TableDesc::Scratch);
    tdpat.comment() = String("Table containing the patches in the Local Sky Model");
    tdpat.addColumn (ScalarColumnDesc<String>("PATCHNAME"));
    tdpat.addColumn (ScalarColumnDesc<uint>  ("CATEGORY"));
    tdpat.addColumn (ScalarColumnDesc<double>("APPARENT_BRIGHTNESS"));
    tdpat.addColumn (ScalarColumnDesc<double>("RA"));
    tdpat.addColumn (ScalarColumnDesc<double>("DEC"));
    // Create the tables.
    string tabNameSrc = tableName + "/SOURCES";
    SetupNewTable newtab(tabNameSrc, td, Table::New);
    SetupNewTable newpattab(tabNameSrc + "/PATCHES", tdpat, Table::New);
    Table tab(newtab);
    Table pattab(newpattab);
    // PATCHES is subtable of SOURCES.
    tab.rwKeywordSet().defineTable("PATCHES", pattab);  
    // Set type info.
    tab.tableInfo().setType ("LSM");
    tab.tableInfo().readmeAddLine ("Sources in the Local Sky Model");
    pattab.tableInfo().setType ("LSMpatches");
    pattab.tableInfo().readmeAddLine ("Patches in the Local Sky Model");
    // Make SOURCES subtable of the ParmDB.
    Table ptab(tableName, Table::Update);
    TableLocker locker(ptab, FileLocker::Write);
    ptab.rwKeywordSet().defineTable("SOURCES", tab);  
  }

  void SourceDBCasa::clearTables()
  {
    {
      TableLocker locker(itsSourceTable, FileLocker::Write);
      Vector<uint> rows = itsSourceTable.rowNumbers();
      itsSourceTable.removeRow(rows);
    }
    {
      TableLocker locker(itsPatchTable, FileLocker::Write);
      Vector<uint> rows = itsPatchTable.rowNumbers();
      itsPatchTable.removeRow(rows);
    }
  }

  void SourceDBCasa::checkDuplicates()
  {
    TableLocker lockerp(itsPatchTable, FileLocker::Read);
    Table tabp = itsPatchTable.sort ("PATCHNAME", Sort::Ascending,
                                     Sort::HeapSort + Sort::NoDuplicates);
    ASSERTSTR (tabp.nrow() == itsPatchTable.nrow(),
               "The PATCHES table has " <<
               itsPatchTable.nrow() - tabp.nrow() <<
               " duplicate patch names");
    TableLocker lockers(itsSourceTable, FileLocker::Read);
    Table tabs = itsSourceTable.sort ("SOURCENAME", Sort::Ascending,
                                     Sort::HeapSort + Sort::NoDuplicates);
    ASSERTSTR (tabs.nrow() == itsSourceTable.nrow(),
               "The SOURCES table has " <<
               itsSourceTable.nrow() - tabs.nrow() <<
               " duplicate source names");
  }

  vector<string> SourceDBCasa::findDuplicates (Table& table,
                                               const string& columnName)
  {
    TableLocker locker(table, FileLocker::Read);
    TableIterator iter(table, columnName);
    vector<string> result;
    while (!iter.pastEnd()) {
      if (iter.table().nrow() > 1) {
        result.push_back (ROScalarColumn<String>(table, columnName)(0));
      }
      ++iter;
    }
    return result;
  }

  vector<string> SourceDBCasa::findDuplicatePatches()
  {
    return findDuplicates (itsPatchTable, "PATCHNAME");
  }

  vector<string> SourceDBCasa::findDuplicateSources()
  {
    return findDuplicates (itsSourceTable, "SOURCENAME");
  }

  bool SourceDBCasa::patchExists (const string& patchName)
  {
    TableLocker locker(itsPatchTable, FileLocker::Read);
    // See if existing.
    Table table = itsPatchTable(itsPatchTable.col("PATCHNAME") ==
                                String(patchName));
    return (table.nrow() > 0);
  }

  bool SourceDBCasa::sourceExists (const string& sourceName)
  {
    TableLocker locker(itsSourceTable, FileLocker::Read);
    // See if existing.
    Table table = itsSourceTable(itsSourceTable.col("SOURCENAME") ==
                                 String(sourceName));
    return (table.nrow() > 0);
  }

  uint SourceDBCasa::addPatch (const string& patchName, int catType,
                               double apparentBrightness,
                               double ra, double dec,
                               bool check)
  {
    itsPatchTable.reopenRW();
    TableLocker locker(itsPatchTable, FileLocker::Write);
    // See if already existing.
    if (check) {
      ASSERTSTR (!patchExists(patchName),
                 "Patch " << patchName << " already exists");
    }
    // Okay, add it to the patch table.
    ScalarColumn<String> nameCol(itsPatchTable, "PATCHNAME");
    ScalarColumn<uint>   catCol (itsPatchTable, "CATEGORY");
    ScalarColumn<double> brCol  (itsPatchTable, "APPARENT_BRIGHTNESS");
    ScalarColumn<double> raCol  (itsPatchTable, "RA");
    ScalarColumn<double> decCol (itsPatchTable, "DEC");
    uint rownr = itsPatchTable.nrow();
    itsPatchTable.addRow();
    nameCol.put (rownr, patchName);
    catCol.put  (rownr, catType);
    brCol.put   (rownr, apparentBrightness);
    raCol.put   (rownr, ra);
    decCol.put  (rownr, dec);
    return rownr;
  }

  void SourceDBCasa::addSource (const string& patchName,
                                const string& sourceName,
                                SourceInfo::Type sourceType,
                                const ParmMap& defaultParameters,
                                double ra, double dec,
                                bool check)
  {
    uint patchId;
    {
      // Find the patch.
      TableLocker locker(itsPatchTable, FileLocker::Read);
      Table table = itsPatchTable(itsPatchTable.col("PATCHNAME") ==
                                  String(patchName));
      ASSERTSTR (table.nrow() == 1,
                 "Patch " << patchName << " does not exist");
      patchId = table.rowNumbers()[0];
    }
    itsSourceTable.reopenRW();
    TableLocker locker(itsSourceTable, FileLocker::Write);
    if (check) {
      ASSERTSTR (!sourceExists(sourceName),
                 "Source " << sourceName << " already exists");
    }
    addSrc (patchId, sourceName, sourceType, defaultParameters, ra, dec);
  }

  void SourceDBCasa::addSource (const string& sourceName, int catType,
                                double apparentBrightness,
                                SourceInfo::Type sourceType,
                                const ParmMap& defaultParameters,
                                double ra, double dec,
                                bool check)
  {
    itsPatchTable.reopenRW();
    itsSourceTable.reopenRW();
    TableLocker lockerp(itsPatchTable, FileLocker::Write);
    TableLocker lockers(itsSourceTable, FileLocker::Write);
    if (check) {
      ASSERTSTR (!patchExists(sourceName),
                 "Patch " << sourceName << " already exists");
      ASSERTSTR (!sourceExists(sourceName),
                 "Source " << sourceName << " already exists");
    }
    uint patchId = addPatch (sourceName, catType, apparentBrightness,
                             ra, dec, false);
    addSrc (patchId, sourceName, sourceType, defaultParameters, ra, dec);
  }

  void SourceDBCasa::addSrc (uint patchId,
                             const string& sourceName,
                             SourceInfo::Type sourceType,
                             const ParmMap& defaultParameters,
                             double ra, double dec)
  {
    // Okay, add it to the source table.
    ScalarColumn<String> nameCol(itsSourceTable, "SOURCENAME");
    ScalarColumn<uint>   idCol  (itsSourceTable, "PATCHID");
    ScalarColumn<int>    typeCol(itsSourceTable, "SOURCETYPE");
    uint rownr = itsSourceTable.nrow();
    itsSourceTable.addRow();
    nameCol.put (rownr, sourceName);
    idCol.put   (rownr, patchId);
    typeCol.put (rownr, sourceType);
    // Now add the default parameters to the ParmDB DEFAULTVALUES table.
    bool foundRa = false;
    bool foundDec = false;
    for (ParmMap::const_iterator iter=defaultParameters.begin();
         iter!=defaultParameters.end(); ++iter) {
      if (iter->first == "Ra")  foundRa  = true;
      if (iter->first == "Dec") foundDec = true;
      getParmDB().putDefValue (iter->first + ':' + sourceName, iter->second);
    }
    // If Ra or Dec given and not in parameters, put it.
    // Use absolute perturbations for them.
    if (!foundRa  &&  ra != -1e9) {
      ParmValue pval(ra);
      getParmDB().putDefValue ("Ra:" + sourceName,
                               ParmValueSet(pval, ParmValue::Scalar,
                                            1e-6, false));
    }
    if (!foundDec  &&  dec != -1e9) {
      ParmValue pval(dec);
      getParmDB().putDefValue ("Dec:" + sourceName,
                               ParmValueSet(pval, ParmValue::Scalar,
                                            1e-6, false));
    }
  }

  void SourceDBCasa::deleteSources (const string& sourceNamePattern)
  {
    Table table = itsSourceTable;
    table.reopenRW();
    TableLocker locker(table, FileLocker::Write);
    // Get the selection from the patch table.
    Regex regex(Regex::fromPattern(sourceNamePattern));
    table = table (table.col("SOURCENAME") == regex);
    // Delete all rows found.
    itsSourceTable.removeRow (table.rowNumbers());
    // A patch will never be removed from the PATCH table, otherwise the
    // PATCHID keys (which are row numbers) do not match anymore.
    // Delete the sources from the ParmDB tables.
    string parmNamePattern = "*:" + sourceNamePattern;
    getParmDB().deleteDefValues (parmNamePattern);
    getParmDB().deleteValues (parmNamePattern, Box(Point(-1e30,-1e30),
                                                   Point( 1e30, 1e30)));
  }

  vector<string> SourceDBCasa::getPatches (int category, const string& pattern,
                                           double minBrightness,
                                           double maxBrightness)
  {
    TableLocker locker(itsPatchTable, FileLocker::Read);
    Table table = itsPatchTable;
    if (category >= 0) {
      table = table(table.col("CATEGORY") == category);
    }
    if (!pattern.empty()  &&  pattern != "*") {
      Regex regex(Regex::fromPattern(pattern));
      table = table(table.col("PATCHNAME") == regex);
    }
    if (minBrightness >= 0) {
      table = table(table.col("APPARENT_BRIGHTNESS") >= minBrightness);
    }
    if (maxBrightness >= 0) {
      table = table(table.col("APPARENT_BRIGHTNESS") <= maxBrightness);
    }
    Block<String> keys(2);
    Block<Int> orders(2);
    keys[0] = "CATEGORY";
    keys[1] = "APPARENT_BRIGHTNESS";
    orders[0] = Sort::Ascending;
    orders[1] = Sort::Descending;
    table = table.sort (keys, orders);
    Vector<String> nm(ROScalarColumn<String>(table, "PATCHNAME").getColumn());
    return vector<string>(nm.cbegin(), nm.cend());
  }

  vector<SourceInfo> SourceDBCasa::getPatchSources (const string& patchName)
  {
    TableLocker lockerp(itsPatchTable, FileLocker::Read);
    TableLocker lockers(itsSourceTable, FileLocker::Read);
    Table table = itsPatchTable(itsPatchTable.col("PATCHNAME") ==
                                String(patchName));
    if (table.nrow() == 0) {
      return vector<SourceInfo>();
    }
    ASSERT (table.nrow() == 1);
    uint patchid = table.rowNumbers()[0];
    table = itsSourceTable(itsSourceTable.col("PATCHID") == patchid);
    Vector<String> nm(ROScalarColumn<String>(table, "SOURCENAME").getColumn());
    Vector<int>    tp(ROScalarColumn<int>   (table, "SOURCETYPE").getColumn());
    vector<SourceInfo> res;
    res.reserve (nm.size());
    for (uint i=0; i<nm.size(); ++i) {
      res.push_back (SourceInfo(nm[i], SourceInfo::Type(tp[i])));
    }
    return res;
  }

  SourceInfo SourceDBCasa::getSource (const string& sourceName)
  {
    TableLocker lockers(itsSourceTable, FileLocker::Read);
    Table table = itsSourceTable(itsSourceTable.col("SOURCENAME") ==
                                 String(sourceName));
    ASSERT (table.nrow() == 1);
    return SourceInfo(ROScalarColumn<String>(table, "SOURCENAME")(0),
                      SourceInfo::Type(ROScalarColumn<int>(table,
                                                           "SOURCETYPE")(0)));
  }

  vector<SourceInfo> SourceDBCasa::getSources (const string& pattern)
  {
    TableLocker locker(itsSourceTable, FileLocker::Read);
    // Get the selection from the patch table.
    Regex regex(Regex::fromPattern(pattern));
    Table table = itsSourceTable (itsSourceTable.col("SOURCENAME") == regex);
    Vector<String> nm(ROScalarColumn<String>(table, "SOURCENAME").getColumn());
    Vector<int>    tp(ROScalarColumn<int>   (table, "SOURCETYPE").getColumn());
    vector<SourceInfo> res;
    res.reserve (nm.size());
    for (uint i=0; i<nm.size(); ++i) {
      res.push_back (SourceInfo(nm[i], SourceInfo::Type(tp[i])));
    }
    return res;
  }

} // namespace BBS
} // namespace LOFAR
