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
#include <Common/Debug.h>
#include <aips/Tables/ExprNode.h>
#include <aips/Tables/ScalarColumn.h>
#include <aips/Tables/ArrayColumn.h>
#include <aips/Tables/TableRecord.h>
#include <aips/Arrays/Matrix.h>
#include <aips/Mathematics/Math.h>

ParmTable::ParmTable (const string& tableName)
: itsTable         (tableName),
  itsNameIndex     (itsTable, "NAME"),
  itsIndexField    (itsNameIndex.accessKey(), "NAME"),
  itsInitNameIndex (0)
{
  if (itsTable.keywordSet().isDefined ("INITIALVALUES")) {
    itsInitTable = itsTable.keywordSet().asTable ("INITIALVALUES");
    itsInitNameIndex = new ColumnsIndex (itsInitTable, "NAME");
    itsInitIndexField = RecordFieldPtr<String> (itsInitNameIndex->accessKey(),
						"NAME");
  }
}

vector<MeqPolc> ParmTable::getPolcs (const string& parmName,
				     const MeqDomain& domain)
{
  vector<MeqPolc> result;
  Table sel = find (parmName, domain);
  if (sel.nrow() > 0) {
    ROScalarColumn<double> stCol (sel, "STARTTIME");
    ROScalarColumn<double> etCol (sel, "ENDTIME");
    ROScalarColumn<double> sfCol (sel, "STARTFREQ");
    ROScalarColumn<double> efCol (sel, "ENDFREQ");
    ROArrayColumn<bool> maskCol (sel, "MASK");
    ROArrayColumn<Double> valCol (sel, "RVALUES");
    if (valCol.isDefined (0)) {
      for (unsigned int i=0; i<sel.nrow(); i++) {
	MeqPolc polc;
	if (maskCol.isDefined(i)) {
	  polc.setCoeff (Matrix<double>(valCol(i)), maskCol(i));
	} else {
	  polc.setCoeff (Matrix<double>(valCol(i)));
	}
	polc.setDomain (MeqDomain(stCol(i), etCol(i), sfCol(i), efCol(i)));
	result.push_back (polc);
      }
    } else {
      ROArrayColumn<DComplex> valDCol (sel, "CVALUES");
      for (unsigned int i=0; i<sel.nrow(); i++) {
	MeqPolc polc;
	if (maskCol.isDefined(i)) {
	  polc.setCoeff (Matrix<DComplex>(valDCol(i)), maskCol(i));
	} else {
	  polc.setCoeff (Matrix<DComplex>(valDCol(i)));
	}
	polc.setDomain (MeqDomain(stCol(i), etCol(i), sfCol(i), efCol(i)));
	result.push_back (polc);
      }
    }
  }
  return result;
}

MeqPolc ParmTable::getInitCoeff (const string& parmName)
{
  // Try to find the default initial values in the InitialValues subtable.
  // The parameter name consists of parts (separated by dots), so the
  // parameters are categorised in that way.
  // An initial value can be defined for the full name or for a higher
  // category.
  // So look up until found or until no more parts are left.
  MeqPolc result;
  if (itsInitNameIndex) {
    string name = parmName;
    while (true) {
      *itsInitIndexField = name;
      Vector<uInt> rownrs = itsInitNameIndex->getRowNumbers();
      if (rownrs.nelements() > 0) {
	Assert (rownrs.nelements() == 1);
	int row = rownrs(0);
	ROArrayColumn<bool> maskCol (itsInitTable, "MASK");
	ROArrayColumn<Double> valCol (itsInitTable, "RVALUES");
	if (valCol.isDefined (row)) {
	  if (maskCol.isDefined(row)) {
	    result.setCoeff (Matrix<double>(valCol(row)), maskCol(row));
	  } else {
	    result.setCoeff (Matrix<double>(valCol(row)));
	  }
	} else {
	  ROArrayColumn<DComplex> valDCol (itsInitTable, "CVALUES");
	  if (maskCol.isDefined(row)) {
	    result.setCoeff (Matrix<DComplex>(valDCol(row)), maskCol(row));
	  } else {
	    result.setCoeff (Matrix<DComplex>(valDCol(row)));
	  }
	}
	break;
      }
      string::size_type idx = name.rfind ('.');
      if (idx == string::npos) {
	break;
      }
      name = name.substr (0, idx);
    }
  }
  return result;
}
				    
void ParmTable::putCoeff (const string& parmName,
			  const MeqDomain& domain,
			  const MeqMatrix& values)
{
  itsTable.reopenRW();
  Table sel = find (parmName, domain);
  if (sel.nrow() > 0) {
    AssertMsg (sel.nrow()==1, "Parameter " << parmName <<
		 " has multiple entries for time "
		 << domain.startX() << ':' << domain.endX() << " and freq "
		 << domain.startY() << ':' << domain.endY());
    ROScalarColumn<Double> stCol (sel, "STARTTIME");
    ROScalarColumn<Double> etCol (sel, "ENDTIME");
    ROScalarColumn<Double> sfCol (sel, "STARTFREQ");
    ROScalarColumn<Double> efCol (sel, "ENDFREQ");
    AssertMsg (near(domain.startX(), stCol(0))  &&
	       near(domain.endX(), etCol(0))  &&
	       near(domain.startY(), sfCol(0))  &&
	       near(domain.endY(), efCol(0)),
	       "Parameter " << parmName <<
	       " has a partially instead of fully matching entry for time "
		 << domain.startX() << ':' << domain.endX() << " and freq "
		 << domain.startY() << ':' << domain.endY());
    if (values.isDouble()) {
      ArrayColumn<Double> valCol (sel, "RVALUES");
      valCol.put (0, values.getDoubleMatrix());
    } else {
      ArrayColumn<DComplex> valCol (sel, "CVALUES");
      valCol.put (0, values.getDComplexMatrix());
    }
  } else {
    uInt rownr = itsTable.nrow();
    itsTable.addRow();
    ScalarColumn<String> namCol (itsTable, "NAME");
    ScalarColumn<Double> stCol (itsTable, "STARTTIME");
    ScalarColumn<Double> etCol (itsTable, "ENDTIME");
    ScalarColumn<Double> sfCol (itsTable, "STARTFREQ");
    ScalarColumn<Double> efCol (itsTable, "ENDFREQ");
    namCol.put (rownr, parmName);
    stCol.put (rownr, domain.startX());
    etCol.put (rownr, domain.endX());
    sfCol.put (rownr, domain.startY());
    efCol.put (rownr, domain.endY());
    if (values.isDouble()) {
      ArrayColumn<Double> valCol (itsTable, "RVALUES");
      valCol.put (rownr, values.getDoubleMatrix());
    } else {
      ArrayColumn<DComplex> valCol (itsTable, "CVALUES");
      valCol.put (rownr, values.getDComplexMatrix());
    }
  }
}

Table ParmTable::find (const string& parmName, const MeqDomain& domain)
{
  // First see if the parameter name exists at all.
  Table result;
  *itsIndexField = parmName;
  Vector<uInt> rownrs = itsNameIndex.getRowNumbers();
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
