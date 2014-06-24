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
#include <LofarFT/VisImagingWeightRobust.h>
#include <LofarFT/VisBuffer.h>

namespace LOFAR {
namespace LofarFT {
  
VisImagingWeightRobust::VisImagingWeightRobust(
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
  VisImagingWeight()
{
  casa::VisBufferAutoPtr vb (vi);
  
  itsUScale = (nx*cellx.get("rad").getValue());
  itsVScale = (ny*celly.get("rad").getValue());
  itsUOrigin = nx/2;
  itsVOrigin = ny/2;
  itsNX = nx;
  itsNY = ny;
  
  itsWeightMap.resize(itsNX, itsNY);
  itsWeightMap.set(0.0);
  
  casa::Float sumwt = 0.0;
  
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) 
  {
    for (vi.origin();vi.more();vi++) 
    {
      casa::Int nRow=vb->nRow();
      casa::Int nChan=vb->nChannel();
      for (casa::Int row=0; row<nRow; row++) 
      {
        for (casa::Int chn=0; chn<nChan; chn++) 
        {
          if (!vb->flagCube()(0, chn,row) && !vb->flagCube()(3, chn,row)) 
          {
            casa::Float f = vb->frequency()(chn)/casa::C::c;
            casa::Float u = vb->uvw()(row)(0)*f;
            casa::Float v = vb->uvw()(row)(1)*f;
            casa::Int ucell = casa::Int(itsUScale*u + itsUOrigin);
            casa::Int vcell = casa::Int(itsVScale*v + itsVOrigin);
            if (((ucell-uBox)>0) && ((ucell+uBox)<nx) && ((vcell-vBox)>0) && ((vcell+vBox)<ny)) 
            {
              for (casa::Int iv=-vBox;iv<=vBox;iv++) 
              {
                for (casa::Int iu=-uBox;iu<=uBox;iu++) 
                {
                  itsWeightMap(ucell+iu,vcell+iv) += vb->weightSpectrum()(0, chn, row) * !vb->flagCube()(0, chn,row);
                  sumwt += vb->weightSpectrum()(0, chn, row) * !vb->flagCube()(0, chn,row);
                  itsWeightMap(ucell+iu,vcell+iv) += vb->weightSpectrum()(3, chn, row) * !vb->flagCube()(3, chn,row);
                  sumwt += vb->weightSpectrum()(3, chn, row) * !vb->flagCube()(3, chn,row);
                }
              }
            }
            ucell = casa::Int(-itsUScale*u + itsUOrigin);
            vcell = casa::Int(-itsVScale*v + itsVOrigin);
            if (((ucell-uBox)>0) && ((ucell+uBox)<nx) && ((vcell-vBox)>0) && ((vcell+vBox)<ny)) 
            {
              for (casa::Int iv=-vBox;iv<=vBox;iv++) 
              {
                for (casa::Int iu=-uBox;iu<=uBox;iu++) 
                {
                  itsWeightMap(ucell+iu,vcell+iv) += vb->weightSpectrum()(0, chn, row) * !vb->flagCube()(0, chn,row);
                  sumwt += vb->weightSpectrum()(0, chn, row) * !vb->flagCube()(0, chn,row);
                  itsWeightMap(ucell+iu,vcell+iv) += vb->weightSpectrum()(3, chn, row) * !vb->flagCube()(3, chn,row);
                  sumwt += vb->weightSpectrum()(3, chn, row) * !vb->flagCube()(3, chn,row);
                }
              }
            }
          }
        }
      }
    }
  }

  // We use the approximation that all statistical weights are equal to
  // calculate the average summed weights (over visibilities, not bins!)
  // This is simply to try an ensure that the normalization of the robustness
  // parameter is similar to that of the ungridded case, but it doesn't have
  // to be exact, since any given case will require some experimentation.

  if (rmode=="norm") 
  {
    casa::Double sumlocwt = 0.;
    for(casa::Int vgrid=0;vgrid<ny;vgrid++) 
    {
      for(casa::Int ugrid=0;ugrid<nx;ugrid++) 
      {
        if (itsWeightMap(ugrid, vgrid)>0.0) sumlocwt += casa::square(itsWeightMap(ugrid,vgrid));
      }
    }
    itsF2 = casa::square(5.0*pow(10.0,casa::Double(-robust))) / (sumlocwt / sumwt);
    itsD2 = 1.0;
  }
  else if (rmode=="abs") 
  {
    itsF2 = casa::square(robust);
    itsD2 = 2.0 * casa::square(noise.get("Jy").getValue());

  }
  else // Uniform weighting
  {
    itsF2 = 1.0;
    itsD2 = 0.0;
  }
  std::cout << "f2: " << itsF2 << std::endl;
  std::cout << "d2: " << itsD2 << std::endl;
}
    
void VisImagingWeightRobust::weight(
  casa::Cube<casa::Float>& imagingWeight, 
  const casa::VisBuffer& vb) const
{
  std::cout << "VisImagingWeightRobust::weight" << std::endl;
  imagingWeight.assign(vb.weightSpectrum());
  imagingWeight(vb.flagCube()) = 0.0;
  
  casa::Int nRow = imagingWeight.shape()(2);
  casa::Int nChannel = imagingWeight.shape()(1);
  casa::Int nPol = imagingWeight.shape()(0);

  casa::Float u, v;
  for (casa::Int row=0; row<nRow; row++) 
  {
    for (casa::Int chn=0; chn<nChannel; chn++) 
    {
      casa::Float f = vb.frequency()(chn)/casa::C::c;
      u = vb.uvw()(row)(0)*f;
      v = vb.uvw()(row)(1)*f;
      casa::Int ucell = casa::Int(itsUScale*u + itsUOrigin);
      casa::Int vcell = casa::Int(itsVScale*v + itsVOrigin);
      if ((ucell>0) && (ucell<itsNX) && (vcell>0) && (vcell<itsNY)) 
      {
        if(itsWeightMap(ucell,vcell)>0.0) 
        {
          for (casa::Int pol=0; pol<nPol; pol++) 
          {
            imagingWeight(pol, chn,row) /= itsWeightMap(ucell,vcell)*itsF2 + itsD2;
          }
        }
      }
    }
  }
}
    
    
} // end LofarFT namespace
} // end LOFAR namespace
