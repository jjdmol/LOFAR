//# StatPointSourceDFT.cc: The point source DFT component for a station
//#
//# Copyright (C) 2004
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

#include <MEQ/StatPointSourceDFT.h>
#include <aips/Mathematics/Constants.h>

namespace Meq {    

using namespace VellsMath;

const HIID child_labels[] = { AidL,AidM,AidN,AidU,AidV,AidW };
const int num_children = sizeof(child_labels)/sizeof(child_labels[0]);

StatPointSourceDFT::~StatPointSourceDFT()
{}

int StatPointSourceDFT::getResult (Result::Ref &resref, 
				   const std::vector<Result::Ref> &childres,
				   const Request &request, bool newreq)
{
  // Check that child results are all OK (no fails, 1 vellset per child)
  string fails;
  for( int i=0; i<num_children; i++ )
  {
    int nvs = childres[i]->numVellSets();
    if( nvs != 1 )
      Debug::appendf(fails,"child %s: expecting single VellsSet, got %d;",
          child_labels[i].toString().c_str(),nvs);
    if( childres[i]->hasFails() )
      Debug::appendf(fails,"child %s: has fails",child_labels[i].toString().c_str());
  }
  if( !fails.empty() )
    NodeThrow1(fails);
  // Get L,M,N and U,V,W.
  const Vells& vl = childres[0]().vellSetWr(0).getValue();
  const Vells& vm = childres[1]().vellSetWr(0).getValue();
  const Vells& vn = childres[2]().vellSetWr(0).getValue();
  const Vells& vu = childres[3]().vellSetWr(0).getValue();
  const Vells& vv = childres[4]().vellSetWr(0).getValue();
  const Vells& vw = childres[5]().vellSetWr(0).getValue();
  // For the time being we only support scalars
  Assert (vl.nelements()==1 && vm.nelements()==1
      	  && vn.nelements()==1);
  // For the time being we only support 1 frequency range.
  const Cells& cells = request.cells();
  Assert (cells.numSegments(FREQ) == 1);
  // Allocate a 2-plane result for F0, dF (one such pair per frequency range).
  // It is assumed that UVW is in meters and is frequency independent.
  Result &result = resref <<= new Result(2,request);
  // create a vellset for each plane
  VellSet &f0vellset = result.setNewVellSet(0,0,0);
  VellSet &dfvellset = result.setNewVellSet(1,0,0);
  // Calculate 2pi/wavelength, where wavelength=c/freq.
  // Calculate it for the frequency step if needed.
  double f0 = cells.center(FREQ)(0);
  double df = cells.cellSize(FREQ)(0);
  double wavel0 = C::_2pi * f0 / C::c;
  double dwavel = df / f0;
  Vells r1 = (vu*vl + vv*vm + vw*vn) * wavel0;
  f0vellset.setValue (tocomplex(cos(r1), sin(r1)));
  r1 *= dwavel;
  dfvellset.setValue (tocomplex(cos(r1), sin(r1)));
}

void StatPointSourceDFT::checkChildren()
{
  Function::convertChildren (6);
}

} // namespace Meq
