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
//# $Id: ConvolutionFunction.cc 29374 2014-05-28 07:57:17Z vdtol $

#include <lofar_config.h>
#include <LofarFT/ConvolutionFunctionDiagonal.h>
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

ConvolutionFunctionDiagonal::ConvolutionFunctionDiagonal
( const MeasurementSet& ms,
  double wmax,
  uInt oversample,
  Int verbose,
  Int maxsupport,
  ParameterSet& parset)
  : ConvolutionFunction(ms, wmax, oversample, verbose, maxsupport, parset)
{
}

// Compute the fft of the beam at the minimal resolution for all antennas
// if not done yet.
// Put it in a map object with a (double time) key.
void ConvolutionFunctionDiagonal::computeAterm (Double time)
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
      nPixelsConv = itsSupportCF;
      aPixelAngSize = imageDiameter / nPixelsConv;
      if (itsVerbose > 1) 
      {
        cout.precision(20);
        cout<<"Number of pixels in the Aplane of "<<i<<": "<<nPixelsConv
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
      
      vector<Matrix<casa::Complex> > A0 = itsATerm->evaluateStationScalarFactor(
        i,
        0, 
        itsFrequencyList, 
        itsFrequencyList, 
        true);

      vector<Matrix<casa::Complex> > A1 = itsATerm->evaluateStationScalarFactor(
        i,
        1, 
        itsFrequencyList, 
        itsFrequencyList, 
        true);

      vector< Cube<Complex> > aTermA(itsNChannel, Cube<Complex>(shape[0], shape[1], 2,Complex(0.0, 0.0)));
      
      for (uInt ch=0; ch<itsNChannel; ++ch) 
      {
        aTermA[ch][0] = A0[ch];
        aTermA[ch][1] = A1[ch];
      }
      
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

CFStore ConvolutionFunctionDiagonal::makeConvolutionFunction(
  uInt stationA, 
  uInt stationB, 
  Double time, 
  Double w,
  const casa::Matrix<casa::Float> &sum_weight,
  const vector<bool> &channel_selection,
  double w_offset)
{
  // Initialize timers.
  PrecTimer timerFFT;
  PrecTimer timerPar;
  timerPar.start();


  CountedPtr<CFTypeVec> res (new CFTypeVec());
  CFTypeVec& result = *res;

  // Compute the the beam, will be taken from cache if available
  map<Double, vector< vector< Cube<Complex> > > >::const_iterator aiter = itsAtermStore.find(time);
  
  ASSERT (aiter != itsAtermStore.end());
  const vector< vector< Cube<Complex> > >& aterm = aiter->second;

  Int Npix_out2 = 0;

  Vector<Int> xsup(itsNChannel,0);
  Vector<Int> ysup(itsNChannel,0);

  for (uInt ch=0; ch<itsNChannel; ++ch) 
  {
    
    if (not channel_selection[ch])
    {
      // channel has not been selected
      // insert an empty dummy result in result vector
      vector< vector < Matrix<Complex> > > kronecker_product;
      result.push_back(kronecker_product);
      continue;
    }
    
    double wavelength = casa::C::c/itsFrequencyList[ch];
    double w_lambda = abs(w)/wavelength - w_offset;
    // compute wTerm

    Double pixelSize = abs(itsCoordinates.increment()[0]);
    Double imageDiameter = pixelSize * itsShape(0);
    Double wPixelAngSize = min(itsPixelSizeSpheroidal,
                                estimateWResolution(itsShape, pixelSize, abs(w_lambda)));
    Int nPixelsConv = round(imageDiameter / wPixelAngSize);
    
    if (nPixelsConv > itsMaxSupport) 
    {
      nPixelsConv = itsMaxSupport;
    }
    // Make odd and optimal.
    nPixelsConv = FFTCMatrix::optimalOddFFTSize (nPixelsConv);
    wPixelAngSize = imageDiameter / nPixelsConv;

    
    int nPixelsAterm = aterm[stationA][ch].shape()(1);
    
    if (itsSumCF.empty())
    {
      itsSumCF.resize(nPixelsAterm, nPixelsAterm);
      itsSumCF = 0.0;
      itsSumWeight = 0.0;
    }
    
    IPosition shape(2, nPixelsConv, nPixelsConv);
    //Careful with the sign of increment!!!! To check!!!!!!!
    Vector<Double> increment(2, wPixelAngSize);
    
    // Note this is the conjugate!
    Matrix<Complex> wTerm = itsWTerm.evaluate(shape, increment, -w_lambda);
    
    // Load the Aterm
    const Cube<Complex>& aTerm1(aterm[stationA][ch]);
    const Cube<Complex>& aTerm2(aterm[stationB][ch]);
    // Determine maximum support of A, W, and Spheroidal function for zero padding
    int Npix_out = nPixelsConv;
    
    xsup(ch) = Npix_out/2;
    ysup(ch) = Npix_out/2;
    
    // Create the vectors of Matrices giving the convolution functions
    // for each Mueller element.
    vector< vector < Matrix<Complex> > > kronecker_product(4, vector < Matrix<Complex> >(4));
    
    // Compute the Mueller matrix
    
    // Iterate over the row index of the Jones matrices
    // The row index corresponds to the visibility polarizations
    #pragma omp parallel for collapse(2)
    for (uInt row2=0; row2<=1; ++row2) // iterate over the rows of Jones matrix 2
    {
      for (uInt row1=0; row1<=1; ++row1) // iterate over the rows of Jones matrix 1
      {
        uInt col2=row2;
        {
          uInt col1=row1;
          {
            uInt ii = row2*2+row1;
            uInt jj = col2*2+col1;
            
            // This Mueller ordering is for polarisation given as XX,XY,YX YY
            uInt ind1 = col1;
            uInt ind2 = col2;
            
            Matrix<Complex> aTerm;
            
            #pragma omp critical
            {
              // 1. Multiply aTerm1, aTerm2 and spheroidal
              // Note this is the conjugate!
              aTerm.reference(aTerm1.xyPlane(ind1) * conj(aTerm2.xyPlane(ind2)) * getSpheroidalCF());
              if (w<0)
              {
                aTerm = conj(aTerm);
              }
              
              // 2a add squared beam to sum, for scalar avarage beam
              if ((ii == jj) && (not sum_weight.empty()))
              {
                Matrix<Float> aTerm_squared(real(abs(aTerm)));
                aTerm_squared *= aTerm_squared;
                itsSumCF += aTerm_squared * sum_weight(ii, ch);
                itsSumWeight += sum_weight(ii, ch);
              }
              // 2b. for full matrix average beam
              // insert in non padded aterm storage to compute squared beam later on
              // because all cross terms need to be computed
              
              // 3. fft to uv domain
              normalized_fft (timerFFT, aTerm, true);
              
              // 4. zero pad to match size of wterm
            }
            
            Matrix<Complex> aTerm_padded(zero_padding(aTerm, Npix_out));
            
            // 5. fft to image domain
            normalized_fft (timerFFT, aTerm_padded, false);
            
            // 6. multiply with wterm
            aTerm_padded = aTerm_padded * wTerm;
            
            // 7. zero pad with oversampling factor
            Matrix<Complex> aTerm_oversampled(zero_padding(aTerm_padded, Npix_out * itsOversampling));
            
            // 8. fft to uv domain
            normalized_fft(timerFFT, aTerm_oversampled, true);
            
            aTerm_oversampled *= Float(itsOversampling * itsOversampling);

            kronecker_product[ii][jj].reference (aTerm_oversampled);
          }
        }
      }
    }

    // Add the conv.func. for this channel to the result.
    result.push_back(kronecker_product);
  }
        
  // Put the resulting vec(vec(vec))) in a LofarCFStore object
  CoordinateSystem csys;
  Vector<Float> samp(2, itsOversampling);
  Int maxXSup(0);///2);
  Int maxYSup(0);///2);
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
                        pa, mosPointing);
}

} //# end namespace LofarFT
} //# end namespace LOFAR
