//# ParmTableAIPS.cc: Object to hold parameters in an AIPS++ table.
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

#include <BBS3/MNS/ParmTableAIPS.h>
#include <BBS3/MNS/MeqDomain.h>
#include <Common/LofarLogger.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ExprNodeSet.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicMath/Math.h>

using namespace casa;

namespace LOFAR {

ParmTableAIPS::ParmTableAIPS (const string& userName, const string& tableName)
: itsTable       ("/tmp/" + userName + "." + tableName+".MEP"),
  itsIndex       (itsTable, "NAME"),
  itsIndexName   (itsIndex.accessKey(), "NAME"),
  itsInitIndex   (0)
{
  itsTableName = string("/tmp/") + userName + string(".") + tableName + string(".MEP");

  // these things should be done in the connect (just like the initialization of itsTable etc)
  if (itsTable.keywordSet().isDefined ("DEFAULTVALUES")) {
    itsInitTable = itsTable.keywordSet().asTable ("DEFAULTVALUES");
    itsInitIndex = new ColumnsIndex (itsInitTable, "NAME");
    itsInitIndexName = RecordFieldPtr<String> (itsInitIndex->accessKey(),
						 "NAME");
  }
}

ParmTableAIPS::~ParmTableAIPS()
{
  delete itsInitIndex;
}

void ParmTableAIPS::connect() {
  // connect was done in the constructor
}

void ParmTableAIPS::createTable(const string& userName, const string& tableName) {
  string fullTableName = string("/tmp/") + userName + string(".") + tableName + string(".MEP");
  TableDesc td("PSS parameter table", TableDesc::New);
  td.comment() = String("Table containing parameters for PSS");
  td.addColumn (ScalarColumnDesc<String>("NAME"));
  td.addColumn (ScalarColumnDesc<double>("STARTTIME"));
  td.addColumn (ScalarColumnDesc<double>("ENDTIME"));
  td.addColumn (ScalarColumnDesc<double>("STARTFREQ"));
  td.addColumn (ScalarColumnDesc<double>("ENDFREQ"));
  td.addColumn (ArrayColumnDesc <double>("VALUES"));
  td.addColumn (ArrayColumnDesc <double>("SIM_VALUES"));
  td.addColumn (ArrayColumnDesc <double>("SIM_PERT"));
  td.addColumn (ScalarColumnDesc<double>("TIME0"));
  td.addColumn (ScalarColumnDesc<double>("FREQ0"));
  td.addColumn (ScalarColumnDesc<bool>  ("NORMALIZED"));
  td.addColumn (ArrayColumnDesc <bool>  ("SOLVABLE"));
  td.addColumn (ScalarColumnDesc<double>("DIFF"));
  td.addColumn (ScalarColumnDesc<bool>  ("DIFF_REL"));

  SetupNewTable newtab(fullTableName, td, Table::New);

  TableDesc tddef("PSS default parameter values", TableDesc::New);
  tddef.comment() = String("Table containing default parameters for PSS");
  tddef.addColumn (ScalarColumnDesc<String>("NAME"));
  tddef.addColumn (ArrayColumnDesc <double>("VALUES"));
  tddef.addColumn (ArrayColumnDesc <double>("SIM_VALUES"));
  tddef.addColumn (ArrayColumnDesc <double>("SIM_PERT"));
  tddef.addColumn (ScalarColumnDesc<double>("TIME0"));
  tddef.addColumn (ScalarColumnDesc<double>("FREQ0"));
  tddef.addColumn (ScalarColumnDesc<bool>  ("NORMALIZED"));
  tddef.addColumn (ArrayColumnDesc <bool>  ("SOLVABLE"));
  tddef.addColumn (ScalarColumnDesc<double>("DIFF"));
  tddef.addColumn (ScalarColumnDesc<bool>  ("DIFF_REL"));

  SetupNewTable newdeftab(fullTableName+string("/DEFAULTVALUES"), tddef, Table::New);

  Table tab(newtab);
  Table deftab(newdeftab);
  tab.rwKeywordSet().defineTable("DEFAULTVALUES", deftab);  

  tab.tableInfo().setType ("MEP");
  tab.tableInfo().readmeAddLine ("PSS ME Parameter values");
  deftab.tableInfo().setType ("MEPinit");
  deftab.tableInfo().readmeAddLine ("Initial PSS ME Parameter values");


  // some descriptions could be added to the tables
}
void ParmTableAIPS::clearTable() {
  Vector<uInt> initRows = itsInitTable.rowNumbers();
  itsInitTable.removeRow(initRows);
  Vector<uInt> rows = itsTable.rowNumbers();
  itsTable.removeRow(rows);
}

vector<MeqPolc> ParmTableAIPS::getPolcs (const string& parmName,
					 const MeqDomain& domain)
{
  vector<MeqPolc> result;
  Table sel = find (parmName, domain);
  if (sel.nrow() > 0) {
    ROScalarColumn<double> stCol (sel, "STARTTIME");
    ROScalarColumn<double> etCol (sel, "ENDTIME");
    ROScalarColumn<double> sfCol (sel, "STARTFREQ");
    ROScalarColumn<double> efCol (sel, "ENDFREQ");
    ROArrayColumn<bool> maskCol (sel, "SOLVABLE");
    ROArrayColumn<double> valCol (sel, "VALUES");
    ROArrayColumn<double> simvalCol (sel, "SIM_VALUES");
    ROArrayColumn<double> simpertCol (sel, "SIM_PERT");
    ROScalarColumn<double> t0Col (sel, "TIME0");
    ROScalarColumn<double> f0Col (sel, "FREQ0");
    ROScalarColumn<bool> normCol (sel, "NORMALIZED");
    ROScalarColumn<double> diffCol (sel, "DIFF");
    ROScalarColumn<bool> drelCol (sel, "DIFF_REL");
    for (unsigned int i=0; i<sel.nrow(); i++) {
      MeqPolc polc;
      if (maskCol.isDefined(i)) {
	polc.setCoeff (Matrix<double>(valCol(i)), maskCol(i));
      } else {
	polc.setCoeff (Matrix<double>(valCol(i)));
      }
      polc.setX0 (t0Col(i));
      polc.setY0 (f0Col(i));
      polc.setNormalize (normCol(i));
      polc.setSimCoeff (Matrix<double>(simvalCol(i)));
      polc.setPertSimCoeff (Matrix<double>(simpertCol(i)));
      polc.setDomain (MeqDomain(stCol(i), etCol(i), sfCol(i), efCol(i)));
      polc.setPerturbation (diffCol(i), drelCol(i));
      result.push_back (polc);
    }
  }
  return result;
}

MeqPolc ParmTableAIPS::getInitCoeff (const string& parmName)
{
  // Try to find the default initial values in the InitialValues subtable.
  // The parameter name consists of parts (separated by dots), so the
  // parameters are categorised in that way.
  // An initial value can be defined for the full name or for a higher
  // category.
  // So look up until found or until no more parts are left.
  MeqPolc result;
  if (itsInitIndex) {
    string name = parmName;
    while (true) {
      *itsInitIndexName = name;
      Vector<uInt> rownrs = itsInitIndex->getRowNumbers();
      if (rownrs.nelements() > 0) {
	ASSERTSTR (rownrs.nelements() == 1,
		   "Too many default coefficients in parmtableAIPS");
	int row = rownrs(0);
	ROArrayColumn<bool> maskCol (itsInitTable, "SOLVABLE");
	ROArrayColumn<Double> valCol (itsInitTable, "VALUES");
	ROScalarColumn<double> t0Col (itsInitTable, "TIME0");
	ROScalarColumn<double> f0Col (itsInitTable, "FREQ0");
	ROScalarColumn<bool> normCol (itsInitTable, "NORMALIZED");
	ROScalarColumn<double> diffCol (itsInitTable, "DIFF");
	ROScalarColumn<bool> drelCol (itsInitTable, "DIFF_REL");
	ROArrayColumn<double> simvalCol (itsInitTable, "SIM_VALUES");
	ROArrayColumn<double> simpertCol (itsInitTable, "SIM_PERT");
	if (maskCol.isDefined(row)) {
	  result.setCoeff (Matrix<double>(valCol(row)), maskCol(row));
	} else {
	  result.setCoeff (Matrix<double>(valCol(row)));
	}
	result.setX0 (t0Col(row));
	result.setY0 (f0Col(row));
	result.setNormalize (normCol(row));
	result.setPerturbation (diffCol(row), drelCol(row));
	result.setSimCoeff (Matrix<double>(simvalCol(row)));
	result.setPertSimCoeff (Matrix<double>(simpertCol(row)));
	break;
      }
      string::size_type idx = name.rfind ('.');
      // Exit loop if no more name parts.
      if (idx == string::npos) {
	break;
      }
      // Remove last part and try again.
      name = name.substr (0, idx);
    }
  }
  return result;
}
				    
void ParmTableAIPS::putCoeff (const string& parmName,
			      const MeqPolc& polc)
{
  itsTable.reopenRW();
  const MeqDomain& domain = polc.domain();
  //const MeqMatrix& values = polc.getCoeff();
  //const MeqMatrix& simvalues = polc.getSimCoeff();
  //const MeqMatrix& pertsimvalues = polc.getPertSimCoeff();
  Table sel = find (parmName, domain);
  if (sel.nrow() > 0) {
    ASSERTSTR (sel.nrow()==1, "Parameter " << parmName <<
		 " has multiple entries for time "
		 << domain.startX() << ':' << domain.endX() << " and freq "
		 << domain.startY() << ':' << domain.endY());
    ROScalarColumn<double> stCol (sel, "STARTTIME");
    ROScalarColumn<double> etCol (sel, "ENDTIME");
    ROScalarColumn<double> sfCol (sel, "STARTFREQ");
    ROScalarColumn<double> efCol (sel, "ENDFREQ");
    ASSERTSTR (near(domain.startX(), stCol(0))  &&
	       near(domain.endX(), etCol(0))  &&
	       near(domain.startY(), sfCol(0))  &&
	       near(domain.endY(), efCol(0)),
	       "Parameter " << parmName <<
	       " has a partially instead of fully matching entry for time "
		 << domain.startX() << ':' << domain.endX() << " and freq "
		 << domain.startY() << ':' << domain.endY());
    ArrayColumn<double> valCol (sel, "VALUES");
    ArrayColumn<double> simvalCol (sel, "SIM_VALUES");
    ArrayColumn<double> simpertCol (sel, "SIM_PERT");
    valCol.put (0, polc.getCoeff().getDoubleMatrix());
  } else {
    putNewCoeff(parmName, polc);
  }
}

void ParmTableAIPS::putDefCoeff (const string& parmName,
				 const MeqPolc& polc)
{
  itsInitTable.reopenRW();
  //const MeqMatrix& values = polc.getCoeff();
  //const MeqMatrix& simvalues = polc.getSimCoeff();
  //const MeqMatrix& pertsimvalues = polc.getPertSimCoeff();
  // First see if the parameter name exists at all.
  *itsInitIndexName = parmName;
  Vector<uInt> rownrs = itsInitIndex->getRowNumbers();
  if (rownrs.nelements() == 1) {
    Table sel = itsInitTable(rownrs);
    uInt rownr=0;
    ScalarColumn<String> namCol (sel, "NAME");
    ScalarColumn<double> t0Col (sel, "TIME0");
    ScalarColumn<double> f0Col (sel, "FREQ0");
    ScalarColumn<bool> normCol (sel, "NORMALIZED");
    ScalarColumn<double> diffCol (sel, "DIFF");
    ScalarColumn<bool> drelCol (sel, "DIFF_REL");
    namCol.put (rownr, parmName);
    ArrayColumn<double> valCol (sel, "VALUES");
    ArrayColumn<double> simvalCol (sel, "SIM_VALUES");
    ArrayColumn<double> simpertCol (sel, "SIM_PERT");
    valCol.put (rownr, polc.getCoeff().getDoubleMatrix());
    simvalCol.put (rownr, polc.getSimCoeff().getDoubleMatrix());
    simpertCol.put (rownr, polc.getPertSimCoeff().getDoubleMatrix());
    t0Col.put   (rownr, polc.getX0());
    f0Col.put   (rownr, polc.getY0());
    normCol.put (rownr, polc.isNormalized());
    diffCol.put (rownr, polc.getPerturbation());
    drelCol.put (rownr, polc.isRelativePerturbation());
  } else if (rownrs.nelements() == 0) {
    putNewDefCoeff(parmName, polc);
  } else {
    ASSERTSTR (false, "Too many default parms with the same name/domain")
  }
}

void ParmTableAIPS::putNewCoeff (const string& parmName, 
				 const MeqPolc& polc)
{
  itsTable.reopenRW();
  uInt rownr = itsTable.nrow();
  itsTable.addRow();
  ScalarColumn<String> namCol (itsTable, "NAME");
  ScalarColumn<double> stCol (itsTable, "STARTTIME");
  ScalarColumn<double> etCol (itsTable, "ENDTIME");
  ScalarColumn<double> sfCol (itsTable, "STARTFREQ");
  ScalarColumn<double> efCol (itsTable, "ENDFREQ");
  ScalarColumn<double> t0Col (itsTable, "TIME0");
  ScalarColumn<double> f0Col (itsTable, "FREQ0");
  ScalarColumn<bool> normCol (itsTable, "NORMALIZED");
  ScalarColumn<double> diffCol (itsTable, "DIFF");
  ScalarColumn<bool> drelCol (itsTable, "DIFF_REL");
  namCol.put (rownr, parmName);
  stCol.put (rownr, polc.domain().startX());
  etCol.put (rownr, polc.domain().endX());
  sfCol.put (rownr, polc.domain().startY());
  efCol.put (rownr, polc.domain().endY());
  ArrayColumn<double> valCol (itsTable, "VALUES");
  ArrayColumn<double> simvalCol (itsTable, "SIM_VALUES");
  ArrayColumn<double> simpertCol (itsTable, "SIM_PERT");
  valCol.put (rownr, polc.getCoeff().getDoubleMatrix());
  simvalCol.put (rownr, polc.getSimCoeff().getDoubleMatrix());
  simpertCol.put (rownr, polc.getPertSimCoeff().getDoubleMatrix());
  t0Col.put   (rownr, polc.getX0());
  f0Col.put   (rownr, polc.getY0());
  normCol.put (rownr, polc.isNormalized());
  diffCol.put (rownr, polc.getPerturbation());
  drelCol.put (rownr, polc.isRelativePerturbation());
}


void ParmTableAIPS::putNewDefCoeff (const string& parmName, 
				    const MeqPolc& polc)
{
  itsTable.reopenRW();
  itsInitTable.reopenRW();

  uInt rownr = itsInitTable.nrow();
  itsInitTable.addRow();
  ScalarColumn<String> namCol (itsInitTable, "NAME");
  //    ScalarColumn<double> stCol (itsInitTable, "STARTTIME");
  //    ScalarColumn<double> etCol (itsInitTable, "ENDTIME");
  //    ScalarColumn<double> sfCol (itsInitTable, "STARTFREQ");
  //    ScalarColumn<double> efCol (itsInitTable, "ENDFREQ");
  ScalarColumn<double> t0Col (itsInitTable, "TIME0");
  ScalarColumn<double> f0Col (itsInitTable, "FREQ0");
  ScalarColumn<bool> normCol (itsInitTable, "NORMALIZED");
  ScalarColumn<double> diffCol (itsInitTable, "DIFF");
  ScalarColumn<bool> drelCol (itsInitTable, "DIFF_REL");
  namCol.put (rownr, parmName);
  //stCol.put (rownr, polc.domain.startX());
  //    etCol.put (rownr, polc.domain.endX());
  //    sfCol.put (rownr, polc.domain.startY());
  //    efCol.put (rownr, polc.domain.endY());
  ArrayColumn<double> valCol (itsInitTable, "VALUES");
  ArrayColumn<double> simvalCol (itsInitTable, "SIM_VALUES");
  ArrayColumn<double> simpertCol (itsInitTable, "SIM_PERT");
  valCol.put (rownr, polc.getCoeff().getDoubleMatrix());
  simvalCol.put (rownr, polc.getSimCoeff().getDoubleMatrix());
  simpertCol.put (rownr, polc.getPertSimCoeff().getDoubleMatrix());
  t0Col.put   (rownr, polc.getX0());
  f0Col.put   (rownr, polc.getY0());
  normCol.put (rownr, polc.isNormalized());
  diffCol.put (rownr, polc.getPerturbation());
  drelCol.put (rownr, polc.isRelativePerturbation());
}

Table ParmTableAIPS::find (const string& parmName,
			   const MeqDomain& domain)
{
  // First see if the parameter name exists at all.
  Table result;
  *itsIndexName = parmName;
  Vector<uInt> rownrs = itsIndex.getRowNumbers();
  if (rownrs.nelements() > 0) {
    Table sel = itsTable(rownrs);
    // Find all rows overlapping the requested domain.
    Table sel3 = sel(domain.startX() < sel.col("ENDTIME")   &&
		     domain.endX()   > sel.col("STARTTIME") &&
		     domain.startY() < sel.col("ENDFREQ")   &&
		     domain.endY()   > sel.col("STARTFREQ"));
    result = sel3;
  }
  return result;
}

vector<string> ParmTableAIPS::getSources()
{
  // Get all parm rows containing RA in the name.
  // Use the DEFAULTTABLE only if available.
  int st = 0;
  Table tab = itsInitTable;
  if (tab.isNull()) {
    tab = itsTable;
    st = 1;
  }
  vector<string> nams;
  for (int i=st; i<2; i++) {
    TableExprNode expr(tab.col("NAME") == Regex(Regex::fromPattern("RA.*")));
    Table sel = tab(expr);
    if (sel.nrow() > 0) {
      // Sort them uniquely on sourcenr.
      Table sor = sel.sort("NAME", Sort::Ascending,
			   Sort::QuickSort | Sort::NoDuplicates);
      ROScalarColumn<String> namCol(sor, "NAME");
      Vector<String> names = namCol.getColumn();
      nams.reserve (nams.size() + sor.nrow());
      for (unsigned int i=0; i<names.nelements(); i++) {
	nams.push_back (names(i));
      }
    }
    tab = itsTable;
  }
  LOG_TRACE_STAT_STR("Finished retreiving "<<nams.size()<<" sources");
  //cout<<"Finished retreiving "<<nams.size()<<" sources"<<endl;
  return nams;
}

void ParmTableAIPS::unlock()
{
  itsTable.unlock();
}

}
