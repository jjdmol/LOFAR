//# ModelImageFFT.h: 
//#
//#
//# Copyright (C) 2012
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
//# $Id: ModelImageFFT.cc 20029 2012-04-19 15:50:23Z duscha $

#ifndef LOFAR_BBSKERNEL_MODELIMAGEFFT_H
#define LOFAR_BBSKERNEL_MODELIMAGEFFT_H

//#include <ParmDB/Grid.h>
#include <boost/smart_ptr/shared_ptr.hpp>  // ????
#include <boost/multi_array.hpp>

#include <casa/Arrays/Vector.h>
//#include <coordinates/Coordinates/Coordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>    // DirectionCoordinate needed for patch direction
#include <images/Images/PagedImage.h>
#include <images/Images/ImageInterface.h>

// From tConvolveBLAS.cc
#include <iostream>
#include <cmath>
#include <ctime>
#include <complex>
#include <vector>
#include <algorithm>

#ifdef USEBLAS
#ifdef __APPLE_CC__
#include <vecLib/cblas.h>
#endif

#endif

//#include <BBSKernel/ModelImageVisibilityResampler.h>

namespace LOFAR
{
namespace BBS
{
// Options for the ModelImageFFT
typedef struct ModelImageOptions
{
  casa::String name;                            // name of image
  casa::MDirection imageDirection;              // centre patch direction of image
  casa::MDirection phaseDirection;              // phase direction of MS
//  casa::String ConvType;                        // convolution type
  casa::Vector<casa::Double> frequencies;       // vector with channel frequencies
  casa::Vector<casa::Double> lambdas;           // vector with converted lambdas
  casa::Int verbose;                            // verbosity level
  unsigned int oversampling;                    // oversample factor
  casa::Vector<casa::Double> uvScale;           // uvscale in u and v direction
//  double uvscaleX, uvscaleY;                    // pixel size in wavelengths conversion
  casa::Int nwplanes;                           // number of w-planes to use
  casa::Vector<casa::Double> offset;            // uv offset
//  casa::Matrix<casa::Bool> degridMuellerMask;   // degridding Mueller mask
};

class ModelImageFft
{
public:
//  ModelImageFft(const casa::String &name, const casa::Vector<casa::Double> &frequencies,
//                unsigned int oversampling=1, double uvscaleX=1.0, double uvscaleY=1.0);
  ModelImageFft(const casa::String &name, 
                unsigned int nwplanes, 
                unsigned int oversampling=1, 
                double uvscaleX=1.0, double uvscaleY=1.0);
  ~ModelImageFft();

  // Image property functions
  void getImageProperties(const PagedImage<DComplex> &image);
  bool validImage(const casa::String &imageName);
  casa::MDirection getPatchDirection(const PagedImage<casa::DComplex> &image);

  Vector<Double> imageFrequencies(PagedImage<DComplex> *image);
  Vector<Int> matchChannels(const vector<double> frequencies);

  // Setter functions for individual options
  void setPhaseDirection(const casa::MDirection &phaseDir);
  //  void setConvType(const casa::String type="SF");
  void setVerbose(casa::uInt verbose=0);
  void setUVScale(const casa::Vector<casa::Double> &uvscale);
  void setUVScale(double uvscaleX, double uvscaleY);
  void setOversampling(unsigned int oversampling);
  void setNwplanes(unsigned int nwplanes);
//  void setDegridMuellerMask(const casa::Matrix<casa::Bool> &muellerMask);

  // Getter functions for individual options
  inline casa::String     name() const { return itsOptions.name; }
  inline casa::MDirection imageDirection() const { return itsOptions.imageDirection; }
  inline casa::MDirection phaseDirection() const { return itsOptions.phaseDirection; }
//  inline casa::String     convType() const { return itsOptions.ConvType; }
  inline casa::Vector<casa::Double>  frequencies() const { return itsOptions.frequencies; }
  inline casa::uInt       verbose() const { return itsOptions.verbose; }
  inline casa::Vector<casa::Double> uvScale() const { return itsOptions.uvScale; }
  inline double           uvScaleX() const { return itsOptions.uvScale[0]; }
  inline double           uvScaleY() const { return itsOptions.uvScale[1]; }
  casa::Vector<casa::Double> offset() const { return itsOptions.offset; }
  inline unsigned int     nWplanes() const { return itsOptions.nwplanes; }

  // Function to get degridded data into raw pointers
  void degrid(const double *uvwBaselines[3], 
              size_t timeslots, size_t nchans,
              const double *frequencies, 
              casa::DComplex *XX , casa::DComplex *XY, 
              casa::DComplex *XY , casa::DComplex *YY);
  void degrid(const boost::multi_array<double, 3> &uvwBaselines, 
              const vector<double> &frequencies,
              Vector<casa::DComplex> &XX , Vector<casa::DComplex> &XY, 
              Vector<casa::DComplex> &YX , Vector<casa::DComplex> &YY);              
  // Function to get degridded data into BBS::Matrix
  void degrid(const boost::multi_array<double, 3> &uvwBaselines, 
              const casa::Vector<casa::Double> &frequencies,
              casa::Array<DComplex> XX , casa::Array<DComplex> XY, 
              casa::Array<DComplex> XY , casa::Array<DComplex> YY);

  /////////////////////////////////////////////////////////////////////////////////
  // The next two functions are the kernel of the gridding/degridding.
  // The data are presented as a vector. Offsets for the convolution function
  // and for the grid location are precalculated so that the kernel does
  // not need to know anything about world coordinates or the shape of
  // the convolution function. The ordering of cOffset and iu, iv is
  // random - some presorting might be advantageous.
  //
  // Perform gridding
  //
  // data - values to be gridded in a 1D vector
  // support - Total width of convolution function=2*support+1
  // C - convolution function shape: (2*support+1, 2*support+1, *)
  // cOffset - offset into convolution function per data point
  // iu, iv - integer locations of grid points
  // grid - Output grid: shape (gSize, *)
  // gSize - size of one axis of grid
  //
  void gridKernel(const std::vector<std::complex<float> >& data, const int support,
                  const std::vector<std::complex<float> >& C, 
                  const std::vector<unsigned int>& cOffset,
                  const std::vector<unsigned int>& iu, 
                  const std::vector<unsigned int>& iv,
                  std::vector<std::complex<float> >& grid, const int gSize);
  // Perform degridding
  void degridKernel(const std::vector<std::complex<float> >& grid, const int gSize, 
                    const int support, const std::vector<std::complex<float> >& C, 
                    const std::vector<unsigned int>& cOffset,
                    const std::vector<unsigned int>& iu, 
                    const std::vector<unsigned int>& iv,
                    std::vector<std::complex<float> >& data);

  /////////////////////////////////////////////////////////////////////////////////
  
  // Initialize W project convolution function 
  // - This is application specific and should not need any changes.
  //
  // nSamples - number of visibility samples
  // freq - temporal frequency (inverse wavelengths)
  // cellSize - size of one grid cell in wavelengths
  // gSize - size of grid in pixels (per axis)
  // support - Total width of convolution function=2*support+1
  // wCellSize - size of one w grid cell in wavelengths
  // wSize - Size of lookup table in w
  void initC( const int nSamples, const std::vector<double>& w,
              const std::vector<double>& freq, const double cellSize, 
              const double baseline,  const int wSize, const int gSize, 
              int& support, int& overSample, double& wCellSize, 
              std::vector<std::complex<float> >& C);

  // Initialize Lookup function
  // - This is application specific and should not need any changes.
  //
  // nSamples - number of visibility samples
  // freq - temporal frequency (inverse wavelengths)
  // cellSize - size of one grid cell in wavelengths
  // gSize - size of grid in pixels (per axis)
  // support - Total width of convolution function=2*support+1
  // wCellSize - size of one w grid cell in wavelengths
  // wSize - Size of lookup table in w
  void initCOffset(const std::vector<double>& u, const std::vector<double>& v,
      const std::vector<double>& w, const std::vector<double>& freq,
      const double cellSize, const double wCellSize, const double baseline,
      const int wSize, const int gSize, const int support, const int overSample,
      std::vector<unsigned int>& cOffset, std::vector<unsigned int>& iu,
      std::vector<unsigned int>& iv);

private:
  casa::Array<casa::DComplex> itsImage;   // keep fft'ed image in memory
  ModelImageOptions itsOptions;           // struct containing all options
  casa::Vector<casa::Double> convertToLambdas(const casa::Vector<casa::Double> &frequencies);   // convert frequencies to lambda

  // Image properties
  uInt nx, ny;                                  // pixel dimension of image
  uInt nchan , npol;                            // No. of channels and polarizations in image
  casa::SpectralCoordinate spectralCoord_p;     // spectral coordinate of image

  // Old casarest stuff
//  ModelImageVisibilityResampler itsVisResampler;  // modified casarest VisResampler
//  casa::CFStore itsConvFunc;          // convolution function for VisResampler
//  casa::CFStore itsConvFunc;          // w-projection convolution ftns (LofarConv?)

//  casa::SpectralCoordinate spectralCoord_p;         // spectral coordinate of image
//  casa::Vector<casa::Int> polMap_p, chanMap_p;  // polarization map and channel map
//  casa::Vector<casa::Double> dphase_p;
//  void initPolmap();    // initialize polarization map
//  void initChanmap();   // initialize channel map
//  void initChanmap(const Vector<Double> &frequencies);
};

} // end namespace BBS
} // end namespace LOFAR

#endif
