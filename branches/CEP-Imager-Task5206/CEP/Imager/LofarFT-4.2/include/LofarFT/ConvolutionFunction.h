//# ConvolutionFunction.h: Compute LOFAR convolution functions on demand.
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

#ifndef LOFAR_LOFARFT_CONVOLUTIONFUNCTION_H
#define LOFAR_LOFARFT_CONVOLUTIONFUNCTION_H

#include <LofarFT/ATerm.h>
#include <LofarFT/WTerm.h>
#include <LofarFT/CFStore.h>
#include <LofarFT/FFTCMatrix.h>
#include <Common/Timer.h>
#include <Common/ParameterSet.h>

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/ArrayMath.h>
#include <images/Images/PagedImage.h>
#include <casa/Utilities/Assert.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/MDirection.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <casa/OS/PrecTimer.h>

namespace LOFAR {
namespace LofarFT {

class ConvolutionFunction
{

public:
  
  typedef ATerm::Polarization Polarization; 
  
  ConvolutionFunction(
    const casa::MeasurementSet& ms,
    double wmax,
    casa::uInt oversample,
    casa::Int verbose,
    casa::Int maxsupport,
    ParameterSet& parset);

  void init(
    const casa::IPosition& shape,
    const casa::DirectionCoordinate& coordinates,
    const casa::String& imgName);

  virtual ~ConvolutionFunction () {};
  
  Polarization::Type image_polarization() const;

  // set frequency channels, creates itsChanMap
  casa::Vector<casa::Int> set_frequency(const casa::Vector<casa::Double> &frequency);
  
  // Show the relative timings of the various steps.
  void showTimings (std::ostream&, double duration, double timeCF) const;

  // Show percentage of value in total with 1 decimal.
  static void showPerc1 (std::ostream& os, double value, double total);

  // Compute and store W-terms and A-terms in the fourier domain
  void store_all_W_images();

//   // Get the spheroidal cut.
//   const casa::Matrix<casa::Float>& getSpheroidCut();

  // Get the spheroidal cut from the file.
  static casa::Matrix<casa::Float> getSpheroidCut (const casa::String& imgName);

  // Get the average PB from the file.
  static casa::Matrix<casa::Float> getAveragePB (const casa::String& imgName);


  // Compute the fft of the beam at the minimal resolution for all antennas,
  // and append it to a map object with a (double time) key.
  void computeAterm(casa::Double time);

//   void computeElementBeam(const casa::DirectionCoordinate &coordinates, const casa::IPosition &shape, casa::Double time);
  void computeElementBeam(casa::Double time);

  // Compute the convolution function for all channel, for the polarisations
  // specified in the Mueller_mask matrix
  // Also specify weither to compute the Mueller matrix for the forward or
  // the backward step. A dirty way to calculate the average beam has been
  // implemented, by specifying the beam correcting to the given baseline
  // and timeslot.
  // RETURNS in a LofarCFStore: result[channel][Mueller row][Mueller column]

  virtual CFStore makeConvolutionFunction(
    casa::uInt stationA, 
    casa::uInt stationB,
    casa::Double time, 
    casa::Double w,
    const casa::Matrix<casa::Float> &sum_weight,
    const vector<bool> &channel_selection,
    double w_offset);

  // Returns the average Primary Beam from the disk
  casa::Matrix<float> give_avg_pb();

  // Compute the average Primary Beam from the Stack of convolution functions
  casa::Matrix<casa::Float> getAveragePB();
  
  casa::Matrix<casa::Float> getSpheroidal();

  casa::Matrix<casa::Float> getSpheroidalCF();

  // Zero padding of a Cube
  casa::Cube<casa::Complex> zero_padding(
    const casa::Cube<casa::Complex>& Image, 
    int npixel_out);

  // Zero padding of a Matrix
  casa::Matrix<casa::Complex> zero_padding(
    const casa::Matrix<casa::Complex>& Image, 
    int npixel_out);

  // Get the W scale.
  const WScale& wScale() const
    { return itsWScale; }
    
  casa::Vector< casa::Double > &get_frequency_list()
    {return itsFrequencyList;}

  void applyWterm(casa::Array<casa::Complex>& grid, double w);

  void applyElementBeam(casa::Array<casa::Complex>& grid, casa::Bool conjugate = casa::False);
  
  // Get the reference frequency
  const casa::Double refFrequency() const
  { return itsRefFrequency;}
  
  casa::Double get_w_from_support(casa::Int support = 0) const;


protected:
  
  casa::Matrix<casa::Float> itsSumCF;
  casa::Float itsSumWeight;
  
  casa::Int itsSupportCF;
  casa::Matrix<casa::Float> itsAveragePB;
  casa::Matrix<casa::Float> itsSpheroidal;
  casa::Matrix<casa::Float> itsSpheroidalCF;
  casa::Cube<casa::Complex> itsElementBeam;

  void FindNWplanes();
  
  void normalized_fft (
    casa::Matrix<casa::Complex>&, 
    bool toFreq=true);
  
  void normalized_fft (
    casa::PrecTimer& timer, 
    casa::Matrix<casa::Complex>&, bool toFreq=true);

  casa::MEpoch observationStartTime(
    const casa::MeasurementSet &ms,
    casa::uInt idObservation) const;

  casa::Double observationReferenceFreq(
    const casa::MeasurementSet &ms,
    casa::uInt idDataDescription);

  // Estime spheroidal convolution function from the support of the fft
  // of the spheroidal in the image plane
  casa::Double makeSpheroidCut();

  // Return the angular resolution required for making the image of the
  // angular size determined by coordinates and shape.
  // The resolution is assumed to be the same on both direction axes.
  casa::Double estimateWResolution(
    const casa::IPosition &shape,
    casa::Double pixelSize,
    casa::Double w) const;

  // Return the angular resolution required for making the image of the
  // angular size determined by coordinates and shape.
  // The resolution is assumed to be the same on both direction axes.
  casa::Double estimateAResolution(
    const casa::IPosition &shape,
    const casa::DirectionCoordinate &coordinates) const;



  //# Data members.
  casa::IPosition           itsShape;
  casa::DirectionCoordinate itsCoordinates;
  WScale                    itsWScale;
  WTerm                     itsWTerm;
  casa::CountedPtr<ATerm>   itsATerm;
  casa::Double              itsMaxW;
  casa::Double              itsPixelSizeSpheroidal;
  casa::uInt                itsNWPlanes;
  casa::uInt                itsNStations;
  casa::uInt                itsOversampling;
  casa::uInt                itsNChannel;
  casa::Double              itsRefFrequency;
  //# Stack of the convolution functions for the average PB calculation
  casa::Matrix<casa::Complex>     itsSpheroid_cut;
  //# Stack of the convolution functions for the average PB calculation
  casa::Matrix<casa::Float>       itsSpheroid_cut_im;
  //# List of the ferquencies the CF have to be caluclated for
  casa::Vector< casa::Double >    itsFrequencyList;
  casa::Vector< casa::Int >      itsChanMap;
  vector< casa::Matrix<casa::Complex> > itsWplanesStore;
  //# Aterm_store[double time][antenna][channel]=Cube[Npix,Npix,4]
  map<casa::Double, vector< vector< casa::Cube<casa::Complex> > > > itsAtermStore;
  //# Average primary beam
  casa::Matrix<casa::Float>       itsIm_Stack_PB_CF0;
  casa::Int                 itsVerbose;
  casa::Int                 itsMaxSupport;
  casa::String              itsImgName;
  vector<FFTCMatrix>  itsFFTMachines;
  casa::Double              itsTimeW;
  casa::Double              itsTimeWpar;
  casa::Double              itsTimeWfft;
  unsigned long long  itsTimeWcnt;
  casa::Double              itsTimeA;
  casa::Double              itsTimeApar;
  casa::Double              itsTimeAfft;
  unsigned long long  itsTimeAcnt;
  casa::Double              itsTimeCFpar;
  casa::Double              itsTimeCFfft;
  unsigned long long  itsTimeCFcnt;
  casa::Int                 itsChan_block_size;
};


} //# end namespace LofarFT
} //# end namespace LOFAR

#endif
