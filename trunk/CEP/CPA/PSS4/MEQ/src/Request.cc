//# Request.cc: The request for an evaluation of an expression
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

#include "Request.h"
#include "Node.h"
#include "MeqVocabulary.h"

namespace Meq {

static NestableContainer::Register reg(TpMeqRequest,True);

//##ModelId=3F8688700056
Request::Request()
: calcDeriv_(0),hasRider_(false),cache_override_(false)
{
}

//##ModelId=3F8688700061
Request::Request (const DataRecord &other,int flags,int depth)
: DataRecord  (other,flags,depth),
  calcDeriv_(0),hasRider_(false),cache_override_(false)
{
  validateContent();
}

//##ModelId=400E535403DD
Request::Request (const Cells& cells,int calcDeriv,const HIID &id,int cellflags)
: hasRider_(false),cache_override_(false)
{
  setCells(cells,cellflags);
  setId(id);
  setCalcDeriv(calcDeriv);
}

//##ModelId=400E53550016
Request::Request (const Cells * cells, int calcDeriv, const HIID &id,int cellflags)
: hasRider_(false),cache_override_(false)
{
  setCells(cells,cellflags);
  setId(id);
  setCalcDeriv(calcDeriv);
}

// Set the request id.
//##ModelId=3F8688700075
void Request::setId (const HIID &id)
{
  (*this)[FRequestId] = id_ = id;
}

void Request::setCalcDeriv (int calc)
{ 
  (*this)[FCalcDeriv] = calcDeriv_ = calc; 
}

void Request::setCacheOverride (bool flag)
{ 
  (*this)[FCacheOverride] = cache_override_ = flag; 
}

//##ModelId=3F868870006E
void Request::setCells (const Cells * cells,int flags)
{
  // if we have no idea how to attach object, make a copy
  if( !(flags&(DMI::ANON|DMI::EXTERNAL)) && !cells->refCount() )
    cells_ <<= new Cells(*cells);
  else
    cells_ <<= cells;
  DataRecord::replace(FCells,cells_.deref_p(),DMI::READONLY);
}

void Request::privatize (int flags,int depth)
{
  // if deep-privatizing, then detach shortcuts -- they will be reattached 
  // by validateContent()
  if( flags&DMI::DEEP || depth>0 )
  {
    cells_.detach();
    DataRecord::privatize(flags,depth);
  }
}

void Request::revalidateContent ()
{
  protectAllFields();
  if( DataRecord::hasField(FCells) )
    cells_ = DataRecord::fieldWr(FCells);
  else
    cells_.detach();
}

//##ModelId=400E53550049
void Request::validateContent ()
{
  Thread::Mutex::Lock lock(mutex());
  // ensure that our record contains all the right fields, and that they're
  // indeed writable. Setup shortcuts to their contents
  try
  {
    // get cells field
    revalidateContent();
    // request ID
    id_ = (*this)[FRequestId].as<HIID>(HIID());
    // calc-deriv flag
    calcDeriv_ = (*this)[FCalcDeriv].as<int>(0);
    cache_override_ = (*this)[FCacheOverride].as<bool>(false);
   // rider
    validateRider();
  }
  catch( std::exception &err )
  {
    cerr<<"Failed request: "<<sdebug(10);
    Throw(string("validate of Request record failed: ") + err.what());
  }
  catch( ... )
  {
    Throw("validate of Request record failed with unknown exception");
  }  
}

void Request::validateRider ()
{
  hasRider_ = DataRecord::hasField(FRider);
}

void Request::clearRider ()
{
  Thread::Mutex::Lock lock(mutex());
  DataRecord::remove(FRider);
  hasRider_ = False;
}

void Request::copyRider (const Request &other)
{
  Thread::Mutex::Lock lock(mutex());
  if( other.hasRider() )
  {
    DataRecord::replace(FRider,other.field(FRider));
    hasRider_ = True;
  }
  else
    clearRider();
}

int Request::remove (const HIID &id)
{ 
  Thread::Mutex::Lock lock(mutex());
  if( id == FCells || id == FRequestId || id==FCalcDeriv || id == FCacheOverride ) {
    Throw("remove(" + id.toString() +" from a Meq::Request not allowed"); 
  }
  return DataRecord::remove(id);
}


} // namespace Meq
