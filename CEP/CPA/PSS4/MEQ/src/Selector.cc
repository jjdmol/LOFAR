//# Selector.cc: Selects result planes from a result set
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

#include <MEQ/Selector.h>
#include <MEQ/Request.h>
#include <MEQ/Result.h>
#include <MEQ/Cells.h>

namespace Meq {    

const HIID FIndex  = AidIndex;

Selector::Selector()
{}

Selector::~Selector()
{}

void Selector::init (DataRecord::Ref::Xfer &initrec, Forest* frst)
{
  selection = (*initrec)[FIndex].as_vector<int>();
  Node::init(initrec,frst);
  FailWhen(numChildren()!=1,"Selector node must have exactly one child");
}

void Selector::setState (const DataRecord &rec)
{
  if( rec[FIndex].exists() )
  {
    wstate()[FIndex].replace() = selection = rec[FIndex].as_vector<int>();
  }
}

int Selector::getResultImpl (ResultSet::Ref &resref, const Request& request, bool)
{
  ResultSet::Ref childref;
  int flag = getChild(0).getResult(childref,request);
  if( flag == RES_FAIL || flag&RES_WAIT )
    return flag;
  ResultSet &result = resref <<= new ResultSet(selection.size()),
            &childres = childref();
  // select results from child set
  for( uint i=0; i<selection.size(); i++ )
  {
    int isel = selection[i];
    if( isel<0 || isel>=childres.numResults() )
      return RES_FAIL;
    result.setResult(i,&(childres.result(isel)));
  }
  // copy cells from child set
  result.setCells(request.cells()); 
  return flag;
}

} // namespace Meq
