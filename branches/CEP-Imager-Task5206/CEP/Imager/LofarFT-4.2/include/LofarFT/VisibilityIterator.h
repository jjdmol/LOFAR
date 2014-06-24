//# LofarVisibilityIterator.h: Calculate Imaging Weights for a buffer from weight
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

#ifndef LOFAR_LOFARFT_VISIBILITYITERATOR_H
#define LOFAR_LOFARFT_VISIBILITYITERATOR_H

#include <synthesis/MSVis/VisibilityIterator.h>
#include <LofarFT/VisImagingWeight.h>
#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Quanta/Quantum.h>


namespace LOFAR { //# NAMESPACE LOFAR - BEGIN
namespace LofarFT {
  
class VisibilityIterator : public casa::VisibilityIterator
{
  public:
    // Constructors.
    // Note: The VisibilityIterator is not initialized correctly by default, you
    // need to call origin() before using it to iterate.
    VisibilityIterator();
    VisibilityIterator(casa::MeasurementSet & ms, const casa::Block<casa::Int>& sortColumns, 
        casa::Double timeInterval=0);
    
    VisibilityIterator & operator= (const VisibilityIterator & other);
    
    //assign a LofarVisImagingWeight object to this iterator
    void useImagingWeight(casa::CountedPtr<VisImagingWeight> imWgt);
    const VisImagingWeight & getImagingWeightGenerator () const;
  
  private:
    casa::CountedPtr<VisImagingWeight>      lofar_imwgt_p;    // object to calculate imaging weight
  
};

} // end namespace LofarFT
} // end namespace LOFAR

#endif // LOFAR_LOFARFT_VISIBILITYITERATOR_H
