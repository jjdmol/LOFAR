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

#include "Selector.h"
#include "Request.h"
#include "Result.h"
#include "Cells.h"
#include "MeqVocabulary.h"

namespace Meq {    

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
  // if child returns a fail, pass it on up
  if( flag == RES_FAIL )
  {
    resref = childref;
    return RES_FAIL;
  }
  // return wait if child waits
  if( flag&RES_WAIT )
    return flag;
  // otherwise, select sub-results
  ResultSet &resset = resref <<= new ResultSet(selection.size(),request),
            &childres = childref();
  // select results from child set
  for( uint i=0; i<selection.size(); i++ )
  {
    int isel = selection[i];
    if( isel<0 || isel>=childres.numResults() )
    {
      Result &res = resset.setNewResult(i);
      MakeFailResult(res,
          Debug::ssprintf("selection index %d is out of range (%d results in set)",
                        isel,childres.numResults()));
    }
    else
    {
      resset.setResult(i,&(childres.result(isel)));
    }
  }
  return flag;
}

} // namespace Meq
