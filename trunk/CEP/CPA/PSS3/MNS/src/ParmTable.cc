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
#include <MNS/MeqMatrix.h>
#include <Common/Debug.h>
#include <aips/Tables/ExprNode.h>
#include <aips/Tables/ScalarColumn.h>
#include <aips/Tables/ArrayColumn.h>
#include <aips/Tables/TableRecord.h>
#include <aips/Arrays/Matrix.h>

ParmTable::ParmTable (const string& tableName)
: itsTable         (tableName, Table::Update),
  itsNameIndex     (itsTable, "Name"),
  itsIndexField    (itsNameIndex.accessKey(), "Name"),
  itsInitNameIndex (0)
{
  if (itsTable.keywordSet().isDefined ("InitialValues")) {
    itsInitTable = itsTable.keywordSet().asTable ("InitialValues");
    itsInitNameIndex = new ColumnsIndex (itsInitTable, "Name");
    itsInitIndexField = RecordFieldPtr<String> (itsInitNameIndex->accessKey(),
						"Name");
  }
}

MeqMatrix ParmTable::getValues (bool& matchDomain,
				const string& parmName,
				const MeqDomain& domain)
{
  MeqMatrix result;
  Table sel = find (matchDomain, parmName, domain);
  if (sel.nrow() > 0) {
    ROArrayColumn<Double> valCol (sel, "RValues");
    if (valCol.isDefined (0)) {
      result = Matrix<double>(valCol(0));
    } else {
      ROArrayColumn<DComplex> valDCol (sel, "CValues");
      result = Matrix<DComplex>(valDCol(0));
    }
  }
  return result;
}

MeqMatrix ParmTable::getInitValues (const string& parmName)
{
  // Try to find the default initial values in the InitialValues subtable.
  // The parameter name consists of parts (separated by dots), so the
  // parameters are categorised in that way.
  // An initial value can be defined for the full name or for a higher
  // category.
  // So look up until found or until no more parts are left.
  MeqMatrix result;
  if (itsInitNameIndex) {
    string name = parmName;
    while (true) {
      *itsInitIndexField = name;
      Vector<uInt> rownrs = itsNameIndex.getRowNumbers();
      if (rownrs.nelements() > 0) {
	Assert (rownrs.nelements() == 1);
	ROArrayColumn<Double> valCol (itsInitTable, "RValues");
	if (valCol.isDefined (rownrs(0))) {
	  result = Matrix<double>(valCol(rownrs(0)));
	} else {
	  ROArrayColumn<DComplex> valDCol (itsInitTable, "CValues");
	  result = Matrix<DComplex>(valDCol(rownrs(0)));
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
				    
void ParmTable::putValues (const string& parmName,
			   const MeqDomain& domain,
			   const MeqMatrix& values)
{
  bool matchDomain;
  Table sel = find (matchDomain, parmName, domain);
  if (matchDomain  &&  sel.nrow() > 0) {
    if (values.isDouble()) {
      ArrayColumn<Double> valCol (sel, "RValues");
      valCol.put (0, values.getDoubleMatrix());
    } else {
      ArrayColumn<DComplex> valCol (sel, "CValues");
      valCol.put (0, values.getDComplexMatrix());
    }
  } else {
    uInt rownr = itsTable.nrow();
    itsTable.addRow();
    ScalarColumn<String> namCol (itsTable, "Name");
    ScalarColumn<Double> stCol (itsTable, "StartTime");
    ScalarColumn<Double> etCol (itsTable, "EndTime");
    ScalarColumn<Double> sfCol (itsTable, "StartFreq");
    ScalarColumn<Double> efCol (itsTable, "EndFreq");
    namCol.put (rownr, parmName);
    stCol.put (rownr, domain.startX());
    etCol.put (rownr, domain.endX());
    sfCol.put (rownr, domain.startY());
    efCol.put (rownr, domain.endY());
    if (values.isDouble()) {
      ArrayColumn<Double> valCol (itsTable, "RValues");
      valCol.put (rownr, values.getDoubleMatrix());
    } else {
      ArrayColumn<DComplex> valCol (itsTable, "CValues");
      valCol.put (rownr, values.getDComplexMatrix());
    }
  }
}

Table ParmTable::find (bool& matchDomain, const string& parmName,
		       const MeqDomain& domain)
{
  // First see if the parameter name exists at all.
  matchDomain = false;
  Table result;
  *itsIndexField = parmName;
  Vector<uInt> rownrs = itsNameIndex.getRowNumbers();
  if (rownrs.nelements() > 0) {
    Table sel = itsTable(rownrs);
    // Now see if an exact domain match is found (and exactly 1).
    Table sel2 = sel(near(domain.startX(), sel.col("StartTime"))  &&
		     near(domain.endX(), sel.col("EndTime")) &&
		     near(domain.startY(), sel.col("StartFreq"))  &&
		     near(domain.endY(), sel.col("EndFreq")));
    if (sel2.nrow() > 0) {
      AssertMsg (sel2.nrow()==1, "Parameter " << parmName <<
		 " has multiple exact entries for time "
		 << domain.startX() << ':' << domain.endX() << " and freq "
		 << domain.startY() << ':' << domain.endY());
      result = sel2;
      matchDomain = true;
    } else {
      // No exact domain match.
      // Now see if a wider domain match is found (and exactly 1).
      Table sel3 = sel((domain.startX() >= sel.col("StartTime")  ||
			near(domain.startX(), sel.col("StartTime")))  && 
		       (domain.endX() <= sel.col("StartTime")    ||
			near(domain.endX(), sel.col("EndTime")))      &&
		       (domain.startY() >= sel.col("StartFreq")  ||
			near(domain.startY(), sel.col("StartFreq")))  && 
		       (domain.endY() <= sel.col("StartFreq")    ||
			near(domain.endY(), sel.col("EndFreq"))));
      if (sel3.nrow() > 0) {
	AssertMsg (sel3.nrow()==1, "Parameter " << parmName <<
		   " has multiple wider entries for time "
		   << domain.startX() << ':' << domain.endX() << " and freq "
		   << domain.startY() << ':' << domain.endY());
	result = sel3;
      }
    }
  }
  return result;
}
