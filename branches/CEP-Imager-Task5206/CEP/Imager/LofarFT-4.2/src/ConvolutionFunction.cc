//# ConvolutionFunction.cc: Compute the LOFAR convolution function
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
#include <LofarFT/ConvolutionFunction.h>
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

#include "helper_functions.tcc"

using casa::IPosition;
using casa::DirectionCoordinate;
using casa::MeasurementSet;
using casa::uInt;
using casa::String;
using casa::Int;
using casa::MEpoch;
using casa::Matrix;
using casa::Cube;
using casa::Vector;
using casa::Float;
using casa::Double;
using casa::ROMSAntennaColumns;
using casa::Complex;
using casa::PrecTimer;
using casa::Quantity;
using casa::sign;
using casa::False;
using casa::True;
using casa::ROMSSpWindowColumns;
using casa::ROMSObservationColumns;
using casa::PagedImage;
using casa::ROMSDataDescColumns;
using casa::CoordinateSystem;
using casa::CountedPtr;
using casa::Bool;
using casa::File;

namespace LOFAR {
namespace LofarFT {

ConvolutionFunction::ConvolutionFunction
(const IPosition& shape,
  const DirectionCoordinate& coordinates,
  const MeasurementSet& ms,
  uInt nW, 
  double wmax,
  uInt oversample,
  Int verbose,
  Int maxsupport,
  const String& imgName,
  const casa::Record& parameters)
  : itsShape(shape),
    itsCoordinates(coordinates),
    itsATerm(ATerm::create(ms, parameters)),
    itsMaxW(wmax), //maximum W set by ft machine to flag the w>wmax
    itsNWPlanes(nW),
    itsOversampling(oversample),
    itsVerbose (verbose),
    itsMaxSupport(maxsupport),
    itsImgName(imgName),
    itsParameters(parameters),
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
{
  if (itsVerbose > 0) 
  {
    cout<<"ConvolutionFunction:shape  "<<shape<<endl;
  }
  itsFFTMachines.resize (OpenMP::maxThreads());

  itsWScale = WScale(itsMaxW, itsNWPlanes);
  
  MEpoch start = observationStartTime(ms, 0);

  itsRefFrequency = observationReferenceFreq(ms, 0);
  
  // Temporary hack to be able to compute a wavelength, 
  // even though set_frequency has not been called yet.
  itsFrequencyList = Vector<Double>(1, itsRefFrequency);

  // Make OverSampling an odd number
  if (itsOversampling % 2 == 0) 
  {
    itsOversampling++;
  }

  ROMSAntennaColumns antenna(ms.antenna());
  itsNStations = antenna.nrow();

  itsPixelSizeSpheroidal = makeSpheroidCut();
  Matrix<Complex> Stack_pb_cf0(IPosition(2,itsShape(0),itsShape(0)),0.);
  Matrix<float> Stack_pb_cf1(IPosition(2,itsShape(0),itsShape(0)),0.);

  if(parameters.asBool("FindNWplanes")) FindNWplanes();

  // Precalculate the Wtwerm fft for all w-planes.
  store_all_W_images();
}

Vector<Int> ConvolutionFunction::set_frequency(const Vector<Double> &frequency)
{
  Int chan_block_size = itsParameters.asInt("ChanBlockSize");
  Vector<Int> chan_map;
  
  Int nfreq = frequency.size();
  chan_map.resize(nfreq);
  
  if (chan_block_size==0) chan_block_size = nfreq;
  
  itsNChannel = ((nfreq-1)/chan_block_size)+1;
    
  itsFrequencyList.resize(itsNChannel);
  
  Int chan = 0;
  for(Int i = 0; i<itsNChannel; ++i)
  {
    Int j;
    Double f = 0;
    for(j = 0; ((j<chan_block_size) && (chan<nfreq)); j++)
    {
      f += frequency[chan];
      chan_map[chan++] = i;
    }
    itsFrequencyList[i] = f/j;
  }
  return chan_map;  
}


// Precalculate all W-terms in the fourier domain
void ConvolutionFunction::store_all_W_images()
{
  PrecTimer wTimer;
  wTimer.start();
  Double pixelSize = abs(itsCoordinates.increment()[0]);
  Double imageDiameter = pixelSize * itsShape(0);
  // Size the vector, but give each element its own default matrix,
  // so the vector can be safely filled in parallel.
  itsWplanesStore.reserve (itsNWPlanes);
  for (uInt i=0; i<itsNWPlanes; ++i) 
  {
    itsWplanesStore.push_back (Matrix<Complex>());
  }
//   #pragma omp parallel
  {
    // Thread private variables.
    PrecTimer timerFFT;
    PrecTimer timerPar;
//     #pragma omp for schedule(dynamic)
    for (uInt i=0; i<itsNWPlanes; ++i) 
    {
      timerPar.start();
      Double w = itsWScale.center(i);
      Double wPixelAngSize = min(itsPixelSizeSpheroidal,
                                  estimateWResolution(itsShape,
                                                      pixelSize, w));
      Int nPixelsConv = imageDiameter / wPixelAngSize;
      if (itsVerbose > 1) 
      {
        cout<<"Number of pixel in the "<<i<<"-wplane: "<<nPixelsConv
            <<"  (w="<<w<<")"<<endl;
      }
      
      if (nPixelsConv > itsMaxSupport) 
      {
        nPixelsConv = itsMaxSupport;
      }
      // Make odd and optimal.
      nPixelsConv = FFTCMatrix::optimalOddFFTSize (nPixelsConv);
      wPixelAngSize = imageDiameter / nPixelsConv;
      IPosition shape(2, nPixelsConv, nPixelsConv);
      //Careful with the sign of increment!!!! To check!!!!!!!
      Vector<Double> increment(2, wPixelAngSize);
      double wavelength(casa::C::c / itsFrequencyList[0]);
      Matrix<Complex> wTerm = itsWTerm.evaluate(shape, increment,
                                                w/wavelength);
      normalized_fft(timerFFT, wTerm);
      itsWplanesStore[i].reference (wTerm);
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

Int ConvolutionFunction::FindNWplanes()
{
  Double pixelSize = abs(itsCoordinates.increment()[0]);
  Double imageDiameter = pixelSize * itsShape(0);
  float NMeans(500);
  // Size the vector, but give each element its own default matrix,
  // so the vector can be safely filled in parallel.
  Double wPixelAngSize = min(itsPixelSizeSpheroidal, estimateAResolution(itsShape, itsCoordinates));
  Int nPixelsConv = imageDiameter / wPixelAngSize;
  nPixelsConv = FFTCMatrix::optimalOddFFTSize (nPixelsConv);
  IPosition shape(2, nPixelsConv, nPixelsConv);
  //Careful with the sign of increment!!!! To check!!!!!!!
  Vector<Double> increment(2, wPixelAngSize);
  double wavelength(casa::C::c / itsFrequencyList[0]);
  Double wmax_plane(itsMaxW);
  {
    // Thread private variables.
    Float wStep(5000.);
    for (uInt i=0; i<NMeans; ++i) 
    {
      Double w = wStep*float(i+0.5)/float(NMeans);

      // Make odd and optimal.
      Double W_Pixel_Ang_Size=estimateWResolution(itsShape, pixelSize, w/wavelength);
      
      Int nPixelsConvW = imageDiameter /  W_Pixel_Ang_Size;
      if (nPixelsConvW > nPixelsConv)
      {
        wmax_plane = w;
        break;
      };
    }
    itsNWPlanes = int(itsMaxW/wmax_plane)+2;
    cout << " Number of w-planes set to: " << itsNWPlanes << endl;
  } // end omp parallel

}

// Compute the fft of the beam at the minimal resolution for all antennas
// if not done yet.
// Put it in a map object with a (double time) key.
void ConvolutionFunction::computeAterm (Double time)
{
  if (itsAtermStore.find(time) != itsAtermStore.end()) 
  {
    // Already done.
    return;
  }
  PrecTimer aTimer;
  aTimer.start();
  Double pixelSize = abs(itsCoordinates.increment()[0]);
  Double imageDiameter = pixelSize * itsShape(0);
  // Try to avoid making copies when inserting elements in vector or map.
  // Therefore first create the elements and resize them.
  itsAtermStore[time] = vector< vector< Cube<Complex> > >();
  vector< vector< Cube<Complex> > >& aTermList = itsAtermStore[time];
  // Calculate the A-term and fill the vector for all stations.
  aTermList.resize (itsNStations);
  ///#pragma omp parallel
  {
    // Thread private variables.
    PrecTimer timerFFT;
    PrecTimer timerPar;
    ///#pragma omp for
    for (uInt i=0; i<itsNStations; ++i) {
      timerPar.start();
      DirectionCoordinate coordinate = itsCoordinates;
      Double aPixelAngSize = min(itsPixelSizeSpheroidal, estimateAResolution(itsShape, itsCoordinates));
      Int nPixelsConv = imageDiameter / aPixelAngSize;
      if (nPixelsConv > itsMaxSupport) {
        nPixelsConv = itsMaxSupport;
      }
      // Make odd and optimal.
      nPixelsConv = FFTCMatrix::optimalOddFFTSize (nPixelsConv);
      aPixelAngSize = imageDiameter / nPixelsConv;
      if (itsVerbose > 1) 
      {
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

      itsATerm->setDirection(coordinate, shape);
      
      MEpoch binEpoch;
      binEpoch.set(Quantity(time, "s"));
      
      itsATerm->setEpoch(binEpoch);
      
      vector< Cube<Complex> > aTermA = itsATerm->evaluate(
        i, 
        itsFrequencyList, 
        itsFrequencyList, 
        true);
      // Compute the fft on the beam
      for (uInt ch=0; ch<itsNChannel; ++ch) 
      {
        for (uInt pol=0; pol<4; ++pol) 
        {
          Matrix<Complex> plane (aTermA[ch].xyPlane(pol));
          ASSERT (plane.contiguousStorage());
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

CFStore ConvolutionFunction::makeConvolutionFunction(
  uInt stationA, 
  uInt stationB, 
  Double time, 
  Double w,
  const Matrix<bool>& mask_mueller, 
  bool degridding_step,
  double Append_average_PB_CF, 
  Matrix<Complex>& Stack_PB_CF,
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
  Matrix<Complex> Stack_PB_CF_fft(IPosition(2,itsShape(0),itsShape(0)),0.);
  Bool stack = (Append_average_PB_CF != 0.);

  // If the beam is not in memory, compute it
      
  map<Double, vector< vector< Cube<Complex> > > >::const_iterator aiter = itsAtermStore.find(time);
  
  ASSERT (aiter != itsAtermStore.end());
  const vector< vector< Cube<Complex> > >& aterm = aiter->second;

  // Load the Wterm
  uInt w_index = itsWScale.plane(w);
  Matrix<Complex> wTerm;
  wTerm = itsWplanesStore[w_index];
  Int Npix_out = 0;
  Int Npix_out2 = 0;

  if (w > 0.) 
  {
    wTerm.reference (conj(wTerm));
  }

  for (uInt ch=0; ch<itsNChannel; ++ch) 
  {
    // Load the Aterm
    const Cube<Complex>& aTermA(aterm[stationA][ch]);
    const Cube<Complex>& aTermB(aterm[stationB][ch]);
    // Determine maximum support of A, W, and Spheroidal function for zero padding
    Npix_out = std::max(std::max(aTermA.shape()[0], aTermB.shape()[0]),
                        std::max(wTerm.shape()[0], itsSpheroid_cut.shape()[0]));
    if (itsVerbose > 1) 
    {
      cout << "Number of pixel in the final conv function for baseline [" << stationA << ", " << stationB << "] = " << Npix_out
          << " " << aTermA.shape()[0] << " " << aTermB.shape()[0] << " " << wTerm.shape()[0] << endl;
    }

    // Zero pad to make the image planes of the A1, A2, and W term have the same resolution in the image plane
    Matrix<Complex> spheroid_cut_paddedf(zero_padding(itsSpheroid_cut,Npix_out));
    Matrix<Complex> wTerm_paddedf(zero_padding(wTerm, Npix_out));
    Cube<Complex> aTermA_padded(zero_padding(aTermA, Npix_out));
    Cube<Complex> aTermB_padded(zero_padding(aTermB, Npix_out));

    // FFT (backward) the A and W terms
    normalized_fft (timerFFT, wTerm_paddedf, false);
    normalized_fft (timerFFT, spheroid_cut_paddedf, false);
    if (itsVerbose > 1) 
    {
      cout << "fft shapes " << wTerm_paddedf.shape() << ' ' << spheroid_cut_paddedf.shape()
            << ' ' << aTermA_padded.shape() << ' ' << aTermB_padded.shape() << endl;
    }
    for (uInt i=0; i<4; ++i) 
    {
      Matrix<Complex> planeAf(aTermA_padded.xyPlane(i));
      Matrix<Complex> planeBf(aTermB_padded.xyPlane(i));
      ASSERT(planeAf.contiguousStorage());
      normalized_fft (timerFFT, planeAf, false);
      normalized_fft (timerFFT, planeBf, false);
    }

    // Create the vectors of Matrices giving the convolution functions
    // for each Mueller element.
    vector< vector < Matrix<Complex> > > kronecker_product;
    kronecker_product.reserve(4);

    // Something I still don't completely understand: for the average PB calculation.
    // The convolution functions padded with a higher value than the minimum one give a
    // better result in the end. If you try Npix_out2=Npix_out, then the average PB shows
    // structure like aliasing, producing high values in the devided dirty map... This
    // is likely to be due to the way fft works?...
    // FIX: I now do the average of the PB by stacking the CF, FFT the result and square 
    // it in the end. This is not the way to do in principle but the result is almost the 
    // same. It should pose no problem I think.
    Matrix<Complex> spheroid_cut_padded2f;
    Cube<Complex> aTermA_padded2;
    Cube<Complex> aTermB_padded2;

    // Keep the non-padded convolution functions for average PB calculation.
    vector< vector < Matrix<Complex> > > kronecker_product_non_padded;
    kronecker_product_non_padded.reserve(4);
          
    if (stack) 
    {
      Npix_out2 = Npix_out;
      spheroid_cut_padded2f = zero_padding(itsSpheroid_cut, Npix_out2);
      aTermA_padded2 = zero_padding(aTermA, Npix_out2);
      aTermB_padded2 = zero_padding(aTermB, Npix_out2);
      normalized_fft (timerFFT, spheroid_cut_padded2f, false);
      for (uInt i=0; i<4; ++i) 
      {
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
    for (uInt row0=0; row0<=1; ++row0) 
    {
      for (uInt col0=0; col0<=1; ++col0) 
      {
        vector < Matrix<Complex> > Row(4);
        vector < Matrix<Complex> > row_non_padded(4);
        uInt jj = 0;
        for (uInt row1=0; row1<=1; ++row1) 
        {
          for (uInt col1=0; col1<=1; ++col1) 
          {
            // This Mueller ordering is for polarisation given as XX,XY,YX YY
            ind0 = row0 + 2*row1;
            ind1 = col0 + 2*col1;
            // Compute the convolution function for the given Mueller element
            if (mask_mueller(ii,jj)) 
            {
              // Padded version for oversampling the convolution function
              Matrix<Complex> plane_product (aTermB_padded.xyPlane(ind0) *
                                              aTermA_padded.xyPlane(ind1));
              plane_product *= wTerm_paddedf;
              plane_product *= spheroid_cut_paddedf;
              Matrix<Complex> plane_product_paddedf
                (zero_padding(plane_product,
                              plane_product.shape()[0] * itsOversampling));
              normalized_fft (timerFFT, plane_product_paddedf);

              plane_product_paddedf *= static_cast<Float>(itsOversampling *
                                                          itsOversampling);
              if (itsVerbose>3 && row0==0 && col0==0 && row1==0 && col1==0) 
              {
                #pragma omp critical
                store (plane_product_paddedf, "awfft"+String::toString(stationA)+'-'+String::toString(stationB));
              }

              Row[jj].reference (plane_product_paddedf);
              cfShape = plane_product_paddedf.shape();
              // Non padded version for PB calculation (no W-term)
              if (stack) 
              {
                Matrix<Complex> plane_productf(aTermB_padded2.xyPlane(ind0)*
                                                aTermA_padded2.xyPlane(ind1));
                plane_productf *= spheroid_cut_padded2f;
                normalized_fft (timerFFT, plane_productf);
                row_non_padded[jj].reference (plane_productf);
              }
            } 
            else 
            {
              allElem = False;
            }
            ++jj;
          }
        }
        ++ii;
        kronecker_product.push_back(Row);
        if (stack) 
        {
          // Keep non-padded for primary beam calculation.
          kronecker_product_non_padded.push_back(row_non_padded);
        }
      }
    }

    // When degridding, transpose and use conjugate.
    if (degridding_step) 
    {
      for (uInt i=0; i<4; ++i) 
      {
        for (uInt j=i; j<4; ++j) 
        {
          ASSERT (mask_mueller(i,j) == mask_mueller(j,i));
          if (mask_mueller(i,j)) 
          {
            if (i!=j) 
            {
              Matrix<Complex> conj_product(conj(kronecker_product[i][j]));
              kronecker_product[i][j].reference (conj(kronecker_product[j][i]));
              kronecker_product[j][i].reference (conj_product);
            } 
            else 
            {
              kronecker_product[i][j].reference (conj(kronecker_product[i][j]));
            }
          }
        }
      }
    }

    // Put similarly shaped matrix with zeroes for missing Mueller elements.
    if (!allElem) 
    {
      Matrix<Complex> zeroCF(cfShape);
      for (uInt i=0; i<4; ++i) 
      {
        for (uInt j=0; j<4; ++j) 
        {
          if (! mask_mueller(i,j)) 
          {
            kronecker_product[i][j].reference (zeroCF);
          }
        }
      }
    }
    // Add the conv.func. for this channel to the result.
    result.push_back(kronecker_product);
    if (stack) 
    {
      result_non_padded.push_back(kronecker_product_non_padded);
    }

  }

        
  // Stacks the weighted quadratic sum of the convolution function of
  // average PB estimate (!!!!! done for channel 0 only!!!)
  if (stack) 
  {
    Double weight_square = 4. * Append_average_PB_CF * Append_average_PB_CF;
    Double weight_sqsq = weight_square * weight_square;
    for (uInt i=0; i<4; ++i) 
    {
      for (uInt j=0; j<4; ++j) 
      {
        // Only use diagonal terms for average primary beam.
        if (i==j  &&  mask_mueller(i,j)) 
        {
          double istart = 0.5 * (itsShape[0] - Npix_out2);
          if (istart-floor(istart) != 0.) 
          {
            istart += 0.5; //If number of pixel odd then 0th order at the center, shifted by one otherwise
          }
          for (Int jj=0; jj<Npix_out2; ++jj) 
          {
            for (Int ii=0; ii<Npix_out2; ++ii) 
            {
              Complex gain = result_non_padded[0][i][j](ii,jj);
              Stack_PB_CF(istart+ii,istart+jj) += gain*Float(weight_sqsq);
            }
          }
          sum_weight_square += weight_sqsq;
        }
      }
    }
  }
//       
  // Put the resulting vec(vec(vec))) in a LofarCFStore object
  CoordinateSystem csys;
  Vector<Float> samp(2, itsOversampling);
  Vector<Int> xsup(1, Npix_out/2);
  Vector<Int> ysup(1, Npix_out/2);
  Int maxXSup(Npix_out);///2);
  Int maxYSup(Npix_out);///2);
  Quantity pa(0., "deg");
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

  return CFStore (res, csys, samp,  xsup, ysup, maxXSup, maxYSup,
                        pa, mosPointing, mask_mueller);
}

//================================================
    
// Returns the average Primary Beam from the disk
Matrix<Float> ConvolutionFunction::give_avg_pb()
{
  // Only read if not available.
  if (itsIm_Stack_PB_CF0.empty()) 
  {
    if (itsVerbose > 0) 
    {
      cout<<"==============Give_avg_pb()"<<endl;
    }
    String PBFile_name(itsImgName + ".avgpb");
    File PBFile(PBFile_name);
    if (! PBFile.exists()) 
    {
      throw casa::SynthesisError (PBFile_name + " not found");
    }
    if (itsVerbose > 0) 
    {
      cout<<"..... loading Primary Beam image from disk ....."<<endl;
    }
    PagedImage<Float> tmp(PBFile_name);
    IPosition shape(tmp.shape());
    ASSERT (shape[0] == itsShape[0] && shape[1] == itsShape[1]);
    tmp.get (itsIm_Stack_PB_CF0, True);   // remove degenerate axes.
  }
  return itsIm_Stack_PB_CF0;
}

// Compute the average Primary Beam from the Stack of convolution functions
Matrix<Float> ConvolutionFunction::compute_avg_pb(
  Matrix<Complex>& sum_stack_PB_CF, 
  double sum_weight_square)
{
  // Only calculate if not done yet.
  if (itsIm_Stack_PB_CF0.empty()) 
  {
    if (itsVerbose > 0) 
    {
      cout<<"..... Compute average PB"<<endl;
    }
    sum_stack_PB_CF /= float(sum_weight_square);

    normalized_fft(sum_stack_PB_CF, false);
    itsIm_Stack_PB_CF0.resize (IPosition(2, itsShape[0], itsShape[1]));
      
    float threshold = 1.e-6;
    for (Int jj=0; jj<itsShape[1]; ++jj) 
    {
      for (Int ii=0; ii<itsShape[0]; ++ii) 
      {
        Float absVal = abs(sum_stack_PB_CF(ii,jj));
        itsIm_Stack_PB_CF0(ii,jj) = std::max (absVal*absVal, threshold);
      }
    }
    // Make it persistent.
    store(itsIm_Stack_PB_CF0, itsImgName + ".avgpb");
  }
  return itsIm_Stack_PB_CF0;
}

//================================================
// Does Zeros padding of a Cube
Cube<Complex> ConvolutionFunction::zero_padding(
  const Cube<Complex>& Image, 
  int Npixel_Out)
{
  if (Image.shape()[0] == Npixel_Out) 
  {
    return Image.copy();
  }
  if ((Npixel_Out%2) != 1) 
  {
    Npixel_Out++;
  }
  Cube<Complex> image_enlarged(Npixel_Out,Npixel_Out,Image.shape()[2]);
  uInt Dii = Image.shape()(0)/2;
  uInt Start_image_enlarged=Npixel_Out/2-Dii; //Is an even number, Assume square image
  if ((Start_image_enlarged-floor(Start_image_enlarged))!=0.) 
  {
    Start_image_enlarged += 0.5; //If number of pixel odd then 0th order at the center, shifted by one otherwise
  }
  double ratio=1.;

  for (Int pol=0; pol<Image.shape()[2]; ++pol) 
  {
    for (Int jj=0; jj<Image.shape()[1]; ++jj) 
    {
      for (Int ii=0; ii<Image.shape()[0]; ++ii) 
      {
        image_enlarged(Start_image_enlarged+ii,
                        Start_image_enlarged+jj,pol) = Float(ratio)*Image(ii,jj,pol);
      }
    }
  }
  return image_enlarged;
}

//================================================
// Zeros padding of a Matrix

Matrix<Complex> ConvolutionFunction::zero_padding
(const Matrix<Complex>& Image, int Npixel_Out)
{
  if (Image.shape()[0] == Npixel_Out) 
  {
    return Image.copy();
  }
  if (Npixel_Out%2 != 1) 
  {
    Npixel_Out++;
  }
  IPosition shape_im_out(2, Npixel_Out, Npixel_Out);
  Matrix<Complex> image_enlarged(shape_im_out, 0.);

  double ratio=1.;

  uInt Dii = Image.shape()[0]/2;
  uInt Start_image_enlarged = shape_im_out[0]/2-Dii;
  //Is an even number, Assume square image
  //If number of pixel odd then 0th order at the center, shifted by one otherwise
  if ((Start_image_enlarged-floor(Start_image_enlarged))!=0.) 
  {
    Start_image_enlarged += 0.5;
  }

  for (Int jj=0; jj<Image.shape()[1]; ++jj) 
  {
    for (Int ii=0; ii<Image.shape()[0]; ++ii) 
    {
      image_enlarged(Start_image_enlarged+ii,Start_image_enlarged+jj) = Float(ratio)*Image(ii,jj);
    }
  }
  return image_enlarged;
}

//================================================
void ConvolutionFunction::normalized_fft(
  Matrix<Complex> &im, 
  bool toFreq)
{
  ASSERT (im.ncolumn() == im.nrow()  &&  im.size() > 0  &&
                im.contiguousStorage());
  int tnr = OpenMP::threadNum();
  if (toFreq) 
  {
    itsFFTMachines[tnr].normalized_forward (im.nrow(), im.data());
  } 
  else 
  {
    itsFFTMachines[tnr].normalized_backward (im.nrow(), im.data());
  }
}

void ConvolutionFunction::normalized_fft(
  PrecTimer& timer, 
  Matrix<Complex> &im, 
  bool toFreq)
{
  timer.start();
  normalized_fft (im, toFreq);
  timer.stop();
}

//=================================================
MEpoch ConvolutionFunction::observationStartTime(
  const MeasurementSet &ms, 
  uInt idObservation) const
{
  // Get phase center as RA and DEC (J2000).
  ROMSObservationColumns observation(ms.observation());
  ASSERT(observation.nrow() > idObservation);
  ASSERT(!observation.flagRow()(idObservation));

  return observation.timeRangeMeas()(0)(IPosition(1, 0));
}

//=================================================
Double ConvolutionFunction::observationReferenceFreq(
  const MeasurementSet &ms, 
  uInt idDataDescription)
{
  // Read polarization id and spectral window id.
  ROMSDataDescColumns desc(ms.dataDescription());
  ASSERT(desc.nrow() > idDataDescription);
  ASSERT(!desc.flagRow()(idDataDescription));

  const uInt idWindow = desc.spectralWindowId()(idDataDescription);

  // Get spectral information.
  ROMSSpWindowColumns window(ms.spectralWindow());
  ASSERT(window.nrow() > idWindow);
  ASSERT(!window.flagRow()(idWindow));

  return window.refFrequency()(idWindow);
}

//=================================================
// Estime spheroidal convolution function from the support of the fft of the spheroidal in the image plane

Double ConvolutionFunction::makeSpheroidCut()
{
  // Only calculate if not done yet.
  if (! itsSpheroid_cut_im.empty()) 
  {
    return itsPixelSizeSpheroidal;
  }
  
  Matrix<Complex> spheroidal(itsShape[0], itsShape[1], 1.);
  taper(spheroidal);
  if (itsVerbose > 0) 
  {
    store(spheroidal, itsImgName + ".spheroidal");
  }
  normalized_fft(spheroidal);
  Double support_spheroidal = findSupport(spheroidal, 0.0001);
  if (itsVerbose > 0) 
  {
    store(spheroidal, itsImgName + ".spheroidal_fft");
  }

  Double res_ini = abs(itsCoordinates.increment()(0));
  Double diam_image = res_ini*itsShape[0];
  Double pixel_size_spheroidal = diam_image/support_spheroidal;
  uInt npix = floor(diam_image/pixel_size_spheroidal);
  if (npix%2 != 1) 
  {
  // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
    ++npix;
    pixel_size_spheroidal = diam_image/npix;
  }
  Matrix<Complex> spheroid_cut0(IPosition(2,npix,npix),0.);
  itsSpheroid_cut=spheroid_cut0;
  double istart(itsShape[0]/2.-npix/2.);
  if ((istart-floor(istart))!=0.) 
  {
    //If number of pixel odd then 0th order at the center, shifted by one otherwise
    istart += 0.5;
  }
  for (uInt j=0; j<npix; ++j) 
  {
    for (uInt i=0; i<npix; ++i) 
    {
      itsSpheroid_cut(i,j) = spheroidal(istart+i,istart+j);
    }
  }
  Matrix<Complex> spheroid_cut_paddedf = zero_padding(itsSpheroid_cut, itsShape[0]);
  normalized_fft(spheroid_cut_paddedf, false);
  itsSpheroid_cut_im.reference (real(spheroid_cut_paddedf));
  // Only this one is really needed.
  store(itsSpheroid_cut_im, itsImgName + ".spheroid_cut_im");
  if (itsVerbose > 0) 
  {
    store(itsSpheroid_cut, itsImgName + ".spheroid_cut");
  }	
  return pixel_size_spheroidal;
}

const Matrix<Float>& ConvolutionFunction::getSpheroidCut()
{
  if (itsSpheroid_cut_im.empty()) 
  {
    makeSpheroidCut();
  }
  return itsSpheroid_cut_im;
}

Matrix<Float> ConvolutionFunction::getSpheroidCut (const String& imgName)
{
  PagedImage<Float> im(imgName+".spheroid_cut_im");
  return im.get (True);
}

Matrix<Float> ConvolutionFunction::getAveragePB (const String& imgName)
{
  PagedImage<Float> im(imgName+".avgpb");
  return im.get (True);
}

//=================================================
// Return the angular resolution required for making the image of the angular size determined by
// coordinates and shape. The resolution is assumed to be the same on both direction axes.
Double ConvolutionFunction::estimateWResolution(
  const IPosition &shape, 
  Double pixelSize,
  Double w) const
{
  Double diam_image = pixelSize*shape[0];         // image diameter in radian
  if (w == 0.) 
  {
    return diam_image;
  }
  // Get pixel size in W-term image in radian
  Double Res_w_image = 0.5/(sqrt(2.)*w*(shape[0]/2.)*pixelSize);
  // Get number of pixel size in W-term image
  uInt Npix=floor(diam_image/Res_w_image);
  Res_w_image = diam_image/Npix;
  if (Npix%2 != 1) 
  {
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
Double ConvolutionFunction::estimateAResolution(
  const IPosition &shape, 
  const DirectionCoordinate &coordinates) const
{
  Double res_ini = abs(coordinates.increment()(0));                      // pixel size in image in radian
  Double diam_image = res_ini*shape(0);                                  // image diameter in radian
  Double station_diam = 70.;                                           // station diameter in meters: To be adapted to the individual station size.
  Double res_beam_image= ((casa::C::c/itsRefFrequency)/station_diam)/2.;      // pixel size in A-term image in radian
  uInt Npix = floor(diam_image/res_beam_image);                         // Number of pixel size in A-term image
  res_beam_image = diam_image/Npix;
  if (Npix%2 != 1) 
  {
    // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
    ++Npix;
    res_beam_image = diam_image/Npix;
  }
  return res_beam_image;
}

void ConvolutionFunction::showTimings (
  ostream& os,
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

void ConvolutionFunction::showPerc1 (ostream& os,
                                          double value, double total)
{
  int perc = (total==0  ?  0 : int(1000. * value / total + 0.5));
  os << std::setw(3) << perc/10 << '.' << perc%10 << '%';
}


} //# end namespace LofarFT
} //# end namespace LOFAR
