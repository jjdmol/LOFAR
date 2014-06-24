//# VisImagingWeight.h: Calculate Imaging Weights for a buffer from weight
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

#ifndef LOFAR_LOFARFT_VISIMAGINGWEIGHT_H
#define LOFAR_LOFARFT_VISIMAGINGWEIGHT_H

#include <synthesis/MSVis/VisImagingWeight.h>
#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Arrays/Matrix.h>

namespace casa
{
  class ROVisibilityIterator;
  class VisBuffer;
  template<class T> class Vector;
}

namespace LOFAR { //# NAMESPACE LOFAR - BEGIN
namespace LofarFT { //# NAMESPACE LOFAR - BEGIN

// <summary>
// Object to hold type of imaging weight scheme to be used on the fly and to provide
// facilities to do that.
// </summary>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// </motivation>
//
// <todo asof="">
// </todo>


class VisImagingWeight : public casa::VisImagingWeight
{
public:
  VisImagingWeight() : casa::VisImagingWeight() {};
  VisImagingWeight(const casa::String& type);
  virtual void weight(casa::Cube<casa::Float>& imagingWeight, const casa::VisBuffer& vb) const;
  
private:
  casa::String itsType;

};
  
} //end namespace LofarFT
} //end namespace LOFAR

#endif // LOFAR_LOFARFT_VISIMAGINGWEIGHT_H
