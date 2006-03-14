//# ParmDBAIPS.cc: Object to hold parameters in an AIPS++ table.
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
#include <ParmDB/ParmDBAIPS.h>
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
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicMath/Math.h>
//# Include this file for automatic instantiation of Vector<vector>.
#include <casa/Arrays/Vector2.cc>

using namespace casa;
using namespace std;

namespace LOFAR {
namespace ParmDB {

ParmDBAIPS::ParmDBAIPS (const string& tableName, bool forceNew)
{
  // Create the table if needed or if it does not exist yet.
  if (forceNew  ||  !Table::isReadable (tableName)) {
    createTables (tableName);
  }
  // Open the main table.
  itsTables[0] = Table(tableName, TableLock::UserLocking);
  // Open the old table.
  itsTables[1] = itsTables[0].keywordSet().asTable ("OLDVALUES");
  // Open the default values table.
  itsTables[2] = itsTables[0].keywordSet().asTable ("DEFAULTVALUES");
}

ParmDBAIPS::~ParmDBAIPS()
{}

void ParmDBAIPS::lock (bool lockForWrite)
{
  itsTables[0].lock (lockForWrite);
  itsTables[1].lock (lockForWrite);
  itsTables[2].lock (lockForWrite);
}

void ParmDBAIPS::unlock()
{
  itsTables[0].unlock();
  itsTables[1].unlock();
  itsTables[2].unlock();
}

void ParmDBAIPS::createTables (const string& tableName)
{
  TableDesc td("ME parameter table", TableDesc::Scratch);
  td.comment() = String("Table containing parameters for ME");
  td.addColumn (ScalarColumnDesc<String>("NAME"));
  td.addColumn (ScalarColumnDesc<String>("TYPE"));
  td.addColumn (ArrayColumnDesc<double> ("CONSTANTS", 1));
  td.addColumn (ScalarColumnDesc<double> ("STARTX", 1));
  td.addColumn (ScalarColumnDesc<double> ("ENDX", 1));
  td.addColumn (ScalarColumnDesc<double> ("STARTY", 1));
  td.addColumn (ScalarColumnDesc<double> ("ENDY", 1));
  td.addColumn (ArrayColumnDesc<double> ("START", 1));
  td.addColumn (ArrayColumnDesc<double> ("END", 1));
  td.addColumn (ArrayColumnDesc <double>("VALUES"));
  td.addColumn (ArrayColumnDesc <double>("ERRORS"));
  td.addColumn (ArrayColumnDesc <bool>  ("SOLVABLE"));
  td.addColumn (ArrayColumnDesc<double> ("OFFSET", 1));
  td.addColumn (ArrayColumnDesc<double> ("SCALE", 1));
  td.addColumn (ScalarColumnDesc<double>("PERTURBATION"));
  td.addColumn (ScalarColumnDesc<bool>  ("PERT_REL"));
  td.addColumn (ScalarColumnDesc<double>("WEIGHT"));
  td.addColumn (ScalarColumnDesc<int>   ("ID"));
  td.addColumn (ScalarColumnDesc<int>   ("PARENTID"));

  TableDesc tddef("ME default parameter values", TableDesc::Scratch);
  tddef.comment() = String("Table containing default parameters for ME");
  tddef.addColumn (ScalarColumnDesc<String>("NAME"));
  tddef.addColumn (ScalarColumnDesc<String>("TYPE"));
  tddef.addColumn (ArrayColumnDesc<double> ("CONSTANTS", 1));
  tddef.addColumn (ArrayColumnDesc <double>("VALUES"));
  tddef.addColumn (ArrayColumnDesc <bool>  ("SOLVABLE"));
  tddef.addColumn (ScalarColumnDesc<double>("PERTURBATION"));
  tddef.addColumn (ScalarColumnDesc<bool>  ("PERT_REL"));

  SetupNewTable newtab(tableName, td, Table::New);
  SetupNewTable newoldtab(tableName+string("/OLDVALUES"), td, Table::New);
  SetupNewTable newdeftab(tableName+string("/DEFAULTVALUES"), tddef,
			  Table::New);

  Table tab(newtab);
  Table deftab(newdeftab);
  Table oldtab(newoldtab);
  tab.rwKeywordSet().defineTable("DEFAULTVALUES", deftab);  
  tab.rwKeywordSet().defineTable("OLDVALUES", oldtab);  
  tab.rwKeywordSet().define("UNIQUE_ID", 1);  

  tab.tableInfo().setType ("MEP");
  tab.tableInfo().readmeAddLine ("ME Parameter values");
  deftab.tableInfo().setType ("MEPinit");
  deftab.tableInfo().readmeAddLine ("Initial ME Parameter values");
  deftab.tableInfo().setType ("MEPold");
  deftab.tableInfo().readmeAddLine ("Old ME Parameter values");
}

void ParmDBAIPS::clearTables()
{
  for (int i=0; i<3; ++i) {
    TableLocker locker(itsTables[i], FileLocker::Write);
    Vector<uInt> rows = itsTables[i].rowNumbers();
    itsTables[i].removeRow(rows);
  }
}

void ParmDBAIPS::fillDefMap (map<string,ParmValue>& defMap)
{
  Table table = itsTables[2];
  TableLocker locker(table, FileLocker::Read);
  ROScalarColumn<String> nameCol(table, "NAME");
  for (uint row=0; row<table.nrow(); ++row) {
    defMap.insert (make_pair(nameCol(row), extractDefValue(table, row)));
  }
}

void ParmDBAIPS::extractValues (map<string,ParmValueSet>& result,
				const Table& tab, int tabinx)
{
  // Get rownrs in parm table itself.
  Vector<uInt> rownrs = tab.rowNumbers();
  ROScalarColumn<String> nameCol (tab, "NAME");
  ROScalarColumn<String> typeCol (tab, "TYPE");
  ROArrayColumn<double> consCol (tab, "CONSTANTS");
  ROArrayColumn<double> stCol (tab, "START");
  ROArrayColumn<double> endCol (tab, "END");
  ROArrayColumn<bool> maskCol (tab, "SOLVABLE");
  ROArrayColumn<double> valCol (tab, "VALUES");
  ///ROArrayColumn<double> errCol (tab, "ERRORS");
  ROArrayColumn<double> offCol (tab, "OFFSET");
  ROArrayColumn<double> scCol (tab, "SCALE");
  ROScalarColumn<double> pertCol (tab, "PERTURBATION");
  ROScalarColumn<bool> prelCol (tab, "PERT_REL");
  ROScalarColumn<double> weightCol (tab, "WEIGHT");
  ROScalarColumn<int> idCol (tab, "ID");
  ROScalarColumn<int> paridCol (tab, "PARENTID");
  for (unsigned int i=0; i<tab.nrow(); i++) {
    ParmValue pvalue;
    ParmValueRep& pval = pvalue.rep();
    Array<double> val = valCol(i);
    if (maskCol.isDefined(i)) {
      Array<bool> mask = maskCol(i);
      pval.setCoeff (val.data(), mask.data(), toVector(val.shape()));
    } else {
      pval.setCoeff (val.data(), toVector(val.shape()));
    }
    pval.setDomain (ParmDomain(toVector(stCol(i)), toVector(endCol(i))));
    toVector (pval.itsOffset, offCol(i));
    toVector (pval.itsScale, scCol(i));
    pval.setPerturbation (pertCol(i), prelCol(i));
    if (consCol.isDefined(i)) {
      pval.setType (typeCol(i), toVector(consCol(i)));
    } else {
      pval.setType (typeCol(i));
    }
    pval.itsWeight   = weightCol(i);
    pval.itsID       = idCol(i);
    pval.itsParentID = paridCol(i);
    // Set the row number as the Ref, so it can be used in the put.
    pval.itsDBTabRef = tabinx;           //# read from given table
    pval.itsDBRowRef = rownrs[i];
    pval.itsDBSeqNr = getParmDBSeqNr();
    // Insert in the map.
    // Create a new name entry if needed.
    string parmName = nameCol(i);
    map<string,ParmValueSet>::iterator pos = result.find (parmName);
    if (pos == result.end()) {
      ParmValueSet pset(parmName);
      pset.getValues().push_back (pvalue);
      result.insert (make_pair(parmName, pset));
    } else {
      pos->second.getValues().push_back (pvalue);
    }
  }
}

ParmValue ParmDBAIPS::extractDefValue (const Table& tab, int row)
{
  ParmValue pvalue;
  ParmValueRep& result = pvalue.rep();
  ROScalarColumn<String> typeCol (tab, "TYPE");
  ROArrayColumn<double> consCol (tab, "CONSTANTS");
  ROArrayColumn<bool> maskCol (tab, "SOLVABLE");
  ROArrayColumn<double> valCol (tab, "VALUES");
  ROScalarColumn<double> pertCol (tab, "PERTURBATION");
  ROScalarColumn<bool> prelCol (tab, "PERT_REL");
  Array<double> val = valCol(row);
  if (maskCol.isDefined(row)) {
    Array<bool> mask = maskCol(row);
    result.setCoeff (val.data(), mask.data(), toVector(val.shape()));
  } else {
    result.setCoeff (val.data(), toVector(val.shape()));
  }
  if (consCol.isDefined(row)) {
    result.setType (typeCol(row), toVector(consCol(row)));
  } else {
    result.setType (typeCol(row));
  }
  result.setPerturbation (pertCol(row), prelCol(row));
  result.itsDBTabRef = -2;
  result.itsDBSeqNr  = getParmDBSeqNr();
  return pvalue;
}

void ParmDBAIPS::getValues (map<string,ParmValueSet>& result,
			    const vector<string>& parmNames,
			    const ParmDomain& domain,
			    int parentId,
			    ParmDBRep::TableType tableType)
{
  int tabinx = getTableIndex (tableType);
  Table table = itsTables[tabinx];
  TableLocker locker(table, FileLocker::Read);
  // Find all rows overlapping the requested domain.
  // Only look for values without a parent; thus results of possible refit.
  TableExprNode expr = makeExpr (table, domain, parentId);
  if (parmNames.size() > 0) {
    Vector<String> nams(parmNames.size());
    for (uint i=0; i<parmNames.size(); ++i) {
      nams(i) = parmNames[i];
    }
    andExpr (expr, table.col("NAME").in (nams));
  }
  Table sel;
  if (expr.isNull()) {
    sel = table;
  } else {
    sel = table(expr);
  }
  extractValues (result, sel, tabinx);
}

ParmValueSet ParmDBAIPS::getValues (const string& parmName,
				    const ParmDomain& domain,
				    int parentId,
				    ParmDBRep::TableType tableType)
{
  int tabinx = getTableIndex (tableType);
  TableLocker locker(itsTables[tabinx], FileLocker::Read);
  ParmValueSet resultSet(parmName);
  Table sel = find (parmName, domain, parentId, tabinx);
  if (sel.nrow() > 0) {
    map<string,ParmValueSet> res;
    extractValues (res, sel, tabinx);
    resultSet = res.begin()->second;
  }
  return resultSet;
}

void ParmDBAIPS::getValues (map<string,ParmValueSet>& result,
			    const string& parmNamePattern,
			    const ParmDomain& domain,
			    int parentId,
			    ParmDBRep::TableType tableType)
{
  int tabinx = getTableIndex (tableType);
  TableLocker locker(itsTables[tabinx], FileLocker::Read);
  vector<ParmValueSet> resvec;
  // Find all rows overlapping the requested domain.
  // Only look for values without a parent; thus results of possible refit.
  Table table = itsTables[tabinx];
  TableExprNode expr = makeExpr (table, domain, parentId);
  if (parmNamePattern != ""  &&  parmNamePattern != "*") {
    Regex regex(Regex::fromPattern(parmNamePattern));
    andExpr (expr, table.col("NAME") == regex);
  }
  Table sel;
  if (expr.isNull()) {
    sel = table;
  } else {
    sel = table(expr);
  }
  extractValues (result, sel, tabinx);
}

void ParmDBAIPS::getDefValues (map<string,ParmValueSet>& result,
			       const string& parmNamePattern)
{
  vector<ParmValueSet> resvec;
  TableLocker locker(itsTables[2], FileLocker::Read);
  // Find all rows.
  Table table = itsTables[2];
  Regex regex(Regex::fromPattern(parmNamePattern));
  Table sel = table(table.col("NAME") == regex);
  ROScalarColumn<String> nameCol(sel, "NAME");
  for (uint row=0; row<sel.nrow(); ++row) {
    string parmName = nameCol(row);
    ParmValueSet pset(parmName);
    pset.getValues().push_back (extractDefValue (sel, row));
    result.insert (make_pair (parmName, pset));
  }
}

void ParmDBAIPS::putValue (const string& parmName,
			   ParmValue& pvalue,
			   ParmDBRep::TableType tableType)
{
  int tabinx = getTableIndex (tableType);
  itsTables[tabinx].reopenRW();
  TableLocker locker(itsTables[tabinx], FileLocker::Write);
  doPutValue (parmName, pvalue.rep(), tabinx);
}

void ParmDBAIPS::doPutValue (const string& parmName,
			     ParmValueRep& pval,
			     int tabinx)
{
  if (pval.itsDBTabRef == -2) {
    // It is certainly a new row.
    putNewValue (parmName, pval, tabinx);
  } else if (pval.itsDBTabRef != tabinx) {
    // It might be a new row.
    putValueCheck (parmName, pval, tabinx);
  } else {
    int rownr = pval.itsDBRowRef;
    ArrayColumn<double> valCol (itsTables[tabinx], "VALUES");
    valCol.put (rownr, fromVector(pval.itsCoeff, pval.itsShape));
  }
}

void ParmDBAIPS::putValues (map<string,ParmValueSet>& parmSet,
			    ParmDBRep::TableType tableType)
{
  int tabinx = getTableIndex (tableType);
  itsTables[tabinx].reopenRW();
  TableLocker locker(itsTables[tabinx], FileLocker::Write);
  for (map<string,ParmValueSet>::iterator iter = parmSet.begin();
       iter != parmSet.end();
       iter++) {
    vector<ParmValue>& vec = iter->second.getValues();
    const string& parmName = iter->first;
    if (!vec.empty()  &&  vec[0].rep().itsDBSeqNr == getParmDBSeqNr()) {
      for (uint i=0; i<vec.size(); ++i) {
	doPutValue (parmName, vec[i].rep(), tabinx);
      }
    }
  }
}

void ParmDBAIPS::putValueCheck (const string& parmName,
				ParmValueRep& pval,
				int tabinx)
{
  const ParmDomain& domain = pval.itsDomain;
  Table sel = find (parmName, domain, pval.itsParentID, tabinx);
  if (sel.nrow() > 0) {
    ASSERTSTR (sel.nrow()==1, "Parameter " << parmName <<
	       " has multiple entries for domain " << domain);
    ROArrayColumn<double> stCol (sel, "START");
    ROArrayColumn<double> endCol (sel, "END");
    ASSERTSTR (allNear(Vector<double>(domain.getStart()), stCol(0), 1e-7)  &&
	       allNear(Vector<double>(domain.getEnd()),  endCol(0), 1e-7),
	       "Parameter " << parmName <<
	       " has a partially instead of fully matching entry for domain "
	       << domain);
    ArrayColumn<double> valCol (sel, "VALUES");
    valCol.put (0, fromVector(pval.itsCoeff, pval.itsShape));
  } else {
    putNewValue (parmName, pval, tabinx);
  }
}

void ParmDBAIPS::putNewValue (const string& parmName, 
			      ParmValueRep& pval,
			      int tabinx)
{
  Table& table = itsTables[tabinx];
  uInt rownr = table.nrow();
  table.addRow();
  ScalarColumn<String> typeCol (table, "TYPE");
  ArrayColumn<double> consCol (table, "CONSTANTS");
  ScalarColumn<String> namCol (table, "NAME");
  ScalarColumn<double> stxCol (table, "STARTX");
  ScalarColumn<double> endxCol (table, "ENDX");
  ScalarColumn<double> styCol (table, "STARTY");
  ScalarColumn<double> endyCol (table, "ENDY");
  ArrayColumn<double> stCol (table, "START");
  ArrayColumn<double> endCol (table, "END");
  ArrayColumn<bool> maskCol (table, "SOLVABLE");
  ArrayColumn<double> valCol (table, "VALUES");
  ///ArrayColumn<double> errCol (table, "ERRORS");
  ArrayColumn<double> offCol (table, "OFFSET");
  ArrayColumn<double> scCol (table, "SCALE");
  ScalarColumn<double> pertCol (table, "PERTURBATION");
  ScalarColumn<bool> prelCol (table, "PERT_REL");
  ScalarColumn<double> weightCol (table, "WEIGHT");
  ScalarColumn<int> idCol (table, "ID");
  ScalarColumn<int> paridCol (table, "PARENTID");
  namCol.put (rownr, parmName);
  typeCol.put (rownr, pval.itsType);
  if (pval.itsConstants.size() > 0) {
    consCol.put (rownr, fromVector(pval.itsConstants));
  }
  valCol.put (rownr, fromVector(pval.itsCoeff, pval.itsShape));
  if (pval.itsSolvMask.size() > 0) {
    maskCol.put (rownr, fromVector(pval.itsSolvMask, pval.itsShape));
  }
  stCol.put (rownr, fromVector(pval.itsDomain.getStart()));
  endCol.put (rownr, fromVector(pval.itsDomain.getEnd()));
  if (pval.itsDomain.getStart().size() == 0) {
    stxCol.put (rownr, -1.);
    endxCol.put (rownr, -2.);
  } else {
    stxCol.put (rownr, pval.itsDomain.getStart()[0]);
    endxCol.put (rownr, pval.itsDomain.getEnd()[0]);
    if (pval.itsDomain.getStart().size() == 1) {
      styCol.put (rownr, -1.);
      endyCol.put (rownr, -2.);
    } else {
      styCol.put (rownr, pval.itsDomain.getStart()[1]);
      endyCol.put (rownr, pval.itsDomain.getEnd()[1]);
    }
  }
  offCol.put (rownr, fromVector(pval.itsOffset));
  scCol.put (rownr, fromVector(pval.itsScale));
  pertCol.put (rownr, pval.itsPerturbation);
  prelCol.put (rownr, pval.itsIsRelPert);
  weightCol.put (rownr, pval.itsWeight);
  idCol.put (rownr, pval.itsID);
  paridCol.put (rownr, pval.itsParentID);
  // ParmValue is now stored here.
  pval.itsDBTabRef = tabinx;
  pval.itsDBRowRef = rownr;
  pval.itsDBSeqNr = getParmDBSeqNr();
}

void ParmDBAIPS::putDefValue (const string& parmName,
			      const ParmValue& pvalue)
{
  itsTables[2].reopenRW();
  TableLocker locker(itsTables[2], FileLocker::Write);
  const ParmValueRep& pval = pvalue.rep();
  // First see if the parameter name exists at all.
  Table table = itsTables[2];
  Table sel = table(table.col("NAME") == String(parmName));
  if (sel.nrow() == 1) {
    uInt rownr=0;
    ScalarColumn<String> typeCol (sel, "TYPE");
    ArrayColumn<double> consCol (sel, "CONSTANTS");
    ArrayColumn<bool> maskCol (sel, "SOLVABLE");
    ArrayColumn<double> valCol (sel, "VALUES");
    ScalarColumn<double> pertCol (sel, "PERTURBATION");
    ScalarColumn<bool> prelCol (sel, "PERT_REL");
    typeCol.put (rownr, pval.itsType);
    if (pval.itsConstants.size() > 0) {
      consCol.put (rownr, fromVector(pval.itsConstants));
    }
    valCol.put (rownr, fromVector(pval.itsCoeff, pval.itsShape));
    if (pval.itsSolvMask.size() > 0) {
      maskCol.put (rownr, fromVector(pval.itsSolvMask, pval.itsShape));
    }
    pertCol.put (rownr, pval.itsPerturbation);
    prelCol.put (rownr, pval.itsIsRelPert);
  } else if (sel.nrow() == 0) {
    putNewDefValue (parmName, pval);
  } else {
    ASSERTSTR (false, "Too many default parms with the same name/domain");
  }
  clearDefFilled();
}

void ParmDBAIPS::putNewDefValue (const string& parmName, 
				 const ParmValueRep& pval)
{
  Table& table = itsTables[2];
  uInt rownr = table.nrow();
  table.addRow();
  ScalarColumn<String> namCol (table, "NAME");
  ScalarColumn<String> typeCol (table, "TYPE");
  ArrayColumn<double> consCol (table, "CONSTANTS");
  ArrayColumn<bool> maskCol (table, "SOLVABLE");
  ArrayColumn<double> valCol (table, "VALUES");
  ScalarColumn<double> pertCol (table, "PERTURBATION");
  ScalarColumn<bool> prelCol (table, "PERT_REL");
  typeCol.put (rownr, pval.itsType);
  if (pval.itsConstants.size() > 0) {
    consCol.put (rownr, fromVector(pval.itsConstants));
  }
  valCol.put (rownr, fromVector(pval.itsCoeff, pval.itsShape));
  if (pval.itsSolvMask.size() > 0) {
    maskCol.put (rownr, fromVector(pval.itsSolvMask, pval.itsShape));
  }
  pertCol.put (rownr, pval.itsPerturbation);
  prelCol.put (rownr, pval.itsIsRelPert);
  namCol.put (rownr, parmName);
  clearDefFilled();
}

void ParmDBAIPS::deleteValues (const string& parmNamePattern,
			       const ParmDomain& domain,
			       int parentId,
			       ParmDBRep::TableType tableType)
{
  int tabinx = getTableIndex(tableType);
  Table& table = itsTables[tabinx];
  table.reopenRW();
  TableLocker locker(table, FileLocker::Write);
  // Find all rows.
  TableExprNode expr = makeExpr (table, domain, parentId);
  Regex regex(Regex::fromPattern(parmNamePattern));
  andExpr (expr, table.col("NAME") == regex);
  Table sel = table(expr);
  // Delete all rows found.
  table.removeRow (sel.rowNumbers (table));
}

void ParmDBAIPS::deleteDefValues (const string& parmNamePattern)
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

Table ParmDBAIPS::find (const string& parmName,
			const ParmDomain& domain,
			int parentId,
			int tabinx)
{
  Table table = itsTables[tabinx];
  // Find all rows overlapping the requested domain.
  TableExprNode expr = makeExpr (table, domain, parentId);
  andExpr (expr, table.col("NAME") == String(parmName));
  return table(expr);
}

vector<string> ParmDBAIPS::getNames (const string& pattern,
				     ParmDBRep::TableType tableType)
{
  int tabinx = getTableIndex (tableType);
  // Get all parm rows where the name matches the pattern.
  vector<string> nams;
  Regex regex(Regex::fromPattern(pattern));
  Table& tab = itsTables[tabinx];
  TableLocker locker(tab, FileLocker::Read);
  Table sel = tab(tab.col("NAME") == regex);
  if (sel.nrow() > 0) {
    // Sort them uniquely on name.
    Table sor = sel.sort("NAME", Sort::Ascending,
			 Sort::QuickSort | Sort::NoDuplicates);
    ROScalarColumn<String> namCol(sor, "NAME");
    Vector<String> names = namCol.getColumn();
    nams.reserve (nams.size() + sor.nrow());
    for (unsigned int i=0; i<names.nelements(); i++) {
      nams.push_back (names(i));
    }
  }
  LOG_TRACE_STAT_STR("Finished retrieving "<<nams.size()<<" names");
  return nams;
}

TableExprNode ParmDBAIPS::makeExpr (const Table& table,
				    const ParmDomain& domain,
				    int parentId) const
{
  TableExprNode expr;
  if (parentId >= 0) {
    andExpr (expr, table.col("PARENTID") == parentId);
  }
  if (domain.getStart().size() > 0) {
    andExpr (expr,
	     domain.getStart()[0] < table.col("ENDX")  &&
             domain.getEnd()[0]   > table.col("STARTX"));
    if (domain.getStart().size() > 1) {
      andExpr (expr,
	       domain.getStart()[1] < table.col("ENDY")  &&
	       domain.getEnd()[1]   > table.col("STARTY"));
      if (domain.getStart().size() > 0) {
	andExpr (expr,
	     nelements(table.col("START")) == int(domain.getStart().size())  &&
	     all(fromVector(domain.getStart()) < table.col("END"))  &&
	     all(fromVector(domain.getEnd())   > table.col("START")));
      }
    }
  }
  return expr;
}

void ParmDBAIPS::andExpr (TableExprNode& expr,
			  const TableExprNode& right) const
{
  if (expr.isNull()) {
    expr = right;
  } else {
    expr = expr && right;
  }
}

void ParmDBAIPS::toVector (vector<double>& vec,
			   const Array<double>& arr) const
{
  vec.resize (arr.nelements());
  bool deleteIt;
  const double* ptr = arr.getStorage (deleteIt);
  for (uint i=0; i<vec.size(); ++i) {
    vec[i] = ptr[i];
  }
  arr.freeStorage (ptr, deleteIt);
}
vector<double> ParmDBAIPS::toVector (const Array<double>& arr) const
{
  vector<double> vec;
  toVector (vec, arr);
  return vec;
}
vector<int> ParmDBAIPS::toVector (const IPosition& shape) const
{
  vector<int> vec(shape.nelements());
  for (uint i=0; i<vec.size(); ++i) {
    vec[i] = shape[i];
  }
  return vec;
}
Array<double> ParmDBAIPS::fromVector (const vector<double>& vec,
				      const vector<int>& shape) const
{
  IPosition shp (shape.size());
  for (uint i=0; i<shape.size(); ++i) {
    shp[i] = shape[i];
  }
  Array<double> arr(shp);
  double* ptr = arr.data();
  for (uint i=0; i<vec.size(); ++i) {
    ptr[i] = vec[i];
  }
  return arr;
}
Array<bool> ParmDBAIPS::fromVector (const vector<bool>& vec,
				    const vector<int>& shape) const
{
  IPosition shp (shape.size());
  for (uint i=0; i<shape.size(); ++i) {
    shp[i] = shape[i];
  }
  Array<bool> arr(shp);
  bool* ptr = arr.data();
  for (uint i=0; i<vec.size(); ++i) {
    ptr[i] = vec[i];
  }
  return arr;
}
Array<double> ParmDBAIPS::fromVector (const vector<double>& vec) const
{
  return fromVector (vec, vector<int>(1, vec.size()));
}


} // namespace ParmDB
} // namespace LOFAR
