//# Freq.cc: Give the frequencies
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

#include <MEQ/Freq.h>
#include <MEQ/Request.h>
#include <MEQ/Result.h>
#include <MEQ/Cells.h>

namespace Meq {    


Freq::Freq()
{}

Freq::~Freq()
{}

void Freq::init (DataRecord::Ref::Xfer &initrec, Forest* frst)
{
  Node::init(initrec,frst);
  FailWhen(numChildren(),"Freq node cannot have children");
}

int Freq::getResultImpl (ResultSet::Ref &resref, const Request& request, bool)
{
  // Get cells.
  const Cells& cells = request.cells();
  int nfreq = cells.nfreq();
  int ntime = cells.ntime();
  // Create result object and attach to the ref that was passed in.
  resref <<= new ResultSet(1);                // 1 plane
  Result& result = resref().setNewResult(0);  // create new object for plane 0
  LoMat_double& arr = result.setReal (nfreq,ntime);
  // Evaluate the main value.
  for (int i=0; i<ntime; i++) {
    double freq = cells.domain().startFreq();
    for (int j=0; j<nfreq; j++) {
      arr(j,i) = freq;
      freq += cells.stepFreq();
    }
  }
  // There are no perturbations.
  return 0;
}

} // namespace Meq
