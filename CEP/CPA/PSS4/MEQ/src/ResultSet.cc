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

const HIID FCells           = AidCells,
           FResults         = AidResults;

int ResultSet::nctor = 0;
int ResultSet::ndtor = 0;

static NestableContainer::Register reg(TpMeqResultSet,True);


ResultSet::ResultSet (int nresults)
: itsResults(new DataField(TpMeqResult,nresults)),itsCells(0)
{
  nctor++;
  DataRecord::add(FResults,itsResults,DMI::ANONWR);
}

ResultSet::ResultSet (const DataRecord &other,int flags,int depth)
: DataRecord(other,flags,depth)
{
  nctor++;
  validateContent();
}

ResultSet::~ResultSet()
{
  ndtor--;
}

void ResultSet::validateContent ()
{
  // ensure that our record contains all the right fields, and that they're
  // indeed writable. Setup shortcuts to their contents
  try
  {
    if( (*this)[FCells].exists() ) // verify cells field
      itsCells = (*this)[FCells].as_p<Cells>();
    else
      itsCells = 0;
    // get pointer to results field
    itsResults = (*this)[FResults].as_wp<DataField>();
    FailWhen(itsResults->type()!=TpMeqResult,"illegal results field");
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

void ResultSet::setCells (const Cells *cells,int flags)
{
  itsCells = flags&DMI::CLONE ? new Cells(*cells) : cells;
  DataRecord::replace(FCells,itsCells,flags|DMI::READONLY);
}

void ResultSet::setResult (int i,Result *result)
{
  itsResults->put(i,result,DMI::ANONWR);
}
  
void ResultSet::setResult (int i,Result::Ref::Xfer &result)
{
  itsResults->put(i,result.dewr_p(),DMI::ANONWR);
  result.detach();
}

void ResultSet::show (std::ostream& os) const
{
  for( int i=0; i<numResults(); i++ )
  {
    os << "Result "<<i<<endl;
    resultConst(i).show(os);
  }
}

} // namespace Meq
