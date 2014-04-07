//# LofarVisResampler.cc: Implementation of the LofarVisResampler class
//#
//# Copyright (C) 2011
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <LofarFT/VisResampler.h>
#include <synthesis/TransformMachines/Utils.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <cassert>
#include <Common/OpenMP.h>

using namespace casa;

namespace LOFAR {
namespace LofarFT {

void VisResampler::set_chan_map(const casa::Vector<casa::Int> &map)
{
  itsChanMap.resize();
  itsChanMap = map;
}


void VisResampler::set_chan_map_CF(const casa::Vector<casa::Int> &map)
{
  itsChanMapCF.resize();
  itsChanMapCF = map;
}
  
void VisResampler::set_pol_map(const casa::Vector<casa::Int> &map)
{
  itsPolMap.resize();
  itsPolMap = map;
}
  
  
template
void VisResampler::DataToGridImpl_p(
  Array<DComplex>& grid, 
  VBStore& vbs,
  const Vector<uInt>& rows,
  Int rbeg, 
  Int rend,
  Matrix<Double>& sumwt,
  const Bool& dopsf,
  CFStore& cfs);

template
void VisResampler::DataToGridImpl_p(
  Array<Complex>& grid, 
  VBStore& vbs,
  const Vector<uInt>& rows,
  Int rbeg, 
  Int rend,
  Matrix<Double>& sumwt,
  const Bool& dopsf,
  CFStore& cfs);

template <class T>
void VisResampler::DataToGridImpl_p(
  Array<T>& grid,  
  VBStore& vbs,
  const Vector<uInt>& rows,
  Int rbeg, 
  Int rend,
  Matrix<Double>& sumwt,
  const Bool& dopsf,
  CFStore& cfs)
{
  // grid[nx,ny,np,nf]
  // vbs.data[np,nf,nrow]
  // cfs[nmx,nmy,nf,ncx,ncy]   (mueller,freq,convsize)

  // Get size of convolution functions.
  Int nConvX = (*(cfs.vdata()))[0][0][0].shape()[0];
  Int nConvY = (*(cfs.vdata()))[0][0][0].shape()[1];
  // Get size of grid.
  Int nGridX    = grid.shape()[0];
  Int nGridY    = grid.shape()[1];
  Int nGridPol  = grid.shape()[2];
  Int nGridChan = grid.shape()[3];
  //cout<<"nGridPol<<nGridChan "<<nGridPol<<" "<<nGridChan<<endl;

  // Get visibility data size.
  Int nVisPol   = vbs.flagCube().shape()[0];
  Int nVisChan  = vbs.flagCube().shape()[1];
  //cout<<"nVisPol<<nVisChan "<<nVisPol<<" "<<nVisChan<<endl;
  // Get oversampling and support size.
  Int sampx = SynthesisUtils::nint (cfs.sampling()[0]);
  Int sampy = SynthesisUtils::nint (cfs.sampling()[1]);
  Int supx = cfs.xSupport()[0];
  Int supy = cfs.ySupport()[0];

  ///AlwaysAssert ((2*supx+1)*sampx == nConvX, AipsError);
  ///AlwaysAssert ((2*supy+1)*sampy == nConvY, AipsError);

  Double* __restrict__ sumWtPtr = sumwt.data();
  Complex psfValues[4];
  psfValues[0] = psfValues[1] = psfValues[2] = psfValues[3] = Complex(1,0);


  // Loop over all visibility rows to process.
  for (Int inx=rbeg; inx<=rend; ++inx) {
    Int irow = rows[inx];
    const Double*  __restrict__ uvwPtr   = vbs.uvw().data() + irow*3;
    const Float*   __restrict__ imgWtPtr = vbs.imagingWeight().data() +
                                            irow * nVisChan;
    // Loop over all channels in the visibility data.
    // Map the visibility channel to the grid channel.
    // Skip channel if data are not needed.
    for (Int visChan=0; visChan<nVisChan; ++visChan) {
      Int gridChan = itsChanMap[visChan];
      Int CFChan = itsChanMapCF[visChan];

      if (gridChan >= 0  &&  gridChan < nGridChan) {

        // Determine the grid position from the UV coordinates in wavelengths.
        Double recipWvl = vbs.freq()[visChan] / C::c;
        Double posx = uvwScale_p[0] * uvwPtr[0] * recipWvl + offset_p[0];
        Double posy = uvwScale_p[1] * uvwPtr[1] * recipWvl + offset_p[1];
        Int locx = SynthesisUtils::nint (posx);    // location in grid
        Int locy = SynthesisUtils::nint (posy);
        Double diffx = locx - posx;
        ///if (diffx < 0) diffx += 1;
        Double diffy = locy - posy;
        ///if (diffy < 0) diffy += 1;
        Int offx = SynthesisUtils::nint (diffx * sampx); // location in
        Int offy = SynthesisUtils::nint (diffy * sampy); // oversampling
        ///          cout<<"  pos= ["<<posx<<", "<<posy<<"]"
        ///              <<", loc= ["<<locx<<", "<<locy<<"]"
        ///              <<", off= ["<<offx<<", "<<offy<<"]" << endl;
        offx += (nConvX-1)/2;
        offy += (nConvY-1)/2;

        // Only use visibility point if the full support is within grid.

        if (locx-supx >= 0  &&  locx+supx < nGridX  &&
            locy-supy >= 0  &&  locy+supy < nGridY) {


          ///            cout << "in grid"<<endl;
          // Get pointer to data and flags for this channel.
          Int doff = (irow * nVisChan + visChan) * nVisPol;
          const Complex* __restrict__ visPtr  = vbs.visCube().data()  + doff;
          const Bool*    __restrict__ flagPtr = vbs.flagCube().data() + doff;
          if (dopsf) {
            visPtr = psfValues;
          }
          // Handle a visibility if not flagged.
          for (Int ipol=0; ipol<nVisPol; ++ipol) {
            if (! flagPtr[ipol]) {

              // Map to grid polarization. Only use pol if needed.
              Int gridPol = itsPolMap(ipol);
              if (gridPol >= 0  &&  gridPol < nGridPol) {

                //cout<<"ipol: "<<ipol<<endl;
                // Get the offset in the grid data array.
                Int goff = (gridChan*nGridPol + gridPol) * nGridX * nGridY;
                // Loop over the scaled support.
                for (Int sy=-supy; sy<=supy; ++sy) {
                  // Get the pointer in the grid for the first x in this y.
                  T* __restrict__ gridPtr = grid.data() + goff +
                                            (locy+sy)*nGridX + locx-supx;
                  //cout<<"goff<<locy<<sy<<nGridX<<locx<<supx "<<goff<<" "<<locy<<" "<<sy<<" "<<nGridX<<" "<<locx<<" "<<supx<<endl;
                  // Get pointers to the first element to use in the 4
                  // convolution functions for this channel,pol.

                  // Fast version

                  const Complex* __restrict__ cf;
                  Int cfoff = (offy + sy*sampy)*nConvX + offx - supx*sampx;
                  cf = (*cfs.vdata())[CFChan][0][0].data() + cfoff;
                  for (Int sx=-supx; sx<=supx; ++sx) {
                    *gridPtr++ += visPtr[ipol] * *cf * *imgWtPtr;
                    cf += sampx;
                  }

                  // // Full version
                  // const Complex* __restrict__ cf[4];
                  // Int cfoff = (offy + sy*sampy)*nConvX + offx - supx*sampx;
                  // for (int i=0; i<4; ++i) {
                  //   cf[i] = (*cfs.vdata())[gridChan][i][ipol].data() + cfoff;
                  // }
                  // for (Int sx=-supx; sx<=supx; ++sx) {
                  //   // Loop over polarizations to correct for leakage.
                  //   Complex polSum(0,0);
                  //   for (Int i=0; i<4; ++i) {
                  //     polSum += visPtr[i] * *cf[i];
                  //     cf[i] += sampx;
                  //   }
                  //   polSum *= *imgWtPtr;
                  //   *gridPtr++ += polSum;
                  // }

                }
                sumWtPtr[gridPol+gridChan*nGridPol] += *imgWtPtr;
              } // end if gridPol
            } // end if !flagPtr
          } // end for ipol
        } // end if ongrid
      } // end if gridChan
      imgWtPtr++;
    } // end for visChan
  } // end for inx
}



//
//-----------------------------------------------------------------------------------
// Re-sample VisBuffer from a regular grid (griddedData) (a.k.a. de-gridding)
//
void VisResampler::GridToData(
  VBStore& vbs,
  const Array<Complex>& grid,
  const Vector<uInt>& rows,
  Int rbeg, 
  Int rend,
  CFStore& cfs)
{
  // grid[nx,ny,np,nf]
  // vbs.data[np,nf,nrow]
  // cfs[nmx,nmy,nf,ncx,ncy]   (mueller,freq,convsize)

  // Get size of convolution functions.
  Int nConvX = (*(cfs.vdata()))[0][0][0].shape()[0];
  Int nConvY = (*(cfs.vdata()))[0][0][0].shape()[1];
  // Get size of grid.
  Int nGridX    = grid.shape()[0];
  Int nGridY    = grid.shape()[1];
  Int nGridPol  = grid.shape()[2];
  Int nGridChan = grid.shape()[3];
  // Get visibility data size.
  Int nVisPol   = vbs.flagCube().shape()[0];
  Int nVisChan  = vbs.flagCube().shape()[1];
  // Get oversampling and support size.
  Int sampx = SynthesisUtils::nint (cfs.sampling()[0]);
  Int sampy = SynthesisUtils::nint (cfs.sampling()[1]);
  Int supx = cfs.xSupport()[0];
  Int supy = cfs.ySupport()[0];
  ///AlwaysAssert ((2*supx+1)*sampx == nConvX, AipsError);
  ///AlwaysAssert ((2*supy+1)*sampy == nConvY, AipsError);

  vector< Float > Weights_Lin_Interp;
  Weights_Lin_Interp.resize(4);
  vector< Int > deltax_pix_interp;
  deltax_pix_interp.resize(4);
  vector< Int > deltay_pix_interp;
  deltay_pix_interp.resize(4);
  //ofstream outFile("output.txt");

  // Loop over all visibility rows to process.
  for (Int inx=rbeg; inx<=rend; ++inx) {
    Int irow = rows[inx];
    const Double*  __restrict__ uvwPtr   = vbs.uvw().data() + irow*3;
    // Loop over all channels in the visibility data.
    // Map the visibility channel to the grid channel.
    // Skip channel if data are not needed.
    for (Int visChan=0; visChan<nVisChan; ++visChan) {
      Int gridChan = itsChanMap[visChan];
      Int CFChan = itsChanMapCF[visChan];

      if (gridChan >= 0  &&  gridChan < nGridChan) {
        // Determine the grid position from the UV coordinates in wavelengths.
        Double recipWvl = vbs.freq()[visChan] / C::c;
        Double posx = uvwScale_p[0] * uvwPtr[0] * recipWvl + offset_p[0];
        Double posy = uvwScale_p[1] * uvwPtr[1] * recipWvl + offset_p[1];
        Int locx = SynthesisUtils::nint (posx);    // location in grid
        Int locy = SynthesisUtils::nint (posy);
        Double diffx = locx - posx;
        ///if (diffx < 0) diffx += 1;
        Double diffy = locy - posy;
        ///if (diffy < 0) diffy += 1;


        Weights_Lin_Interp[0]=(1.-diffx)*(1.-diffy);
        Weights_Lin_Interp[1]=(1.-diffx)*diffy;
        Weights_Lin_Interp[2]=diffx*(1.-diffy);
        Weights_Lin_Interp[3]=diffx*diffy;

        Int offx = SynthesisUtils::nint (diffx * sampx); // location in
        Int offy = SynthesisUtils::nint (diffy * sampy); // oversampling
        ///          cout<<"  pos= ["<<posx<<", "<<posy<<"]"
        ///              <<", loc= ["<<locx<<", "<<locy<<"]"
        ///              <<", off= ["<<offx<<", "<<offy<<"]" << endl;
        offx += (nConvX-1)/2;
        offy += (nConvY-1)/2;
        
        // Only use visibility point if the full support is within grid.
        if (locx-supx >= 0  &&  locx+supx < nGridX  &&
            locy-supy >= 0  &&  locy+supy < nGridY) {
          ///            cout << "in grid"<<endl;
          // Get pointer to data and flags for this channel.
          Int doff = (irow * nVisChan + visChan) * nVisPol;
          Complex* __restrict__ visPtr  = vbs.visCube().data()  + doff;
          const Bool*    __restrict__ flagPtr = vbs.flagCube().data() + doff;
          // Handle a visibility if not flagged.
          for (Int ipol=0; ipol<nVisPol; ++ipol) {
            if (! flagPtr[ipol]) {
              visPtr[ipol] = Complex(0,0);
            }
          }
          //for (Int w=0; w<4; ++w) {
          //  Double weight_interp(Weights_Lin_Interp[w]);
          for (Int ipol=0; ipol<nVisPol; ++ipol) {
            if (! flagPtr[ipol]) {
              // Map to grid polarization. Only use pol if needed.
              Int gridPol = polMap_p(ipol);
              if (gridPol >= 0  &&  gridPol < nGridPol) {
                /// Complex norm(0,0);
                // Get the offset in the grid data array.
                Int goff = (gridChan*nGridPol + gridPol) * nGridX * nGridY;
                // Loop over the scaled support.
                for (Int sy=-supy; sy<=supy; ++sy) {
                  // Get the pointer in the grid for the first x in this y.
                  const Complex* __restrict__ gridPtr = grid.data() + goff +
                                                (locy+sy)*nGridX + locx-supx;
                  // Get pointers to the first element to use in the 4
                  // convolution functions for this channel,pol.

                  // fast version
                  const Complex* __restrict__ cf[1];
                  Int cfoff = (offy + sy*sampy)*nConvX + offx - supx*sampx;
                  cf[0] = (*cfs.vdata())[CFChan][0][0].data() + cfoff;
                  for (Int sx=-supx; sx<=supx; ++sx) {
                    visPtr[ipol] += *gridPtr * *cf[0];
                    cf[0] += sampx;
                    gridPtr++;
                  }

                  // // Full version
                  // const Complex* __restrict__ cf[4];
                  // Int cfoff = (offy + sy*sampy)*nConvX + offx - supx*sampx;
                  // for (int i=0; i<4; ++i) {
                  //   cf[i] = (*cfs.vdata)[gridChan][i][ipol].data() + cfoff;
                  // }
                  // for (Int sx=-supx; sx<=supx; ++sx) {
                  //   for (Int i=0; i<nVisPol; ++i) {
                  //     visPtr[i] += *gridPtr * *cf[i];
                  //     cf[i] += sampx;
                  //   }
                  //   gridPtr++;
                  // }

                }
              } // end if gridPol
            } // end if !flagPtr
          } // end for ipol
        } // end if ongrid
      } // end if gridChan
      //}
    } // end for visChan
  } // end for inx
}

void VisResampler::ComputeResiduals(VBStore& vbs)
{
  Int rbeg = vbs.beginRow();
  Int rend = vbs.endRow();
  IPosition vbDataShape = vbs.modelVisCube().shape();
  IPosition start(vbDataShape);
  IPosition last(vbDataShape);
  start=0; start(2)=rbeg;
  last(2)=rend; //last=last-1;

  for(uInt ichan = start(0); ichan < last(0); ichan++)
    for(uInt ipol = start(1); ipol < last(1); ipol++)
      for(uInt irow = start(2); irow < last(2); irow++)
        vbs.modelVisCube()(ichan,ipol,irow) -= vbs.visCube()(ichan,ipol,irow);
}

void VisResampler::sgrid(
  Vector<Double>& pos, 
  Vector<Int>& loc,
  Vector<Int>& off, 
  Complex& phasor,
  const Int& irow, 
  const Matrix<Double>& uvw,
  const Double&, 
  const Double& freq,
  const Vector<Double>& scale,
  const Vector<Double>& offset,
  const Vector<Float>& sampling)
{
  //Double phase;
  Vector<Double> uvw_l(3,0); // This allows gridding of weights
                              // centered on the uv-origin
  if (uvw.nelements() > 0) for(Int i=0;i<3;i++) uvw_l[i]=uvw(i,irow);

  pos(2)=0;//sqrt(abs(scale[2]*uvw_l(2)*freq/C::c))+offset[2];
  loc(2)=0;//SynthesisUtils::nint(pos[2]);
  off(2)=0;

  for(Int idim=0;idim<2;idim++)
    {
      pos[idim]=scale[idim]*uvw_l(idim)*freq/C::c+offset[idim];
      loc[idim]=SynthesisUtils::nint(pos[idim]);
      off[idim]=SynthesisUtils::nint((loc[idim]-pos[idim])*sampling[idim]);
    }

    phasor=Complex(1.0);
}

} // end namespace LofarFT
} // end namespace LOFAR
