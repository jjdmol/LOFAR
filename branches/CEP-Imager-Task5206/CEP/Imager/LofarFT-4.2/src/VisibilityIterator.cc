// -*- C++ -*-
//# VisImagingWeight.cc: Implementation of the VisImagingWeight class
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
#include <LofarFT/VisibilityIterator.h>
#include <LofarFT/VisImagingWeight.h>

namespace LOFAR {
namespace LofarFT {
  
VisibilityIterator::VisibilityIterator() : casa::VisibilityIterator() {};

VisibilityIterator::VisibilityIterator(
  casa::MeasurementSet & ms, 
  const casa::Block<casa::Int>& sortColumns, 
  casa::Double timeInterval) 
: 
  casa::VisibilityIterator(ms, sortColumns, timeInterval), 
  lofar_imwgt_p(0) 
{};
  
VisibilityIterator &
VisibilityIterator::operator= (const VisibilityIterator & other)
{
    // Let the superclass handle its part of the assignment

    VisibilityIterator::operator= (other);

    lofar_imwgt_p = other.lofar_imwgt_p;

    return * this;
}
  
void
VisibilityIterator::useImagingWeight (casa::CountedPtr<VisImagingWeight> imWgt)
{
    lofar_imwgt_p = imWgt;
}
  
const VisImagingWeight &
VisibilityIterator::getImagingWeightGenerator () const
{
    return *lofar_imwgt_p;
}
    
} // end namespace LofarFT
} // end namespace LOFAR
