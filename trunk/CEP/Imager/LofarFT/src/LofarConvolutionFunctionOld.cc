//# LofarConvolutionFunctionOld.cc: Compute the LOFAR convolution function
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
#include <LofarFT/LofarConvolutionFunctionOld.h>
#include <LofarFT/LofarConvolutionFunction.h>   // needed for store function
#include <Common/LofarLogger.h>
#include <Common/OpenMP.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayMath.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaParse.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSObservation.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSPolarization.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <measures/Measures/MeasTable.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <casa/OS/PrecTimer.h>
#include <casa/sstream.h>
#include <iomanip>

namespace LOFAR
{

  LofarConvolutionFunctionOld::LofarConvolutionFunctionOld
  (const IPosition& shape,
   const DirectionCoordinate& coordinates,
   const MeasurementSet& ms,
   uInt nW, double Wmax,
   uInt oversample,
   const String& beamElementPath,
   Int verbose,
   Int maxsupport,
   const String& imgName)
    : m_shape(shape),
      m_coordinates(coordinates),
      m_aTerm(ms, beamElementPath),
      m_maxW(Wmax), //maximum W set by ft machine to flag the w>wmax
      m_nWPlanes(nW),
      m_oversampling(oversample),
      itsVerbose (verbose),
      itsMaxSupport(maxsupport),
      itsImgName(imgName),
      itsTimeW    (0),
      itsTimeWpar (0),
      itsTimeWfft (0),
      itsTimeWcnt (0),
      itsTimeA    (0),
      itsTimeApar (0),
      itsTimeAfft (0),
      itsTimeAcnt (0),
      itsTimeCFpar(0),
      itsTimeCFfft(0),
      itsTimeCFcnt(0)
      //Not sure how useful that is
  {
    if (itsVerbose > 0) {
      cout<<"LofarConvolutionFunctionOld:shape  "<<shape<<endl;
    }
    itsFFTMachines.resize (OpenMP::maxThreads());

    //    m_maxCFSupport=0; //need this parameter to stack all the CF for average PB estimate

    m_wScale = WScale(m_maxW, m_nWPlanes);
    MEpoch start = observationStartTime(ms, 0);

    m_refFrequency = observationReferenceFreq(ms, 0);

    if (m_oversampling%2 == 0) {
      // Make OverSampling an odd number
      m_oversampling++;
    }

    list_freq   = Vector<Double>(1, m_refFrequency);
    m_nChannel  = list_freq.size();
    ROMSAntennaColumns antenna(ms.antenna());
    m_nStations = antenna.nrow();

    m_pixelSizeSpheroidal = makeSpheroidCut();
    //Double PixelSize=abs(m_coordinates.increment()(0));
    //Double ImageDiameter=PixelSize * m_shape(0);
    //Double W_Pixel_Ang_Size=min(Pixel_Size_Spheroidal,estimateWResolution(m_shape, m_coordinates, m_maxW));
    //m_maxCFSupport= ImageDiameter / W_Pixel_Ang_Size;
    //Matrix<Complex> Stack_pb_cf0(IPosition(2,m_maxCFSupport,m_maxCFSupport),0.);
    Matrix<Complex> Stack_pb_cf0(IPosition(2,m_shape(0),m_shape(0)),0.);
    Matrix<float> Stack_pb_cf1(IPosition(2,m_shape(0),m_shape(0)),0.);

    //Stack_pb_cf0(256,300)=1.;
    //Matrix<Complex> Avg_PB_padded00(give_normalized_fft(Stack_pb_cf0,false));
    //store(Avg_PB_padded00,"Avg_PB_padded00.img");

    // Precalculate the Wtwerm fft for all w-planes.
    store_all_W_images();
  }

  //      ~LofarConvolutionFunctionOld ()
  //      {
  //      }

  // Precalculate all W-terms in the fourier domain
  void LofarConvolutionFunctionOld::store_all_W_images()
  {
    PrecTimer wTimer;
    wTimer.start();
    Double pixelSize = abs(m_coordinates.increment()[0]);
    Double imageDiameter = pixelSize * m_shape(0);
    // Size the vector, but give each element its own default matrix,
    // so the vector can be safely filled in parallel.
    m_WplanesStore.reserve (m_nWPlanes);
    for (uInt i=0; i<m_nWPlanes; ++i) {
      m_WplanesStore.push_back (Matrix<Complex>());
    }
#pragma omp parallel
    {
      // Thread private variables.
      PrecTimer timerFFT;
      PrecTimer timerPar;
#pragma omp for schedule(dynamic)
      for (uInt i=0; i<m_nWPlanes; ++i) {
        timerPar.start();
        Double w = m_wScale.center(i);
        Double wPixelAngSize = min(m_pixelSizeSpheroidal,
                                   estimateWResolution(m_shape,
                                                       pixelSize, w));
        Int nPixelsConv = imageDiameter / wPixelAngSize;
        if (itsVerbose > 0) {
          cout<<"Number of pixel in the "<<i<<"-wplane: "<<nPixelsConv
              <<"  (w="<<w<<")"<<endl;
        }
        if (nPixelsConv > itsMaxSupport) {
          nPixelsConv = itsMaxSupport;
        }
        // Make odd and optimal.
        nPixelsConv = FFTCMatrix::optimalOddFFTSize (nPixelsConv);
        wPixelAngSize = imageDiameter / nPixelsConv;
        IPosition shape(2, nPixelsConv, nPixelsConv);
        //Careful with the sign of increment!!!! To check!!!!!!!
        Vector<Double> increment(2, wPixelAngSize);
        double wavelength(C::c / list_freq[0]);
        Matrix<Complex> wTerm = m_wTerm.evaluate(shape, increment,
                                                 w/wavelength);
        normalized_fft(timerFFT, wTerm);
        m_WplanesStore[i].reference (wTerm);
        timerPar.stop();
      }
      // Update the timing info.
      double ftime = timerFFT.getReal();
#pragma omp atomic
      itsTimeWfft += ftime;
      unsigned long long cnt = timerFFT.getCount();
#pragma omp atomic
      itsTimeWcnt += cnt;
      double ptime = timerPar.getReal();
#pragma omp atomic
      itsTimeWpar += ptime;
    } // end omp parallel
    wTimer.stop();
    itsTimeW = wTimer.getReal();
  }


  // Compute the fft of the beam at the minimal resolution for all antennas
  // if not done yet.
  // Put it in a map object with a (double time) key.
  void LofarConvolutionFunctionOld::computeAterm (Double time)
  {
    if (m_AtermStore.find(time) != m_AtermStore.end()) {
      // Already done.
      return;
    }
    PrecTimer aTimer;
    aTimer.start();
    Double pixelSize = abs(m_coordinates.increment()[0]);
    Double imageDiameter = pixelSize * m_shape(0);
    // Try to avoid making copies when inserting elements in vector or map.
    // Therefore first create the elements and resize them.
    m_AtermStore[time] = vector< vector< Cube<Complex> > >();
    vector< vector< Cube<Complex> > >& aTermList = m_AtermStore[time];
    // Calculate the A-term and fill the vector for all stations.
    aTermList.resize (m_nStations);
    ///#pragma omp parallel
    {
      // Thread private variables.
      PrecTimer timerFFT;
      PrecTimer timerPar;
      ///#pragma omp for
      for (uInt i=0; i<m_nStations; ++i) {
        timerPar.start();
	DirectionCoordinate coordinate = m_coordinates;
        Double aPixelAngSize = min(m_pixelSizeSpheroidal,
                                   estimateAResolution(m_shape, m_coordinates));
        Int nPixelsConv = imageDiameter / aPixelAngSize;
        if (nPixelsConv > itsMaxSupport) {
          nPixelsConv = itsMaxSupport;
        }
        // Make odd and optimal.
        nPixelsConv = FFTCMatrix::optimalOddFFTSize (nPixelsConv);
        aPixelAngSize = imageDiameter / nPixelsConv;
        if (itsVerbose > 0) {
          cout.precision(20);
          cout<<"Number of pixel in the Aplane of "<<i<<": "<<nPixelsConv
              <<", time="<<fixed<<time<<endl;
        }
        IPosition shape(2, nPixelsConv, nPixelsConv);
        Vector<Double> increment_old(coordinate.increment());
        Vector<Double> increment(2);
        increment[0] = aPixelAngSize*sign(increment_old[0]);
        increment[1] = aPixelAngSize*sign(increment_old[1]);
        coordinate.setIncrement(increment);
        Vector<Double> refpix(2, 0.5*(nPixelsConv-1));
        coordinate.setReferencePixel(refpix);

        //======================================
        // Disable the beam
        //======================================
        //Cube<Complex> aterm_cube(IPosition(3,nPixels_Conv,nPixels_Conv,4),1.);
        //for (uInt iiii=0;iiii<nPixels_Conv;++iiii) {
        //  for (uInt iiiii=0;iiiii<nPixels_Conv;++iiiii) {
        //    aterm_cube(iiii,iiiii,1)=0.;
        //    aterm_cube(iiii,iiiii,2)=0.;
        //  }
        //}
        //vector< Cube<Complex> > aTermA;
        //aTermA.push_back(aterm_cube);
        //======================================
        // Enable the beam
        //======================================
        MEpoch binEpoch;
        binEpoch.set(Quantity(time, "s"));
        vector< Cube<Complex> > aTermA = m_aTerm.evaluate(shape,
                                                          coordinate,
                                                          i, binEpoch,
                                                          list_freq, true);
        // Compute the fft on the beam
        for (uInt ch=0; ch<m_nChannel; ++ch) {
          for (uInt pol=0; pol<4; ++pol) {
            Matrix<Complex> plane (aTermA[ch].xyPlane(pol));
            AlwaysAssert (plane.contiguousStorage(), AipsError);
            normalized_fft (timerFFT, plane);
          }
        }
        // Note that push_back uses the copy constructor, so for the Cubes
        // in the vector the copy constructor is called too (which is cheap).
        aTermList[i] = aTermA;
        timerPar.stop();
      } // end omp for
      // Update the timing info.
      double ftime = timerFFT.getReal();
      ///#pragma omp atomic
      itsTimeAfft += ftime;
      unsigned long long cnt = timerFFT.getCount();
      ///#pragma omp atomic
      itsTimeAcnt += cnt;
      double ptime = timerPar.getReal();
      ///#pragma omp atomic
      itsTimeApar += ptime;
    } // end omp parallel
    aTimer.stop();
    itsTimeA = aTimer.getReal();
  }

  //================================================
  // Compute the convolution function for all channel, for the polarisations specified in the Mueller_mask matrix
  // Also specify weither to compute the Mueller matrix for the forward or the backward step. A dirty way to calculate
  // the average beam has been implemented, by specifying the beam correcping to the given baseline and timeslot.
  // RETURNS in a LofarCFStore: result[channel][Mueller row][Mueller column]

  LofarCFStore LofarConvolutionFunctionOld::makeConvolutionFunction
  (uInt stationA, uInt stationB, Double time, Double w,
   const Matrix<bool>& Mask_Mueller, bool degridding_step,
   double Append_average_PB_CF, Matrix<Complex>& Stack_PB_CF,
   double& sum_weight_square)
  {
    // Initialize timers.
    PrecTimer timerFFT;
    PrecTimer timerPar;
    timerPar.start();

    // Stack_PB_CF should be called Sum_PB_CF (it is a sum, no stack).
    CountedPtr<CFTypeVec> res (new vector< vector< vector < Matrix<Complex> > > >());
    CFTypeVec& result = *res;
    vector< vector< vector < Matrix<Complex> > > > result_non_padded;

    // Stack the convolution function if averagepb.img don't exist
    Matrix<Complex> Stack_PB_CF_fft(IPosition(2,m_shape(0),m_shape(0)),0.);
    Bool Stack = (Append_average_PB_CF != 0.);

    // If the beam is not in memory, compute it
        
    map<Double, vector< vector< Cube<Complex> > > >::const_iterator aiter =
      m_AtermStore.find(time);
    AlwaysAssert (aiter!=m_AtermStore.end(), AipsError);
    const vector< vector< Cube<Complex> > >& aterm = aiter->second;
    ///        if(m_AtermStore.find(time)==m_AtermStore.end()){computeAterm(time);}

    // Load the Wterm
    uInt w_index = m_wScale.plane(w);
    Matrix<Complex> wTerm;
    wTerm = m_WplanesStore[w_index];
    Int Npix_out = 0;
    Int Npix_out2 = 0;

    // Matrix<Complex> Term_test(IPosition(2,101,101),1.);
    // normalized_fft(Term_test);
    // store (Term_test,"Term_test.img");
    // normalized_fft(Term_test,false);
    // store (Term_test,"Term_test_0.img");
    // normalized_fft(Term_test);
    // store (Term_test,"Term_test_1.img");
    // assert(false);

    if (w > 0.) {
      wTerm.reference (conj(wTerm));
    }

    for (uInt ch=0; ch<m_nChannel; ++ch) {
      // Load the Aterm
      const Cube<Complex>& aTermA(aterm[stationA][ch]);
      const Cube<Complex>& aTermB(aterm[stationB][ch]);
      // Determine maximum support of A, W, and Spheroidal function for zero padding
      Npix_out = std::max(std::max(aTermA.shape()[0], aTermB.shape()[0]),
                          std::max(wTerm.shape()[0], Spheroid_cut.shape()[0]));
      if (itsVerbose > 0) {
        cout<<"Number of pixel in the final conv function for baseline ["<< stationA<<", "<<stationB<<"] = "<<Npix_out
            <<" "<<aTermA.shape()[0]<<" "<<aTermB.shape()[0]<<" "<<wTerm.shape()[0]<<endl;
      }

      // Zero pad to make the image planes of the A1, A2, and W term have the same resolution in the image plane
      Matrix<Complex> Spheroid_cut_paddedf(zero_padding(Spheroid_cut,Npix_out));
      Matrix<Complex> wTerm_paddedf(zero_padding(wTerm, Npix_out));
      Cube<Complex> aTermA_padded(zero_padding(aTermA, Npix_out));
      Cube<Complex> aTermB_padded(zero_padding(aTermB, Npix_out));

      // FFT (backward) the A and W terms
      normalized_fft (timerFFT, wTerm_paddedf, false);
      normalized_fft (timerFFT, Spheroid_cut_paddedf, false);
      if (itsVerbose > 0) {
        cout << "fft shapes " << wTerm_paddedf.shape() << ' ' << Spheroid_cut_paddedf.shape()
             << ' ' << aTermA_padded.shape() << ' ' << aTermB_padded.shape() << endl;
      }
      for (uInt i=0; i<4; ++i) {
        //Matrix<Complex> planeAf(aTermA_padded.xyPlane(i));
        //Matrix<Complex> planeBf(aTermB_padded.xyPlane(i));
        // AlwaysAssert(planeAf.contiguousStorage(), AipsError);
        // Make a matrix referencing the data in the cube's plane.
        Matrix<Complex> planeAf(aTermA_padded.xyPlane(i));
        Matrix<Complex> planeBf(aTermB_padded.xyPlane(i));
        AlwaysAssert(planeAf.contiguousStorage(), AipsError);
        normalized_fft (timerFFT, planeAf, false);
        normalized_fft (timerFFT, planeBf, false);
      }

      // Create the vectors of Matrices giving the convolution functions
      // for each Mueller element.
      vector< vector < Matrix<Complex> > > Kron_Product;
      Kron_Product.reserve(4);

      // Something I still don't completely understand: for the average PB calculation.
      // The convolution functions padded with a higher value than the minimum one give a
      // better result in the end. If you try Npix_out2=Npix_out, then the average PB shows
      // structure like aliasing, producing high values in the devided disrty map... This
      // is likely to be due to the way fft works?...
      // FIX: I now do the average of the PB by stacking the CF, FFT the result and square 
      // it in the end. This is not the way to do in principle but the result is almost the 
      // same. It should pose no problem I think.
      Matrix<Complex> Spheroid_cut_padded2f;
      Cube<Complex> aTermA_padded2;
      Cube<Complex> aTermB_padded2;

      // Keep the non-padded convolution functions for average PB calculation.
      vector< vector < Matrix<Complex> > > Kron_Product_non_padded;
      Kron_Product_non_padded.reserve(4);
	    
      if (Stack) {
        Npix_out2 = Npix_out;
        Spheroid_cut_padded2f = zero_padding(Spheroid_cut, Npix_out2);
        aTermA_padded2 = zero_padding(aTermA, Npix_out2);
        aTermB_padded2 = zero_padding(aTermB, Npix_out2);
        normalized_fft (timerFFT, Spheroid_cut_padded2f, false);
        for (uInt i=0; i<4; ++i) {
          Matrix<Complex> planeA2f(aTermA_padded2.xyPlane(i));
          Matrix<Complex> planeB2f(aTermB_padded2.xyPlane(i));
          normalized_fft (timerFFT, planeA2f, false);
          normalized_fft (timerFFT, planeB2f, false);
        }
      }

      // Compute the Mueller matrix considering the Mueller Mask
      uInt ind0;
      uInt ind1;
      uInt ii = 0;
      IPosition cfShape;
      Bool allElem = True;
      for (uInt row0=0; row0<=1; ++row0) {
        for (uInt col0=0; col0<=1; ++col0) {
          vector < Matrix<Complex> > Row(4);
          vector < Matrix<Complex> > Row_non_padded(4);
          uInt jj = 0;
          for (uInt row1=0; row1<=1; ++row1) {
            for (uInt col1=0; col1<=1; ++col1) {
              // This Mueller ordering is for polarisation given as XX,XY,YX YY
              ind0 = row0 + 2*row1;
              ind1 = col0 + 2*col1;
              // Compute the convolution function for the given Mueller element
              if (Mask_Mueller(ii,jj)) {
                // Padded version for oversampling the convolution function
                Matrix<Complex> plane_product (aTermB_padded.xyPlane(ind0) *
                                               aTermA_padded.xyPlane(ind1));
                plane_product *= wTerm_paddedf;
                plane_product *= Spheroid_cut_paddedf;
                Matrix<Complex> plane_product_paddedf
                  (zero_padding(plane_product,
                                plane_product.shape()[0] * m_oversampling));
                normalized_fft (timerFFT, plane_product_paddedf);

                plane_product_paddedf *= static_cast<Float>(m_oversampling *
                                                            m_oversampling);
		if (itsVerbose>3 && row0==0 && col0==0 && row1==0 && col1==0) {
		  store (plane_product_paddedf, "awfft"+String::toString(stationA)+'-'+String::toString(stationB));
		}

                // Maybe to do:
                // Find circle (from outside to inside) until value > peak*1e-3.
                // Cut out that box to use as the convolution function.
                // See nPBWProjectFT.cc (findSupport).

                Row[jj].reference (plane_product_paddedf);
                cfShape = plane_product_paddedf.shape();
                // Non padded version for PB calculation (no W-term)
                if (Stack) {
                  Matrix<Complex> plane_productf(aTermB_padded2.xyPlane(ind0)*
                                                 aTermA_padded2.xyPlane(ind1));
                  plane_productf *= Spheroid_cut_padded2f;
                  normalized_fft (timerFFT, plane_productf);
                  Row_non_padded[jj].reference (plane_productf);
                }
              } else {
                allElem = False;
              }
              ++jj;
            }
          }
          ++ii;
          Kron_Product.push_back(Row);
          if (Stack) {
            // Keep non-padded for primary beam calculation.
            Kron_Product_non_padded.push_back(Row_non_padded);
          }
        }
      }

      // When degridding, transpose and use conjugate.
      if (degridding_step) {
        for (uInt i=0; i<4; ++i) {
          for (uInt j=i; j<4; ++j) {
            AlwaysAssert (Mask_Mueller(i,j) == Mask_Mueller(j,i), AipsError);
            if (Mask_Mueller(i,j)) {
              if (i!=j) {
                Matrix<Complex> conj_product(conj(Kron_Product[i][j]));
                Kron_Product[i][j].reference (conj(Kron_Product[j][i]));
                Kron_Product[j][i].reference (conj_product);
              } else {
                Kron_Product[i][j].reference (conj(Kron_Product[i][j]));
              }
            }
          }
        }
      }

      // Put similarly shaped matrix with zeroes for missing Mueller elements.
      if (!allElem) {
        Matrix<Complex> zeroCF(cfShape);
        for (uInt i=0; i<4; ++i) {
          for (uInt j=0; j<4; ++j) {
            if (! Mask_Mueller(i,j)) {
              Kron_Product[i][j].reference (zeroCF);
            }
          }
        }
      }
      // Add the conv.func. for this channel to the result.
      result.push_back(Kron_Product);
      if (Stack) {
        result_non_padded.push_back(Kron_Product_non_padded);
      }
    }
          
    // Stacks the weighted quadratic sum of the convolution function of
    // average PB estimate (!!!!! done for channel 0 only!!!)
    if (Stack) {
      //	  cout<<"...Stack CF for PB estimate"<<endl;
      double weight_square = 4. * Append_average_PB_CF * Append_average_PB_CF;
      double weight_sqsq = weight_square * weight_square;
      for (uInt i=0; i<4; ++i) {
        //if((i==2)||(i==1)) break;
        for (uInt j=0; j<4; ++j) {
          // Only use diagonal terms for average primary beam.
          if (i==j  &&  Mask_Mueller(i,j)) {
            //Stack_PB_CF=0.;
            double istart = 0.5 * (m_shape[0] - Npix_out2);
            if (istart-floor(istart) != 0.) {
              istart += 0.5; //If number of pixel odd then 0th order at the center, shifted by one otherwise
            }
            for (Int jj=0; jj<Npix_out2; ++jj) {
              for (Int ii=0; ii<Npix_out2; ++ii) {
                Complex gain = result_non_padded[0][i][j](ii,jj);
                Stack_PB_CF(istart+ii,istart+jj) += gain*weight_sqsq;
              }
            }
            sum_weight_square += weight_sqsq;
          }
        }
      }
    }
	
    // Put the resulting vec(vec(vec))) in a LofarCFStore object
    CoordinateSystem csys;
    Vector<Float> samp(2, m_oversampling);
    Vector<Int> xsup(1, Npix_out/2);
    Vector<Int> ysup(1, Npix_out/2);
    Int maxXSup(Npix_out);///2);
    Int maxYSup(Npix_out);///2);
    Quantity PA(0., "deg");
    Int mosPointing(0);

    // Update the timing info.
    timerPar.stop();
    double ftime = timerFFT.getReal();
#pragma omp atomic
    itsTimeCFfft += ftime;
    unsigned long long cnt = timerFFT.getCount();
#pragma omp atomic
    itsTimeCFcnt += cnt;
    double ptime = timerPar.getReal();
#pragma omp atomic
    itsTimeCFpar += ptime;

    return LofarCFStore (res, csys, samp,  xsup, ysup, maxXSup, maxYSup,
                         PA, mosPointing, Mask_Mueller);
  }

  //================================================
      
  // Returns the average Primary Beam from the disk
  Matrix<Float> LofarConvolutionFunctionOld::Give_avg_pb()
  {
    // Only read if not available.
    if (Im_Stack_PB_CF0.empty()) {
      if (itsVerbose > 0) {
        cout<<"==============Give_avg_pb()"<<endl;
      }
      String PBFile_name(itsImgName + ".avgpb");
      File PBFile(PBFile_name);
      if (! PBFile.exists()) {
        throw SynthesisError (PBFile_name + " not found");
      }
      if (itsVerbose > 0) {
        cout<<"..... loading Primary Beam image from disk ....."<<endl;
      }
      PagedImage<Float> tmp(PBFile_name);
      IPosition shape(tmp.shape());
      AlwaysAssert (shape[0]==m_shape[0] && shape[1]==m_shape[1], AipsError);
      tmp.get (Im_Stack_PB_CF0, True);   // remove degenerate axes.
    }
    return Im_Stack_PB_CF0;
  }

  // Compute the average Primary Beam from the Stack of convolution functions
  Matrix<Float> LofarConvolutionFunctionOld::Compute_avg_pb
  (Matrix<Complex>& Sum_Stack_PB_CF, double sum_weight_square)
  {
    // Only calculate if not done yet.
    if (Im_Stack_PB_CF0.empty()) {
      if (itsVerbose > 0) {
        cout<<"..... Compute average PB"<<endl;
      }
      Sum_Stack_PB_CF /= float(sum_weight_square);
      //store(Stack_PB_CF,"Stack_PB_CF.img");

      normalized_fft(Sum_Stack_PB_CF, false);
      //store(Im_Stack_PB_CF00,"Im_Stack_PB_CF00.img");
      Im_Stack_PB_CF0.resize (IPosition(2, m_shape[0], m_shape[1]));
	
      float threshold = 1.e-6;
      for (Int jj=0; jj<m_shape[1]; ++jj) {
        for (Int ii=0; ii<m_shape[0]; ++ii) {
          Float absVal = abs(Sum_Stack_PB_CF(ii,jj));
          Im_Stack_PB_CF0(ii,jj) = std::max (absVal*absVal, threshold);
        }
      }
      // Make it persistent.
      store(Im_Stack_PB_CF0, itsImgName + ".avgpb");
    }
    return Im_Stack_PB_CF0;
  }

  //================================================
  // Does Zeros padding of a Cube
  Cube<Complex> LofarConvolutionFunctionOld::zero_padding
  (const Cube<Complex>& Image, int Npixel_Out)
  {
    if (Image.shape()[0] == Npixel_Out) {
      return Image.copy();
    }
    if ((Npixel_Out%2) != 1) {
      Npixel_Out++;
    }
    Cube<Complex> Image_Enlarged(Npixel_Out,Npixel_Out,Image.shape()[2]);
    uInt Dii = Image.shape()(0)/2;
    uInt Start_image_enlarged=Npixel_Out/2-Dii; //Is an even number, Assume square image
    if ((Start_image_enlarged-floor(Start_image_enlarged))!=0.) {
      Start_image_enlarged += 0.5; //If number of pixel odd then 0th order at the center, shifted by one otherwise
    }
    /* cout<<Start_image_enlarged<<"  "<<floor(Start_image_enlarged)<<endl; */
    /* if((Start_image_enlarged-floor(Start_image_enlarged))!=0.){ */
    /*   cout<<"Not even!!!"<<endl; */
    /*   Start_image_enlarged+=0.5;} */

    //double ratio(double(Npixel_Out)*double(Npixel_Out)/(Image.shape()(0)*Image.shape()(0)));
    //if(!toFrequency){ratio=1./ratio;}
    double ratio=1.;

    for (Int pol=0; pol<Image.shape()[2]; ++pol) {
      //cout<<"pol: "<<pol<<endl;
      for (Int jj=0; jj<Image.shape()[1]; ++jj) {
        for (Int ii=0; ii<Image.shape()[0]; ++ii) {
          Image_Enlarged(Start_image_enlarged+ii,
                         Start_image_enlarged+jj,pol) = ratio*Image(ii,jj,pol);
        }
      }
    }
    return Image_Enlarged;
  }

  //================================================
  // Zeros padding of a Matrix

  Matrix<Complex> LofarConvolutionFunctionOld::zero_padding
  (const Matrix<Complex>& Image, int Npixel_Out)
  {
    if (Image.shape()[0] == Npixel_Out) {
      return Image.copy();
    }
    if (Npixel_Out%2 != 1) {
      Npixel_Out++;
    }
    IPosition shape_im_out(2, Npixel_Out, Npixel_Out);
    Matrix<Complex> Image_Enlarged(shape_im_out, 0.);

    double ratio=1.;

    //if(!toFrequency){ratio=double(Npixel_Out)*double(Npixel_Out)/(Image.shape()(0)*Image.shape()(0));}

    uInt Dii = Image.shape()[0]/2;
    uInt Start_image_enlarged = shape_im_out[0]/2-Dii;
    //Is an even number, Assume square image
    //If number of pixel odd then 0th order at the center, shifted by one otherwise
    if ((Start_image_enlarged-floor(Start_image_enlarged))!=0.) {
      Start_image_enlarged += 0.5;
    }

    /* cout<<Start_image_enlarged<<"  "<<floor(Start_image_enlarged)<<endl; */
    /* if((Start_image_enlarged-floor(Start_image_enlarged))!=0.){ */
    /*   cout<<"Not even!!!"<<endl; */
    /*   Start_image_enlarged+=0.5;} */
    for (Int jj=0; jj<Image.shape()[1]; ++jj) {
      for (Int ii=0; ii<Image.shape()[0]; ++ii) {
        Image_Enlarged(Start_image_enlarged+ii,Start_image_enlarged+jj) = 
          ratio*Image(ii,jj);
      }
    }
    return Image_Enlarged;
  }

  //================================================
  void LofarConvolutionFunctionOld::normalized_fft
  (Matrix<Complex> &im, bool toFreq)
  {
    AlwaysAssert (im.ncolumn() == im.nrow()  &&  im.size() > 0  &&
                  im.contiguousStorage(), AipsError);
    int tnr = OpenMP::threadNum();
    if (toFreq) {
      itsFFTMachines[tnr].normalized_forward (im.nrow(), im.data());
    } else {
      itsFFTMachines[tnr].normalized_backward (im.nrow(), im.data());
    }
  }

  void LofarConvolutionFunctionOld::normalized_fft
  (PrecTimer& timer, Matrix<Complex> &im, bool toFreq)
  {
    timer.start();
    normalized_fft (im, toFreq);
    timer.stop();
  }

  //=================================================
  MEpoch LofarConvolutionFunctionOld::observationStartTime
  (const MeasurementSet &ms, uInt idObservation) const
  {
    // Get phase center as RA and DEC (J2000).
    ROMSObservationColumns observation(ms.observation());
    AlwaysAssert(observation.nrow() > idObservation, SynthesisError);
    AlwaysAssert(!observation.flagRow()(idObservation), SynthesisError);

    return observation.timeRangeMeas()(0)(IPosition(1, 0));
  }

  //=================================================
  Double LofarConvolutionFunctionOld::observationReferenceFreq
  (const MeasurementSet &ms, uInt idDataDescription)
  {
    // Read polarization id and spectral window id.
    ROMSDataDescColumns desc(ms.dataDescription());
    AlwaysAssert(desc.nrow() > idDataDescription, SynthesisError);
    AlwaysAssert(!desc.flagRow()(idDataDescription), SynthesisError);

    const uInt idWindow = desc.spectralWindowId()(idDataDescription);

    /*        logIO() << LogOrigin("LofarATerm", "initReferenceFreq") << LogIO::NORMAL
              << "spectral window: " << desc.spectralWindowId()(idDataDescription) << LogIO::POST;*/
    //            << "spectral window: " << desc.spectralWindowId() << LogIO::POST;
    // Get spectral information.
    ROMSSpWindowColumns window(ms.spectralWindow());
    AlwaysAssert(window.nrow() > idWindow, SynthesisError);
    AlwaysAssert(!window.flagRow()(idWindow), SynthesisError);

    return window.refFrequency()(idWindow);
  }

  //=================================================
  // Estime spheroidal convolution function from the support of the fft of the spheroidal in the image plane

  Double LofarConvolutionFunctionOld::makeSpheroidCut()
  {
    // Only calculate if not done yet.
    if (! Spheroid_cut_im.empty()) {
      return m_pixelSizeSpheroidal;
    }
    Matrix<Complex> spheroidal(m_shape[0], m_shape[1], 1.);
    taper(spheroidal);
    if (itsVerbose > 0) {
      store(spheroidal, itsImgName + ".spheroidal");
    }
    normalized_fft(spheroidal);
    Double Support_Speroidal = findSupport(spheroidal, 0.0001);
    if (itsVerbose > 0) {
      store(spheroidal, itsImgName + ".spheroidal_fft");
    }

    Double res_ini = abs(m_coordinates.increment()(0));
    Double diam_image = res_ini*m_shape[0];
    Double Pixel_Size_Spheroidal = diam_image/Support_Speroidal;
    uInt Npix = floor(diam_image/Pixel_Size_Spheroidal);
    if (Npix%2 != 1) {
    // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
      ++Npix;
      Pixel_Size_Spheroidal = diam_image/Npix;
    }
    Matrix<Complex> Spheroid_cut0(IPosition(2,Npix,Npix),0.);
    Spheroid_cut=Spheroid_cut0;
    double istart(m_shape[0]/2.-Npix/2.);
    if ((istart-floor(istart))!=0.) {
      //If number of pixel odd then 0th order at the center, shifted by one otherwise
      istart += 0.5;
    }
    for (uInt j=0; j<Npix; ++j) {
      for (uInt i=0; i<Npix; ++i) {
        Spheroid_cut(i,j) = spheroidal(istart+i,istart+j);
      }
    }
    Matrix<Complex> Spheroid_cut_paddedf=zero_padding(Spheroid_cut,m_shape[0]);
    normalized_fft(Spheroid_cut_paddedf, false);
    Spheroid_cut_im.reference (real(Spheroid_cut_paddedf));
    // Only this one is really needed.
    store(Spheroid_cut_im, itsImgName + ".spheroid_cut_im");
    if (itsVerbose > 0) {
      store(Spheroid_cut, itsImgName + ".spheroid_cut");
    }	
    return Pixel_Size_Spheroidal;
  }

  const Matrix<Float>& LofarConvolutionFunctionOld::getSpheroidCut()
  {
    if (Spheroid_cut_im.empty()) {
      makeSpheroidCut();
    }
    return Spheroid_cut_im;
  }

  Matrix<Float> LofarConvolutionFunctionOld::getSpheroidCut (const String& imgName)
  {
    PagedImage<Float> im(imgName+".spheroid_cut_im");
    return im.get (True);
  }

  Matrix<Float> LofarConvolutionFunctionOld::getAveragePB (const String& imgName)
  {
    PagedImage<Float> im(imgName+".avgpb");
    return im.get (True);
  }

  //=================================================
  // Return the angular resolution required for making the image of the angular size determined by
  // coordinates and shape. The resolution is assumed to be the same on both direction axes.
  Double LofarConvolutionFunctionOld::estimateWResolution
  (const IPosition &shape, Double pixelSize,
   Double w) const
  {
    Double diam_image = pixelSize*shape[0];         // image diameter in radian
    if (w == 0.) {
      return diam_image;
    }
    // Get pixel size in W-term image in radian
    Double Res_w_image = 0.5/(sqrt(2.)*w*(shape[0]/2.)*pixelSize);
    // Get number of pixel size in W-term image
    uInt Npix=floor(diam_image/Res_w_image);
    Res_w_image = diam_image/Npix;
    if (Npix%2 != 1) {
      // Make the resulting image have an even number of pixel
      // (to make the zeros padding step easier)
      ++Npix;
      Res_w_image = diam_image/Npix;
    }
    return Res_w_image;
  }

  //=================================================
  // Return the angular resolution required for making the image of the angular size determined by
  // coordinates and shape. The resolution is assumed to be the same on both direction axes.
  Double LofarConvolutionFunctionOld::estimateAResolution
  (const IPosition &shape, const DirectionCoordinate &coordinates) const
  {
    Double res_ini=abs(coordinates.increment()(0));                      // pixel size in image in radian
    Double diam_image=res_ini*shape(0);                                  // image diameter in radian
    Double station_diam = 70.;                                           // station diameter in meters: To be adapted to the individual station size.
    Double Res_beam_image= ((C::c/m_refFrequency)/station_diam)/2.;      // pixel size in A-term image in radian
    uInt Npix=floor(diam_image/Res_beam_image);                         // Number of pixel size in A-term image
    Res_beam_image=diam_image/Npix;
    if (Npix%2 != 1) {
      // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
      ++Npix;
      Res_beam_image = diam_image/Npix;
    }
    return Res_beam_image;
  }

  //=================================================
  Double LofarConvolutionFunctionOld::spheroidal(Double nu) const
  {
    static Double P[2][5] = {{ 8.203343e-2, -3.644705e-1, 6.278660e-1,
                              -5.335581e-1,  2.312756e-1},
                             { 4.028559e-3, -3.697768e-2, 1.021332e-1,
                              -1.201436e-1, 6.412774e-2}};
    static Double Q[2][3] = {{1.0000000e0, 8.212018e-1, 2.078043e-1},
                             {1.0000000e0, 9.599102e-1, 2.918724e-1}};
    uInt part = 0;
    Double end = 0.0;
    if (nu >= 0.0 && nu < 0.75) {
      part = 0;
      end = 0.75;
    } else if (nu >= 0.75 && nu <= 1.00) {
      part = 1;
      end = 1.00;
    } else {
      return 0.0;
    }
    Double nusq = nu * nu;
    Double delnusq = nusq - end * end;
    Double delnusqPow = delnusq;
    Double top = P[part][0];
    for (uInt k=1; k<5; ++k) {
      top += P[part][k] * delnusqPow;
      delnusqPow *= delnusq;
    }

    Double bot = Q[part][0];
    delnusqPow = delnusq;
    for (uInt k=1; k<3; ++k) {
      bot += Q[part][k] * delnusqPow;
      delnusqPow *= delnusq;
    }
	
    double result = (bot == 0  ?  0 : (1.0 - nusq) * (top / bot));
    //if(result<1.e-3){result=1.e-3;}
    return result;
  }

  void LofarConvolutionFunctionOld::showTimings (ostream& os,
                                              double duration,
					      double timeCF) const
  {
    os << "  Wterm calculation ";
    showPerc1 (os, itsTimeW, duration);
    os << "    fft-part ";
    showPerc1 (os, itsTimeWfft, itsTimeW);
    os << "  (";
    showPerc1 (os, itsTimeWfft, duration);
    os << " of total;   #ffts=" << itsTimeWcnt << ')' << endl;
    os << "  Aterm calculation ";
    showPerc1 (os, itsTimeA, duration);
    os << "    fft-part ";
    showPerc1 (os, itsTimeAfft, itsTimeA);
    os << "  (";
    showPerc1 (os, itsTimeAfft, duration);
    os << " of total;   #ffts=" << itsTimeAcnt << ')' << endl;
    os << "  CFunc calculation ";
    showPerc1 (os, timeCF, duration);
    os << "    fft-part ";
    showPerc1 (os, itsTimeCFfft, timeCF);
    os << "  (";
    showPerc1 (os, itsTimeCFfft, duration);
    os << " of total;   #ffts=" << itsTimeCFcnt << ')' << endl;
  }

  void LofarConvolutionFunctionOld::showPerc1 (ostream& os,
                                            double value, double total)
  {
    int perc = (total==0  ?  0 : int(1000. * value / total + 0.5));
    os << std::setw(3) << perc/10 << '.' << perc%10 << '%';
  }


} //# end namespace casa
