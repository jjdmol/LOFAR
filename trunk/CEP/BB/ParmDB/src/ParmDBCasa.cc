//# ParmDBCasa.cc: Object to hold parameters in an Casa table.
//#
//# Copyright (C) 2002
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
#include <ParmDB/ParmDBCasa.h>
#include <Common/LofarLogger.h>

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
#include <tables/Tables/ColumnsIndex.h>
#include <casa/Containers/RecordField.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Utilities/Regex.h>

using namespace casa;
using namespace std;

namespace LOFAR {
namespace BBS {

  ParmDBCasa::ParmDBCasa (const string& tableName, bool forceNew)
  {
    // Create the table if needed or if it does not exist yet.
    if (forceNew  ||  !Table::isReadable (tableName)) {
      createTables (tableName);
    }
    // Open the main table.
    itsTables[0] = Table(tableName, TableLock::UserLocking);
    // Open the names table.
    itsTables[1] = itsTables[0].keywordSet().asTable ("NAMES");
    // Open the default values table.
    itsTables[2] = itsTables[0].keywordSet().asTable ("DEFAULTVALUES");
  }

  ParmDBCasa::~ParmDBCasa()
  {}

  void ParmDBCasa::lock (bool lockForWrite)
  {
    itsTables[0].lock (lockForWrite);
    itsTables[1].lock (lockForWrite);
    itsTables[2].lock (lockForWrite);
  }

  void ParmDBCasa::unlock()
  {
    itsTables[0].unlock();
    itsTables[1].unlock();
    itsTables[2].unlock();
  }

  void ParmDBCasa::createTables (const string& tableName)
  {
    TableDesc td("ME parameter table", TableDesc::Scratch);
    td.comment() = String("Table containing ME parameters values");
    td.addColumn (ScalarColumnDesc<uint>  ("NAMEID"));
    td.addColumn (ScalarColumnDesc<double>("STARTX"));
    td.addColumn (ScalarColumnDesc<double>("ENDX"));
    td.addColumn (ScalarColumnDesc<double>("STARTY"));
    td.addColumn (ScalarColumnDesc<double>("ENDY"));
    td.addColumn (ArrayColumnDesc<double> ("INTERVALSX"));
    td.addColumn (ArrayColumnDesc<double> ("INTERVALSY"));
    td.addColumn (ArrayColumnDesc<double> ("VALUES"));
    td.addColumn (ArrayColumnDesc<double> ("ERRORS"));

    TableDesc tdnam("ME parameter names", TableDesc::Scratch);
    tdnam.comment() = String("Table containing ME parameters names");
    tdnam.addColumn (ScalarColumnDesc<String>("NAME"));
    tdnam.addColumn (ScalarColumnDesc<int>   ("FUNKLETTYPE"));
    tdnam.addColumn (ScalarColumnDesc<double>("PERTURBATION"));
    tdnam.addColumn (ScalarColumnDesc<bool>  ("PERT_REL"));
    tdnam.addColumn (ArrayColumnDesc<bool>   ("SOLVABLE"));
    tdnam.addColumn (ScalarColumnDesc<int>   ("NX"));
    tdnam.addColumn (ScalarColumnDesc<int>   ("NY"));

    TableDesc tddef("ME default parameter values", TableDesc::Scratch);
    tddef.comment() = String("Table containing ME default parameter values");
    tddef.addColumn (ScalarColumnDesc<String>("NAME"));
    tddef.addColumn (ScalarColumnDesc<int>   ("FUNKLETTYPE"));
    tddef.addColumn (ScalarColumnDesc<double>("PERTURBATION"));
    tddef.addColumn (ScalarColumnDesc<bool>  ("PERT_REL"));
    tddef.addColumn (ArrayColumnDesc<bool>   ("SOLVABLE"));
    tddef.addColumn (ArrayColumnDesc<double> ("VALUES"));

    SetupNewTable newtab(tableName, td, Table::New);
    SetupNewTable newnamtab(tableName+string("/NAMES"), tdnam, Table::New);
    SetupNewTable newdeftab(tableName+string("/DEFAULTVALUES"), tddef,
                            Table::New);

    Table tab(newtab);
    Table namtab(newnamtab);
    Table deftab(newdeftab);
    tab.rwKeywordSet().defineTable("DEFAULTVALUES", deftab);  
    tab.rwKeywordSet().defineTable("NAMES", namtab);  
    namtab.rwKeywordSet().define("UNIQUE_ID", 0u);  

    tab.tableInfo().setType ("MEP");
    tab.tableInfo().readmeAddLine ("ME Parameter values");
    namtab.tableInfo().setType ("MEPname");
    namtab.tableInfo().readmeAddLine ("ME Parameter names");
    deftab.tableInfo().setType ("MEPinit");
    deftab.tableInfo().readmeAddLine ("Initial ME Parameter values");
  }

  void ParmDBCasa::clearTables()
  {
    for (int i=0; i<3; ++i) {
      TableLocker locker(itsTables[i], FileLocker::Write);
      Vector<uint> rows = itsTables[i].rowNumbers();
      itsTables[i].removeRow(rows);
    }
  }

  Table ParmDBCasa::getNameSel (const string& parmNamePattern) const
  {
    Table table = itsTables[1];
    TableLocker locker(table, FileLocker::Read);
    if (!parmNamePattern.empty()  &&  parmNamePattern != "*") {
      Regex regex(Regex::fromPattern(parmNamePattern));
      table = table(table.col("NAME") == regex);
    }
    return table;
  }

  int ParmDBCasa::getNameId (const std::string& parmName)
  {
    Table table = itsTables[1];
    TableLocker locker(table, FileLocker::Read);
    table = table(table.col("NAME") == String(parmName));
    if (table.nrow() == 0) {
      return -1;
    }
    ASSERT (table.nrow() == 1);
    // The row number forms the id.
    return table.rowNumbers()[0];
  }

  Vector<uint> ParmDBCasa::getNameIds (const string& parmNamePattern) const
  {
    Table table = itsTables[1];
    TableLocker locker(table, FileLocker::Read);
    if (!parmNamePattern.empty()  &&  parmNamePattern != "*") {
      Regex regex(Regex::fromPattern(parmNamePattern));
      table = table(table.col("NAME") == regex);
    }
    return table.rowNumbers();
  }

  Vector<uint> ParmDBCasa::getNameIds (const vector<string>& parmNames) const
  {
    Table table = itsTables[1];
    TableLocker locker(table, FileLocker::Read);
    if (!parmNames.empty()) {
      Vector<String> nams(parmNames.size());
      for (uint i=0; i<parmNames.size(); ++i) {
        nams(i) = parmNames[i];
      }
      table = table(table.col("NAME").in (nams));
    }
    return table.rowNumbers();
  }

  Box ParmDBCasa::getRange (const string& parmNamePattern) const
  {
    Table table = itsTables[0];
    TableLocker locker(table, FileLocker::Read);
    if (!parmNamePattern.empty()  &&  parmNamePattern != "*") {
      table = table(table.col("NAMEID").in (getNameIds(parmNamePattern)));
    }
    return findRange (table);
  }

  Box ParmDBCasa::getRange (const vector<string>& parmNames) const
  {
    Table table = itsTables[0];
    TableLocker locker(table, FileLocker::Read);
    if (!parmNames.empty()) {
      table = table(table.col("NAMEID").in (getNameIds(parmNames)));
    }
    return findRange (table);
  }

  Box ParmDBCasa::findRange (const Table& table) const
  {
    if (table.nrow() == 0) {
      return Box();
    }
    double sx = min (ROScalarColumn<double> (table,"STARTX").getColumn());
    double ex = max (ROScalarColumn<double> (table,"ENDX").getColumn());
    double sy = min (ROScalarColumn<double> (table,"STARTY").getColumn());
    double ey = max (ROScalarColumn<double> (table,"ENDY").getColumn());
    return Box (Point(sx,sy), Point(ex,ey));
  }

  void ParmDBCasa::fillDefMap (ParmMap& defMap)
  {
    defMap.clear();
    Table& table = itsTables[2];
    TableLocker locker(table, FileLocker::Read);
    for (uint row=0; row<table.nrow(); ++row) {
      pair<string,ParmValueSet> val = extractDefValue(table, row);
      defMap.define (val.first, val.second);
    }
  }

  pair<string,ParmValueSet> ParmDBCasa::extractDefValue (const Table& tab,
                                                         int row)
  {
    ROScalarColumn<String> nameCol (tab, "NAME");
    ROScalarColumn<int>    typeCol (tab, "FUNKLETTYPE");
    ROArrayColumn<bool>    maskCol (tab, "SOLVABLE");
    ROArrayColumn<double>  valCol  (tab, "VALUES");
    ROScalarColumn<double> pertCol (tab, "PERTURBATION");
    ROScalarColumn<bool>   prelCol (tab, "PERT_REL");
    ParmValue pval;
    Array<double> val = valCol(row);
    ParmValue::FunkletType type = ParmValue::FunkletType(typeCol(row));
    if (type == ParmValue::Scalar) {
      ASSERT(val.size() == 1);
      pval.setScalars (Grid(), val);
    } else {
      pval.setCoeff (val);
    }
    ParmValueSet valset(pval, type, pertCol(row), prelCol(row));
    if (maskCol.isDefined(row)) {
      valset.setSolvableMask (maskCol(row));
    }
    return make_pair(nameCol(row), valset);
  }

  void ParmDBCasa::getValues (vector<ParmValueSet>& psets,
                              const vector<uint>& nameIds,
                              const vector<ParmId>& parmIds,
                              const Box& domain)
  {
    Table table = itsTables[0];
    TableLocker locker0(table, FileLocker::Read);
    Table& nmtab = itsTables[1];
    TableLocker locker1(nmtab, FileLocker::Read);
    // Select the requested domains.
    TableExprNode expr = makeExpr (table, domain);
    if (! expr.isNull()) {
      table = table(expr);
    }
    Vector<uInt> origRownrs = table.rowNumbers();
    // Create the table accessor objects.
    ROScalarColumn<String> nameCol(nmtab, "NAME");
    ROScalarColumn<int>    typeCol(nmtab, "FUNKLETTYPE");
    ROScalarColumn<double> pertCol(nmtab, "PERTURBATION");
    ROScalarColumn<bool>   prelCol(nmtab, "PERT_REL");
    ROArrayColumn<bool>    maskCol(nmtab, "SOLVABLE");
    ROScalarColumn<double> sxCol (table, "STARTX");
    ROScalarColumn<double> exCol (table, "ENDX");
    ROScalarColumn<double> syCol (table, "STARTY");
    ROScalarColumn<double> eyCol (table, "ENDY");
    ROArrayColumn<double>  ivxCol(table, "INTERVALSX");
    ROArrayColumn<double>  ivyCol(table, "INTERVALSY");
    ROArrayColumn<double>  valCol(table, "VALUES");
    // Form an index for the nameids.
    ColumnsIndex colInx(table, "NAMEID");
    // Create an accessor for the key in the index,
    RecordFieldPtr<uint> idFld(colInx.accessKey(), "NAMEID");
    // Loop through the required nameids and retrieve their info.
    for (uint inx=0; inx<nameIds.size(); ++inx) {
      ParmValueSet& pvset = psets[parmIds[inx]];
      uint id = nameIds[inx];
      ParmValue::FunkletType type = ParmValue::FunkletType(typeCol(id));
      // Select the rows for the nameId.
      *idFld = id;
      Vector<uint> rownrs = colInx.getRowNumbers();
      uint nrow = rownrs.nelements();
      if (nrow > 0) {
        // Retrieve the rows.
        vector<ParmValue::ShPtr> values;
        vector<Box> domains;
        values.reserve (nrow);
        domains.reserve (nrow);
        for (uint i=0; i<nrow; ++i) {
          int row = rownrs[i];
          double sx = sxCol(row);
          double sy = syCol(row);
          double ex = exCol(row);
          double ey = eyCol(row);
          ParmValue::ShPtr pval = ParmValue::ShPtr (new ParmValue);
          if (type != ParmValue::Scalar) {
            pval->setCoeff (valCol(row));
          } else {
            Array<double> values = valCol(row);
            uint nx = values.shape()[0];
            uint ny = values.shape()[1];
            pval->setScalars (Grid(getInterval(ivxCol, row, sx, ex, nx),
                                   getInterval(ivyCol, row, sy, ey, ny)),
                              values);
          }
          pval->setRowId (origRownrs[row]);
          values.push_back (pval);
          domains.push_back (Box(Point(sx,sy), Point(ex,ey)));
        }
        pvset = ParmValueSet (domains, values, ParmValue(),
                              type, pertCol(id), prelCol(id));
      } else {
        // No matching values, so get default value.
        // Use perturbation, etc. from NAMES table.
        ParmValueSet pvdef = getDefValue (nameCol(id), ParmValue());
        pvset = ParmValueSet (pvdef.getFirstParmValue(),
                              type, pertCol(id), prelCol(id));
      }
      if (maskCol.isDefined(id)) {
        pvset.setSolvableMask (maskCol(id));
      }
    }
  }

  void ParmDBCasa::getDefValues (ParmMap& result,
                                 const string& parmNamePattern)
  {
    TableLocker locker(itsTables[2], FileLocker::Read);
    // Find all rows.
    Table& table = itsTables[2];
    Regex regex(Regex::fromPattern(parmNamePattern));
    Table sel = table(table.col("NAME") == regex);
    ROScalarColumn<String> nameCol(sel, "NAME");
    for (uint row=0; row<sel.nrow(); ++row) {
      pair<string,ParmValueSet> pset (extractDefValue(sel, row));
      result.define (pset.first, pset.second);
    }
  }

  void ParmDBCasa::putValues (const string& name, int& nameId, ParmValueSet& pset)
  {
    itsTables[0].reopenRW();
    TableLocker locker(itsTables[0], FileLocker::Write);
    doPutValue (name, nameId, pset);
  }

  void ParmDBCasa::doPutValue (const string& name, int& nameId,
                               ParmValueSet& pset)
  {
    const Grid& grid = pset.getGrid();
    for (uint i=0; i<pset.size(); ++i) {
      ParmValue& pval = pset.getParmValue(i);
      if (pval.getRowId() < 0) {
        // It is certainly a new row.
        putNewValue (name, nameId, pset, pval, grid.getCell(i));
      } else {
        // It is an existing row.
        putOldValue (pval, pset.getType());
      }
    }
  }

  void ParmDBCasa::putOldValue (const ParmValue& pval,
                                ParmValue::FunkletType type)
  {
    Table& table = itsTables[0];
    ArrayColumn<double>  valCol  (table, "VALUES");
    ArrayColumn<double>  errCol  (table, "ERRORS");
    // Put the existing ParmValue.
    int rownr = pval.getRowId();
    IPosition oldShape = valCol.shape (rownr);
    valCol.put (rownr, pval.getValues());
    if (pval.hasErrors()) {
      errCol.put (rownr, pval.getErrors());
    }
    // If the value shape is different, the domains have changed.
    if (! oldShape.isEqual (pval.getValues().shape())) {
      ScalarColumn<double> stxCol  (table, "STARTX");
      ScalarColumn<double> endxCol (table, "ENDX");
      ScalarColumn<double> styCol  (table, "STARTY");
      ScalarColumn<double> endyCol (table, "ENDY");
      ArrayColumn<double>  ivxCol  (table, "INTERVALSX");
      ArrayColumn<double>  ivyCol  (table, "INTERVALSY");
      const Grid& pvGrid = pval.getGrid();
      Box domain = pvGrid.getBoundingBox();
      stxCol.put (rownr, domain.lowerX());
      endxCol.put (rownr, domain.upperX());
      styCol.put (rownr, domain.lowerY());
      endyCol.put (rownr, domain.upperY());
      // Write irregular intervals if needed.
      if (type == ParmValue::Scalar) {
        if (! pvGrid.getAxis(0)->isRegular()) {
          putInterval (*pvGrid.getAxis(0), ivxCol, rownr);
        } else {
          if (ivxCol.isDefined (rownr)) {
            // Was irregular, but is regular now. So remove.
            ivxCol.put (rownr, Array<double>());
          }
        }
        if (! pvGrid.getAxis(1)->isRegular()) {
          putInterval (*pvGrid.getAxis(1), ivyCol, rownr);
        } else {
          if (ivyCol.isDefined (rownr)) {
            ivyCol.put (rownr, Array<double>());
          }
        }
      }
    }
  }

  void ParmDBCasa::putNewValue (const string& parmName, int& nameId,
                                ParmValueSet& pset, ParmValue& pval,
                                const Box& domain)
  {
    // First check if name has to be added to name table.
    if (nameId < 0) {
      nameId = putName (parmName, pset);
    }
    Table& table = itsTables[0];
    uint rownr = table.nrow();
    ScalarColumn<uint>   idCol   (table, "NAMEID");
    ScalarColumn<double> stxCol  (table, "STARTX");
    ScalarColumn<double> endxCol (table, "ENDX");
    ScalarColumn<double> styCol  (table, "STARTY");
    ScalarColumn<double> endyCol (table, "ENDY");
    ArrayColumn<double>  ivxCol  (table, "INTERVALSX");
    ArrayColumn<double>  ivyCol  (table, "INTERVALSY");
    ArrayColumn<double>  valCol  (table, "VALUES");
    ArrayColumn<double>  errCol  (table, "ERRORS");
    // Create a new row for the ParmValue.
    table.addRow();
    idCol.put (rownr, nameId);
    stxCol.put (rownr, domain.lowerX());
    endxCol.put (rownr, domain.upperX());
    styCol.put (rownr, domain.lowerY());
    endyCol.put (rownr, domain.upperY());
    if (pset.getType() == ParmValue::Scalar) {
      const Grid& pvGrid = pval.getGrid();
      if (! pvGrid.getAxis(0)->isRegular()) {
        putInterval (*pvGrid.getAxis(0), ivxCol, rownr);
      }
      if (! pvGrid.getAxis(1)->isRegular()) {
        putInterval (*pvGrid.getAxis(1), ivyCol, rownr);
      }
    }
    valCol.put (rownr, pval.getValues());
    if (pval.hasErrors()) {
      errCol.put (rownr, pval.getErrors());
    }
    // Remember where the value is stored.
    pval.setRowId (rownr);
  }

  void ParmDBCasa::putInterval (const Axis& axis, ArrayColumn<double>& col,
                                uint rownr)
  {
    int nv = axis.size();
    Array<double> arr(IPosition(2,2,nv));
    double* arrp = arr.data();
    for (int i=0; i<nv; ++i) {
      *arrp++ = axis.center(i);
      *arrp++ = axis.width(i);
    }
    col.put (rownr, arr);
  }

  Axis::ShPtr ParmDBCasa::getInterval (ROArrayColumn<double>& col, uint rownr,
                                       double st, double end, uint n)
  {
    if (! col.isDefined(rownr)) {
      return Axis::ShPtr(new RegularAxis (st, end, n, true));
    }
    Array<double> arr = col(rownr);
    if (arr.size() == 0) {
      return Axis::ShPtr(new RegularAxis (st, end, n, true));
    }
    ASSERT (arr.shape()[1] == int(n));
    const double* arrp = arr.data();
    vector<double> vc, vw;
    vc.reserve(n);
    vw.reserve(n);
    for (uint i=0; i<n; ++i) {
      vc.push_back (*arrp++);
      vw.push_back (*arrp++);
    }
    return Axis::ShPtr(new OrderedAxis (vc, vw, false));
  }

  int ParmDBCasa::putName (const string& name, const ParmValueSet& pset)
  {
    Table& table = itsTables[1];
    table.reopenRW();
    TableLocker locker(table, FileLocker::Write);
    ScalarColumn<String> nameCol(table, "NAME");
    ScalarColumn<int>    typeCol(table, "FUNKLETTYPE");
    ScalarColumn<double> pertCol(table, "PERTURBATION");
    ScalarColumn<bool>   prelCol(table, "PERT_REL");
    ArrayColumn<bool>    maskCol(table, "SOLVABLE");
    uint rownr = table.nrow();
    table.addRow();
    // Create a unique id.
    uint id = table.keywordSet().asuInt("UNIQUE_ID");
    table.rwKeywordSet().define ("UNIQUE_ID", id+1);
    ASSERTSTR (rownr == id,
               "It looks as if a row has been deleted from the NAME table");
    nameCol.put (rownr, name);
    typeCol.put (rownr, pset.getType());
    pertCol.put (rownr, pset.getPerturbation());
    prelCol.put (rownr, pset.getPertRel());
    maskCol.put (rownr, pset.getSolvableMask());
    table.flush();
    return id;
  }

  void ParmDBCasa::putDefValue (const string& name, const ParmValueSet& pset)
  {
    itsTables[2].reopenRW();
    TableLocker locker(itsTables[2], FileLocker::Write);
    const ParmValue& pval = pset.getFirstParmValue();
    // First see if the parameter name exists at all.
    Table& table = itsTables[2];
    Table sel = table(table.col("NAME") == String(name));
    if (sel.nrow() == 1) {
      uint rownr=0;
      ScalarColumn<int>    typeCol (sel, "FUNKLETTYPE");
      ArrayColumn<bool>    maskCol (sel, "SOLVABLE");
      ArrayColumn<double>  valCol  (sel, "VALUES");
      ScalarColumn<double> pertCol (sel, "PERTURBATION");
      ScalarColumn<bool>   prelCol (sel, "PERT_REL");
      typeCol.put (rownr, pset.getType());
      valCol.put (rownr, pval.getValues());
      if (pset.getSolvableMask().size() > 0  ||  maskCol.isDefined(rownr)) {
        maskCol.put (rownr, pset.getSolvableMask());
      }
      pertCol.put (rownr, pset.getPerturbation());
      prelCol.put (rownr, pset.getPertRel());
    } else if (sel.nrow() == 0) {
      putNewDefValue (name, pset);
    } else {
      ASSERTSTR (false, "Too many default parms with the same name/domain");
    }
    clearDefFilled();
  }

  void ParmDBCasa::putNewDefValue (const string& name, const ParmValueSet& pset)
  {
    const ParmValue& pval = pset.getFirstParmValue();
    Table& table = itsTables[2];
    uint rownr = table.nrow();
    table.addRow();
    ScalarColumn<String> nameCol (table, "NAME");
    ScalarColumn<int>    typeCol (table, "FUNKLETTYPE");
    ArrayColumn<bool>    maskCol (table, "SOLVABLE");
    ArrayColumn<double>  valCol  (table, "VALUES");
    ScalarColumn<double> pertCol (table, "PERTURBATION");
    ScalarColumn<bool>   prelCol (table, "PERT_REL");
    nameCol.put (rownr, name);
    typeCol.put (rownr, pset.getType());
    valCol.put (rownr, pval.getValues());
    if (pset.getSolvableMask().size() > 0) {
      maskCol.put (rownr, pset.getSolvableMask());
    }
    pertCol.put (rownr, pset.getPerturbation());
    prelCol.put (rownr, pset.getPertRel());
    clearDefFilled();
  }

  void ParmDBCasa::deleteValues (const string& parmNamePattern,
                                 const Box& domain)
  {
    Table& table = itsTables[0];
    table.reopenRW();
    TableLocker locker(table, FileLocker::Write);
    // Get the selection from the name table.
    Table nameSel = getNameSel (parmNamePattern);
    // Find all rows. Test if a domain selection was given.
    TableExprNode expr = makeExpr (table, domain);
    andExpr (expr, table.col("NAMEID").in (nameSel.rowNumbers()));
    Table sel = table(expr);
    // Delete all rows found.
    table.removeRow (sel.rowNumbers (table));
    // A name will never be removed from the NAME table, otherwise the
    // NAMEID keys (which are row numbers) do not match anymore.
  }

  void ParmDBCasa::deleteDefValues (const string& parmNamePattern)
  {
    Table& table = itsTables[2];
    table.reopenRW();
    TableLocker locker(table, FileLocker::Write);
    // Find all rows.
    Regex regex(Regex::fromPattern(parmNamePattern));
    Table sel = table(table.col("NAME") == regex);
    // Delete all rows found.
    table.removeRow (sel.rowNumbers (table));
    clearDefFilled();
  }

  Table ParmDBCasa::find (const string& parmName,
                          const Box& domain)
  {
    Table& table = itsTables[0];
    TableLocker locker(table, FileLocker::Read);
    // Find all rows overlapping the requested domain.
    TableExprNode expr = makeExpr (table, domain);
    andExpr (expr, table.col("NAMEID").in
             (getNameIds(vector<string>(1, parmName))));
    return table(expr);
  }

  vector<string> ParmDBCasa::getNames (const string& parmNamePattern)
  {
    // Get all parm rows where the name matches the pattern.
    Table table = itsTables[1];
    TableLocker locker(table, FileLocker::Read);
    if (!parmNamePattern.empty()  &&  parmNamePattern != "*") {
      Regex regex(Regex::fromPattern(parmNamePattern));
      table = table(table.col("NAME") == regex);
    }
    Vector<String> names = ROScalarColumn<String>(table,"NAME").getColumn();
    LOG_TRACE_STAT_STR("Finished retrieving "<<names.size()<<" names");
    return vector<string> (names.cbegin(), names.cend());
  }

  TableExprNode ParmDBCasa::makeExpr (const Table& table,
                                      const Box& domain) const
  {
    TableExprNode expr;
    if (domain.lowerX() < domain.upperX()) {
      TableExprNode s (table.col("STARTX"));
      TableExprNode e (table.col("ENDX"));
      andExpr (expr,
               domain.lowerX() < e  &&  !near(domain.lowerX(), e, 1e-12)  &&
               domain.upperX() > s  &&  !near(domain.upperX(), s, 1e-12));
    }
    if (domain.lowerY() < domain.upperY()) {
      TableExprNode s (table.col("STARTY"));
      TableExprNode e (table.col("ENDY"));
      andExpr (expr,
               domain.lowerY() < e  &&  !near(domain.lowerY(), e, 1e-12)  &&
               domain.upperY() > s  &&  !near(domain.upperY(), s, 1e-12));
    }
    return expr;
  }

  void ParmDBCasa::andExpr (TableExprNode& expr,
                            const TableExprNode& right) const
  {
    if (expr.isNull()) {
      expr = right;
    } else {
      expr = expr && right;
    }
  }


} // namespace BBS
} // namespace LOFAR
