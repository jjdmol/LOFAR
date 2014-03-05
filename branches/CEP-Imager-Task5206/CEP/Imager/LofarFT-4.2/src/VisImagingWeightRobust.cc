// -*- C++ -*-
//# LofarVisImagingWeight.cc: Implementation of the LofarVisImagingWeight class
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
//# $Id: $

#include <lofar_config.h>
#include <LofarFT/VisImagingWeight.h>
#include <LofarFT/VisBuffer.h>

namespace LOFAR {
namespace LofarFT {
  
//empty constructor
VisImagingWeight::VisImagingWeight() : 
  casa::VisImagingWeight() {};
  
//Constructor to calculate natural and radial weights
VisImagingWeight::VisImagingWeight(const casa::String& type) :
  casa::VisImagingWeight(type) {};

//Constructor to calculate uniform weight schemes; include Brigg's and super/uniform
//If multiField=True, the weight density calcution is done on a per field basis, 
//else it is all fields combined
VisImagingWeight::VisImagingWeight(
  casa::ROVisibilityIterator& vi, 
  const casa::String& rmode, 
  const casa::Quantity& noise,
  const casa::Double robust, 
  const casa::Int nx, 
  const casa::Int ny,
  const casa::Quantity& cellx, 
  const casa::Quantity& celly,
  const casa::Int uBox, 
  const casa::Int vBox, 
  const casa::Bool multiField) 
  :
    casa::VisImagingWeight(vi, rmode, noise, robust, nx, ny, cellx, celly, uBox, vBox, multiField) {};  

  
void VisImagingWeight::weight(
  casa::Matrix<casa::Float>& imagingWeight, 
  const casa::VisBuffer& vb) const
{
    casa::String type = getType();
    if (type == "none") {
        throw (casa::AipsError ("Programmer Error... imaging weights not set"));
    }

    casa::Vector<casa::Float> weightvec = vb.weight ();
    casa::Matrix<casa::Bool> flagmat = vb.flag ();
    imagingWeight.resize (flagmat.shape ());

    if (getType () == "uniform") 
    {
        weightUniform (imagingWeight, flagmat, vb.uvwMat(), vb.frequency(), weightvec, vb.msId (), vb.fieldId ());
    } 
    else if (getType () == "radial") 
    {
        weightRadial (imagingWeight, flagmat, uvwmat, vb.frequency(), weightvec);
    }
    else 
    {
      weightNatural (imagingWeight, flagmat, weightvec);
    }

    if (doFilter ()) 
    {
        filter (imagingWeight, flagmat, vb.uvwMat(), vb.frequency(), weightvec);
    }

  
}

    
} // end LofarFT namespace
} // end LOFAR namespace
