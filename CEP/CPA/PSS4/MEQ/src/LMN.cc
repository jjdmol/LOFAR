//# LMN.cc: Calculate station LMN from station position and phase center
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

#include <MEQ/LMN.h>
#include <MEQ/Request.h>
#include <MEQ/VellSet.h>
#include <MEQ/Vells.h>
#include <MEQ/AID-Meq.h>

namespace Meq {

using namespace VellsMath;

const HIID child_labels[] = { AidRA|0,AidDec|0,AidRA,AidDec };
const int num_children = sizeof(child_labels)/sizeof(child_labels[0]);

const HIID FDomain = AidDomain;

//##ModelId=400E535502D1
LMN::LMN()
: Function(num_children,child_labels)
{
}

//##ModelId=400E535502D2
LMN::~LMN()
{}

//##ModelId=400E535502D6
int LMN::getResult (Result::Ref &resref, 
                    const std::vector<Result::Ref> &childres,
                    const Request &request,bool newreq)
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
  // Get RA and DEC of phase center and source.
  const Vells& vra   = childres[0]->vellSet(0).getValue();
  const Vells& vdec  = childres[1]->vellSet(0).getValue();
  const Vells& vras  = childres[2]->vellSet(0).getValue();
  const Vells& vdecs = childres[3]->vellSet(0).getValue();
  // Allocate a 3-plane result for L, M, and N.
  Result &result = resref <<= new Result(3,request);
  // create a vellset for each plane
  VellSet &lvellset = result.setNewVellSet(0,0,0);
  VellSet &mvellset = result.setNewVellSet(1,0,0);
  VellSet &nvellset = result.setNewVellSet(2,0,0);
  lvellset.setValue (cos(vdecs) * sin(vras-vra));
  mvellset.setValue (sin(vdecs) * cos(vdec) -
		     cos(vdecs) * sin(vdec) * cos(vras-vra));
  nvellset.setValue (sqrt(1 - sqr(lvellset.getValue()) -
			  sqr(mvellset.getValue())));
  return 0;
}

} // namespace Meq
