//# Time.cc: Give the times
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

#include <MEQ/Time.h>
#include <MEQ/Request.h>
#include <MEQ/VellSet.h>
#include <MEQ/Cells.h>

namespace Meq {    


Time::Time()
{}

Time::~Time()
{}

void Time::init (DataRecord::Ref::Xfer &initrec, Forest* frst)
{
  Node::init(initrec,frst);
  FailWhen(numChildren(),"Time node cannot have children");
}

int Time::getResult (Result::Ref &resref, const Request& request, bool)
{
  // Get cells.
  const Cells& cells = request.cells();
  int nfreq = cells.nfreq();
  int ntime = cells.ntime();
  // Create result object and attach to the ref that was passed in.
  resref <<= new Result(1);                // 1 plane
  VellSet& result = resref().setNewVellSet(0);  // create new object for plane 0
  LoMat_double& arr = result.setReal (nfreq,ntime);
  // Evaluate the main value.
  for (int i=0; i<ntime; i++) {
    double time = cells.time(i);
    for (int j=0; j<nfreq; j++) {
      arr(j,i) = time;
    }
  }
  // There are no perturbations.
  return 0;
}

} // namespace Meq
