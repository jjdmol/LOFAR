//# SourceDBCasa.cc: Class for a Casa table holding sources and their parameters
//#
//# Copyright (C) 2008
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
    : SourceDBRep   (pdm, forceNew),
      itsSetsFilled (false)
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
    // Lock all tables involved to avoid unwanted flushes.
    getParmDB().lock (lockForWrite);
    itsSourceTable.lock (lockForWrite);
    itsPatchTable.lock (lockForWrite);
  }

  void SourceDBCasa::unlock()
  {
    itsPatchTable.unlock();
    itsSourceTable.unlock();
    getParmDB().unlock();
  }

  void SourceDBCasa::createTables (const string& tableName)
  {
    // Create description of SOURCES.
    TableDesc td("Local Sky Model Sources", TableDesc::Scratch);
    td.comment() = String("Table containing the sources in the Local Sky Model");
    td.addColumn (ScalarColumnDesc<String>("SOURCENAME"));
    td.addColumn (ScalarColumnDesc<uint>  ("PATCHID"));
    td.addColumn (ScalarColumnDesc<int>   ("SOURCETYPE"));
    td.addColumn (ScalarColumnDesc<uint>  ("SPINX_NTERMS"));
    td.addColumn (ScalarColumnDesc<double>("SPINX_REFFREQ"));
    td.addColumn (ScalarColumnDesc<bool>  ("USE_ROTMEAS"));
    td.addColumn (ScalarColumnDesc<double>("SHAPELET_ISCALE"));
    td.addColumn (ScalarColumnDesc<double>("SHAPELET_QSCALE"));
    td.addColumn (ScalarColumnDesc<double>("SHAPELET_USCALE"));
    td.addColumn (ScalarColumnDesc<double>("SHAPELET_VSCALE"));
    td.addColumn (ArrayColumnDesc<double> ("SHAPELET_ICOEFF"));
    td.addColumn (ArrayColumnDesc<double> ("SHAPELET_QCOEFF"));
    td.addColumn (ArrayColumnDesc<double> ("SHAPELET_UCOEFF"));
    td.addColumn (ArrayColumnDesc<double> ("SHAPELET_VCOEFF"));
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

  void SourceDBCasa::fillSets()
  {
    if (!itsSetsFilled) {
      TableLocker plocker(itsPatchTable, FileLocker::Read);
      ROScalarColumn<String> patchCol(itsPatchTable, "PATCHNAME");
      itsPatchSet.clear();
      for (uint i=0; i<itsPatchTable.nrow(); ++i) {
        itsPatchSet.insert (patchCol(i));
      }
      TableLocker slocker(itsSourceTable, FileLocker::Read);
      ROScalarColumn<String> sourceCol(itsSourceTable, "SOURCENAME");
      itsSourceSet.clear();
      for (uint i=0; i<itsSourceTable.nrow(); ++i) {
        itsSourceSet.insert (sourceCol(i));
      }
      itsSetsFilled = true;
    }
  }

  bool SourceDBCasa::patchExists (const string& patchName)
  {
    if (!itsSetsFilled) {
      fillSets();
    }
    return itsPatchSet.find(patchName) != itsPatchSet.end();
  }

  bool SourceDBCasa::sourceExists (const string& sourceName)
  {
    if (!itsSetsFilled) {
      fillSets();
    }
    return itsSourceSet.find(sourceName) != itsSourceSet.end();
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
    itsPatchSet.insert (patchName);
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

  void SourceDBCasa::addSource (const SourceInfo& sourceInfo,
                                const string& patchName,
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
      ASSERTSTR (!sourceExists(sourceInfo.getName()),
                 "Source " << sourceInfo.getName() << " already exists");
    }
    itsSourceSet.insert (sourceInfo.getName());
    addSrc (sourceInfo, patchId, defaultParameters, ra, dec);
  }

  void SourceDBCasa::addSource (const SourceInfo& sourceInfo,
                                int catType,
                                double apparentBrightness,
                                const ParmMap& defaultParameters,
                                double ra, double dec,
                                bool check)
  {
    itsPatchTable.reopenRW();
    itsSourceTable.reopenRW();
    TableLocker lockerp(itsPatchTable, FileLocker::Write);
    TableLocker lockers(itsSourceTable, FileLocker::Write);
    if (check) {
      ASSERTSTR (!patchExists(sourceInfo.getName()),
                 "Patch " << sourceInfo.getName() << " already exists");
      ASSERTSTR (!sourceExists(sourceInfo.getName()),
                 "Source " << sourceInfo.getName() << " already exists");
    }
    itsPatchSet.insert  (sourceInfo.getName());
    itsSourceSet.insert (sourceInfo.getName());
    uint patchId = addPatch (sourceInfo.getName(), catType,
                             apparentBrightness, ra, dec, false);
    addSrc (sourceInfo, patchId, defaultParameters, ra, dec);
  }

  void SourceDBCasa::addSrc (const SourceInfo& sourceInfo,
                             uint patchId,
                             const ParmMap& defaultParameters,
                             double ra, double dec)
  {
    // Okay, add it to the source table.
    ScalarColumn<String> nameCol (itsSourceTable, "SOURCENAME");
    ScalarColumn<uint>   idCol   (itsSourceTable, "PATCHID");
    ScalarColumn<int>    typeCol (itsSourceTable, "SOURCETYPE");
    ScalarColumn<uint>   spinxCol(itsSourceTable, "SPINX_NTERMS");
    ScalarColumn<double> sirefCol(itsSourceTable, "SPINX_REFFREQ");
    ScalarColumn<bool>   usermCol(itsSourceTable, "USE_ROTMEAS");
    ScalarColumn<double> iscalCol(itsSourceTable, "SHAPELET_ISCALE");
    ScalarColumn<double> qscalCol(itsSourceTable, "SHAPELET_QSCALE");
    ScalarColumn<double> uscalCol(itsSourceTable, "SHAPELET_USCALE");
    ScalarColumn<double> vscalCol(itsSourceTable, "SHAPELET_VSCALE");
    ArrayColumn<double>  icoefCol(itsSourceTable, "SHAPELET_ICOEFF");
    ArrayColumn<double>  qcoefCol(itsSourceTable, "SHAPELET_QCOEFF");
    ArrayColumn<double>  ucoefCol(itsSourceTable, "SHAPELET_UCOEFF");
    ArrayColumn<double>  vcoefCol(itsSourceTable, "SHAPELET_VCOEFF");
    uint rownr = itsSourceTable.nrow();
    itsSourceTable.addRow();
    nameCol.put  (rownr, sourceInfo.getName());
    idCol.put    (rownr, patchId);
    typeCol.put  (rownr, sourceInfo.getType());
    spinxCol.put (rownr, sourceInfo.getSpectralIndexNTerms());
    sirefCol.put (rownr, sourceInfo.getSpectralIndexRefFreq());
    usermCol.put (rownr, sourceInfo.getUseRotationMeasure());
    iscalCol.put (rownr, sourceInfo.getShapeletScaleI());
    qscalCol.put (rownr, sourceInfo.getShapeletScaleQ());
    uscalCol.put (rownr, sourceInfo.getShapeletScaleU());
    vscalCol.put (rownr, sourceInfo.getShapeletScaleV());
    if (sourceInfo.getType() == SourceInfo::SHAPELET) {
      ASSERTSTR (! sourceInfo.getShapeletCoeffI().empty(),
                 "No coefficients defined for shapelet source "
                 << sourceInfo.getName());
      icoefCol.put (rownr, sourceInfo.getShapeletCoeffI());
      qcoefCol.put (rownr, sourceInfo.getShapeletCoeffQ());
      ucoefCol.put (rownr, sourceInfo.getShapeletCoeffU());
      vcoefCol.put (rownr, sourceInfo.getShapeletCoeffV());
    }
    // Now add the default parameters to the ParmDB DEFAULTVALUES table.
    bool foundRa = false;
    bool foundDec = false;
    for (ParmMap::const_iterator iter=defaultParameters.begin();
         iter!=defaultParameters.end(); ++iter) {
      if (iter->first == "Ra")  foundRa  = true;
      if (iter->first == "Dec") foundDec = true;
      getParmDB().putDefValue (iter->first + ':' + sourceInfo.getName(),
                               iter->second);
    }
    // If Ra or Dec given and not in parameters, put it.
    // Use absolute perturbations for them.
    if (!foundRa  &&  ra != -1e9) {
      ParmValue pval(ra);
      getParmDB().putDefValue ("Ra:" + sourceInfo.getName(),
                               ParmValueSet(pval, ParmValue::Scalar,
                                            1e-6, false));
    }
    if (!foundDec  &&  dec != -1e9) {
      ParmValue pval(dec);
      getParmDB().putDefValue ("Dec:" + sourceInfo.getName(),
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
    ASSERTSTR (table.nrow() == 1, "Patch name " << patchName
               << " multiply defined in " << itsPatchTable.tableName());
    uint patchid = table.rowNumbers()[0];
    table = itsSourceTable(itsSourceTable.col("PATCHID") == patchid);
    return readSources(table);
  }

  SourceInfo SourceDBCasa::getSource (const string& sourceName)
  {
    TableLocker lockers(itsSourceTable, FileLocker::Read);
    Table table = itsSourceTable(itsSourceTable.col("SOURCENAME") ==
                                 String(sourceName));
    ASSERTSTR (table.nrow() > 0,  "Source name " << sourceName
               << " not found in " << itsSourceTable.tableName());
    ASSERTSTR (table.nrow() == 1,  "Source name " << sourceName
               << " multiply defined in " << itsSourceTable.tableName());
    return readSources(table)[0];
  }

  vector<SourceInfo> SourceDBCasa::getSources (const string& pattern)
  {
    TableLocker locker(itsSourceTable, FileLocker::Read);
    // Get the selection from the patch table.
    Regex regex(Regex::fromPattern(pattern));
    Table table = itsSourceTable (itsSourceTable.col("SOURCENAME") == regex);
    return readSources(table);
  }

  vector<SourceInfo> SourceDBCasa::readSources (const Table& table)
  {
    Vector<String> nm(ROScalarColumn<String>(table, "SOURCENAME").getColumn());
    Vector<int>    tp(ROScalarColumn<int>   (table, "SOURCETYPE").getColumn());
    vector<SourceInfo> res;
    res.reserve (nm.size());
    if (table.tableDesc().isColumn("SPINX_NTERMS")) {
      Vector<uint>   sd(ROScalarColumn<uint>(table,"SPINX_NTERMS").getColumn());
      Vector<double> sr(ROScalarColumn<double>(table,"SPINX_REFFREQ").getColumn());
      Vector<bool>   rm(ROScalarColumn<bool>(table,"USE_ROTMEAS").getColumn());
      ROScalarColumn<double> iscalCol(table, "SHAPELET_ISCALE");
      ROScalarColumn<double> qscalCol(table, "SHAPELET_QSCALE");
      ROScalarColumn<double> uscalCol(table, "SHAPELET_USCALE");
      ROScalarColumn<double> vscalCol(table, "SHAPELET_VSCALE");
      ROArrayColumn<double>  icoefCol(table, "SHAPELET_ICOEFF");
      ROArrayColumn<double>  qcoefCol(table, "SHAPELET_QCOEFF");
      ROArrayColumn<double>  ucoefCol(table, "SHAPELET_UCOEFF");
      ROArrayColumn<double>  vcoefCol(table, "SHAPELET_VCOEFF");
      for (uint i=0; i<nm.size(); ++i) {
        SourceInfo::Type type = SourceInfo::Type((tp[i]));
        res.push_back (SourceInfo(nm[i], type, sd[i], sr[i], rm[i]));
        if (type == SourceInfo::SHAPELET) {
          ASSERTSTR (icoefCol.isDefined(i), "No coefficients defined for "
                     " shapelet source " << nm[i]);
          res[i].setShapeletScale (iscalCol(i), qscalCol(i),
                                   uscalCol(i), vscalCol(i));
          res[i].setShapeletCoeff (icoefCol(i), qcoefCol(i),
                                   ucoefCol(i), vcoefCol(i));
        }
      }
    } else {
      // Columns SPINX_NTERMS, SPINX_REFFREQ, and USE_ROTMEAS were added later,
      // so be backward compatible.
      // In this case get degree and reffreq from associated parmdb.
      for (uint i=0; i<nm.size(); ++i) {
        ParmMap parmd;
        int degree = -1;
        double refFreq = 0.;
        getParmDB().getDefValues (parmd, nm[i] + ":SpectralIndexDegree");
        if (parmd.size() == 1) {
          degree = *(parmd.begin()->second.getDefParmValue().
                     getValues().data());
          ParmMap parmf;
          getParmDB().getDefValues (parmf, nm[i] + ":SpectralIndexDegree");
          if (parmf.size() == 1) {
            refFreq = *(parmf.begin()->second.getDefParmValue().
                        getValues().data());
          }
        }
        res.push_back (SourceInfo(nm[i], SourceInfo::Type(tp[i]),
                                  degree+1, refFreq));
      }
    }
    return res;
  }

} // namespace BBS
} // namespace LOFAR
