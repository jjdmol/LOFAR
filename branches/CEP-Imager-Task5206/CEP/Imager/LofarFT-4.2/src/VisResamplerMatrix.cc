//# VisResamplerMatrix.cc: Implementation of the LofarVisResampler class
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
//# $Id: VisResampler.cc 28512 2014-03-05 01:07:53Z vdtol $

#include <lofar_config.h>
#include <LofarFT/VisResamplerMatrix.h>
#include <synthesis/TransformMachines/Utils.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <cassert>
#include <Common/OpenMP.h>

using namespace casa;

namespace LOFAR {
namespace LofarFT {

template
void VisResamplerMatrix::DataToGridImpl_p(
  Array<DComplex>& grid, 
  VBStore& vbs,
  const Vector<uInt>& rows,
  Int rbeg, 
  Int rend,
  Matrix<Double>& sumwt,
  const Bool& dopsf,
  CFStore& cfs);

template
void VisResamplerMatrix::DataToGridImpl_p(
  Array<Complex>& grid, 
  VBStore& vbs,
  const Vector<uInt>& rows,
  Int rbeg, 
  Int rend,
  Matrix<Double>& sumwt,
  const Bool& dopsf,
  CFStore& cfs);

template <class T>
void VisResamplerMatrix::DataToGridImpl_p(
  Array<T>& grid,  
  VBStore& vbs,
  const Vector<uInt>& rows,
  Int rbeg, 
  Int rend,
  Matrix<Double>& sumwt,
  const Bool& dopsf,
  CFStore& cfs)
{
  // Get size of convolution functions.
  Int nConvX = (*(cfs.vdata()))[0][0][0].shape()[0];
  Int nConvY = (*(cfs.vdata()))[0][0][0].shape()[1];
  // Get size of grid.
  Int nGridX    = grid.shape()[0];
  Int nGridY    = grid.shape()[1];
  Int nGridChan = grid.shape()[3];

  // Get visibility data size.
  Int nVisPol   = vbs.flagCube().shape()[0];
  Int nVisChan  = vbs.flagCube().shape()[1];

  // Get oversampling and support size.
  Int sampx = SynthesisUtils::nint (cfs.sampling()[0]);
  Int sampy = SynthesisUtils::nint (cfs.sampling()[1]);
  Int supx = cfs.xSupport()[0];
  Int supy = cfs.ySupport()[0];

  Double* __restrict__ sumWtPtr = sumwt.data();
  Complex psfValues[4];
  psfValues[0] = psfValues[1] = psfValues[2] = psfValues[3] = Complex(1,0);


  // Loop over all visibility rows to process.
  for (Int inx=rbeg; inx<=rend; ++inx) {
    Int irow = rows[inx];
    const Double*  __restrict__ uvwPtr   = vbs.uvw().data() + irow*3;
    // Loop over all channels in the visibility data.
    // Map the visibility channel to the grid channel.
    // Skip channel if data are not needed.
    for (Int visChan=0; visChan<nVisChan; ++visChan) {
      Int gridChan = itsChanMap[visChan];

      if (gridChan >= 0  &&  gridChan < nGridChan) {
        
        // Determine the grid position from the UV coordinates in wavelengths.
        Double recipWvl = vbs.freq()[visChan] / C::c;
        Double posx = uvwScale_p[0] * uvwPtr[0] * recipWvl + offset_p[0];
        Double posy = uvwScale_p[1] * uvwPtr[1] * recipWvl + offset_p[1];
        Int locx = SynthesisUtils::nint (posx);    // location in grid
        Int locy = SynthesisUtils::nint (posy);
        Double diffx = locx - posx;
        Double diffy = locy - posy;
        Int offx = SynthesisUtils::nint (diffx * sampx); // location in
        Int offy = SynthesisUtils::nint (diffy * sampy); // oversampling
        offx += (nConvX-1)/2;
        offy += (nConvY-1)/2;
        // Scaling with frequency is not necessary (according to Cyril).
        Double freqFact = 1;   // = cfFreq / vbs.freq_p[visChan];
        Int fsampx = SynthesisUtils::nint (sampx * freqFact);
        Int fsampy = SynthesisUtils::nint (sampy * freqFact);
        Int fsupx  = SynthesisUtils::nint (supx / freqFact);
        Int fsupy  = SynthesisUtils::nint (supy / freqFact);

        // Only use visibility point if the full support is within grid.

        if (locx-supx >= 0  &&  locx+supx < nGridX  &&
            locy-supy >= 0  &&  locy+supy < nGridY) {


          ///            cout << "in grid"<<endl;
          // Get pointer to data and flags for this channel.
          Int doff = (irow * nVisChan + visChan) * nVisPol;
          const Complex* __restrict__ visPtr  = vbs.visCube().data()  + doff;
          const Bool*    __restrict__ flagPtr = vbs.flagCube().data() + doff;
          const Float*   __restrict__ imgWtPtr = vbs.imagingWeightCube().data() + doff;
          if (dopsf) {
            visPtr = psfValues;
          }
          // Handle a visibility if not flagged.
          for (Int ipol=0; ipol<4; ++ipol) 
          {
            if (! flagPtr[ipol]) 
            {

              // Get the offset in the grid data array.
              Int goff = (gridChan*4 + ipol) * nGridX * nGridY;
              // Loop over the scaled support.
              for (Int sy=-fsupy; sy<=fsupy; ++sy) {
                // Get the pointer in the grid for the first x in this y.
                T* __restrict__ gridPtr = grid.data() + goff +
                                          (locy+sy)*nGridX + locx-supx;
                // Get pointers to the first element to use in the 4
                // convolution functions for this channel,pol.

                const Complex* __restrict__ cf[4];
                Int cfoff = (offy + sy*fsampy)*nConvX + offx - fsupx*fsampx;
                for (int i=0; i<4; ++i) {
                  cf[i] = (*cfs.vdata())[gridChan][i][ipol].data() + cfoff;
                }
                for (Int sx=-fsupx; sx<=fsupx; ++sx) {
                  // Loop over polarizations to correct for leakage.
                  Complex polSum(0,0);
                  for (Int i=0; i<4; ++i) {
                    polSum += visPtr[i] * *cf[i];
                    cf[i] += fsampx;
                  }
                  polSum *= *imgWtPtr;
                  *gridPtr++ += polSum;
                }

              }
              sumWtPtr[ipol+gridChan*4] += imgWtPtr[ipol];
            } // end if !flagPtr
          } // end for ipol
        } // end if ongrid
      } // end if gridChan
    } // end for visChan
  } // end for inx
}



//
//-----------------------------------------------------------------------------------
// Re-sample VisBuffer from a regular grid (griddedData) (a.k.a. de-gridding)
//
void VisResamplerMatrix::GridToData(
  VBStore& vbs,
  const Array<Complex>& grid,
  const Vector<uInt>& rows,
  Int rbeg, 
  Int rend,
  CFStore& cfs)
{
  // Get size of convolution functions.
  Int nConvX = (*(cfs.vdata()))[0][0][0].shape()[0];
  Int nConvY = (*(cfs.vdata()))[0][0][0].shape()[1];
  // Get size of grid.
  Int nGridX    = grid.shape()[0];
  Int nGridY    = grid.shape()[1];
  Int nGridChan = grid.shape()[3];
  // Get visibility data size.
  Int nVisPol   = vbs.flagCube().shape()[0];
  Int nVisChan  = vbs.flagCube().shape()[1];
  // Get oversampling and support size.
  Int sampx = SynthesisUtils::nint (cfs.sampling()[0]);
  Int sampy = SynthesisUtils::nint (cfs.sampling()[1]);
  Int supx = cfs.xSupport()[0];
  Int supy = cfs.ySupport()[0];


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
        Double diffy = locy - posy;


        Int offx = SynthesisUtils::nint (diffx * sampx); // location in
        Int offy = SynthesisUtils::nint (diffy * sampy); // oversampling
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
          for (Int ipol=0; ipol<4; ++ipol) {
            if (! flagPtr[ipol]) {
              visPtr[ipol] = Complex(0,0);
            }
          }
          for (Int ipol=0; ipol<4; ++ipol) {
            if (! flagPtr[ipol]) {
              /// Complex norm(0,0);
              // Get the offset in the grid data array.
              Int goff = (gridChan*4 + ipol) * nGridX * nGridY;
              // Loop over the scaled support.
              for (Int sy=-supy; sy<=supy; ++sy) 
              {
                // Get the pointer in the grid for the first x in this y.
                const Complex* __restrict__ gridPtr = grid.data() + goff +
                                              (locy+sy)*nGridX + locx-supx;
                // Get pointers to the first element to use in the 4
                // convolution functions for this channel,pol.

                const Complex* __restrict__ cf[4];
                Int cfoff = (offy + sy*sampy)*nConvX + offx - supx*sampx;
                for (int i=0; i<4; ++i) {
                  cf[i] = (*cfs.vdata())[gridChan][i][ipol].data() + cfoff;
                }
                for (Int sx=-supx; sx<=supx; ++sx) {
                  for (Int i=0; i<4; ++i) {
                    visPtr[i] += *gridPtr * *cf[i];
                    cf[i] += sampx;
                  }
                  gridPtr++;
                }
              }
            } // end if !flagPtr
          } // end for ipol
        } // end if ongrid
      } // end if gridChan
      //}
    } // end for visChan
  } // end for inx
}

} // end namespace LofarFT
} // end namespace LOFAR
