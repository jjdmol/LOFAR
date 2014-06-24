//# VisImagingWeightRobust.h: Calculate Imaging Weights for a buffer from weight
//# Copyright (C) 2009
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef LOFAR_LOFARFT_VISIMAGINGWEIGHTROBUST_H
#define LOFAR_LOFARFT_VISIMAGINGWEIGHTROBUST_H

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Arrays/Matrix.h>
#include <LofarFT/VisImagingWeight.h>

namespace casa
{
  class ROVisibilityIterator;
  class VisBuffer;
  template<class T> class Vector;
}

namespace LOFAR { //# NAMESPACE LOFAR - BEGIN
namespace LofarFT { //# NAMESPACE LOFAR - BEGIN

class VisImagingWeightRobust : public VisImagingWeight

{
  
public:
      
VisImagingWeightRobust(
  casa::ROVisibilityIterator& vi, 
  const casa::String& rmode, 
  const casa::Quantity& noise,
  casa::Double robust, 
  casa::Int nx, 
  casa::Int ny,
  const casa::Quantity& cellx, 
  const casa::Quantity& celly,
  casa::Int uBox = 0, 
  casa::Int vBox = 0,
  casa::Bool multiField = casa::False);

virtual void weight(
  casa::Cube<casa::Float>& imagingWeight, 
  const casa::VisBuffer& vb) const;

private:
  casa::Float itsF2;
  casa::Float itsD2;
  casa::Int itsNX;
  casa::Int itsNY;
  casa::Int itsUOrigin;
  casa::Int itsVOrigin;
  casa::Float itsUScale;
  casa::Float itsVScale;
  
  casa::Matrix<casa::Float> itsWeightMap;
  
};
  
} //end namespace LofarFT
} //end namespace LOFAR

#endif // LOFAR_LOFARFT_VISIMAGINGWEIGHTROBUST_H
