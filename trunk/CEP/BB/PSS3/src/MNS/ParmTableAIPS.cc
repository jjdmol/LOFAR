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

#include <PSS3/MNS/ParmTableAIPS.h>
#include <PSS3/MNS/MeqDomain.h>
#include <Common/Debug.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ExprNodeSet.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicMath/Math.h>


namespace LOFAR {

ParmTableAIPS::ParmTableAIPS (const string& tableName)
: itsTable       (tableName+".MEP"),
  itsIndex       (itsTable, stringToVector("SRCNR,STATNR,NAME")),
  itsIndexSrcnr  (itsIndex.accessKey(), "SRCNR"),
  itsIndexStatnr (itsIndex.accessKey(), "STATNR"),
  itsIndexName   (itsIndex.accessKey(), "NAME"),
  itsInitIndex   (0)
{
  if (itsTable.keywordSet().isDefined ("DEFAULTVALUES")) {
    itsInitTable = itsTable.keywordSet().asTable ("DEFAULTVALUES");
    itsInitIndex = new ColumnsIndex (itsInitTable,
				     stringToVector("SRCNR,STATNR,NAME"));

    itsInitIndexSrcnr  = RecordFieldPtr<Int> (itsInitIndex->accessKey(),
					      "SRCNR");
    itsInitIndexStatnr = RecordFieldPtr<Int> (itsInitIndex->accessKey(),
					      "STATNR");
    itsInitIndexName   = RecordFieldPtr<String> (itsInitIndex->accessKey(),
						 "NAME");
  }
}

ParmTableAIPS::~ParmTableAIPS()
{
  delete itsInitIndex;
}

vector<MeqPolc> ParmTableAIPS::getPolcs (const string& parmName,
					 int srcnr, int statnr,
					 const MeqDomain& domain)
{
  vector<MeqPolc> result;
  Table sel = find (parmName, srcnr, statnr, domain);
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

MeqPolc ParmTableAIPS::getInitCoeff (const string& parmName,
				     int srcnr, int statnr)
{
  // Try to find the default initial values in the InitialValues subtable.
  // The parameter name consists of parts (separated by dots), so the
  // parameters are categorised in that way.
  // An initial value can be defined for the full name or for a higher
  // category.
  // So look up until found or until no more parts are left.
  MeqPolc result;
  if (itsInitIndex) {
    *itsInitIndexSrcnr = srcnr;
    for (int i=0; i<2; i++) {
      *itsInitIndexStatnr = statnr;
      for (int j=0; j<2; j++) {
	string name = parmName;
	while (true) {
	  *itsInitIndexName   = name;
	  Vector<uInt> rownrs = itsInitIndex->getRowNumbers();
	  if (rownrs.nelements() > 0) {
	    Assert (rownrs.nelements() == 1);
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
	// Try to find if for any station.
	*itsInitIndexStatnr = -1;
      }
	// Try to find if for any source.
      *itsInitIndexSrcnr = -1;
    }
  }
  return result;
}
				    
void ParmTableAIPS::putCoeff (const string& parmName,
			      int srcnr, int statnr,
			      const MeqPolc& polc)
{
  itsTable.reopenRW();
  const MeqDomain& domain = polc.domain();
  const MeqMatrix& values = polc.getCoeff();
  const MeqMatrix& simvalues = polc.getSimCoeff();
  const MeqMatrix& pertsimvalues = polc.getPertSimCoeff();
  Table sel = find (parmName, srcnr, statnr, domain);
  if (sel.nrow() > 0) {
    AssertMsg (sel.nrow()==1, "Parameter " << parmName <<
		 " has multiple entries for time "
		 << domain.startX() << ':' << domain.endX() << " and freq "
		 << domain.startY() << ':' << domain.endY());
    ROScalarColumn<double> stCol (sel, "STARTTIME");
    ROScalarColumn<double> etCol (sel, "ENDTIME");
    ROScalarColumn<double> sfCol (sel, "STARTFREQ");
    ROScalarColumn<double> efCol (sel, "ENDFREQ");
    AssertMsg (near(domain.startX(), stCol(0))  &&
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
    valCol.put (0, values.getDoubleMatrix());
  } else {
    uInt rownr = itsTable.nrow();
    itsTable.addRow();
    ScalarColumn<String> namCol (itsTable, "NAME");
    ScalarColumn<int> srcCol   (itsTable, "SRCNR");
    ScalarColumn<int> statCol  (itsTable, "STATNR");
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
    srcCol.put (rownr, srcnr);
    statCol.put (rownr, statnr);
    stCol.put (rownr, domain.startX());
    etCol.put (rownr, domain.endX());
    sfCol.put (rownr, domain.startY());
    efCol.put (rownr, domain.endY());
    ArrayColumn<double> valCol (itsTable, "VALUES");
    ArrayColumn<double> simvalCol (itsTable, "SIM_VALUES");
    ArrayColumn<double> simpertCol (itsTable, "SIM_PERT");
    valCol.put (rownr, values.getDoubleMatrix());
    simvalCol.put (rownr, simvalues.getDoubleMatrix());
    simpertCol.put (rownr, pertsimvalues.getDoubleMatrix());
    t0Col.put   (rownr, polc.getX0());
    f0Col.put   (rownr, polc.getY0());
    normCol.put (rownr, polc.isNormalized());
    diffCol.put (rownr, polc.getPerturbation());
    drelCol.put (rownr, polc.isRelativePerturbation());
  }
}

Table ParmTableAIPS::find (const string& parmName,
			   int srcnr, int statnr,
			   const MeqDomain& domain)
{
  // First see if the parameter name exists at all.
  Table result;
  *itsIndexSrcnr  = srcnr;
  *itsIndexStatnr = statnr;
  *itsIndexName   = parmName;
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
      Table sor = sel.sort("SRCNR", Sort::Ascending,
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
  return nams;
}

void ParmTableAIPS::unlock()
{
  itsTable.unlock();
}

}
