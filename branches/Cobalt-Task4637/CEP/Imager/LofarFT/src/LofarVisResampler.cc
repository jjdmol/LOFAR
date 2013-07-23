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
#include <LofarFT/LofarVisResampler.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <cassert>
#include <Common/OpenMP.h>

namespace LOFAR {

    // Instantiate both templates.
  // template
  // void LofarVisResampler::DataToGridImpl_p(Array<DComplex>& grid, LofarVBStore& vbs,
  //                                          const Vector<uInt>& rows,
  //                                          Int rbeg, Int rend,
  // 					Matrix<Double>& sumwt,const Bool& dopsf,
  // 					LofarCFStore& cfs) __restrict__;
  // template
  // void LofarVisResampler::DataToGridImpl_p(Array<Complex>& grid, LofarVBStore& vbs,
  //                                          const Vector<uInt>& rows,
  //                                          Int rbeg, Int rend,
  // 					Matrix<Double>& sumwt,const Bool& dopsf,
  // 					LofarCFStore& cfs) __restrict__;

  // template <class T>
  // void LofarVisResampler::DataToGridImpl_p(Array<T>& grid,  LofarVBStore& vbs,
  //                                          const Vector<uInt>& rows,
  //                                          Int rbeg, Int rend,
  //                                          Matrix<Double>& sumwt,
  //                                          const Bool& dopsf,
  //                                          LofarCFStore& cfs) __restrict__



  template
  void LofarVisResampler::lofarDataToGrid_interp(Array<Complex>& grid,  LofarVBStore& vbs,
						 const Vector<uInt>& rows,
						 Matrix<Double>& sumwt,
						 const Bool& dopsf,
						 LofarCFStore& cfs);

  template
  void LofarVisResampler::lofarDataToGrid_interp(Array<DComplex>& grid,  LofarVBStore& vbs,
						 const Vector<uInt>& rows,
						 Matrix<Double>& sumwt,
						 const Bool& dopsf,
						 LofarCFStore& cfs);

  template <class T>
    void LofarVisResampler::lofarDataToGrid_interp(Array<T>& grid,  LofarVBStore& vbs,
						 const Vector<uInt>& rows,
						 Matrix<Double>& sumwt,
						 const Bool& dopsf,
						 LofarCFStore& cfs)//,
						 //vector<Float> wvec, Float wStep, Float wcf, vector<Complex> vecCorr)
  {
    // grid[nx,ny,np,nf]
    // vbs.data[np,nf,nrow]
    // cfs[nmx,nmy,nf,ncx,ncy]   (mueller,freq,convsize)

    // Get size of convolution functions.
    Int nConvX = (*(cfs.vdata))[0][0][0].shape()[0];
    Int nConvY = (*(cfs.vdata))[0][0][0].shape()[1];
    // Get size of grid.
    Int nGridX    = grid.shape()[0];
    Int nGridY    = grid.shape()[1];
    Int nGridPol  = grid.shape()[2];
    Int nGridChan = grid.shape()[3];
    //cout<<"nGridPol<<nGridChan "<<nGridPol<<" "<<nGridChan<<endl;

    // Get visibility data size.
    Int nVisPol   = vbs.flagCube_p.shape()[0];
    Int nVisChan  = vbs.flagCube_p.shape()[1];
    //cout<<"nVisPol<<nVisChan "<<nVisPol<<" "<<nVisChan<<endl;
    // Get oversampling and support size.
    Int sampx = SynthesisUtils::nint (cfs.sampling[0]);
    Int sampy = SynthesisUtils::nint (cfs.sampling[1]);
    Int supx = cfs.xSupport[0];
    Int supy = cfs.ySupport[0];

    ///AlwaysAssert ((2*supx+1)*sampx == nConvX, AipsError);
    ///AlwaysAssert ((2*supy+1)*sampy == nConvY, AipsError);

    Double* __restrict__ sumWtPtr = sumwt.data();
    Complex psfValues[4];
    psfValues[0] = psfValues[1] = psfValues[2] = psfValues[3] = Complex(1,0);
    // psfValues[0] = -Complex(2,0);
    // psfValues[1] = -Complex(1,1);
    // psfValues[2] = -Complex(1,-1);
    // psfValues[3] = -Complex(0,0);

    uInt inxRowWCorr(0);

    // Loop over all visibility rows to process.
    for (Int inx=0; inx<rows.size(); ++inx) {
      Int irow = rows[inx];

      // Float icorr=(abs(wvec[inxRowWCorr])-wcf)/wStep;
      // Complex factor=vecCorr[floor(abs(icorr))];
      // if(wvec[inxRowWCorr]<0.){
      // 	if((abs(wvec[inxRowWCorr])-wcf)>0.){factor=conj(factor);}
      // } else {
      // 	if((abs(wvec[inxRowWCorr])-wcf)<0.){factor=conj(factor);}
      // }
      // //cout<<"inx="<<inxRowWCorr<<" wvec="<<wvec[inxRowWCorr]<<" wcf="<<wcf<<" icorr="<<floor(abs(icorr))<<" wstep="<<wStep<<" icorrf="<<icorr<<" factor="<<factor<<endl;
      // inxRowWCorr+=1;
      // factor=conj(factor);
      // if(isnan(abs(factor))){cout<<"nan"<<endl;}

      const Double*  __restrict__ uvwPtr   = vbs.uvw_p.data() + irow*3;
      const Float*   __restrict__ imgWtPtr = vbs.imagingWeight_p.data() +
                                             irow * nVisChan;

      //cout<<vbs.imagingWeight_p.shape()<<endl;
      //cout<<vbs.uvw_p.shape()<<endl;
      // Loop over all channels in the visibility data.
      // Map the visibility channel to the grid channel.
      // Skip channel if data are not needed.
      for (Int visChan=0; visChan<nVisChan; ++visChan) {
        Int gridChan = chanMap_p[visChan];
        Int CFChan = ChanCFMap[visChan];
	//cout<<"  chan="<<visChan<<"taking CF="<<CFChan<<endl;


	// !! dirty trick to select all channels
	//cout<<chanMap_p<<endl;
	chanMap_p[visChan]=0;

        if (gridChan >= 0  &&  gridChan < nGridChan) {

          // Determine the grid position from the UV coordinates in wavelengths.
          Double recipWvl = vbs.freq_p[visChan] / C::c;
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
            const Complex* __restrict__ visPtr  = vbs.visCube_p.data()  + doff;
            const Bool*    __restrict__ flagPtr = vbs.flagCube_p.data() + doff;
            if (dopsf) {
              visPtr = psfValues;
            }
            // Handle a visibility if not flagged.
            for (Int ipol=0; ipol<nVisPol; ++ipol) {
              if (! flagPtr[ipol]) {


                // Map to grid polarization. Only use pol if needed.
                Int gridPol = polMap_p(ipol);
                if (gridPol >= 0  &&  gridPol < nGridPol) {

  		  //cout<<"ipol: "<<ipol<<endl;
                  // Get the offset in the grid data array.
                  Int goff = (gridChan*nGridPol + gridPol) * nGridX * nGridY;
                  // Loop over the scaled support.
                  for (Int sy=-fsupy; sy<=fsupy; ++sy) {
                    // Get the pointer in the grid for the first x in this y.
                    T* __restrict__ gridPtr = grid.data() + goff +
                                              (locy+sy)*nGridX + locx-supx;
  		    //cout<<"goff<<locy<<sy<<nGridX<<locx<<supx "<<goff<<" "<<locy<<" "<<sy<<" "<<nGridX<<" "<<locx<<" "<<supx<<endl;
                    // Get pointers to the first element to use in the 4
                    // convolution functions for this channel,pol.

		    // Fast version

                    const Complex* __restrict__ cf[1];
                    Int cfoff = (offy + sy*fsampy)*nConvX + offx - fsupx*fsampx;
		    cf[0] = (*cfs.vdata)[CFChan][0][0].data() + cfoff;
                    for (Int sx=-fsupx; sx<=fsupx; ++sx) {
                      // Loop over polarizations to correct for leakage.
                      Complex polSum(0,0);
		      polSum += visPtr[ipol] * *cf[0];
		      cf[0] += fsampx;
  		      polSum *= *imgWtPtr;
                      *gridPtr++ += polSum;
                    }

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



  void LofarVisResampler::lofarGridToData_interp(LofarVBStore& vbs,
						 const Array<Complex>& grid,
						 const Vector<uInt>& rows,
						 //Int rbeg, Int rend,
						 LofarCFStore& cfs)//;,
						 //vector<Float> wvec, Float wStep, Float wcf, vector<Complex> vecCorr)
    
  {
    // grid[nx,ny,np,nf]
    // vbs.data[np,nf,nrow]
    // cfs[nmx,nmy,nf,ncx,ncy]   (mueller,freq,convsize)

    // Get size of convolution functions.
    Int nConvX = (*(cfs.vdata))[0][0][0].shape()[0];
    Int nConvY = (*(cfs.vdata))[0][0][0].shape()[1];
    // Get size of grid.
    Int nGridX    = grid.shape()[0];
    Int nGridY    = grid.shape()[1];
    Int nGridPol  = grid.shape()[2];
    Int nGridChan = grid.shape()[3];
    // Get visibility data size.
    Int nVisPol   = vbs.flagCube_p.shape()[0];
    Int nVisChan  = vbs.flagCube_p.shape()[1];
    // Get oversampling and support size.
    Int sampx = SynthesisUtils::nint (cfs.sampling[0]);
    Int sampy = SynthesisUtils::nint (cfs.sampling[1]);
    Int supx = cfs.xSupport[0];
    Int supy = cfs.ySupport[0];
    ///AlwaysAssert ((2*supx+1)*sampx == nConvX, AipsError);
    ///AlwaysAssert ((2*supy+1)*sampy == nConvY, AipsError);

    vector< Float > Weights_Lin_Interp;
    Weights_Lin_Interp.resize(4);
    vector< Int > deltax_pix_interp;
    deltax_pix_interp.resize(4);
    vector< Int > deltay_pix_interp;
    deltay_pix_interp.resize(4);
    //ofstream outFile("output.txt",ios::app);

    Int inxRowWCorr(0);

    // Loop over all visibility rows to process.


    for (Int inx=0; inx<rows.size(); ++inx) {
      Int irow = rows[inx];
    // for (Int inx=rbeg; inx<=rend; ++inx) {
    //   Int irow = rows[inx];

      
    //   Float icorr=(abs(wvec[inxRowWCorr])-wcf)/wStep;

    //   Complex factor=vecCorr[floor(abs(icorr))];
    //   if(wvec[inxRowWCorr]<0.){
    // 	if((abs(wvec[inxRowWCorr])-wcf)>0.){factor=conj(factor);}
    //   } else {
    // 	if((abs(wvec[inxRowWCorr])-wcf)<0.){factor=conj(factor);}

    //   }
    //   //cout<<"inx="<<inxRowWCorr<<" wvec="<<wvec[inxRowWCorr]<<" wcf="<<wcf<<" icorr="<<floor(abs(icorr))<<" wstep="<<wStep<<" icorrf="<<icorr<<" factor="<<factor<<endl;
    //   inxRowWCorr+=1;



      const Double*  __restrict__ uvwPtr   = vbs.uvw_p.data() + irow*3;
      // Loop over all channels in the visibility data.
      // Map the visibility channel to the grid channel.
      // Skip channel if data are not needed.
      for (Int visChan=0; visChan<nVisChan; ++visChan) {
        Int gridChan = chanMap_p[visChan];
        Int CFChan = ChanCFMap[visChan];
	//cout<<"  chan="<<visChan<<"taking CF="<<CFChan<<endl;

	// !! dirty trick to select all channels
	chanMap_p[visChan]=0;

        if (gridChan >= 0  &&  gridChan < nGridChan) {
          // Determine the grid position from the UV coordinates in wavelengths.
          Double recipWvl = vbs.freq_p[visChan] / C::c;
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
            Complex* __restrict__ visPtr  = vbs.visCube_p.data()  + doff;
            const Bool*    __restrict__ flagPtr = vbs.flagCube_p.data() + doff;
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
                  for (Int sy=-fsupy; sy<=fsupy; ++sy) {
                    // Get the pointer in the grid for the first x in this y.
                    const Complex* __restrict__ gridPtr = grid.data() + goff +
                                                  (locy+sy)*nGridX + locx-supx;
                    // Get pointers to the first element to use in the 4
                    // convolution functions for this channel,pol.

		    // fast version
                    const Complex* __restrict__ cf[1];
                    Int cfoff = (offy + sy*fsampy)*nConvX + offx - fsupx*fsampx;
 		    cf[0] = (*cfs.vdata)[CFChan][0][0].data() + cfoff;
                    for (Int sx=-fsupx; sx<=fsupx; ++sx) {
		      //outFile<<irow <<" "<<ipol<<" "<<posx<<" "<<posy<<" "<<(offx+ sx*fsampx-(nConvX-1.)/2.)/float(fsampx)<<" "<<(offy + sy*fsampy-(nConvX-1)/2.)/float(fsampy)
		      //<<" "<<real(*gridPtr * *cf[0])<<" "<<imag(*gridPtr * *cf[0])<<" "<<real(*gridPtr)<<" "<<imag(*gridPtr)<<endl;
		      visPtr[ipol] += *gridPtr  * *cf[0];//* factor;
		      cf[0] += fsampx;
                      gridPtr++;
                    }

		    // // Full version
                    // const Complex* __restrict__ cf[4];
                    // Int cfoff = (offy + sy*fsampy)*nConvX + offx - fsupx*fsampx;
                    // for (int i=0; i<4; ++i) {
                    //   cf[i] = (*cfs.vdata)[gridChan][i][ipol].data() + cfoff;
                    // }
                    // for (Int sx=-fsupx; sx<=fsupx; ++sx) {
                    //   for (Int i=0; i<nVisPol; ++i) {
                    //     visPtr[i] += *gridPtr * *cf[i];
                    //     cf[i] += fsampx;
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
    //assert(false);
  }


  void LofarVisResampler::lofarGridToData_linear(LofarVBStore& vbs,
                                          const Array<Complex>& grid,
                                          const Vector<uInt>& rows,
                                          Int rbeg, Int rend,
                                          LofarCFStore& cfs0,
                                          LofarCFStore& cfs1)
  {
    // grid[nx,ny,np,nf]
    // vbs.data[np,nf,nrow]
    // cfs[nmx,nmy,nf,ncx,ncy]   (mueller,freq,convsize)

    // Get size of convolution functions.
    Int nConvX = (*(cfs0.vdata))[0][0][0].shape()[0];
    Int nConvY = (*(cfs0.vdata))[0][0][0].shape()[1];
    // Get size of grid.
    Int nGridX    = grid.shape()[0];
    Int nGridY    = grid.shape()[1];
    Int nGridPol  = grid.shape()[2];
    Int nGridChan = grid.shape()[3];
    // Get visibility data size.
    Int nVisPol   = vbs.flagCube_p.shape()[0];
    Int nVisChan  = vbs.flagCube_p.shape()[1];
    // Get oversampling and support size.
    Int sampx = SynthesisUtils::nint (cfs0.sampling[0]);
    Int sampy = SynthesisUtils::nint (cfs0.sampling[1]);
    Int supx = cfs0.xSupport[0];
    Int supy = cfs0.ySupport[0];
    ///AlwaysAssert ((2*supx+1)*sampx == nConvX, AipsError);
    ///AlwaysAssert ((2*supy+1)*sampy == nConvY, AipsError);

    vector< Float > Weights_Lin_Interp;
    Weights_Lin_Interp.resize(4);
    vector< Int > deltax_pix_interp;
    deltax_pix_interp.resize(4);
    vector< Int > deltay_pix_interp;
    deltay_pix_interp.resize(4);
    ofstream outFile("output.txt");

    // Loop over all visibility rows to process.
    for (Int inx=rbeg; inx<=rend; ++inx) {
      Int irow = rows[inx];

      //float Factor0 (1.-float(irow-rows[rbeg])/float(rows[rend]-rows[rbeg]));
      float Factor0 (1.-float(inx-rbeg)/float(rend-rbeg));
      float Factor1 (1.-Factor0);
      if(rows[rend]==rows[rbeg]){
	Factor0=1.;
	Factor1=0.;
      }
      
      cout<<Factor0<<" "<<Factor1<<endl;

      const Double*  __restrict__ uvwPtr   = vbs.uvw_p.data() + irow*3;
      // Loop over all channels in the visibility data.
      // Map the visibility channel to the grid channel.
      // Skip channel if data are not needed.
      for (Int visChan=0; visChan<nVisChan; ++visChan) {
        Int gridChan = chanMap_p[visChan];


	// !! dirty trick to select all channels
	chanMap_p[visChan]=0;

        if (gridChan >= 0  &&  gridChan < nGridChan) {
          // Determine the grid position from the UV coordinates in wavelengths.
          Double recipWvl = vbs.freq_p[visChan] / C::c;
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
            Complex* __restrict__ visPtr  = vbs.visCube_p.data()  + doff;
            const Bool*    __restrict__ flagPtr = vbs.flagCube_p.data() + doff;
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
                  for (Int sy=-fsupy; sy<=fsupy; ++sy) {
                    // Get the pointer in the grid for the first x in this y.
                    const Complex* __restrict__ gridPtr = grid.data() + goff +
                                                  (locy+sy)*nGridX + locx-supx;
                    // Get pointers to the first element to use in the 4
                    // convolution functions for this channel,pol.

		    // fast version
                    const Complex* __restrict__ cf0[1];
                    const Complex* __restrict__ cf1[1];
                    Int cfoff = (offy + sy*fsampy)*nConvX + offx - fsupx*fsampx;
 		    cf0[0] = (*cfs0.vdata)[gridChan][0][0].data() + cfoff;
 		    cf1[0] = (*cfs1.vdata)[gridChan][0][0].data() + cfoff;
                    for (Int sx=-fsupx; sx<=fsupx; ++sx) {
		      //outFile<<irow <<" "<<ipol<<" "<<posx<<" "<<posy<<" "<<(offx+ sx*fsampx-(nConvX-1.)/2.)/float(fsampx)<<" "<<(offy + sy*fsampy-(nConvX-1)/2.)/float(fsampy)
		//	     <<" "<<real(*gridPtr * *cf[0])<<" "<<imag(*gridPtr * *cf[0])<<endl;
		      visPtr[ipol] += *gridPtr * (*cf0[0]*Factor0+ *cf1[0]*Factor1);
		      cf0[0] += fsampx;
		      cf1[0] += fsampx;
                      gridPtr++;
                    }

		    // // Full version
                    // const Complex* __restrict__ cf[4];
                    // Int cfoff = (offy + sy*fsampy)*nConvX + offx - fsupx*fsampx;
                    // for (int i=0; i<4; ++i) {
                    //   cf[i] = (*cfs.vdata)[gridChan][i][ipol].data() + cfoff;
                    // }
                    // for (Int sx=-fsupx; sx<=fsupx; ++sx) {
                    //   for (Int i=0; i<nVisPol; ++i) {
                    //     visPtr[i] += *gridPtr * *cf[i];
                    //     cf[i] += fsampx;
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
    // assert(false);
  }


  // ================================
  // "Traditional" gridder

  template
  void LofarVisResampler::DataToGridImpl_p(Array<DComplex>& grid, LofarVBStore& vbs,
                                           const Vector<uInt>& rows,
                                           Int rbeg, Int rend,
					Matrix<Double>& sumwt,const Bool& dopsf,
					LofarCFStore& cfs);
  template
  void LofarVisResampler::DataToGridImpl_p(Array<Complex>& grid, LofarVBStore& vbs,
                                           const Vector<uInt>& rows,
                                           Int rbeg, Int rend,
					Matrix<Double>& sumwt,const Bool& dopsf,
					LofarCFStore& cfs);

  template <class T>
  void LofarVisResampler::DataToGridImpl_p(Array<T>& grid,  LofarVBStore& vbs,
                                           const Vector<uInt>& rows,
                                           Int rbeg, Int rend,
                                           Matrix<Double>& sumwt,
                                           const Bool& dopsf,
                                           LofarCFStore& cfs)
  {
    // grid[nx,ny,np,nf]
    // vbs.data[np,nf,nrow]
    // cfs[nmx,nmy,nf,ncx,ncy]   (mueller,freq,convsize)

    // Get size of convolution functions.
    Int nConvX = (*(cfs.vdata))[0][0][0].shape()[0];
    Int nConvY = (*(cfs.vdata))[0][0][0].shape()[1];
    // Get size of grid.
    Int nGridX    = grid.shape()[0];
    Int nGridY    = grid.shape()[1];
    Int nGridPol  = grid.shape()[2];
    Int nGridChan = grid.shape()[3];
    //cout<<"nGridPol<<nGridChan "<<nGridPol<<" "<<nGridChan<<endl;

    // Get visibility data size.
    Int nVisPol   = vbs.flagCube_p.shape()[0];
    Int nVisChan  = vbs.flagCube_p.shape()[1];
    //cout<<"nVisPol<<nVisChan "<<nVisPol<<" "<<nVisChan<<endl;
    // Get oversampling and support size.
    Int sampx = SynthesisUtils::nint (cfs.sampling[0]);
    Int sampy = SynthesisUtils::nint (cfs.sampling[1]);
    Int supx = cfs.xSupport[0];
    Int supy = cfs.ySupport[0];

    ///AlwaysAssert ((2*supx+1)*sampx == nConvX, AipsError);
    ///AlwaysAssert ((2*supy+1)*sampy == nConvY, AipsError);

    Double* __restrict__ sumWtPtr = sumwt.data();
    Complex psfValues[4];
    psfValues[0] = psfValues[1] = psfValues[2] = psfValues[3] = Complex(1,0);
    // psfValues[0] = -Complex(2,0);
    // psfValues[1] = -Complex(1,1);
    // psfValues[2] = -Complex(1,-1);
    // psfValues[3] = -Complex(0,0);


    // Loop over all visibility rows to process.
    for (Int inx=rbeg; inx<=rend; ++inx) {
      Int irow = rows[inx];
      const Double*  __restrict__ uvwPtr   = vbs.uvw_p.data() + irow*3;
      const Float*   __restrict__ imgWtPtr = vbs.imagingWeight_p.data() +
                                             irow * nVisChan;
      // Loop over all channels in the visibility data.
      // Map the visibility channel to the grid channel.
      // Skip channel if data are not needed.
      for (Int visChan=0; visChan<nVisChan; ++visChan) {
        Int gridChan = chanMap_p[visChan];


	// !! dirty trick to select all channels
	chanMap_p[visChan]=0;


        if (gridChan >= 0  &&  gridChan < nGridChan) {

          // Determine the grid position from the UV coordinates in wavelengths.
          Double recipWvl = vbs.freq_p[visChan] / C::c;
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
            const Complex* __restrict__ visPtr  = vbs.visCube_p.data()  + doff;
            const Bool*    __restrict__ flagPtr = vbs.flagCube_p.data() + doff;
            if (dopsf) {
              visPtr = psfValues;
            }
            // Handle a visibility if not flagged.
            for (Int ipol=0; ipol<nVisPol; ++ipol) {
              if (! flagPtr[ipol]) {

                // Map to grid polarization. Only use pol if needed.
                Int gridPol = polMap_p(ipol);
                if (gridPol >= 0  &&  gridPol < nGridPol) {

  		  //cout<<"ipol: "<<ipol<<endl;
                  // Get the offset in the grid data array.
                  Int goff = (gridChan*nGridPol + gridPol) * nGridX * nGridY;
                  // Loop over the scaled support.
                  for (Int sy=-fsupy; sy<=fsupy; ++sy) {
                    // Get the pointer in the grid for the first x in this y.
                    T* __restrict__ gridPtr = grid.data() + goff +
                                              (locy+sy)*nGridX + locx-supx;
  		    //cout<<"goff<<locy<<sy<<nGridX<<locx<<supx "<<goff<<" "<<locy<<" "<<sy<<" "<<nGridX<<" "<<locx<<" "<<supx<<endl;
                    // Get pointers to the first element to use in the 4
                    // convolution functions for this channel,pol.

		    // Fast version

                    const Complex* __restrict__ cf[1];
                    Int cfoff = (offy + sy*fsampy)*nConvX + offx - fsupx*fsampx;
		    cf[0] = (*cfs.vdata)[gridChan][0][0].data() + cfoff;
                    for (Int sx=-fsupx; sx<=fsupx; ++sx) {
                      // Loop over polarizations to correct for leakage.
                      Complex polSum(0,0);
		      polSum += visPtr[ipol] * *cf[0];
		      cf[0] += fsampx;
  		      polSum *= *imgWtPtr;
                      *gridPtr++ += polSum;
                    }

		    // // Full version
                    // const Complex* __restrict__ cf[4];
                    // Int cfoff = (offy + sy*fsampy)*nConvX + offx - fsupx*fsampx;
                    // for (int i=0; i<4; ++i) {
                    //   cf[i] = (*cfs.vdata)[gridChan][i][ipol].data() + cfoff;
                    // }
                    // for (Int sx=-fsupx; sx<=fsupx; ++sx) {
                    //   // Loop over polarizations to correct for leakage.
                    //   Complex polSum(0,0);
                    //   for (Int i=0; i<4; ++i) {
                    //     polSum += visPtr[i] * *cf[i];
                    //     cf[i] += fsampx;
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




  /*
  template <class T>
  void LofarVisResampler::DataToGridImpl_p(Array<T>& grid,  LofarVBStore& vbs,
                                           const Vector<uInt>& rows,
                                           Int rbeg, Int rend,
                                           Matrix<Double>& sumwt,
                                           const Bool& dopsf,
                                           LofarCFStore& cfs)
  {
    Vector<Float> sampling(2);
    Vector<Double> pos(3);
    Vector<Int> support(2),loc(3), off(3), iloc(4),tiloc(4);
    Vector<Int> scaledSampling(2), scaledSupport(2);
    Vector<Int> igrdpos(4);
    //Float sampling[2];
    //Double pos[3]
    //  Int support[2], loc[3], off[3], iloc[4], tiloc[4];
    //Int scaledSampling[2], scledSupport[2];
    //Int igrdpos[4];

    Double norm=0.0;
    Complex phasor, nvalue, wt;
    // A conv. fucntion per pol.
    // For time being only one channel.
    Vector<Int> cfShape(4,1);
    cfShape[0] = (*(cfs.vdata))[0][0][0].shape()[0];
    cfShape[1] = (*(cfs.vdata))[0][0][0].shape()[1];
    Vector<Int> convOrigin = (cfShape-1)/2;

    Int nx = grid.shape()[0];
    Int ny = grid.shape()[1];
    Int nw = 1;
    Int nGridPol = grid.shape()[2];
    Int nGridChan = grid.shape()[3];

    Int nDataPol  = vbs.flagCube_p.shape()[0];
    Int nDataChan = vbs.flagCube_p.shape()[1];

    sampling[0] = sampling[1] = cfs.sampling[0];
    support[0] = cfs.xSupport[0];
    support[1] = cfs.ySupport[0];

    T* __restrict__ gridStore = grid.data();
    const Int * __restrict__ iPosPtr = igrdpos.data();
    // const Complex* __restrict__ convFuncV[4];
    // for (int i=0; i<4; ++i) {
    //   convFuncV[i] = (*(cfs.vdata))[0][i][i].data();
    // }

    const Complex* __restrict__ convFuncV[4][4];
    for (int i=0; i<4; ++i) {
      for (int j=0; j<4; ++j) {
        const Array<Complex>& conv = (*(cfs.vdata))[0][j][i];
        if (conv.empty()) {
          convFuncV[j][i] = 0;
        } else {
          convFuncV[j][i] = conv.data();
        }
      //Matrix<Complex> im((*(cfs.vdata))[0][i][i]);
      //store2(im,"Aterm-ch"+String::toString(i)+".img");
      }
    }
    //cout<<"cfs.vdata= "<<&cfs<<endl;

    const Double *freq  = vbs.freq_p.data();

    // Cache increment values for adding to grid in gridInc.  This is
    // supplied to addTo4DArray later.
    cacheAxisIncrements(grid.shape().asVector(), gridInc_p);
    // Cache the CF related increments internally in
    // VisibilityResamplerBase for use in getFrom4DArray later.
    cacheAxisIncrements(cfShape, cfInc_p);

    const Bool * __restrict__ flagCube_ptr=vbs.flagCube_p.data();
    const Float * __restrict__ imgWts_ptr = vbs.imagingWeight_p.data();
    const Complex * __restrict__ visCube_ptr = vbs.visCube_p.data();
    Double * __restrict__ sumWt_ptr = sumwt.data();

    // {
    //   IPosition tt(4);
    //   for(tt(0)=0;tt(0)<grid.shape()(0);tt(0)++)
    // 	for(tt(1)=0;tt(1)<grid.shape()(1);tt(1)++)
    // 	  for(tt(2)=0;tt(2)<grid.shape()(2);tt(2)++)
    // 	    for(tt(3)=0;tt(3)<grid.shape()(3);tt(3)++)
    // 	      grid(tt)*=1.0;
    // }

    for(Int inx=rbeg; inx< rend; inx++){
        Int irow = rows[inx];
	for(Int ichan=0; ichan< nDataChan; ichan++){
	    Int achan=chanMap_p[ichan];

	    if((achan>=0) && (achan<nGridChan)) {

	      scaledSampling[0] = SynthesisUtils::nint(sampling[0]);
	      scaledSampling[1] = SynthesisUtils::nint(sampling[1]);
	      scaledSupport[0]  = support[0];
	      scaledSupport[1]  = support[1];

	      sgrid(pos,loc,off, phasor, irow,
		    vbs.uvw_p, dphase_p[irow], freq[ichan],
		    uvwScale_p, offset_p, sampling);

///	      cout<<"  pos= ["<<pos[0]<<", "<<pos[1]<<"]"
///                  <<", loc= ["<<loc[0]<<", "<<loc[1]<<", "<<loc[2]<<"]"
///                  <<", off= ["<<off[0]<<", "<<off[1]<<", "<<off[2]<<"] "
///                  << nx << ' '<<ny<<' '<<support[0]<<' '<<support[1]
///                  <<endl;

	      iloc[2]=max(0, min(nw-1, loc[2]));

	      //cout<< nx <<" "<<ny<<" "<<nw<<" "<<loc<<" "<< support<<" "<<onGrid(nx, ny, nw, loc, support)<<endl;
	      //assert(false);
	      if (onGrid(nx, ny, nw, loc, support)) {
///                cout << "on grid"<<endl;

		for(Int ipol=0; ipol< nDataPol; ipol++) {
		  //		  if((!flagCube(ipol,ichan,irow))){
		  if((!(*(flagCube_ptr + ipol + ichan*nDataPol + irow*nDataPol*nDataChan)))){
		    Int apol=polMap_p(ipol);
		    if ((apol>=0) && (apol<nGridPol)) {
		      igrdpos[2]=apol; igrdpos[3]=achan;

		      norm=0.0;
		      //int ConjPlane = cfMap_p[ipol];
		      //int PolnPlane = conjCFMap_p[ipol];

		      iloc[3]=ipol; //PolnPlane;

		      //cout<<"Weight= "<<Complex(*(imgWts_ptr + ichan + irow*nDataChan))<<", Vis= "<<(*(visCube_ptr+ipol+ichan*nDataPol+irow*nDataChan*nDataPol)*phasor)<<endl;


		      if(dopsf)  nvalue=Complex(*(imgWts_ptr + ichan + irow*nDataChan));
		      else	 nvalue= *(imgWts_ptr+ichan+irow*nDataChan)*
		      		   (*(visCube_ptr+ipol+ichan*nDataPol+irow*nDataChan*nDataPol)*phasor);


		      //cout<<"nvalue: "<<nvalue<<endl;

		      for(Int iy=-scaledSupport[1]; iy <= scaledSupport[1]; iy++)
			{
			  iloc[1]=(Int)(scaledSampling[1]*iy+off[1]);
			  igrdpos[1]=loc[1]+iy;
			  for(Int ix=-scaledSupport[0]; ix <= scaledSupport[0]; ix++)
			    {
			      iloc[0]=(Int)(scaledSampling[0]*ix+off[0]);
			      tiloc=iloc;
                              if (reindex(iloc,tiloc, 0, 1,
                                          convOrigin, cfShape)) {
                                //cout << "reindexed"<<iloc<<tiloc<<cfShape<<endl;

                                for (int ic=0; ic<4; ++ic) {
                                  if (convFuncV[iloc[3]][ic]) {
				    //cout<<ic<<" "<<iloc[3]<<" "<<tiloc[0]<<endl;
                                    wt = convFuncV[iloc[3]][ic][tiloc[1]*cfInc_p[1]+tiloc[0]];
///                                    cout<<"cf="<<iloc[3]<<' '<<tiloc[1]*cfInc_p[1]+tiloc[0]<<',';
				    //cout<<wt<<endl;
				    //wt=convFuncV[iloc[3]][tiloc[1]*cfInc_p[1]+tiloc[0]];
				//wt = (*(cfs.vdata))[0][iloc[3]][iloc[3]](tiloc[0],tiloc[1]);
                                ///                              wt = getFrom4DArray(convFuncV, tiloc,cfInc_p);
				    igrdpos[0]=loc[0]+ix;
				//				  grid(igrdpos) += nvalue*wt;
				    //cout<<igrdpos[0]<<endl;
				    //cout<<"ipol="<<ipol<<", iloc[1]="<<iloc[1]<<", cfInc_p[1]="<<cfInc_p[1]<<", iloc[0]="<<iloc[0]<<", wt="<<wt<<", vis="<<nvalue<<endl;
                                //assert (wt > 1e-10  &&  wt < 1);
				// The following uses raw index on the 4D grid
///                                    cout << "add " << igrdpos<< gridInc_p
///                                         << igrdpos[0] + igrdpos[1]*gridInc_p[1] + igrdpos[2]*gridInc_p[2] +igrdpos[3]*gridInc_p[3]<<endl;
                                    addTo4DArray(gridStore,iPosPtr,gridInc_p, nvalue,wt);
				//				  norm+=real(wt);
				  }
				}
                              }
			    }
			}
		      //		      sumwt(apol,achan)+=imagingWeight(ichan,irow);// *norm;
		      *(sumWt_ptr+apol+achan*nGridChan)+= *(imgWts_ptr+ichan+irow*nDataChan);
		    }
		  }
		}
	      }
	    }
        }
    }
  }
  */


  //=====================================================================================================================================
  //=====================================================================================================================================
  //=====================================================================================================================================
  //=====================================================================================================================================
  //=====================================================================================================================================
  //=====================================================================================================================================
  //=====================================================================================================================================




  //
  //-----------------------------------------------------------------------------------
  // Re-sample VisBuffer from a regular grid (griddedData) (a.k.a. de-gridding)
  //
  void LofarVisResampler::lofarGridToData(LofarVBStore& vbs,
                                          const Array<Complex>& grid,
                                          const Vector<uInt>& rows,
                                          Int rbeg, Int rend,
                                          LofarCFStore& cfs)
  {
    // grid[nx,ny,np,nf]
    // vbs.data[np,nf,nrow]
    // cfs[nmx,nmy,nf,ncx,ncy]   (mueller,freq,convsize)

    // Get size of convolution functions.
    Int nConvX = (*(cfs.vdata))[0][0][0].shape()[0];
    Int nConvY = (*(cfs.vdata))[0][0][0].shape()[1];
    // Get size of grid.
    Int nGridX    = grid.shape()[0];
    Int nGridY    = grid.shape()[1];
    Int nGridPol  = grid.shape()[2];
    Int nGridChan = grid.shape()[3];
    // Get visibility data size.
    Int nVisPol   = vbs.flagCube_p.shape()[0];
    Int nVisChan  = vbs.flagCube_p.shape()[1];
    // Get oversampling and support size.
    Int sampx = SynthesisUtils::nint (cfs.sampling[0]);
    Int sampy = SynthesisUtils::nint (cfs.sampling[1]);
    Int supx = cfs.xSupport[0];
    Int supy = cfs.ySupport[0];
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
      const Double*  __restrict__ uvwPtr   = vbs.uvw_p.data() + irow*3;
      // Loop over all channels in the visibility data.
      // Map the visibility channel to the grid channel.
      // Skip channel if data are not needed.
      for (Int visChan=0; visChan<nVisChan; ++visChan) {
        Int gridChan = chanMap_p[visChan];


	// !! dirty trick to select all channels
	chanMap_p[visChan]=0;

        if (gridChan >= 0  &&  gridChan < nGridChan) {
          // Determine the grid position from the UV coordinates in wavelengths.
          Double recipWvl = vbs.freq_p[visChan] / C::c;
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
            Complex* __restrict__ visPtr  = vbs.visCube_p.data()  + doff;
            const Bool*    __restrict__ flagPtr = vbs.flagCube_p.data() + doff;
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
                  for (Int sy=-fsupy; sy<=fsupy; ++sy) {
                    // Get the pointer in the grid for the first x in this y.
                    const Complex* __restrict__ gridPtr = grid.data() + goff +
                                                  (locy+sy)*nGridX + locx-supx;
                    // Get pointers to the first element to use in the 4
                    // convolution functions for this channel,pol.

		    // fast version
                    const Complex* __restrict__ cf[1];
                    Int cfoff = (offy + sy*fsampy)*nConvX + offx - fsupx*fsampx;
 		    cf[0] = (*cfs.vdata)[gridChan][0][0].data() + cfoff;
                    for (Int sx=-fsupx; sx<=fsupx; ++sx) {
		      //outFile<<irow <<" "<<ipol<<" "<<posx<<" "<<posy<<" "<<(offx+ sx*fsampx-(nConvX-1.)/2.)/float(fsampx)<<" "<<(offy + sy*fsampy-(nConvX-1)/2.)/float(fsampy)
		//	     <<" "<<real(*gridPtr * *cf[0])<<" "<<imag(*gridPtr * *cf[0])<<endl;
		      visPtr[ipol] += *gridPtr * *cf[0];
		      cf[0] += fsampx;
                      gridPtr++;
                    }

		    // // Full version
                    // const Complex* __restrict__ cf[4];
                    // Int cfoff = (offy + sy*fsampy)*nConvX + offx - fsupx*fsampx;
                    // for (int i=0; i<4; ++i) {
                    //   cf[i] = (*cfs.vdata)[gridChan][i][ipol].data() + cfoff;
                    // }
                    // for (Int sx=-fsupx; sx<=fsupx; ++sx) {
                    //   for (Int i=0; i<nVisPol; ++i) {
                    //     visPtr[i] += *gridPtr * *cf[i];
                    //     cf[i] += fsampx;
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
    // assert(false);
  }

  // void LofarVisResampler::lofarGridToData(LofarVBStore& vbs,
  //                                         const Array<Complex>& grid,
  //                                         const Vector<uInt>& rows,
  //                                         Int rbeg, Int rend,
  //                                         LofarCFStore& cfs)
  // {
  //   // grid[nx,ny,np,nf]
  //   // vbs.data[np,nf,nrow]
  //   // cfs[nmx,nmy,nf,ncx,ncy]   (mueller,freq,convsize)

  //   // Get size of convolution functions.
  //   Int nConvX = (*(cfs.vdata))[0][0][0].shape()[0];
  //   Int nConvY = (*(cfs.vdata))[0][0][0].shape()[1];
  //   // Get size of grid.
  //   Int nGridX    = grid.shape()[0];
  //   Int nGridY    = grid.shape()[1];
  //   Int nGridPol  = grid.shape()[2];
  //   Int nGridChan = grid.shape()[3];
  //   // Get visibility data size.
  //   Int nVisPol   = vbs.flagCube_p.shape()[0];
  //   Int nVisChan  = vbs.flagCube_p.shape()[1];
  //   // Get oversampling and support size.
  //   Int sampx = SynthesisUtils::nint (cfs.sampling[0]);
  //   Int sampy = SynthesisUtils::nint (cfs.sampling[1]);
  //   Int supx = cfs.xSupport[0];
  //   Int supy = cfs.ySupport[0];
  //   ///AlwaysAssert ((2*supx+1)*sampx == nConvX, AipsError);
  //   ///AlwaysAssert ((2*supy+1)*sampy == nConvY, AipsError);

  //   // Loop over all visibility rows to process.
  //   for (Int inx=rbeg; inx<rend; ++inx) {
  //     Int irow = rows[inx];
  //     const Double*  __restrict__ uvwPtr   = vbs.uvw_p.data() + irow*3;
  //     // Loop over all channels in the visibility data.
  //     // Map the visibility channel to the grid channel.
  //     // Skip channel if data are not needed.
  //     for (Int visChan=0; visChan<nVisChan; ++visChan) {
  //       Int gridChan = chanMap_p[visChan];
  //       if (gridChan >= 0  &&  gridChan < nGridChan) {
  //         // Determine the grid position from the UV coordinates in wavelengths.
  //         Double recipWvl = vbs.freq_p[visChan] / C::c;
  //         Double posx = uvwScale_p[0] * uvwPtr[0] * recipWvl + offset_p[0];
  //         Double posy = uvwScale_p[1] * uvwPtr[1] * recipWvl + offset_p[1];
  //         Int locx = SynthesisUtils::nint (posx);    // location in grid
  //         Int locy = SynthesisUtils::nint (posy);
  //         Double diffx = locx - posx;
  //         ///if (diffx < 0) diffx += 1;
  //         Double diffy = locy - posy;
  //         ///if (diffy < 0) diffy += 1;
  //         Int offx = SynthesisUtils::nint (diffx * sampx); // location in
  //         Int offy = SynthesisUtils::nint (diffy * sampy); // oversampling
  //         ///          cout<<"  pos= ["<<posx<<", "<<posy<<"]"
  //         ///              <<", loc= ["<<locx<<", "<<locy<<"]"
  //         ///              <<", off= ["<<offx<<", "<<offy<<"]" << endl;
  //         offx += (nConvX-1)/2;
  //         offy += (nConvY-1)/2;
  //         // Scaling with frequency is not necessary (according to Cyril).
  //         Double freqFact = 1;   // = cfFreq / vbs.freq_p[visChan];
  //         Int fsampx = SynthesisUtils::nint (sampx * freqFact);
  //         Int fsampy = SynthesisUtils::nint (sampy * freqFact);
  //         Int fsupx  = SynthesisUtils::nint (supx / freqFact);
  //         Int fsupy  = SynthesisUtils::nint (supy / freqFact);

  //         // Only use visibility point if the full support is within grid.
  //         if (locx-supx >= 0  &&  locx+supx < nGridX  &&
  //             locy-supy >= 0  &&  locy+supy < nGridY) {
  //           ///            cout << "in grid"<<endl;
  //           // Get pointer to data and flags for this channel.
  //           Int doff = (irow * nVisChan + visChan) * nVisPol;
  //           Complex* __restrict__ visPtr  = vbs.visCube_p.data()  + doff;
  //           const Bool*    __restrict__ flagPtr = vbs.flagCube_p.data() + doff;
  //           // Handle a visibility if not flagged.
  //           for (Int ipol=0; ipol<nVisPol; ++ipol) {
  //             if (! flagPtr[ipol]) {
  //               visPtr[ipol] = Complex(0,0);
  //             }
  //           }
  //           for (Int ipol=0; ipol<nVisPol; ++ipol) {
  //             if (! flagPtr[ipol]) {
  //               // Map to grid polarization. Only use pol if needed.
  //               Int gridPol = polMap_p(ipol);
  //               if (gridPol >= 0  &&  gridPol < nGridPol) {
  //                 /// Complex norm(0,0);
  //                 // Get the offset in the grid data array.
  //                 Int goff = (gridChan*nGridPol + gridPol) * nGridX * nGridY;
  //                 // Loop over the scaled support.
  //                 for (Int sy=-fsupy; sy<=fsupy; ++sy) {
  //                   // Get the pointer in the grid for the first x in this y.
  //                   const Complex* __restrict__ gridPtr = grid.data() + goff +
  //                                                 (locy+sy)*nGridX + locx-supx;
  //                   // Get pointers to the first element to use in the 4
  //                   // convolution functions for this channel,pol.
  //                   const Complex* __restrict__ cf[4];
  //                   Int cfoff = (offy + sy*fsampy)*nConvX + offx - fsupx*fsampx;
  //                   for (int i=0; i<4; ++i) {
  //                     cf[i] = (*cfs.vdata)[gridChan][i][ipol].data() + cfoff;
  //                   }
  //                   for (Int sx=-fsupx; sx<=fsupx; ++sx) {
  //                     // Loop over polarizations to correct for leakage.
  //                     for (Int i=0; i<nVisPol; ++i) {
  //                       ///                        cout<<"cf="<< cf[i]-(*cfs.vdata)[gridChan][ipol][i].data()<<',';
  //                       visPtr[i] += *gridPtr * *cf[i];
  //                       cf[i] += fsampx;
  //                     }
  //                     ///                      cout<<"  g="<<gridPtr-grid.data()<<' '<<nvalue<<endl;
  //                     gridPtr++;
  //                   }
  //                 }
  //               } // end if gridPol
  //             } // end if !flagPtr
  //           } // end for ipol
  //         } // end if ongrid
  //       } // end if gridChan
  //     } // end for visChan
  //   } // end for inx
  // }


  /*
  void LofarVisResampler::lofarGridToData(LofarVBStore& vbs,
                                          const Array<Complex>& grid,
                                          const Vector<uInt>& rows,
                                          Int rbeg, Int rend,
                                          LofarCFStore& cfs)
  {
    Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny,nw;
    Int achan, apol, PolnPlane, ConjPlane;
    Vector<Float> sampling(2);
    Vector<Int> support(2),loc(3), off(3), iloc(4),tiloc(4), scaledSampling(2), scaledSupport(2);
    Vector<Double> pos(3);

    IPosition grdpos(4);

    Complex phasor, nvalue, norm, wt;
    Vector<Int> cfShape(4,1);
    cfShape[0] = (*(cfs.vdata))[0][0][0].shape()[0];
    cfShape[1] = (*(cfs.vdata))[0][0][0].shape()[1];
    //    Vector<Int> convOrigin = (cfShape-1)/2;
    Vector<Int> convOrigin = (cfShape-1)/2;
    Double sinDPA=0.0, cosDPA=1.0;

    nx       = grid.shape()[0]; ny        = grid.shape()[1];
    nw       = cfShape[2];
    nGridPol = grid.shape()[2]; nGridChan = grid.shape()[3];

    nDataPol  = vbs.flagCube_p.shape()[0];
    nDataChan = vbs.flagCube_p.shape()[1];

    sampling[0] = sampling[1] = cfs.sampling[0];
    support(0) = cfs.xSupport[0];
    support(1) = cfs.ySupport[0];
    //
    // The following code reduces most array accesses to the simplest
    // possible to improve performance.  However this made no
    // difference in the run-time performance compared to Vector,
    // Matrix and Cube indexing.
    //
    Bool Dummy;
    const Complex *gridStore = grid.getStorage(Dummy);
    Vector<Int> igrdpos(4);
    const Int *iPosPtr = igrdpos.getStorage(Dummy);

    // Take all Mueller elements into account.
    const Complex* __restrict__ convFuncV[4][4];
    for (int i=0; i<4; ++i) {
      for (int j=0; j<4; ++j) {
        const Array<Complex>& conv = (*(cfs.vdata))[0][j][i];
        if (conv.empty()) {
          convFuncV[j][i] = 0;
        } else {
          convFuncV[j][i] = conv.data();
        }
      //Matrix<Complex> im((*(cfs.vdata))[0][i][i]);
      //store2(im,"Aterm-ch"+String::toString(i)+".img");
      }
    }


    // Vector<Double> UVWSCALE_MOI(uvwScale_p*1.4);
    // Vector<Double> UVWOFF_MOI(offset_p);
    // UVWOFF_MOI(0)=320;
    // UVWOFF_MOI(1)=320;
    // UVWOFF_MOI(2)=0;

    Double *freq=vbs.freq_p.getStorage(Dummy);

    Matrix<Float>&  imagingWeight=vbs.imagingWeight_p;
    Matrix<Double>& uvw=vbs.uvw_p;
    Cube<Complex>&  visCube=vbs.visCube_p;
    Cube<Bool>&     flagCube=vbs.flagCube_p;

    Vector<Int> gridInc, cfInc;

    //    cacheAxisIncrements(nx,ny,nGridPol, nGridChan);
    cacheAxisIncrements(grid.shape().asVector(), gridInc_p);
    cacheAxisIncrements(cfShape, cfInc_p);

    for(Int inx=rbeg; inx<rend; inx++) {
        Int irow = rows[inx];

	for (Int ichan=0; ichan < nDataChan; ichan++) {
	  achan=chanMap_p[ichan];

	  if((achan>=0) && (achan<nGridChan)) {

	    scaledSampling[0] = SynthesisUtils::nint(sampling[0]);
	    scaledSampling[1] = SynthesisUtils::nint(sampling[1]);
	    scaledSupport[0]  = support[0];
	    scaledSupport[1]  = support[1];

	    sgrid(pos,loc,off,phasor,irow,uvw,dphase_p[irow],freq[ichan],
	    uvwScale_p,offset_p,sampling);
	    //sgrid(pos,loc,off,phasor,irow,uvw,dphase_p[irow],freq[ichan],
	//	  UVWSCALE_MOI,UVWOFF_MOI,sampling);



	    iloc[2]=max(0, min(nw-1, loc[2]));
	    //cout<<"-----------------------"<<endl;
	    if (onGrid(nx, ny, nw, loc, support)) {
	      for(Int ipol=0; ipol < nDataPol; ipol++) {
		//cout<<"ipol "<<ipol<<endl;
		if(!flagCube(ipol,ichan,irow)) {
		  apol=polMap_p[ipol];

		  if((apol>=0) && (apol<nGridPol)) {
		    igrdpos[2]=apol; igrdpos[3]=achan;
                    nvalue=0.0;
                    norm  =0.0;

		    //ConjPlane = cfMap_p(ipol);
		    //PolnPlane = conjCFMap_p(ipol);

		    iloc[3]=ipol;

		    for(Int iy=-scaledSupport[1]; iy <= scaledSupport[1]; iy++)
		      {
			iloc(1)=(Int)(scaledSampling[1]*iy+off[1]);
			igrdpos[1]=loc[1]+iy;

			for(Int ix=-scaledSupport[0]; ix <= scaledSupport[0]; ix++)
			  {
			    iloc(0)=(Int)(scaledSampling[0]*ix+off[0]);
			    igrdpos[0]=loc[0]+ix;
			    tiloc=iloc;
			    if (reindex(iloc,tiloc,sinDPA, cosDPA,
			    		convOrigin, cfShape))
			      {
                                // Use polarization leakage terms if defined.
                                for (int ic=0; ic<4; ++ic) {
                                  if (convFuncV[ic][iloc[3]]) {
				    //cout<<"iloc[3]<<ic "<<iloc[3]<<" "<<ic<<endl;
                                    wt = convFuncV[iloc[3]][ic][tiloc[1]*cfInc_p[1]+tiloc[0]];
                                    //wt = conj(convFuncV[ic][iloc[3]][tiloc[1]*cfInc_p[1]+tiloc[0]]);
                                    norm+=(wt);
				    //			    nvalue+=wt*grid(grdpos);
				// The following uses raw index on the 4D grid
				//				nvalue+=wt*getFrom4DArray(gridStore,iPosPtr,gridInc);
                                    igrdpos[2] = ic;
                                    //igrdpos[2] = iloc[3];//ic;
				    Complex wt2=getFrom4DArray(gridStore,igrdpos,gridInc_p);
				    //cout<<"Resampler : ic iloc cf sumcf vis: "<<ic<<" "<<iloc[3]<<" "<<wt<<" "<<nvalue<<" "<<wt2<<endl;
                                    nvalue+=wt*wt2;
				    //cout<<"wt wt2 wt*wt2 = "<<wt<<" "<<wt2<<" "<<wt*wt2<<endl;
                                  }
                                }
			      }
			  }
		      }
		    //cout<<"nvalue<<" "<<conj(phasor)<<" "<<norm= "<<nvalue<<" "<<conj(phasor)<<" "<<norm<<endl;
		    visCube(ipol,ichan,irow)=nvalue;//(nvalue*conj(phasor))/norm;
        	    //cout<<"Vis  "<<visCube(ipol,ichan,irow)<<"  "<<ipol<<"  "<<ichan<<"  "<<irow<<endl;
		    //modelCube(ipol,ichan,irow)=(nvalue*conj(phasor))/norm;
		    //cout<<"modelCube  = "<<modelCube(ipol,ichan,irow)<<endl;
		  }
		}
	      }
	    }
	  }
	}
    }
  }
  */

  void LofarVisResampler::lofarComputeResiduals(LofarVBStore& vbs)
  {
    Int rbeg = vbs.beginRow_p, rend = vbs.endRow_p;
    IPosition vbDataShape=vbs.modelCube_p.shape();
    IPosition start(vbDataShape), last(vbDataShape);
    start=0; start(2)=rbeg;
    last(2)=rend; //last=last-1;
    //cout<<"vbDataShape "<<vbDataShape<<" ,last "<<" ,start "<<start<<" ,vbs.modelCube_p "<< (vbs.modelCube_p).shape() << endl;

    //cout<<"//////////////////// Compuute residual!!!!!!"<<"vbs.useCorrected_p"<<vbs.useCorrected_p<<endl;



    if (!vbs.useCorrected_p)
      {
    	for(uInt ichan = start(0); ichan < last(0); ichan++)
    	  for(uInt ipol = start(1); ipol < last(1); ipol++)
    	    for(uInt irow = start(2); irow < last(2); irow++)
    	      vbs.modelCube_p(ichan,ipol,irow) = vbs.modelCube_p(ichan,ipol,irow) - vbs.visCube_p(ichan,ipol,irow);

      }
    else
      {
    	for(uInt ichan = start(0); ichan < last(0); ichan++){
	  //cout<< ichan << vbs.modelCube_p(ichan,0,347) <<vbs.correctedCube_p(ichan,0,347)<< endl;
    	  for(uInt ipol = start(1); ipol < last(1); ipol++){
	    for(uInt irow = start(2); irow < last(2); irow++){
    	      //cout<<"===="<<endl;
	      //if(!(abs(vbs.modelCube_p(ichan,ipol,irow))==0.)){cout<<ipol<<" "<<ichan<<" "<<irow<<" "<<vbs.modelCube_p(ichan,ipol,irow)<<"  "<<vbs.correctedCube_p(ichan,ipol,irow)<<endl;};

	      //if(!(abs(vbs.modelCube_p(ichan,ipol,irow))==0.)){cout<<"data "<<ipol<<" "<<ichan<<" "<<irow<<" "<<vbs.modelCube_p(ichan,ipol,irow)<<" "<<vbs.correctedCube_p(ichan,ipol,irow)<<endl;};

    	      vbs.modelCube_p(ichan,ipol,irow) = vbs.modelCube_p(ichan,ipol,irow) - vbs.correctedCube_p(ichan,ipol,irow);
    	      //cout<<vbs.modelCube_p(ichan,ipol,irow)<<"  "<<vbs.modelCube_p(ichan,ipol,irow)<<"  "<<vbs.correctedCube_p(ichan,ipol,irow)<<endl;
    	    }
	  }
	}
      }

    // if (!vbs.useCorrected_p)
    //   {
    // 	for(uInt ichan = start(0); ichan < last(0); ichan++)
    // 	  for(uInt ipol = start(1); ipol < last(1); ipol++)
    // 	    for(uInt irow = start(2); irow < last(2); irow++)
    // 	      vbs.modelCube_p(ichan,ipol,irow) = vbs.modelCube_p(ichan,ipol,irow) - vbs.visCube_p(ichan,ipol,irow);

    //   }
    // else
    //   {
    // 	for(uInt ichan = start(0); ichan < last(0); ichan++)
    // 	  for(uInt ipol = start(1); ipol < last(1); ipol++)
    // 	    for(uInt irow = start(2); irow < last(2); irow++){
    // 	      //cout<<"===="<<endl;
    // 	      //cout<<vbs.correctedCube_p(ichan,ipol,irow)<<"  "<< vbs.correctedCube_p(ichan,ipol,irow)<<"  "<< vbs.modelCube_p(ichan,ipol,irow)<<endl;
    // 	      vbs.correctedCube_p(ichan,ipol,irow) = vbs.correctedCube_p(ichan,ipol,irow) - vbs.modelCube_p(ichan,ipol,irow);
    // 	      //cout<<vbs.correctedCube_p(ichan,ipol,irow)<<"  "<< vbs.correctedCube_p(ichan,ipol,irow)<<"  "<< vbs.modelCube_p(ichan,ipol,irow)<<endl;
    // 	    };

    //   }


  }

  void LofarVisResampler::sgrid(Vector<Double>& pos, Vector<Int>& loc,
			     Vector<Int>& off, Complex& phasor,
			     const Int& irow, const Matrix<Double>& uvw,
			     const Double&, const Double& freq,
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
        ///        cout<<scale[idim]<<' '<<uvw_l[idim]<<' '<<offset[idim]<<' '<<pos[idim]<<endl;
	loc[idim]=SynthesisUtils::nint(pos[idim]);
	//off[idim]=SynthesisUtils::nint((loc[idim]-pos[idim])*sampling[idim]); // Cyr: I've added "+1" next line, and it solves a difficult problem, i don't know why
	////off[idim]=SynthesisUtils::nint((loc[idim]-pos[idim])*sampling[idim]+1);
	off[idim]=SynthesisUtils::nint((loc[idim]-pos[idim])*sampling[idim]);
      }

    //if (dphase != 0.0)
    //  {
//	phase=-2.0*C::pi*dphase*freq/C::c;
//	phasor=Complex(cos(phase), sin(phase));
    //  }
    //else
      phasor=Complex(1.0);
  }

} // end namespace
