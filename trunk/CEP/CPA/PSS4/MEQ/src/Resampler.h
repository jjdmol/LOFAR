//# Resampler.h: resamples result resolutions
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

#ifndef MEQ_RESAMPLER_H
#define MEQ_RESAMPLER_H
    
#include <MEQ/Function.h>
#include <MEQ/AID-Meq.h>

#pragma aidgroup Meq
#pragma types #Meq::Resampler 
#pragma aid Integrate Flag Density

// The comments below are used to automatically generate a default
// init-record for the class 

//defrec begin MeqResampler
//  Resamples the Vells in its child's Result to the Cells of the parent's
//  Request. Current version ignores cell centers, sizes, domains, etc.,
//  and goes for a simple integrate/expand by an integer factor.
//field: integrate F
//  If True, then Vells are treated as integral values over a cell (i.e.,
//  when downsampling, values are integrated; when upsampling, values are 
//  divided). If False, then Vells are treated as samples at cell center 
//  (i.e., when downsampling, values are averaged; when upsampling, values 
//  are duplicated).
//field: flag_mask -1
//  Flag mask applied to child's result. -1 for all flags, 0 to ignore 
//  flags. Flagged values are ignored during integration.
//field: flag_bit 0
//  Flag bit(s) used to indicate flagged integrated results. If 0, then 
//  flag_mask&input_flags is used.
//field: flag_density 0.5
//  Critical ration of flagged/total pixels for integration. If this ratio
//  is exceeded, the integrated pixel is flagged.
//defrec end

namespace Meq {    

//##ModelId=400E530400A3
class Resampler : public Node
{
public:
    //##ModelId=400E5355029C
  Resampler();

    //##ModelId=400E5355029D
  virtual ~Resampler();

  //##ModelId=400E5355029F
  virtual TypeId objectType() const
  { return TpMeqResampler; }
  
  

protected:
  virtual void setStateImpl (DataRecord &rec,bool initializing);
    
  virtual int getResult (Result::Ref &resref, 
                         const std::vector<Result::Ref> &childres,
                         const Request &req,bool newreq);
  
private:
  // resamples vells according to current setup (see below)
  Vells::Ref resampleVells (const Vells &in);
    
  // templated helper function for above, which does the actual work
  template<class T>
  void doResample (Vells::Ref &voutref,const blitz::Array<T,2> &in);

  // if true, signal is assumed to be integrated over the cell
  // if false, signal is assumed to be a sampling at the cell center
  bool integrate;
  
  int flag_mask;
  
  int flag_bit;
  
  float flag_density;

  // All members below are used by resampleVells() to do the actual
  // resampling. These are meant to be set up by getResult(), before calling
  // resampleVells() repeatedly on all input Vells.
  
  // shape of output Vells
  LoShape outshape;    
  // # of cells to integrate in X/Y (1 if expanding)
  int     nsum[2];
  // # of cells to expand by in X/Y (1 if integrating)
  int     nexpand[2];  
  // renormalization factor applied to output cell after summing up 
  // the input cells. 
  double  renorm_factor;
  // Renormalization matrix, incorporating renorm_factor, plus
  // corrections for partially flagged integrations
  LoMat_double renorm_matrix;
  
  // pointer to output flags, 0 if no flagging
  VellSet::FlagArrayType *outflags;
  
  // pointer to input flags, 0 if no flagging
  const VellSet::FlagArrayType *inflags;
  
  
  
};


} // namespace Meq

#endif
