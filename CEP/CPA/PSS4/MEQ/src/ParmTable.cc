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
#include <aips/Tables/TableLocker.h>
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

// define some column names
const String ColName          = "NAME";
const String ColStartFreq     = "STARTFREQ";
const String ColEndFreq       = "ENDFREQ";
const String ColStartTime     = "STARTTIME";
const String ColEndTime       = "ENDTIME";
const String ColValues        = "VALUES";
const String ColFreq0         = "FREQ0";
const String ColTime0         = "TIME0";
const String ColFreqScale     = "FREQSCALE";
const String ColTimeScale     = "TIMESCALE";
const String ColPerturbation  = "PERT";
const String ColWeight        = "WEIGHT";

const String KeywordDefValues = "DEFAULTVALUES";

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
: itsTable       (tableName, TableLock(TableLock::UserLocking)),
  itsIndex       (itsTable, ColName),
  itsIndexName   (itsIndex.accessKey(), ColName),
  itsInitIndex   (0)
{
  if (itsTable.keywordSet().isDefined (KeywordDefValues)) {
    itsInitTable = itsTable.keywordSet().asTable (KeywordDefValues);
    itsInitIndex = new ColumnsIndex (itsInitTable, ColName);
    itsInitIndexName = RecordFieldPtr<String> (itsInitIndex->accessKey(),
                                               ColName);
  }
}

//##ModelId=3F86886F02BC
ParmTable::~ParmTable()
{
  delete itsInitIndex;
}

//##ModelId=3F86886F02BD
int ParmTable::getPolcs (vector<Polc::Ref> &polcs,
                         const string& parmName,const Domain& domain)
{
  TableLocker locker(itsTable, FileLocker::Read);
  Table sel = find (parmName, domain);
  polcs.resize(sel.nrow());
  if( sel.nrow() > 0 ) 
  {
    ROScalarColumn<double> sfCol (sel, ColStartFreq);
    ROScalarColumn<double> efCol (sel, ColEndFreq);
    ROScalarColumn<double> stCol (sel, ColStartTime);
    ROScalarColumn<double> etCol (sel, ColEndTime);
    ROArrayColumn<double> valCol (sel, ColValues);
    ROScalarColumn<double> f0Col (sel, ColFreq0);
    ROScalarColumn<double> t0Col (sel, ColTime0);
    ROScalarColumn<double> fsCol (sel, ColFreqScale);
    ROScalarColumn<double> tsCol (sel, ColTimeScale);
    ROScalarColumn<double> diffCol (sel, ColPerturbation);
    ROScalarColumn<double> weightCol (sel, ColWeight);
    for( uint i=0; i<sel.nrow(); i++ )
    {
      Polc &polc = polcs[i] <<= new Polc(fromParmMatrix(valCol(i)),
          f0Col(i),fsCol(i),t0Col(i),tsCol(i),diffCol(i),weightCol(i));
      polc.setDomain(Domain(stCol(i), etCol(i), sfCol(i), efCol(i)));
    }
  }
  return polcs.size();
}

//##ModelId=3F86886F02C3
int ParmTable::getInitCoeff (Polc::Ref &polcref,const string& parmName)
{
  // Try to find the default initial values in the InitialValues subtable.
  // The parameter name consists of parts (separated by dots), so the
  // parameters are categorised in that way.
  // An initial value can be defined for the full name or for a higher
  // category.
  // So look up until found or until no more parts are left.
  if( itsInitIndex ) 
  {
    string name = parmName;
    while( true ) 
    {
      *itsInitIndexName   = name;
      Vector<uInt> rownrs = itsInitIndex->getRowNumbers();
      if (rownrs.nelements() > 0) 
      {
        Assert( rownrs.nelements() == 1 );
        Polc &result = polcref <<= new Polc;
        int row = rownrs(0);
        TableLocker locker(itsInitTable, FileLocker::Read);
        ROArrayColumn<Double> valCol (itsInitTable, ColValues);
        ROScalarColumn<double> f0Col (itsInitTable, ColFreq0);
        ROScalarColumn<double> t0Col (itsInitTable, ColTime0);
        ROScalarColumn<double> fsCol (itsInitTable, ColFreqScale);
        ROScalarColumn<double> tsCol (itsInitTable, ColTimeScale);
        ROScalarColumn<double> diffCol (itsInitTable, ColPerturbation);
        polcref <<= new Polc(fromParmMatrix(valCol(row)),
                f0Col(row),fsCol(row),t0Col(row),tsCol(row),diffCol(row));
        return polcref->ncoeff();
      }
      string::size_type idx = name.rfind ('.');
      // Exit loop if no more name parts.
      if (idx == string::npos) 
        break;
      // Remove last part and try again.
      name = name.substr (0, idx);
    }
  }
  return 0;
}
                                    
//##ModelId=3F86886F02C8
void ParmTable::putCoeff (const string& parmName, const Polc& polc)
{
  itsTable.reopenRW();
  TableLocker locker(itsTable, FileLocker::Write);
  const Domain& domain = polc.domain();
  const Vells& values = polc.getCoeff();
  Table sel = find (parmName, domain);
  if (sel.nrow() > 0) {
    AssertMsg (sel.nrow()==1, "Parameter " << parmName <<
                 " has multiple entries for freq "
                 << domain.startFreq() << ':' << domain.endFreq()
                 << " and time "
                 << domain.startTime() << ':' << domain.endTime());
    ROScalarColumn<double> sfCol (sel, ColStartFreq);
    ROScalarColumn<double> efCol (sel, ColEndFreq);
    ROScalarColumn<double> stCol (sel, ColStartTime);
    ROScalarColumn<double> etCol (sel, ColEndTime);
    AssertMsg (near(domain.startFreq(), sfCol(0))  &&
               near(domain.endFreq(), efCol(0))  &&
               near(domain.startTime(), stCol(0))  &&
               near(domain.endTime(), etCol(0)),
               "Parameter " << parmName <<
               " has a partially instead of fully matching entry for freq "
                 << domain.startFreq() << ':' << domain.endFreq()
                 << " and time "
                 << domain.startTime() << ':' << domain.endTime());
    ArrayColumn<double> valCol (sel, ColValues);
    valCol.put (0, toParmMatrix(values));
  } else {
    uInt rownr = itsTable.nrow();
    itsTable.addRow();
    ScalarColumn<String> namCol (itsTable, ColName);
    ScalarColumn<double> sfCol (itsTable, ColStartFreq);
    ScalarColumn<double> efCol (itsTable, ColEndFreq);
    ScalarColumn<double> stCol (itsTable, ColStartTime);
    ScalarColumn<double> etCol (itsTable, ColEndTime);
    ScalarColumn<double> f0Col (itsTable, ColFreq0);
    ScalarColumn<double> t0Col (itsTable, ColTime0);
    ScalarColumn<double> fsCol (itsTable, ColFreqScale);
    ScalarColumn<double> tsCol (itsTable, ColTimeScale);
    ScalarColumn<double> diffCol (itsTable, ColPerturbation);
    namCol.put (rownr, parmName);
    sfCol.put (rownr, domain.startFreq());
    efCol.put (rownr, domain.endFreq());
    stCol.put (rownr, domain.startTime());
    etCol.put (rownr, domain.endTime());
    ArrayColumn<double> valCol (itsTable, ColValues);
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
    Table sel3 = sel(domain.startFreq() < sel.col(ColEndFreq)   &&
                     domain.endFreq()   > sel.col(ColStartFreq) &&
                     domain.startTime() < sel.col(ColEndTime)   &&
                     domain.endTime()   > sel.col(ColStartTime));
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
  tdesc.addColumn (ScalarColumnDesc<String>(ColName));
  tdesc.addColumn (ScalarColumnDesc<Double>(ColEndTime));
  tdesc.addColumn (ScalarColumnDesc<Double>(ColStartTime));
  tdesc.addColumn (ScalarColumnDesc<Double>(ColEndFreq));
  tdesc.addColumn (ScalarColumnDesc<Double>(ColStartFreq));
  tdesc.addColumn (ArrayColumnDesc<Double>(ColValues, 2));
  tdesc.addColumn (ScalarColumnDesc<Double>(ColFreq0));
  tdesc.addColumn (ScalarColumnDesc<Double>(ColTime0));
  tdesc.addColumn (ScalarColumnDesc<Double>(ColFreqScale));
  tdesc.addColumn (ScalarColumnDesc<Double>(ColTimeScale));
  tdesc.addColumn (ScalarColumnDesc<Double>(ColPerturbation));
  SetupNewTable newtab(tableName, tdesc, Table::New);
  Table tab(newtab);
}

void ParmTable::unlock()
{
  itsTable.unlock();
  if (! itsInitTable.isNull()) {
    itsInitTable.unlock();
  }
}

void ParmTable::lock()
{
  itsTable.lock();
}

void ParmTable::lockTables()
{
  for (std::map<string,ParmTable*>::const_iterator iter = theirTables.begin();
       iter != theirTables.end();
       ++iter) {
    iter->second->lock();
  }
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
