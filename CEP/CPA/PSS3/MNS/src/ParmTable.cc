//# ParmTable.cc: Object to hold parameters in a table.
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

#include <MNS/ParmTable.h>
#include <MNS/MeqDomain.h>
#include <MNS/MeqStoredParmPolc.h>
#include <MNS/MeqPointSource.h>
#include <Common/Debug.h>
#include <aips/Tables/ExprNode.h>
#include <aips/Tables/ExprNodeSet.h>
#include <aips/Tables/ScalarColumn.h>
#include <aips/Tables/ArrayColumn.h>
#include <aips/Tables/TableRecord.h>
#include <aips/Arrays/Matrix.h>
#include <aips/Arrays/Vector.h>
#include <aips/Arrays/ArrayUtil.h>
#include <aips/Utilities/Regex.h>
#include <aips/Mathematics/Math.h>

ParmTable::ParmTable (const string& tableName)
: itsTable       (tableName),
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

vector<MeqPolc> ParmTable::getPolcs (const string& parmName,
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
    ROArrayColumn<bool> maskCol (sel, "MASK");
    ROArrayColumn<double> valCol (sel, "RVALUES");
    ROScalarColumn<double> diffCol (sel, "DIFF");
    ROScalarColumn<bool> drelCol (sel, "DIFF_REL");
    if (valCol.isDefined (0)) {
      ROArrayColumn<double> simvalCol (sel, "SIM_RVALUES");
      ROArrayColumn<double> simpertCol (sel, "SIM_RPERT");
      for (unsigned int i=0; i<sel.nrow(); i++) {
	MeqPolc polc;
	if (maskCol.isDefined(i)) {
	  polc.setCoeff (Matrix<double>(valCol(i)), maskCol(i));
	} else {
	  polc.setCoeff (Matrix<double>(valCol(i)));
	}
	polc.setSimCoeff (Matrix<double>(simvalCol(i)));
	polc.setPertSimCoeff (Matrix<double>(simpertCol(i)));
	polc.setDomain (MeqDomain(stCol(i), etCol(i), sfCol(i), efCol(i)));
	polc.setPerturbation (diffCol(i), drelCol(i));
	result.push_back (polc);
      }
    } else {
      ROArrayColumn<DComplex> valDCol (sel, "CVALUES");
      ROArrayColumn<DComplex> simvalDCol (sel, "SIM_CVALUES");
      ROArrayColumn<DComplex> simpertDCol (sel, "SIM_CPERT");
      for (unsigned int i=0; i<sel.nrow(); i++) {
	MeqPolc polc;
	if (maskCol.isDefined(i)) {
	  polc.setCoeff (Matrix<DComplex>(valDCol(i)), maskCol(i));
	} else {
	  polc.setCoeff (Matrix<DComplex>(valDCol(i)));
	}
	polc.setSimCoeff (Matrix<DComplex>(simvalDCol(i)));
	polc.setPertSimCoeff (Matrix<DComplex>(simpertDCol(i)));
	polc.setDomain (MeqDomain(stCol(i), etCol(i), sfCol(i), efCol(i)));
	polc.setPerturbation (diffCol(i), drelCol(i));
	result.push_back (polc);
      }
    }
  }
  return result;
}

MeqPolc ParmTable::getInitCoeff (const string& parmName,
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
	    ROArrayColumn<bool> maskCol (itsInitTable, "MASK");
	    ROArrayColumn<Double> valCol (itsInitTable, "RVALUES");
	    ROScalarColumn<double> diffCol (itsInitTable, "DIFF");
	    ROScalarColumn<bool> drelCol (itsInitTable, "DIFF_REL");
	    if (valCol.isDefined (row)) {
	      ROArrayColumn<double> simvalCol (itsInitTable, "SIM_RVALUES");
	      ROArrayColumn<double> simpertCol (itsInitTable, "SIM_RPERT");
	      if (maskCol.isDefined(row)) {
		result.setCoeff (Matrix<double>(valCol(row)), maskCol(row));
	      } else {
		result.setCoeff (Matrix<double>(valCol(row)));
	      }
	      result.setSimCoeff (Matrix<double>(simvalCol(row)));
	      result.setPertSimCoeff (Matrix<double>(simpertCol(row)));
	    } else {
	      ROArrayColumn<DComplex> valDCol (itsInitTable, "CVALUES");
	      ROArrayColumn<DComplex> simvalDCol (itsInitTable, "SIM_CVALUES");
	      ROArrayColumn<DComplex> simpertDCol (itsInitTable, "SIM_CPERT");
	      if (maskCol.isDefined(row)) {
		result.setCoeff (Matrix<DComplex>(valDCol(row)), maskCol(row));
	      } else {
		result.setCoeff (Matrix<DComplex>(valDCol(row)));
	      }
	      result.setSimCoeff (Matrix<DComplex>(simvalDCol(row)));
	      result.setPertSimCoeff (Matrix<DComplex>(simpertDCol(row)));
	    }
	    result.setPerturbation (diffCol(row), drelCol(row));
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
				    
void ParmTable::putCoeff (const string& parmName,
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
    if (values.isDouble()) {
      ArrayColumn<double> valCol (sel, "RVALUES");
      ArrayColumn<double> simvalCol (sel, "SIM_RVALUES");
      ArrayColumn<double> simpertCol (sel, "SIM_RPERT");
      valCol.put (0, values.getDoubleMatrix());
    } else {
      ArrayColumn<DComplex> valCol (sel, "CVALUES");
      ArrayColumn<DComplex> simvalCol (sel, "SIM_CVALUES");
      ArrayColumn<DComplex> simpertCol (sel, "SIM_CPERT");
      valCol.put (0, values.getDComplexMatrix());
    }
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
    ScalarColumn<double> diffCol (itsTable, "DIFF");
    ScalarColumn<bool> drelCol (itsTable, "DIFF_REL");
    namCol.put (rownr, parmName);
    srcCol.put (rownr, srcnr);
    statCol.put (rownr, statnr);
    stCol.put (rownr, domain.startX());
    etCol.put (rownr, domain.endX());
    sfCol.put (rownr, domain.startY());
    efCol.put (rownr, domain.endY());
    if (values.isDouble()) {
      ArrayColumn<double> valCol (itsTable, "RVALUES");
      ArrayColumn<double> simvalCol (itsTable, "SIM_RVALUES");
      ArrayColumn<double> simpertCol (itsTable, "SIM_RPERT");
      valCol.put (rownr, values.getDoubleMatrix());
      simvalCol.put (rownr, simvalues.getDoubleMatrix());
      simpertCol.put (rownr, pertsimvalues.getDoubleMatrix());
    } else {
      ArrayColumn<DComplex> valCol (itsTable, "CVALUES");
      ArrayColumn<DComplex> simvalCol (itsTable, "SIM_CVALUES");
      ArrayColumn<DComplex> simpertCol (itsTable, "SIM_CPERT");
      valCol.put (rownr, values.getDComplexMatrix());
      simvalCol.put (rownr, simvalues.getDComplexMatrix());
      simpertCol.put (rownr, pertsimvalues.getDComplexMatrix());
    }
    diffCol.put (rownr, polc.getPerturbation());
    drelCol.put (rownr, polc.isRelativePerturbation());
  }
}

Table ParmTable::find (const string& parmName,
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


MeqSourceList ParmTable::getPointSources (const Vector<int>& srcnrs)
{
  // Get all parm rows containing RA in the name.
  // Use the InitTable if available.
  Table sel;
  if (itsInitTable.isNull()) {
    TableExprNode expr(itsTable.col("NAME") ==
                                      Regex(Regex::fromPattern("RA.*")));
    if (srcnrs.nelements() > 0) {
      expr = expr  &&  itsTable.col("SRCNR").in (TableExprNodeSet(srcnrs));
    }
    sel = itsTable(expr);
  } else {
    TableExprNode expr(itsInitTable.col("NAME") ==
                                      Regex(Regex::fromPattern("RA.*")));
    if (srcnrs.nelements() > 0) {
      expr = expr  &&  itsInitTable.col("SRCNR").in (TableExprNodeSet(srcnrs));
    }
    sel = itsInitTable(expr);
  }
  // Sort them uniquely on sourcenr.
  Table sor = sel.sort("SRCNR", Sort::Ascending,
		       Sort::QuickSort | Sort::NoDuplicates);
  AssertMsg (sel.nrow() == sor.nrow(),
	     "Only constant GSM parameters are supported");
  MeqSourceList sources;
  ROScalarColumn<int> srcCol(sor, "SRCNR");
  ROScalarColumn<String> nameCol(sor, "NAME");
  for (unsigned int row=0; row<sor.nrow(); row++) {
    int srcnr = srcCol(row);
    string name = nameCol(row);
    string::size_type idx = name.rfind ('.');
    Assert (idx != string::npos);
    // Remove first part (RA).
    name = name.substr (idx+1);
    MeqStoredParmPolc* mr = new MeqStoredParmPolc("RA."+name,
						  srcnr, -1, this);
    MeqStoredParmPolc* md = new MeqStoredParmPolc("DEC."+name,
						  srcnr, -1, this);
    MeqStoredParmPolc* mi = new MeqStoredParmPolc("StokesI."+name,
						  srcnr, -1, this);
    MeqStoredParmPolc* mq = new MeqStoredParmPolc("StokesQ."+name,
						  srcnr, -1, this);
    MeqStoredParmPolc* mu = new MeqStoredParmPolc("StokesU."+name,
						  srcnr, -1, this);
    MeqStoredParmPolc* mv = new MeqStoredParmPolc("StokesV."+name,
						  srcnr, -1, this);
    sources.add (MeqPointSource(name, mi, mq, mu, mv, mr, md));
    cout << "Found source " << name << " (srcnr=" << srcnr << ')' << endl;
  }
  return sources;
}
