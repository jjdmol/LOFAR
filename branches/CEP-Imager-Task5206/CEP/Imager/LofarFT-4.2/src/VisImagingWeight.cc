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
  
VisImagingWeight::VisImagingWeight(const casa::String& type) :
  casa::VisImagingWeight(type),
  itsType(type)
  {};

void VisImagingWeight::weight(
  casa::Cube<casa::Float>& imagingWeight, 
  const casa::VisBuffer& vb) const
{
  imagingWeight.assign(vb.weightSpectrum());
  imagingWeight(vb.flagCube()) = 0.0;
  
  if (itsType == "radial")
  {
  
    casa::Int nPol = imagingWeight.shape()(0);
    casa::Int nChan = imagingWeight.shape()(1);
    casa::Int nRow = imagingWeight.shape()(2);

    for (casa::Int row=0; row<nRow; row++) 
    {
      for (casa::Int chn=0; chn< nChan; chn++) 
      {
        casa::Float f=vb.frequency()(chn)/casa::C::c;
        casa::Float factor = f * sqrt(casa::square(vb.uvw()(row)(0))+casa::square(vb.uvw()(row)(1)));
        for (casa::Int pol=0; pol< nPol; pol++) 
        {
          imagingWeight(pol, chn,row) *= factor;
        }
      }
    }
  }
}

    
} // end LofarFT namespace
} // end LOFAR namespace
