//# Function.cc: Base class for an expression node
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

#include "Function.h"
#include "Request.h"
    

namespace Meq {

using Debug::ssprintf;

//##ModelId=3F86886E03C5
Function::Function()
  : enable_flags_(true)
{}

//##ModelId=3F86886E03D1
Function::~Function()
{}

//##ModelId=400E53070274
TypeId Function::objectType() const
{
  return TpMeqFunction;
}

//##ModelId=3F95060D0060
void Function::checkChildren()
{
  if (itsChildren.size() == 0) {
    int nch = numChildren();
    itsChildren.resize (nch);
    for (int i=0; i<nch; i++) {
      itsChildren[i] = &(getChild(i));
    }
  }
}

//##ModelId=400E530702E6
bool Function::convertChildren (int nchild)
{
  if (itsChildren.size() > 0) {
    return false;
  }
  testChildren(nchild);
  Function::checkChildren();
  return true;
 }

//##ModelId=400E5308008E
bool Function::convertChildren (const vector<HIID>& childNames, int nchild)
{
  if (itsChildren.size() > 0) {
    return false;
  }
  if (nchild == 0) {
    nchild = childNames.size();
  }
  testChildren(nchild);
  int nch = numChildren();
  itsChildren.resize (nch);
  int nhiid = childNames.size();
  // Do it in order of the HIIDs given.
  for (int i=0; i<nhiid; i++) {
    itsChildren[i] = &(getChild(childNames[i]));
  }
  // It is possible that there are more children than HIIDs.
  // In that case the remaining children are appended at the end.
  if (nch > nhiid) {
    int inx = nhiid;
    for (int i=0; i<nch; i++) {
      Node * ptr = &(getChild(childNames[i]));
      bool fnd = false;
      for (int j=0; j<nhiid; j++) {
        if (ptr == itsChildren[j]) {
          fnd = true;
        }
      }
      if (!fnd) {
        itsChildren[inx++] = ptr;
      }
    }
  }
  return true;
}

//##ModelId=400E53080325
void Function::testChildren (int nchild) const
{
  if (nchild > 0) {
    FailWhen(numChildren()!=nchild,
        Debug::ssprintf("node has %d children, requires %d",numChildren(),nchild));
  } else if (nchild < 0) {
    FailWhen(numChildren() <= -nchild,
        Debug::ssprintf("node has %d children, requires at least %d",numChildren(),-nchild+1));
  }
}

//##ModelId=400E530900C1
void Function::testChildren (const vector<TypeId>& types) const
{
  int nch = std::min (types.size(), itsChildren.size());
  for (int i=0; i<nch; i++) {
    AssertStr (itsChildren[i]->objectType() == types[i],
               "expected type " << types[i] << ", but found "
               << itsChildren[i]->objectType());
  }
}


void Function::setStateImpl (DataRecord &rec,bool initializing)
{
  Node::setStateImpl(rec,initializing);
  // get [vector of] flag mask
  DataRecord::Hook hmask(rec,FFlagMask);
  if( hmask.exists() )
  {
    enable_flags_ = true;
    vector<int> fm = hmask.as_vector<int>();
    // single element? 
    if( fm.size() == 1 )
    {
      int flag = fm.front();
      if( flag == -1 )        // [-1] means full mask (i.e. disable masking completely)
        flagmask_.clear();    //      this is indicated by clearing the vector
      else if( flag == 0 )    // [0] means no flags on output
        enable_flags_ = false;
      else                    // [M] same mask for all elements
        flagmask_.assign(numChildren(),flag);
    }
    else // must have Nchildren masks
    {
      if( fm.size() != uint(numChildren()) )
        NodeThrow1("size of "+FFlagMask.toString()+" vector does not match number of children");
      flagmask_ = fm;
    }
  }
}



//##ModelId=3F86886E03DD
int Function::getResult (Result::Ref &resref,
                         const std::vector<Result::Ref> &childres,
                         const Request &request,bool)
{
  // figure out the max number of child planes, and check for consistency
  // w.r.t. number of perturbation sets
  int nrch = numChildren();
  Assert(nrch>0);
  Assert(flagmask_.empty() || flagmask_.size() == childres.size());
  int nplanes = childres[0]->numVellSets();
  for( int i=1; i<nrch; i++ )
    nplanes = std::max(nplanes,childres[i]->numVellSets());
  // Create result and attach to the ref that was passed in
  Result & result = resref <<= new Result(request,nplanes);
  vector<const VellSet*> child_vs(nrch);
  vector<const Vells*>  values(nrch);
  int nfails = 0;
  for( int iplane = 0; iplane < nplanes; iplane++ )
  {
    int npertsets = 0;
    // create a vellset for this plane
    VellSet &vellset = result.setNewVellSet(iplane,0,0);
    // collect vector of pointers to child vellsets #iplane, and a vector of 
    // pointers to their main values. If a child has fewer vellsets, generate 
    // a fail -- unless the child returned exactly 1 vellset, in which
    // case it is reused repeatedly. If any child vellsets are fails, collect 
    // them for propagation
    for( int i=0; i<nrch; i++ )
    {
      int nvs = childres[i]->numVellSets();
      if( nvs != 1 && iplane >= nvs )
      {
        MakeFailVellSet(vellset,ssprintf("child %d: only %d vellsets",i,nvs));
      }
      else 
      {
        child_vs[i] = &( childres[i]->vellSet(nvs==1?0:iplane) );
        if( child_vs[i]->isFail() ) 
        { // collect fails from child vellset
          for( int j=0; j<child_vs[i]->numFails(); j++ )
            vellset.addFail(&child_vs[i]->getFail(j));
        }
        else
          values[i] = &(child_vs[i]->getValue());
      }
      npertsets = std::max(npertsets,child_vs[i]->numPertSets());
    }
    // continue evaluation only if no fails popped up
    if( !vellset.isFail() )
    {
      // catch exceptions during evaluation and stuff them into fails
      try
      {
        // Find all spids from the children.
        vector<int> spids = findSpids(child_vs);
        // allocate new vellset object with given number of spids, add to set
        vellset.setNumPertSets(npertsets);
        vellset.setSpids(spids);
        // Evaluate the main value.
        LoShape shape = resultShape(values);
        vellset.setShape(shape);
        vellset.setValue(evaluate(request,shape,values).makeNonTemp());
        // Evaluate flags
        if( enable_flags_ )
        {
          for( int i=0; i<nrch; i++ )
            if( child_vs[i]->hasOptCol(VellSet::FLAGS) )
            {
              // if vellset has no flags and no mask is specified, just take 
              // a r/o ref to the child flags
              if( flagmask_.empty() && !vellset.hasOptCol(VellSet::FLAGS) )
                vellset.setOptCol(VellSet::FLAGS,child_vs[i]->getOptColRef(VellSet::FLAGS,DMI::READONLY));
              // else |= the vellset flags. Note that this will automatically
              // privatize a r/o ref upon first access
              else
              {
                const VellSet::FlagArrayType &chflag = 
                      child_vs[i]->getOptCol<VellSet::FLAGS>();
                if( flagmask_.empty() )
                  vellset.getOptColWr<VellSet::FLAGS>() |= chflag;
                else
                  vellset.getOptColWr<VellSet::FLAGS>() |= chflag & flagmask_[i];
              }
            }
        }
        // Evaluate all perturbed values.
        vector<vector<const Vells*> > pert_values(npertsets);
        vector<double> pert(npertsets);
        vector<int> indices(nrch,0);
        vector<int> found(npertsets);
        for( uint j=0; j<spids.size(); j++) 
        {
          found.assign(npertsets,-1);
          // pert_values start with pointers to each child's main value
          pert_values.assign(npertsets,values);
          // loop over children. For every child that contains a perturbed
          // value for spid[j], put a pointer to the perturbed value into 
          // pert_values[ipert][ichild]. For children that do not contain a 
          // perturbed value, it will retain a pointer to the main value.
          // The pertubations themselves are collected into pert[]; these
          // must match across all children
          for( int ich=0; ich<nrch; ich++ )
          {
            const VellSet &vs = *(child_vs[ich]);
            int inx = vs.isDefined(spids[j],indices[ich]);
            if( inx >= 0 )
            {
              for( int ipert=0; ipert<std::max(vs.numPertSets(),npertsets); ipert++ )
              {
                pert_values[ipert][ich] = &( vs.getPerturbedValue(inx,ipert) );
                if( found[ipert] >=0 )
                {
                  FailWhen(pert[ipert]!=vs.getPerturbation(inx,ipert),
                      ssprintf("perturbation %d for spid %d does not match between child results %d and %d",
                        ipert,spids[j],found[ipert],ich));
                }
                else
                {
                  pert[ipert] = vs.getPerturbation(inx,ipert);
                  found[ipert] = ich;
                }
              }
            }
          }
          // now, call evaluate() on the pert_values vectors to obtain the
          // perturbed values of the function
          for( int ipert=0; ipert<npertsets; ipert++ )
          {
            FailWhen(found[ipert]<0,
                     ssprintf("no perturbation set %d found for spid %d",
                              ipert,spids[j]));
            vellset.setPerturbation(j,pert[ipert],ipert);
            vellset.setPerturbedValue(j,evaluate(request,shape,pert_values[ipert]).makeNonTemp(),ipert);
          }
        } // end for(j) over spids
      }
      catch( std::exception &x )
      {
        MakeFailVellSet(vellset,
            string("exception in Function::getResult: ")
            + x.what());
      }
    } // endif( !vellset.isFail() )
    // count the # of fails
    if( vellset.isFail() )
      nfails++;
  }
  // return RES_FAIL is all planes have failed
  if( nfails == nplanes )
    return RES_FAIL;
  // return 0 flag, since we don't add any dependencies of our own
  return 0;
}

//##ModelId=400E5306027C
LoShape Function::resultShape (const vector<const Vells*>& values)
{
  Assert (values.size() > 0);
  int nx = values[0]->nx();
  int ny = values[0]->ny();
  for (unsigned int i=0; i<values.size(); i++) {
    nx = std::max(nx, values[i]->nx());
    ny = std::max(ny, values[i]->ny());
  }
  return makeLoShape(nx,ny);
}

//##ModelId=3F86886F0108
vector<int> Function::findSpids (const vector<const VellSet*> &results)
{
  // Determine the maximum number of spids.
  int nrspid = 0;
  int nrch = results.size();
  for (int i=0; i<nrch; i++) {
    nrspid += results[i]->getNumSpids();
  }
  // Allocate a vector of that size.
  // Exit immediately if nothing to be done.
  vector<int> spids(nrspid);
  if (nrspid == 0) {
    return spids;
  }
  // Merge all spids by doing that child by child.
  // The merged spids are stored from the end of the spids vector, so
  // eventually all resulting spids are at the beginning of the vector.
  int stinx = nrspid;          // start at end
  nrspid = 0;                  // no resulting spids yet
  // Loop through all children.
  for (int ch=0; ch<nrch; ch++) {
    const VellSet &resch = *results[ch];
    int nrchsp = resch.getNumSpids();
    if (nrchsp > 0) {
      // Only handle a child with spids.
      // Get a direct pointer to its spids (is faster).
      int inx = stinx;       // index where previous merge result starts.
      int lastinx = inx + nrspid;
      stinx -= nrchsp;       // index where new result is stored.
      int inxout = stinx;
      int lastspid = -1;
      // Loop through all spids of the child.
      for (int i=0; i<nrchsp; i++) {
        // Copy spids until exceeding current child's spid.
        int spid = resch.getSpid(i);
        while (inx < lastinx  &&  spids[inx] <= spid) {
          lastspid = spids[inx++];
          spids[inxout++] = lastspid;
        }
        // Only store child's spid if different.
        if (spid != lastspid) {
          spids[inxout++] = spid;
        }
      }
      // Copy possible remaining spids.
      while (inx < lastinx) {
        spids[inxout++] = spids[inx++];
      }
      nrspid = inxout - stinx;
    }
  }
  spids.resize(nrspid);
  return spids;
}

Vells Function::evaluate (const Request &,const LoShape &,const vector<const Vells*>&)
{
  Throw("evaluate() not implemented in this class");
}


} // namespace Meq
