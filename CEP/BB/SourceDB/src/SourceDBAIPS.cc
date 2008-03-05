//# SourceDBAIPS.cc: Class to hold sources in an AIPS++ table.
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
#include <SourceDB/SourceDBAIPS.h>
#include <Common/LofarLogger.h>

#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ExprNodeSet.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ColumnDesc.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableLocker.h>
#include <measures/TableMeasures/TableQuantumDesc.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MDirection.h>
#include <casa/Arrays/Vector.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicMath/Math.h>
#include <casa/OS/Time.h>
#include <casa/Quanta/MVTime.h>

using namespace casa;
using namespace std;

namespace LOFAR {
namespace SourceDB {

SourceDBAIPS::SourceDBAIPS (const string& tableName, bool forceNew)
{
  // Create the table if needed or if it does not exist yet.
  if (forceNew  ||  !Table::isReadable (tableName)) {
    createTables (tableName);
  }
  // Open the main table.
  itsTable = Table(tableName, TableLock::UserLocking);
}

SourceDBAIPS::~SourceDBAIPS()
{}

void SourceDBAIPS::lock (bool lockForWrite)
{
  itsTable.lock (lockForWrite);
}

void SourceDBAIPS::unlock()
{
  itsTable.unlock();
}

void SourceDBAIPS::createTables (const string& tableName)
{
  TableDesc td("Source parameter table", TableDesc::Scratch);
  td.comment() = String("Table containing parameters for sources");
  td.addColumn (ScalarColumnDesc<String>("NAME"));
  td.addColumn (ScalarColumnDesc<String>("TYPE"));
  td.addColumn (ArrayColumnDesc<double> ("RADEC", IPosition(1,2),
					 ColumnDesc::Direct));
  td.addColumn (ScalarColumnDesc<double>("FLUXI"));
  td.addColumn (ScalarColumnDesc<double>("FLUXQ"));
  td.addColumn (ScalarColumnDesc<double>("FLUXU"));
  td.addColumn (ScalarColumnDesc<double>("FLUXV"));
  td.addColumn (ScalarColumnDesc<double>("SPINDEX"));
  td.addColumn (ScalarColumnDesc<double>("STARTFREQ", 1));
  td.addColumn (ScalarColumnDesc<double>("ENDFREQ", 1));
  td.addColumn (ScalarColumnDesc<double>("STARTTIME", 1));
  td.addColumn (ScalarColumnDesc<double>("ENDTIME", 1));
  // Define units and measures.
  setQuant (td, "FLUXI", "Jy");
  setQuant (td, "FLUXQ", "Jy");
  setQuant (td, "FLUXU", "Jy");
  setQuant (td, "FLUXV", "Jy");
  setQuant (td, "STARTFREQ", "Hz");
  setQuant (td, "ENDFREQ", "Hz");
  setEpoch (td, "STARTTIME");
  setEpoch (td, "ENDTIME");
  setQuant (td, "RADEC", "rad");
  TableMeasRefDesc measRef(MDirection::J2000);
  TableMeasValueDesc measVal(td, "RADEC");
  TableMeasDesc<MDirection> tmd(measVal, measRef);
  tmd.write(td);

  SetupNewTable newtab(tableName, td, Table::New);
  Table tab(newtab);

  tab.tableInfo().setType ("SOURCES");
  tab.tableInfo().readmeAddLine ("Source Parameter values");
}

void SourceDBAIPS::setQuant (TableDesc& td, const String& name,
			     const String& unit)
{
  TableQuantumDesc tqd(td, name, Unit(unit));
  tqd.write (td);
}

void SourceDBAIPS::setEpoch (TableDesc& td, const String& name)
{
  setQuant (td, name, "s");
  TableMeasRefDesc measRef(MEpoch::UTC);
  TableMeasValueDesc measVal(td, name);
  TableMeasDesc<MEpoch> tmd(measVal, measRef);
  tmd.write(td);
}


void SourceDBAIPS::clearTables()
{
  TableLocker locker(itsTable, FileLocker::Write);
  Vector<uInt> rows = itsTable.rowNumbers();
  itsTable.removeRow(rows);
}

list<SourceValue> SourceDBAIPS::getSources (const vector<string>& sourceNames,
					    const ParmDB::ParmDomain& domain)
{
  TableLocker locker(itsTable, FileLocker::Read);
  // Find all rows overlapping the requested domain.
  TableExprNode expr = makeExpr (itsTable, domain);
  if (sourceNames.size() > 0) {
    Vector<String> nams(sourceNames.size());
    for (unsigned i=0; i<sourceNames.size(); ++i) {
      nams[i] = sourceNames[i];
    }
    andExpr (expr, itsTable.col("NAME").in (nams));
  }
  Table sel;
  if (expr.isNull()) {
    sel = itsTable;
  } else {
    sel = itsTable(expr);
  }
  list<SourceValue> result;
  extractValues (result, sel);
  return result;
}

list<SourceValue> SourceDBAIPS::getSources (double ra, double dec,
					    double radius,
					    const ParmDB::ParmDomain& domain,
					    double minFluxI, double maxFluxI,
					    double minSpInx, double maxSpInx)
{
  TableLocker locker(itsTable, FileLocker::Read);
  // Find all rows overlapping the requested domain.
  TableExprNode expr = makeExpr (itsTable, domain);
  if (minFluxI != -1e30) {
    andExpr (expr, itsTable.col("FLUXI") >= minFluxI);
  }
  if (maxFluxI != -1e30) {
    andExpr (expr, itsTable.col("FLUXI") <= maxFluxI);
  }
  if (minSpInx != -1e30) {
    andExpr (expr, itsTable.col("SPINX") >= minSpInx);
  }
  if (maxSpInx != -1e30) {
    andExpr (expr, itsTable.col("SPINX") <= maxSpInx);
  }
  Vector<Double> rdr(3);
  rdr[0] = ra;
  rdr[1] = dec;
  rdr[2] = radius;
  andExpr (expr, anyCone(itsTable.col("RADEC"), rdr));
  list<SourceValue> result;
  extractValues (result, itsTable(expr));
  return result;
}

void SourceDBAIPS::addSources (const list<SourceValue>& values)
{
  itsTable.reopenRW();
  TableLocker locker(itsTable, FileLocker::Write);
  for (list<SourceValue>::const_iterator iter = values.begin();
       iter != values.end();
       ++iter) {
    itsTable.addRow();
    putValue (itsTable, itsTable.nrow()-1, *iter);
  }
}

void SourceDBAIPS::updateSource (const SourceValue& value)
{
  itsTable.reopenRW();
  TableLocker locker(itsTable, FileLocker::Write);
  Table sel = itsTable(itsTable.col("NAME") == String(value.getName())
		       &&  itsTable.col("STARTFREQ") == value.getStartFreq()
		       &&  itsTable.col("ENDFREQ") == value.getEndFreq()
		       &&  itsTable.col("STARTTIME") == value.getStartTime()
		       &&  itsTable.col("ENDTIME") == value.getEndTime());
  ASSERT (sel.nrow() == 1);
  putValue (sel, 0, value);
}

void SourceDBAIPS::deleteSource (const SourceValue& value)
{
  itsTable.reopenRW();
  TableLocker locker(itsTable, FileLocker::Write);
  // Find all rows.
  Table sel = itsTable(itsTable.col("NAME") == String(value.getName())
		       &&  itsTable.col("STARTFREQ") == value.getStartFreq()
		       &&  itsTable.col("ENDFREQ") == value.getEndFreq()
		       &&  itsTable.col("STARTTIME") == value.getStartTime()
		       &&  itsTable.col("ENDTIME") == value.getEndTime());
  // Delete all rows found.
  itsTable.removeRow (sel.rowNumbers (itsTable));
}

void SourceDBAIPS::deleteSources (const string& sourceNamePattern,
				  const ParmDB::ParmDomain& domain)
{
  itsTable.reopenRW();
  TableLocker locker(itsTable, FileLocker::Write);
  // Find all rows.
  TableExprNode expr = makeExpr (itsTable, domain);
  Regex regex(Regex::fromPattern(sourceNamePattern));
  andExpr (expr, itsTable.col("NAME") == regex);
  Table sel = itsTable(expr);
  // Delete all rows found.
  itsTable.removeRow (sel.rowNumbers (itsTable));
}

vector<string> SourceDBAIPS::getNames (const string& pattern)
{
  TableLocker locker(itsTable, FileLocker::Read);
  vector<string> result;
  // Find all rows.
  Regex regex(Regex::fromPattern(pattern));
  Table sel = itsTable(itsTable.col("NAME") == regex);
  if (sel.nrow() > 0) {
    // Sort them uniquely on name.
    Table sor = sel.sort("NAME", Sort::Ascending,
			 Sort::QuickSort | Sort::NoDuplicates);
    ROScalarColumn<String> nameCol (sor, "NAME");
    Vector<String> names = nameCol.getColumn();
    result.resize (sor.nrow());
    for (unsigned i=0; i<names.nelements(); ++i) {
      result[i] = names[i];
    }
  }
  return result;
}

void SourceDBAIPS::match (const SourceDB& other,
			  SourceDB& match, SourceDB& nonMatch)
{
  ASSERTSTR (false, "SourceDBAIPS::match not implemented yet");
}

void SourceDBAIPS::merge (const SourceDB& otherMatch,
			  const SourceDB& otherNonMatch)
{
  ASSERTSTR (false, "SourceDBAIPS::merge not implemented yet");
}


void SourceDBAIPS::extractValues (list<SourceValue>& result,
				  const Table& tab)
{
  ROScalarColumn<String> nameCol (tab, "NAME");
  ROScalarColumn<String> typeCol (tab, "TYPE");
  ROArrayColumn <double> radecCol(tab, "RADEC");
  ROScalarColumn<double> spCol   (tab, "SPINDEX");
  ROScalarColumn<double> fiCol   (tab, "FLUXI");
  ROScalarColumn<double> fqCol   (tab, "FLUXQ");
  ROScalarColumn<double> fuCol   (tab, "FLUXU");
  ROScalarColumn<double> fvCol   (tab, "FLUXV");
  ROScalarColumn<double> sfCol   (tab, "STARTFREQ");
  ROScalarColumn<double> efCol   (tab, "ENDFREQ");
  ROScalarColumn<double> stCol   (tab, "STARTTIME");
  ROScalarColumn<double> etCol   (tab, "ENDTIME");
  double flux[4];
  for (unsigned int i=0; i<tab.nrow(); i++) {
    SourceValue pvalue;
    flux[0] = fiCol(i);
    flux[1] = fqCol(i);
    flux[2] = fuCol(i);
    flux[3] = fvCol(i);
    Array<double> radec = radecCol(i);
    pvalue.setPointSource (nameCol(i), radec.data()[0], radec.data()[1],
			   flux, spCol(i));
    pvalue.setDomain (ParmDB::ParmDomain(sfCol(i), efCol(i),
					 stCol(i), etCol(i)));
    result.push_back (pvalue);
  }
}

void SourceDBAIPS::putValue (Table& tab,
			     unsigned rownr,
			     const SourceValue& pvalue)
{
  ScalarColumn<String> nameCol (tab, "NAME");
  ScalarColumn<String> typeCol (tab, "TYPE");
  ArrayColumn <double> radecCol(tab, "RADEC");
  ScalarColumn<double> spCol   (tab, "SPINDEX");
  ScalarColumn<double> fiCol   (tab, "FLUXI");
  ScalarColumn<double> fqCol   (tab, "FLUXQ");
  ScalarColumn<double> fuCol   (tab, "FLUXU");
  ScalarColumn<double> fvCol   (tab, "FLUXV");
  ScalarColumn<double> sfCol   (tab, "STARTFREQ");
  ScalarColumn<double> efCol   (tab, "ENDFREQ");
  ScalarColumn<double> stCol   (tab, "STARTTIME");
  ScalarColumn<double> etCol   (tab, "ENDTIME");
  Vector<double> radec(2);
  radec(0) = pvalue.getRA();
  radec(1) = pvalue.getDEC();
  nameCol.put (rownr, pvalue.getName());
  typeCol.put (rownr, pvalue.getType());
  radecCol.put(rownr, radec);
  spCol.put   (rownr, pvalue.getSpectralIndex());
  fiCol.put   (rownr, pvalue.getFlux()[0]);
  fqCol.put   (rownr, pvalue.getFlux()[1]);
  fuCol.put   (rownr, pvalue.getFlux()[2]);
  fvCol.put   (rownr, pvalue.getFlux()[3]);
  sfCol.put   (rownr, pvalue.getStartFreq());
  efCol.put   (rownr, pvalue.getEndFreq());
  stCol.put   (rownr, pvalue.getStartTime());
  etCol.put   (rownr, pvalue.getEndTime());
}

TableExprNode SourceDBAIPS::makeExpr (const Table& table,
				      const ParmDB::ParmDomain& domain) const
{
  TableExprNode expr;
  if (domain.getStart().size() > 0) {
    andExpr (expr,
	     domain.getStart()[0] < table.col("ENDFREQ")  &&
             domain.getEnd()[0]   > table.col("STARTFREQ"));
    if (domain.getStart().size() > 1) {
      andExpr (expr,
	       domain.getStart()[1] < table.col("ENDTIME")  &&
	       domain.getEnd()[1]   > table.col("STARTTIME"));
    }
  }
  return expr;
}

void SourceDBAIPS::andExpr (TableExprNode& expr,
			    const TableExprNode& right) const
{
  if (expr.isNull()) {
    expr = right;
  } else {
    expr = expr && right;
  }
}


} // namespace SourceDB
} // namespace LOFAR
