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

#include <MEQ/Composer.h>
#include <MEQ/Request.h>
#include <MEQ/Result.h>
#include <MEQ/Cells.h>

namespace Meq {    

const HIID FIndex  = AidIndex;

Composer::Composer()
{}

Composer::~Composer()
{}

int Composer::getResultImpl (ResultSet::Ref &resref, const Request& request, bool)
{
  std::vector<ResultSet::Ref> childref;
  // get results from children, fail if failed
  int resflag = getChildResults(childref,resref,request);
  if( resflag == RES_FAIL )
    return RES_FAIL;
  // return wait if some child has returned a wait
  if( resflag&RES_WAIT )
    return resflag;
  // count # of output planes
  int nres = 0;
  for( uint i=0; i<childref.size(); i++ )
    nres += childref[i]->numResults();
  // otherwise, compose result
  ResultSet &result = resref <<= new ResultSet(nres);
  result.setCells(request.cells()); 
  int ires=0;
  for( int i=0; i<numChildren(); i++ )
  {
    ResultSet &childres = childref[i]();
    for( int j=0; j<childres.numResults(); j++ )
      result.setResult(ires++,&(childres.result(j)));
    childref[i].detach();
  }
  return resflag;
}

} // namespace Meq
