//# Composer.cc: Selects result planes from a result set
//#
//# Copyright (C) 2003
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

#include "Composer.h"
#include "Request.h"
#include "VellSet.h"
#include "Cells.h"
#include "MeqVocabulary.h"

namespace Meq {    

Composer::Composer()
{}

Composer::~Composer()
{}


void Composer::init (DataRecord::Ref::Xfer &initrec, Forest* frst)
{
  contagious_fail = (*initrec)[FContagiousFail].as<bool>(false);
  Node::init(initrec,frst);
}

void Composer::setState (const DataRecord &rec)
{
  if( rec[FContagiousFail].exists() )
  {
    wstate()[FContagiousFail] = contagious_fail = 
          rec[FContagiousFail].as<bool>();
  }
}

int Composer::getResult (Result::Ref &resref, const Request& request, bool)
{
  std::vector<Result::Ref> childref;
  // get results from children
  int resflag = getChildResults(childref,request);
  // return wait if some child has returned a wait
  if( resflag != RES_FAIL && resflag&RES_WAIT )
    return resflag;
  // count # of output planes, and # of fails among them
  int nres = 0, nfails = 0;
  for( uint i=0; i<childref.size(); i++ )
  {
    nres += childref[i]->numVellSets();
    nfails += childref[i]->numFails();
  }
  // if fail is contagious, generate a fully failed result
  if( nfails && ( contagious_fail || nres == nfails ) )
  {
    Result &result = resref <<= new Result(nfails,request);
    int ires = 0;
    for( uint i=0; i<childref.size(); i++ )
    {
      Result &childres = childref[i]();
      for( int j=0; j<childres.numVellSets(); j++ )
      {
        VellSet &res = childres.vellSet(j);
        if( res.isFail() )
          result.setVellSet(ires++,&res);
      }
    }
    return RES_FAIL;
  }
  // otherwise, compose normal result
  else
  {
    Result &result = resref <<= new Result(nres,request);
    result.setCells(request.cells()); 
    int ires=0;
    for( int i=0; i<numChildren(); i++ )
    {
      Result &childres = childref[i]();
      for( int j=0; j<childres.numVellSets(); j++ )
        result.setVellSet(ires++,&(childres.vellSet(j)));
      childref[i].detach();
    }
    return resflag;
  }
}

} // namespace Meq
