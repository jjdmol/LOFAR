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

void Composer::checkInitState (DataRecord &rec)
{
  defaultInitField(rec,FContagiousFail,false);
}
    
void Composer::setStateImpl (DataRecord &rec,bool initializing)
{
  Node::setStateImpl(rec,initializing);
  getStateField(contagious_fail,rec,FContagiousFail);
}

int Composer::getResult (Result::Ref &resref, 
                         const std::vector<Result::Ref> &childres,
                         const Request &request,bool)
{
  // count # of output planes, and # of fails among them
  int nres = 0, nfails = 0;
  for( uint i=0; i<childres.size(); i++ )
  {
    nres += childres[i]->numVellSets();
    nfails += childres[i]->numFails();
  }
  // if fail is contagious, generate a fully failed result
  if( nfails && contagious_fail )
  {
    Result &result = resref <<= new Result(nfails,request);
    int ires = 0;
    for( uint i=0; i<childres.size(); i++ )
    {
      Result &chres = childres[i]();
      for( int j=0; j<chres.numVellSets(); j++ )
      {
        VellSet &vs = chres.vellSet(j);
        if( vs.isFail() )
          result.setVellSet(ires++,&vs);
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
      Result &chres = childres[i]();
      for( int j=0; j<chres.numVellSets(); j++ )
        result.setVellSet(ires++,&(chres.vellSet(j)));
    }
  }
  // we do not introduce any dependencies
  return 0;
}

} // namespace Meq
