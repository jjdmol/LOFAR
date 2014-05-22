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
  ParameterSet& parset)
  : itsShape(shape),
    itsCoordinates(coordinates),
    itsATerm(ATerm::create(ms, parset)),
    itsMaxW(wmax), //maximum W set by ft machine to flag the w>wmax
    itsNWPlanes(nW),
    itsOversampling(oversample),
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
    itsTimeCFcnt(0),
    itsSumCF(),
    itsAveragePB(),
    itsSpheroidal(),
    itsSpheroidalCF(),
    itsSupportCF(11)
{
  itsFFTMachines.resize (OpenMP::maxThreads());

  itsWScale = WScale(itsMaxW, itsNWPlanes);
  
  MEpoch start = observationStartTime(ms, 0);

  itsRefFrequency = observationReferenceFreq(ms, 0);
  
  // Temporary hack to be able to compute a wavelength, 
  // even though set_frequency has not been called yet.
  // TODO: fix this
  itsFrequencyList = Vector<Double>(1, itsRefFrequency);

  // Make OverSampling an odd number
  if (itsOversampling % 2 == 0) 
  {
    itsOversampling++;
  }

  ROMSAntennaColumns antenna(ms.antenna());
  itsNStations = antenna.nrow();

  itsPixelSizeSpheroidal = makeSpheroidCut();
  Matrix<Complex> Stack_pb_cf0(IPosition(2,itsShape(0),itsShape(0)),Complex(0.));
  Matrix<float> Stack_pb_cf1(IPosition(2,itsShape(0),itsShape(0)),0.);

//   if (parset.getBool("gridding.findNWplanes",false)) FindNWplanes();
  itsChan_block_size = parset.getInt("gridding.chanBlockSize",0);

  // Precalculate the Wtwerm fft for all w-planes.
//   store_all_W_images();
}

ConvolutionFunction::Polarization::Type ConvolutionFunction::image_polarization() const
{
  return itsATerm->image_polarization();
}


Vector<Int> ConvolutionFunction::set_frequency(const Vector<Double> &frequency)
{

  Vector<Int> chan_map;
  
  Int nfreq = frequency.size();
  chan_map.resize(nfreq);
  
  if (itsChan_block_size==0) itsChan_block_size = nfreq;
  
  itsNChannel = ((nfreq-1)/itsChan_block_size)+1;
    
  itsFrequencyList.resize(itsNChannel);
  
  Int chan = 0;
  for(Int i = 0; i<itsNChannel; ++i)
  {
    Int j;
    Double f = 0;
    for(j = 0; ((j<itsChan_block_size) && (chan<nfreq)); j++)
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

void ConvolutionFunction::FindNWplanes()
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
      nPixelsConv = itsSupportCF;
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

CFStore ConvolutionFunction::makeConvolutionFunction(
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
    #pragma omp parallel for collapse(4)
    for (uInt row2=0; row2<=1; ++row2) // iterate over the rows of Jones matrix 2
    {
      for (uInt row1=0; row1<=1; ++row1) // iterate over the rows of Jones matrix 1
      {
        // Iterate over the column index of the Jones matrices
        // The column index corresponds to the image polarizations
        for (uInt col2=0; col2<=1; ++col2) 
        {
          for (uInt col1=0; col1<=1; ++col1) 
          {
            uInt ii = row2*2+row1;
            uInt jj = col2*2+col1;
            
            // This Mueller ordering is for polarisation given as XX,XY,YX YY
            uInt ind1 = col1 + 2*row1;
            uInt ind2 = col2 + 2*row2;
            
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

Matrix<Float> ConvolutionFunction::getSpheroidal()
{
  if (itsSpheroidal.empty())
  {
    Matrix<Complex> spheroidal(getSpheroidalCF().shape());
    convertArray(spheroidal, getSpheroidalCF());
    
    // fft
    normalized_fft(spheroidal, true);
    
    // zero pad
    Matrix<Complex> spheroidal_padded(zero_padding(spheroidal, itsShape(0)));
    
    // fft
    normalized_fft(spheroidal_padded, false);
    
    itsSpheroidal = real(spheroidal_padded);
  }
  return itsSpheroidal;
}

Matrix<Float> ConvolutionFunction::getSpheroidalCF()
{
  if (itsSpheroidalCF.empty())
  {
    itsSpheroidalCF.resize(itsSupportCF, itsSupportCF);
    itsSpheroidalCF = 1.0;
    taper(itsSpheroidalCF);
  }
  return itsSpheroidalCF;
}


// Compute the average Primary Beam from the Stack of convolution functions
Matrix<Float> ConvolutionFunction::getAveragePB()
{
  // Only calculate if not done yet.
  if (itsAveragePB.empty()) 
  {
    if (itsVerbose > 0) 
    {
      cout<<"..... Compute average PB"<<endl;
    }
    
    // divide out the sum of weights and take square root
    Matrix<Complex> avgpb(itsSumCF.shape());
    convertArray(avgpb, sqrt(itsSumCF/itsSumWeight));
    
    // fft
    normalized_fft(avgpb, true);
    
    // zero pad
    Matrix<Complex> avgpb_padded(zero_padding(avgpb, itsShape(0)));
    
    // fft
    normalized_fft(avgpb_padded, false);
    
    // divide out the spheroidal
    itsAveragePB = real(avgpb_padded) / getSpheroidal();
    
  }
  return itsAveragePB;
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
  
  Matrix<Complex> spheroidal(itsShape[0], itsShape[1], Complex(1.));
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
  npix = itsSupportCF; // TODO: fix this
  if (npix%2 != 1) 
  {
  // Make the resulting image have an odd number of pixel (to make the zeros padding step easier)
    ++npix;
  }
  pixel_size_spheroidal = diam_image/npix;
  
  Matrix<Complex> spheroid_cut0(IPosition(2,npix,npix),Complex(0.));
  itsSpheroid_cut = spheroid_cut0;
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

// const Matrix<Float>& ConvolutionFunction::getSpheroidCut()
// {
//   if (itsSpheroid_cut_im.empty()) 
//   {
//     makeSpheroidCut();
//   }
//   return itsSpheroid_cut_im;
// }
// 
// Matrix<Float> ConvolutionFunction::getSpheroidCut (const String& imgName)
// {
//   PagedImage<Float> im(imgName+".spheroid_cut_im");
//   return im.get (True);
// }

Matrix<Float> ConvolutionFunction::getAveragePB (const String& imgName)
{
  PagedImage<Float> im(imgName+".avgpb");
  return im.get (True);
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
  Matrix<Complex> image_enlarged(shape_im_out, Complex(0.));

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
  double fudge = 1.0;
  Double Res_w_image = 0.5/(sqrt(2.)*w*(shape[0]/2.)*pixelSize)/fudge;
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

void ConvolutionFunction::applyWterm(casa::Array<casa::Complex>& grid, double w)
{

  Double res_ini = abs(itsCoordinates.increment()(0));
  Vector<Double> resolution(2, res_ini);

  int nx(grid.shape()[0]);
  int ny(grid.shape()[0]);
  double radius[2] = {0.5 * (nx), 0.5 * (ny)};
  double twoPiW = 2.0 * casa::C::pi * w;

  IPosition pos(4,1,1,1,1);
  Complex pix;
  uInt jj;
  Complex wterm;
  double l, m, phase;
  for(uInt ch=0; ch<grid.shape()[3]; ++ch)
  {
    pos[3]=ch;
    for(uInt ii=0; ii<grid.shape()[0]; ++ii)
    {
      pos[0]=ii;
      m = resolution[1] * (ii - radius[1]);
      for(jj=0; jj<grid.shape()[0]; ++jj)
      {
        pos[1]=jj;
        l = resolution[0] * (jj - radius[0]);
        phase = twoPiW * (sqrt(1.0 - l*l - m*m) - 1.0);
        wterm=Complex(cos(phase), sin(phase));
        for(uInt pol=0; pol<grid.shape()[2]; ++pol)
        {
          pos[2]=pol;
          grid(pos) *= wterm;
        }
      }
    }
  }
}



} //# end namespace LofarFT
} //# end namespace LOFAR
