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

#include <MEQ/ParmTable.h>
#include <MEQ/Domain.h>
#include <Common/Debug.h>
#include <aips/Tables/TableDesc.h>
#include <aips/Tables/ScaColDesc.h>
#include <aips/Tables/ArrColDesc.h>
#include <aips/Tables/SetupNewTab.h>
#include <aips/Tables/ExprNode.h>
#include <aips/Tables/ExprNodeSet.h>
#include <aips/Tables/ScalarColumn.h>
#include <aips/Tables/ArrayColumn.h>
#include <aips/Tables/TableRecord.h>
#include <aips/Arrays/Matrix.h>
#include <aips/Arrays/Vector.h>
#include <aips/Arrays/ArrayUtil.h>
#include <aips/Arrays/Slice.h>
#include <aips/Utilities/Regex.h>
#include <aips/Utilities/GenSort.h>
#include <aips/Mathematics/Math.h>

namespace Meq {

//##ModelId=3F95060D031A
std::map<string, ParmTable*> ParmTable::theirTables;


Matrix<double> toParmMatrix (const Vells& values)
{
  return Matrix<double> (IPosition(2, values.nx(), values.ny()),
			 const_cast<double*>(values.realStorage()),
			 SHARE);
}

Vells fromParmMatrix (const Array<double>& values)
{
  Assert (values.ndim() == 2);
  LoMat_double mat(values.data(),
		   LoMatShape(values.shape()[0], values.shape()[1]),
		   blitz::duplicateData);
  return Vells(mat);
}

//##ModelId=3F86886F02B7
ParmTable::ParmTable (const string& tableName)
: itsTable       (tableName),
  itsIndex       (itsTable, "NAME"),
  itsIndexName   (itsIndex.accessKey(), "NAME"),
  itsInitIndex   (0)
{
  if (itsTable.keywordSet().isDefined ("DEFAULTVALUES")) {
    itsInitTable = itsTable.keywordSet().asTable ("DEFAULTVALUES");
    itsInitIndex = new ColumnsIndex (itsInitTable, "NAME");
    itsInitIndexName = RecordFieldPtr<String> (itsInitIndex->accessKey(),
					       "NAME");
  }
}

//##ModelId=3F86886F02BC
ParmTable::~ParmTable()
{
  delete itsInitIndex;
}

//##ModelId=3F86886F02BD
vector<Polc> ParmTable::getPolcs (const string& parmName,
				  const Domain& domain)
{
  vector<Polc> result;
  Table sel = find (parmName, domain);
  if (sel.nrow() > 0) {
    ROScalarColumn<double> sfCol (sel, "STARTFREQ");
    ROScalarColumn<double> efCol (sel, "ENDFREQ");
    ROScalarColumn<double> stCol (sel, "STARTTIME");
    ROScalarColumn<double> etCol (sel, "ENDTIME");
    ROArrayColumn<double> valCol (sel, "VALUES");
    ROScalarColumn<double> f0Col (sel, "FREQ0");
    ROScalarColumn<double> t0Col (sel, "TIME0");
    ROScalarColumn<double> fsCol (sel, "FREQSCALE");
    ROScalarColumn<double> tsCol (sel, "TIMESCALE");
    ROScalarColumn<double> diffCol (sel, "DIFF");
    for (unsigned int i=0; i<sel.nrow(); i++) {
      Polc polc;
      polc.setCoeff (fromParmMatrix(valCol(i)));
      polc.setFreq0 (f0Col(i));
      polc.setTime0 (t0Col(i));
      polc.setFreqScale (fsCol(i));
      polc.setTimeScale (tsCol(i));
      polc.setDomain (Domain(stCol(i), etCol(i), sfCol(i), efCol(i)));
      polc.setPerturbation (diffCol(i));
      result.push_back (polc);
    }
  }
  return result;
}

//##ModelId=3F86886F02C3
Polc ParmTable::getInitCoeff (const string& parmName)
{
  // Try to find the default initial values in the InitialValues subtable.
  // The parameter name consists of parts (separated by dots), so the
  // parameters are categorised in that way.
  // An initial value can be defined for the full name or for a higher
  // category.
  // So look up until found or until no more parts are left.
  Polc result;
  if (itsInitIndex) {
    string name = parmName;
    while (true) {
      *itsInitIndexName   = name;
      Vector<uInt> rownrs = itsInitIndex->getRowNumbers();
      if (rownrs.nelements() > 0) {
	Assert (rownrs.nelements() == 1);
	int row = rownrs(0);
	ROArrayColumn<bool> maskCol (itsInitTable, "SOLVABLE");
	ROArrayColumn<Double> valCol (itsInitTable, "VALUES");
	ROScalarColumn<double> f0Col (itsInitTable, "FREQ0");
	ROScalarColumn<double> t0Col (itsInitTable, "TIME0");
	ROScalarColumn<double> fsCol (itsInitTable, "FREQSCALE");
	ROScalarColumn<double> tsCol (itsInitTable, "TIMESCALE");
	ROScalarColumn<double> diffCol (itsInitTable, "DIFF");
	result.setCoeff (fromParmMatrix(valCol(row)));
	result.setFreq0 (f0Col(row));
	result.setTime0 (t0Col(row));
	result.setFreqScale (fsCol(row));
	result.setTimeScale (tsCol(row));
	result.setPerturbation (diffCol(row));
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
				    
//##ModelId=3F86886F02C8
void ParmTable::putCoeff (const string& parmName, const Polc& polc)
{
  itsTable.reopenRW();
  const Domain& domain = polc.domain();
  const Vells& values = polc.getCoeff();
  Table sel = find (parmName, domain);
  if (sel.nrow() > 0) {
    AssertMsg (sel.nrow()==1, "Parameter " << parmName <<
		 " has multiple entries for freq "
		 << domain.startFreq() << ':' << domain.endFreq()
	         << " and time "
		 << domain.startTime() << ':' << domain.endTime());
    ROScalarColumn<double> sfCol (sel, "STARTFREQ");
    ROScalarColumn<double> efCol (sel, "ENDFREQ");
    ROScalarColumn<double> stCol (sel, "STARTTIME");
    ROScalarColumn<double> etCol (sel, "ENDTIME");
    AssertMsg (near(domain.startFreq(), sfCol(0))  &&
	       near(domain.endFreq(), efCol(0))  &&
	       near(domain.startTime(), stCol(0))  &&
	       near(domain.endTime(), etCol(0)),
	       "Parameter " << parmName <<
	       " has a partially instead of fully matching entry for freq "
		 << domain.startFreq() << ':' << domain.endFreq()
	         << " and time "
		 << domain.startTime() << ':' << domain.endTime());
    ArrayColumn<double> valCol (sel, "VALUES");
    valCol.put (0, toParmMatrix(values));
  } else {
    uInt rownr = itsTable.nrow();
    itsTable.addRow();
    ScalarColumn<String> namCol (itsTable, "NAME");
    ScalarColumn<double> sfCol (itsTable, "STARTFREQ");
    ScalarColumn<double> efCol (itsTable, "ENDFREQ");
    ScalarColumn<double> stCol (itsTable, "STARTTIME");
    ScalarColumn<double> etCol (itsTable, "ENDTIME");
    ScalarColumn<double> f0Col (itsTable, "FREQ0");
    ScalarColumn<double> t0Col (itsTable, "TIME0");
    ScalarColumn<double> fsCol (itsTable, "FREQSCALE");
    ScalarColumn<double> tsCol (itsTable, "TIMESCALE");
    ScalarColumn<double> diffCol (itsTable, "DIFF");
    namCol.put (rownr, parmName);
    sfCol.put (rownr, domain.startFreq());
    efCol.put (rownr, domain.endFreq());
    stCol.put (rownr, domain.startTime());
    etCol.put (rownr, domain.endTime());
    ArrayColumn<double> valCol (itsTable, "VALUES");
    valCol.put (rownr, toParmMatrix(values));
    f0Col.put   (rownr, polc.getFreq0());
    t0Col.put   (rownr, polc.getTime0());
    fsCol.put   (rownr, polc.getFreqScale());
    tsCol.put   (rownr, polc.getTimeScale());
    diffCol.put (rownr, polc.getPerturbation());
  }
}

//##ModelId=3F86886F02CE
Table ParmTable::find (const string& parmName,
		       const Domain& domain)
{
  // First see if the parameter name exists at all.
  Table result;
  *itsIndexName   = parmName;
  Vector<uInt> rownrs = itsIndex.getRowNumbers();
  if (rownrs.nelements() > 0) {
    Table sel = itsTable(rownrs);
    // Find all rows overlapping the requested domain.
    Table sel3 = sel(domain.startFreq() < sel.col("ENDFREQ")   &&
		     domain.endFreq()   > sel.col("STARTFREQ") &&
		     domain.startTime() < sel.col("ENDTIME")   &&
		     domain.endTime()   > sel.col("STARTTIME"));
    result = sel3;
  }
  return result;
}

//##ModelId=3F95060D033E
ParmTable* ParmTable::openTable (const String& tableName)
{
  std::map<string,ParmTable*>::const_iterator p = theirTables.find(tableName);
  if (p != theirTables.end()) {
    return p->second;
  }
  ParmTable* tab = new ParmTable(tableName);
  theirTables[tableName] = tab;
  return tab;
}

//##ModelId=3F95060D0372
void ParmTable::closeTables()
{
  for (std::map<string,ParmTable*>::const_iterator iter = theirTables.begin();
       iter != theirTables.end();
       ++iter) {
    delete iter->second;
  }
  theirTables.clear();
}

//##ModelId=400E535402E7
void ParmTable::createTable (const String& tableName)
{
  TableDesc tdesc;
  tdesc.addColumn (ScalarColumnDesc<String>("NAME"));
  tdesc.addColumn (ScalarColumnDesc<Double>("ENDTIME"));
  tdesc.addColumn (ScalarColumnDesc<Double>("STARTTIME"));
  tdesc.addColumn (ScalarColumnDesc<Double>("ENDFREQ"));
  tdesc.addColumn (ScalarColumnDesc<Double>("STARTFREQ"));
  tdesc.addColumn (ArrayColumnDesc<Double>("VALUES", 2));
  tdesc.addColumn (ScalarColumnDesc<Double>("FREQ0"));
  tdesc.addColumn (ScalarColumnDesc<Double>("TIME0"));
  tdesc.addColumn (ScalarColumnDesc<Double>("FREQSCALE"));
  tdesc.addColumn (ScalarColumnDesc<Double>("TIMESCALE"));
  tdesc.addColumn (ScalarColumnDesc<Double>("DIFF"));
  SetupNewTable newtab(tableName, tdesc, Table::New);
  Table tab(newtab);
}

void ParmTable::unlockTables()
{
  for (std::map<string,ParmTable*>::const_iterator iter = theirTables.begin();
       iter != theirTables.end();
       ++iter) {
    iter->second->unlock();
  }
}

} // namespace Meq
