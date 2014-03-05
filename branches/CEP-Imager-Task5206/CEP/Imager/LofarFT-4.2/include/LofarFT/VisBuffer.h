//# LofarVisBuffer.h: Definition of the CFStore class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#ifndef LOFAR_LOFARFT_VISBUFFER_H
#define LOFAR_LOFARFT_VISBUFFER_H

#include <synthesis/MSVis/VisBuffer.h>
#include <casa/Arrays/Matrix.h>
#include <LofarFT/VisibilityIterator.h>
#include <LofarFT/VisImagingWeight.h>

namespace LOFAR {
namespace LofarFT {
  
  class VisBuffer : public casa::VisBuffer
  {
  public:
    // Create empty VisBuffer you can assign to or attach.
    VisBuffer() : casa::VisBuffer() {};
    // Construct VisBuffer for a particular VisibilityIterator
    // The buffer will remain synchronized with the iterator.
    VisBuffer(VisibilityIterator &iter);

    // Copy construct, looses synchronization with iterator: only use buffer for
    // current iteration (or reattach).
    VisBuffer(const VisBuffer & vb) : casa::VisBuffer(vb) {};

    // Invalidate the cache
    virtual void invalidate();
    
    virtual const casa::Matrix<casa::Float>& imagingWeight() const;
    virtual const casa::Cube<casa::Float>& imagingWeightCube() const;
    
  protected:
    virtual const casa::Matrix<casa::Float>& imagingWeight(const VisImagingWeight & weightGenerator) const;  
    virtual const casa::Cube<casa::Float>& imagingWeightCube(const VisImagingWeight & weightGenerator) const;  
    virtual VisibilityIterator * getVisibilityIterator () const;
    
  private:
    VisibilityIterator * visIter_p;
    virtual casa::Bool lofarImagingWeightOK () const { return lofarImagingWeightOK_p;}
    virtual casa::Bool lofarImagingWeightCubeOK () const { return lofarImagingWeightCubeOK_p;}
    mutable casa::Bool lofarImagingWeightOK_p;
    mutable casa::Bool lofarImagingWeightCubeOK_p;
    mutable casa::Matrix<casa::Float> lofarImagingWeight_p;
    mutable casa::Cube<casa::Float> lofarImagingWeightCube_p;

  };

} // end namespace LofarFT
} // end namespace LOFAR

#endif
