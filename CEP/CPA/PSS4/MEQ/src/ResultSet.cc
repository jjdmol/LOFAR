//# ResultSet.cc: A set of ResultSet results
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


#include "ResultSet.h"
#include "Cells.h"
#include <DMI/DataArray.h>

namespace Meq {

const HIID  FCells          = AidCells,
            FResults        = AidResults,
            FFail           = AidFail,
            FNodeName       = AidNode|AidName,
            FClassName      = AidClass|AidName,
            FOrigin         = AidOrigin,
            FOriginLine     = AidOrigin|AidLine,
            FMessage        = AidMessage; 

int ResultSet::nctor = 0;
int ResultSet::ndtor = 0;

static NestableContainer::Register reg(TpMeqResultSet,True);

ResultSet::ResultSet (int nresults)
: itsCells(0)
{
  nctor++;
  initNumResults(nresults);
}

ResultSet::ResultSet (int nresults,const Cells &cells,int flags)
{
  nctor++;
  initNumResults(nresults);
  setCells(cells,flags);
}
  
ResultSet::ResultSet (const Cells &cells,int flags)
{
  nctor++;
  initNumResults(0);
  setCells(cells,flags);
}

void ResultSet::initNumResults (int nresults)
{
  if( nresults >= 0 )
  {
    itsIsFail = false;
    itsResults <<= new DataField(TpMeqResult,nresults);
    DataRecord::add(FResults,itsResults.dewr_p(),DMI::ANONWR);
  }
  else // nresults<0 indicates a fail, so insert one
  {
    itsIsFail = true;
    DataRecord::add(FFail,new DataField(TpDataRecord,0));
  }
}

ResultSet::ResultSet (const DataRecord &other,int flags,int depth)
: DataRecord(other,flags,depth),
  itsIsFail(false)
{
  nctor++;
  validateContent();
}

ResultSet::~ResultSet()
{
  ndtor--;
}


//  implement privatize
void ResultSet::privatize (int flags, int depth)
{
  // if deep-privatizing, detach shortcuts
  if( flags&DMI::DEEP || depth>0 )
    itsResults.detach();
  DataRecord::privatize(flags,depth);
}

void ResultSet::validateContent ()
{
  // ensure that our record contains all the right fields, and that they're
  // indeed writable. Setup shortcuts to their contents
  try
  {
    if( hasField(FCells) ) // verify cells field
      itsCells = (*this)[FCells].as_p<Cells>();
    else
      itsCells = 0;
    itsResults.detach();
    // is it a fail?
    if( DataRecord::hasField(FFail) )
      itsIsFail = true;
    else
    {
      itsIsFail = false;
      // get pointer to results field
      if( DataRecord::hasField(FResults) )
      {
        itsResults <<= (*this)[FResults].ref(DMI::PRESERVE_RW);
        FailWhen(itsResults->type()!=TpMeqResult,"illegal results field");
      }
    }
  }
  catch( std::exception &err )
  {
    Throw(string("validate of ResultSet record failed: ") + err.what());
  }
  catch( ... )
  {
    Throw("validate of ResultSet record failed with unknown exception");
  }  
}

void ResultSet::addFail (const DataRecord *rec,int flags)
{
  FailWhen(!isWritable(),"r/w access violation");
  itsIsFail = true;
  // clear out results
  itsResults.detach();
  if( DataRecord::hasField(FResults) )
    DataRecord::removeField(FResults);
  // insert field of fail records, if necessary
  DataField *fails;
  if( !hasField(FFail) )
  {
    DataRecord::add(FFail,fails = new DataField(TpDataRecord,1),DMI::ANONWR);
    fails->put(0,rec,flags); // insert new
  }
  else
  {
    fails = &(*this)[FFail];
    fails->put(fails->size(),rec,flags);
  }
}

void ResultSet::addFail (const string &nodename,const string &classname,
                      const string &origin,int origin_line,const string &msg)
{
  DataRecord::Ref ref;
  DataRecord & rec = ref <<= new DataRecord;
  // populate the fail record
  rec[FNodeName] = nodename;
  rec[FClassName] = classname;
  rec[FOrigin] = origin;
  rec[FOriginLine] = origin_line;
  rec[FMessage] = msg;
  addFail(&rec);
}

int ResultSet::numFails () const
{
  return (*this)[FFail].size();
}
  
const DataRecord & ResultSet::getFail (int i) const
{
  return (*this)[FFail][i].as<DataRecord>();
}

void ResultSet::setCells (const Cells *cells,int flags)
{
  FailWhen(!isWritable(),"r/w access violation");
  itsCells = flags&DMI::CLONE ? new Cells(*cells) : cells;
  DataRecord::replace(FCells,itsCells,flags|DMI::READONLY);
}

Result & ResultSet::setResult (int i,Result *result)
{
  FailWhen(!isWritable(),"r/w access violation");
  DbgFailWhen(isFail(),"ResultSet marked as a fail, can't set result");
  itsResults().put(i,result,DMI::ANONWR);
  return *result;
}
  
Result & ResultSet::setResult (int i,Result::Ref::Xfer &result)
{
  FailWhen(!isWritable(),"r/w access violation");
  DbgFailWhen(isFail(),"ResultSet marked as a fail, can't set result");
  Result *res;
  itsResults().put(i,res=result.dewr_p(),DMI::ANONWR);
  result.detach();
  return *res;
}

void ResultSet::show (std::ostream& os) const
{
  if( isFail() )
    os << "FAIL";
  if( !isWritable() )
    os << "(readonly)";
  for( int i=0; i<numResults(); i++ )
  {
    os << "Result "<<i<<endl;
    resultConst(i).show(os);
  }
}

} // namespace Meq
