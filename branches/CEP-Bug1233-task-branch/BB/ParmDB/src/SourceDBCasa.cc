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

using namespace casa;

namespace LOFAR {
namespace BBS {

  SourceDBCasa::SourceDBCasa (const ParmDBMeta& pdm, bool forceNew)
    : SourceDBRep (pdm, forceNew)
  {
    string tableName = pdm.getTableName() + "/SourceDB";
    // Create the table if needed or if it does not exist yet.
    if (forceNew  ||  !Table::isReadable (tableName)) {
      createTables (tableName);
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
    TableDesc td("Local Sky Model Sources", TableDesc::Scratch);
    td.comment() = String("Table containing the sources in the Local Sky Model");
    td.addColumn (ScalarColumnDesc<String>("SOURCENAME"));
    td.addColumn (ScalarColumnDesc<uint>  ("PATCHID"));
    td.addColumn (ScalarColumnDesc<int>   ("SOURCETYPE"));
    td.addColumn (ScalarColumnDesc<double>("APPARENT_BRIGHTNESS"));
    td.addColumn (ScalarColumnDesc<double>("RA"));
    td.addColumn (ScalarColumnDesc<double>("DEC"));

    TableDesc tdpat("Local Sky Model patches", TableDesc::Scratch);
    tdpat.comment() = String("Table containing the patches in the Local Sky Model");
    tdpat.addColumn (ScalarColumnDesc<String>("NAME"));
    tdpat.addColumn (ScalarColumnDesc<int>   ("CATEGORY"));

    SetupNewTable newtab(tableName, td, Table::New);
    SetupNewTable newpattab(tableName+string("/PATCHES"), tdpat, Table::New);

    Table tab(newtab);
    Table pattab(newpattab);
    tab.rwKeywordSet().defineTable("PATCHES", pattab);  

    tab.tableInfo().setType ("LSM");
    tab.tableInfo().readmeAddLine ("Sources in the Local Sky Model");
    pattab.tableInfo().setType ("LSMpatches");
    pattab.tableInfo().readmeAddLine ("Patches in the Local Sky Model");
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
  }

  vector<string> SourceDBCasa::getCat1Patches()
  {
    TableLocker locker(itsPatchTable, FileLocker::Read);
    Table table = itsPatchTable(itsPatchTable.col("CATEGORY") == 1);
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
