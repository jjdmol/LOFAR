//# Condeq.cc: Base class for an expression node
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

#include <MEQ/Condeq.h>
#include <MEQ/Function.h>
#include <MEQ/Vells.h>
    

namespace Meq {

//##ModelId=400E5305005F
Condeq::Condeq()
{}

//##ModelId=400E53050060
Condeq::~Condeq()
{}

//##ModelId=400E53050062
TypeId Condeq::objectType() const
{
  return TpMeqCondeq;
}

//##ModelId=400E53050064
void Condeq::checkChildren()
{
  Assert (numChildren() == 2);
}

//##ModelId=400E53050066
int Condeq::getResult (Result::Ref &resref, 
                       const std::vector<Result::Ref> &child_result,
                       const Request &request,bool)
{
  int nrch = child_result.size();
  Assert(nrch==2);
  // Check that number of child planes is the same
  int nplanes = child_result[0]->numVellSets();
  FailWhen(child_result[1]->numVellSets()!=nplanes,
           "mismatch in sizes of child results");
  // Create result object and attach to the ref that was passed in
  Result & result = resref <<= new Result(nplanes,request);
  vector<VellSet*> child_res(nrch);
  for( int iplane=0; iplane<nplanes; iplane++ )
  {
    // collect vector of pointers to children, and vector
    // of pointers to main value
    vector<Vells*> values(nrch);
    for( int i=0; i<nrch; i++ )
    {
      child_res[i] = &(child_result[i]().vellSet(iplane));
      values[i] = &(child_res[i]->getValueRW());
    }
    // Find all spids from the children.
    vector<int> spids = Function::findSpids(child_res);
    // allocate new result object with given number of spids, add to set
    VellSet &vellset = result.setNewVellSet(iplane,spids.size());
    // The main value is measured-predicted.
    vellset.setValue (*values[0] - *values[1]);
    // Evaluate all perturbed values.
    vector<Vells*> perts(nrch);
    vector<int> indices(nrch, 0);
    Vells deriv;
    for (unsigned int j=0; j<spids.size(); j++) 
    {
      int inx0 = child_res[0]->isDefined (spids[j], indices[0]);
      int inx1 = child_res[1]->isDefined (spids[j], indices[1]);
      double pert = 0;
      if (inx1 >= 0) {
        deriv = (child_res[1]->getPerturbedValueRW(inx1) - *values[1]) /
                ( pert = child_res[1]->getPerturbation(inx1) );
        if (inx0 >= 0) {
          deriv -= (child_res[0]->getPerturbedValueRW(inx0) - *values[0]) /
                   child_res[0]->getPerturbation(inx0);
        }
      } else if (inx0 >= 0) {
        deriv = (*values[0] - child_res[0]->getPerturbedValueRW(inx0)) /
                 ( pert = child_res[0]->getPerturbation(inx0) );
      } else {
        deriv = Vells(0.);
      }
      vellset.setPerturbedValue (j, deriv);
      vellset.setPerturbation(j,pert);
    }
    vellset.setSpids (spids);
    
  }
  // no dependencies introduced
  return 0;
}


} // namespace Meq
