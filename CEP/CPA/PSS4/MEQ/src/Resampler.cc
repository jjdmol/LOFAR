//# Resampler.cc: resamples result resolutions
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

#include "Resampler.h"
#include "MeqVocabulary.h"
#include "Request.h"
#include "Result.h"
#include "Cells.h"


namespace Meq {

const HIID FIntegrate = AidIntegrate;
const HIID FFlagDensity = AidFlag|AidDensity;


//##ModelId=400E5355029C
Resampler::Resampler()
: Node(1), // 1 child expected
  integrate(false),flag_mask(-1),flag_bit(0),flag_density(0.5)
{}

//##ModelId=400E5355029D
Resampler::~Resampler()
{}

void Resampler::setStateImpl (DataRecord &rec,bool initializing)
{
  Node::setStateImpl(rec,initializing);
  rec[FIntegrate].get(integrate,initializing);
  rec[FFlagMask].get(flag_mask,initializing);
  rec[FFlagBit].get(flag_bit,initializing);
  rec[FFlagDensity].get(flag_density,initializing);
}

template<class T>
void Resampler::doResample (Vells::Ref &voutref,const blitz::Array<T,2> &in)
{
  int nx = outshape[0];
  int ny = outshape[1];
  
  Vells &vout = voutref <<= new Vells(T(),nx,ny,false);
  blitz::Array<T,2> &out = vout.as(Type2Type<blitz::Array<T,2> >());
  
  // i1,j1: current output cell/block of cells of input array
  // i0,j0: top left of integrated cell in 
  int i0,j0=0; 
  // loop over output
  for( int j1=0; j1<ny; j1+=nexpand[1],j0+=nsum[1] )
  {
    i0=0;
    for( int i1=0; i1<nx; i1+=nexpand[0],i0+=nsum[0] )
    {
      T ss = 0;
      // if output value not fully flagged, do the sum
      if( !outflags || !(*outflags)(i1,j1) )
      {
        // sum up block of input cells at (i0,j0)
        int isum, jsum = j0;
        for( int j=0; j<nsum[1]; j++,jsum++ )
        {
          isum = i0;
          for( int i=0; i<nsum[0]; i++,isum++ )
            if( !inflags || !((*inflags)(isum,jsum)&flag_mask) )
              ss += in(isum,jsum);
        }
        // renormalize
        ss *= renorm_matrix(i1,j1);
      }
      // assign value to block of output cells
      int iexp, jexp = j1;
      for( int j=0; j<nexpand[1]; j++,jexp++ )
      {
        iexp = i1;
        for( int i=0; i<nexpand[0]; i++,iexp++ )
          out(iexp,jexp) = ss;
      }
    }
  }
}

// resamples Vells to a new grid and returns the result 
Vells::Ref Resampler::resampleVells (const Vells &in)
{
  Vells::Ref out;
  // array vells -- do integrate/expand
  if( in.isArray() )
  {
    // init pv to complex or double Vells of the right shape
    if( in.isComplex() )
      doResample(out,in.getComplexArray());
    else
      doResample(out,in.getRealArray());
  }
  // scalar vells -- do trivial rescaling
  else
  {
    if( renorm_factor == 1 )
      out.attach(in,DMI::READONLY);
    else
      out <<= new Vells( in * renorm_factor );
  }
  return out;
}

int Resampler::getResult (Result::Ref &resref, 
                           const std::vector<Result::Ref> &childres,
                           const Request &request,bool)
{
  Assert(childres.size()==1);
  const Result &chres = *( childres.front() );
  const Cells &incells = chres.cells();
  const Cells &outcells = request.cells();

  outshape = outcells.shape();
  double renorm_factor = 1;
  bool identical = true;
  // figure out how to convert between cells
  for( int i=0; i<DOMAIN_NAXES; i++ )
  {  
    nsum[i] = nexpand[i] = 1;
    int nin = incells.ncells(i),
        nout = outshape[i];
    if( nin > nout ) // decreasing resolution in this dimension
    {
      int n = nin/nout;
      if( nin%nout )
        NodeThrow1(
          ssprintf("incompatible number of cells in dimension %d: %d child, %d parent",
            i,nin,nout));
      nsum[i] = n;
      identical = false;
      // if integrating, then we just sum up the cells and leave it at that.
      // else if averaging, we must divide by N to get the average value.
      if( !integrate )
        renorm_factor /= n;
    }
    else if( nin < nout )
    {
      int n = nout/nin;
      if( nout%nin )
        NodeThrow1(
          ssprintf("incompatible number of cells in dimension %d: %d child, %d parent",
            i,nin,nout));
      nexpand[i] = n;
      identical = false;
      // if integrating, then we have to renormalize individual cells
      // else if averaging, then just leave the value as is
      if( integrate )
        renorm_factor /= n;
    }
  }
  // resize & assign the renormalization matrix
  renorm_matrix.resize(outshape);
  renorm_matrix = renorm_factor;
  // if input is identical to output, just pass the child's result on
  if( identical )
  {
    resref <<= childres[0];
    return 0;
  }
      
  // Create result and attach to the ref that was passed in
  int nvs = chres.numVellSets();
  Result & result = resref <<= new Result(request,nvs);
  
  for( int ivs=0; ivs<nvs; ivs++ )
  {
    const VellSet &chvs = chres.vellSet(ivs);
    // copy child vellset to output. This is mostly copy-by-reference
    // if child vellset is a fail, pass as is
    if( chvs.isFail() )
    {
      result.setVellSet(ivs,&chvs);
      continue;
    }
    // else create new output VS
    VellSet &vs = result.setNewVellSet(ivs,chvs.numSpids(),chvs.numPertSets());
    vs.setShape(outshape);
    vs.copySpids(vs);
    vs.copyPerturbations(vs);
    
    // handle flags if supplied (and configured)
    if( flag_mask && chvs.hasOptCol<VellSet::FLAGS>() )
    {
      inflags  = &( chvs.getOptCol<VellSet::FLAGS>() );
      outflags = &( vs.initOptCol<VellSet::FLAGS>() );

      // # of pixels in integarted cell
      int cellsize = nsum[0]*nsum[1];
      // threshold for number of flagged pixels in integrated cell which
      // will cause us to flag the output
      int nfl_threshold = std::max( cellsize,
                                int(floor(cellsize*flag_density+.5)) );
      
      int nx = outshape[0];
      int ny = outshape[1];
      // i1,j1: current output cell/block of cells of input array
      // i0,j0: top left of integrated cell in 
      int i0,j0=0; 
      // loop over output
      for( int j1=0; j1<ny; j1+=nexpand[1],j0+=nsum[1] )
      {
        i0=0;
        for( int i1=0; i1<nx; i1+=nexpand[0],i0+=nsum[0] )
        {
          VellSet::FlagType sumfl = 0; // OR of flags within integration cell
          int nfl = 0;                 // number of flagged pixels in cell
          // sum up block of input cells at (i0,j0)
          int isum, jsum = j0;
          for( int j=0; j<nsum[1]; j++,jsum++ ) {
            isum = i0;
            for( int i=0; i<nsum[0]; i++,isum++ ) {
              int fl = (*inflags)(isum,jsum)&flag_mask;
              if( fl ) {
                sumfl |= fl;
                nfl++;
              }
            }
          }
          // have we found any flags within the integrated cell?
          if( nfl )
          {
            // critical number of flags => flag entire output cell
            if( nfl >= nfl_threshold )
            {
              // if flag_bit is specified, use that instead of the summary flags
              if( flag_bit )
                sumfl = flag_bit;
              // assign flag value to block of output flags 
              int iexp, jexp = j1;
              for( int j=0; j<nexpand[1]; j++,jexp++ ) {
                iexp = i1;
                for( int i=0; i<nexpand[0]; i++,iexp++ )
                  (*outflags)(iexp,jexp) = sumfl;
              }
            }
            // partially flagged cell has to be renormalized since fewer 
            // points will be used in integration
            else
              renorm_matrix(i1,j1) *= double(cellsize)/(cellsize-nfl);
          }
        }
      }
    }
    else // no flags to handle
      inflags = outflags = 0;

    
    // place resampled vells into it
    vs.setValue( resampleVells(chvs.getValue()) );
    
    for( int iset=0; iset<vs.numPertSets(); iset++ )
      for( int ipert=0; ipert<vs.numSpids(); ipert++ )
        vs.setPerturbedValue(ipert,
            resampleVells(chvs.getPerturbedValue(ipert,iset)),iset);
  }
  
  return 0;
}

} // namespace Meq
