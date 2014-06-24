// -*- C++ -*-
//# LofarVisBuffer.cc: Implementation of the CFStore class
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
#include <lofar_config.h>
#include <LofarFT/VisBuffer.h>
#include <casa/Exceptions/Error.h>

namespace LOFAR{
namespace LofarFT {
  
void VisBuffer::invalidate() 
{
  lofarImagingWeightOK_p = casa::False;
  lofarImagingWeightCubeOK_p = casa::False;
  casa::VisBuffer::invalidate();
}

VisBuffer::VisBuffer(VisibilityIterator & iter) : casa::VisBuffer(iter), visIter_p(&iter) {};
 
VisibilityIterator *
VisBuffer::getVisibilityIterator () const
{
    return visIter_p;
}
  
const casa::Matrix<casa::Float>& VisBuffer::imagingWeight () const
{
    const VisImagingWeight & weightGenerator = getVisibilityIterator()->getImagingWeightGenerator ();

    return imagingWeight (weightGenerator);
}

const casa::Cube<casa::Float>& VisBuffer::imagingWeightCube () const
{
    const VisImagingWeight & weightGenerator = getVisibilityIterator()->getImagingWeightGenerator ();
    return imagingWeightCube (weightGenerator);
}
  
const casa::Matrix<casa::Float>& VisBuffer::imagingWeight (const VisImagingWeight & weightGenerator) const
{
  throw casa::AipsError("Use VisBuffer::imagingWeightCube instead of VisBuffer::imagingWeight");
//     if (lofarImagingWeightOK_p)
//     {
//         return lofarImagingWeight_p;
//     }
//     
//     weightGenerator.weight (lofarImagingWeight_p, *this);
// 
//     this->lofarImagingWeightOK_p = casa::True;

    return lofarImagingWeight_p;
}

const casa::Cube<casa::Float>& VisBuffer::imagingWeightCube (const VisImagingWeight & weightGenerator) const
{
  if (lofarImagingWeightCubeOK_p)
  {
    return lofarImagingWeightCube_p;
  }
    
  weightGenerator.weight (lofarImagingWeightCube_p, *this);

  this->lofarImagingWeightCubeOK_p = casa::True;

  return lofarImagingWeightCube_p;
}

    
} // end LofarFT
} // end LOFAR namespace
