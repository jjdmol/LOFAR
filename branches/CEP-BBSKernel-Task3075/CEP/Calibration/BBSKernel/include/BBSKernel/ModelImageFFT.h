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

#include <boost/smart_ptr/shared_ptr.hpp>  // ????
#include <boost/multi_array.hpp>

#include <Common/lofar_iostream.h>
#include <casa/Arrays/Vector.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>    // DirectionCoordinate needed for patch direction
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <measures/Measures/Stokes.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/TempImage.h>
#include <casa/Arrays/Slicer.h>

// From tConvolveBLAS.cc
#include <iostream>
#include <cmath>
#include <ctime>
#include <complex>
#include <vector>
#include <algorithm>

#ifdef USE_CBLAS
# ifdef __APPLE_CC__
#  include <vecLib/cblas.h>
# else
# endif
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
  vector<double> imageFrequencies;              // channel frequencies of image
  std::vector<double> frequencies;              // vector with channel frequencies
  std::vector<double> lambdas;                  // vector with converted lambdas
  casa::Vector<casa::Int> chanMap;              // chanMap for frequencies
  casa::Int verbose;                            // verbosity level
  int oversampling;                             // oversample factor
  casa::Vector<casa::Double> uvScale;           // uvscale in u and v direction
  casa::Int nwplanes;                           // number of w-planes to use
  casa::Vector<casa::Double> offset;            // uv offset
};

typedef struct ImageProperties
{
  casa::String name;                            // name of image
  // Shape and coordinate indices               // needed for image plane and Stokes access
  casa::IPosition shape;
  casa::uInt nCoords;
  casa::uInt DirectionCoordInd;                 // Direction coordinate index
  casa::uInt nPixelAxes;                        // number of pixel axes (must be 2)
  casa::Vector<casa::Int> DirectionCoordAxes;   // axes indices of Direction coordinate
  casa::uInt SpectralCoordInd, StokesCoordInd;  // indices into the CoordinateSystem
  casa::Vector<casa::Int> SpectralCoordAxes;    // pixel axes (needed for shape) for Spectral
  casa::Vector<casa::Int> StokesCoordAxes;      // and Stokes coordinates
  casa::uInt nx, ny;                            // pixel dimension of image
  casa::uInt nchan , npol;                      // No. of channels and polarizations in image
  vector<double> frequencies;                   // channel frequencies of image
  casa::Vector<casa::Int> stokes;
  
  casa::SpectralCoordinate spectralCoord;       // spectral coordinate of image
  bool I, Q, U, V;                              // present Stokes parameters
};

class ModelImageFft
{
public:
  ModelImageFft(const casa::String &name, 
                unsigned int nwplanes=128, 
                unsigned int oversampling=1, 
                double uvscaleX=1.0, double uvscaleY=1.0,
                casa::Int cacheSizeMB=-1);
  ~ModelImageFft();

  // Image property functions
  template <class T> void getImageProperties(const casa::ImageInterface<T> &image);
  bool validUnits(const casa::String &imageName);
  template <class T> bool validUnits(const casa::ImageInterface<T> &image);
  bool validStokes(casa::Vector<casa::Int> stokes);
  template <class T> casa::MDirection getPatchDirection(const casa::ImageInterface<T> &image);

  casa::Vector<casa::Double> getImageFrequencies();
  template <class T> casa::Vector<casa::Int> getStokes(const casa::ImageInterface<T> &image);
  casa::Vector<casa::Int> getStokes(const casa::StokesCoordinate &stokesCoord);
  template <class T> casa::Vector<casa::Bool>  getFourierAxes(const casa::ImageInterface<T> &image);
  casa::Vector<casa::Int> chanMap(const vector<double> &frequencies); // create channel Map
  casa::Vector<casa::Int> chanMap(const double *frequencies, size_t nfreqs);  // overloaded function for array

  void printImageProperties();

  // Setter functions for individual options
  void setPhaseDirection(const casa::MDirection &phaseDir);
  void setVerbose(casa::uInt verbose=0);
  void setUVScale(const casa::Vector<casa::Double> &uvscale);
  void setUVScale(double uvscaleX, double uvscaleY);
  void setOversampling(int oversampling);
  void setNwplanes(unsigned int nwplanes);
  void setFrequencies(const vector<double> &frequencies);
  void setFrequencies(const double *frequencies, size_t n);

  // Getter functions for individual options
  inline casa::String     name() const { return itsOptions.name; }
  inline casa::MDirection imageDirection() const { return itsOptions.imageDirection; }
  inline casa::MDirection phaseDirection() const { return itsOptions.phaseDirection; }
  inline vector<double> imageFrequencies() const { return itsImageProperties.frequencies; }
  inline std::vector<double>  frequencies() const { return itsOptions.frequencies; }
  inline std::vector<double>  lambdas() const { return itsOptions.lambdas; }
  inline casa::Vector<casa::Int> chanMap() const { return itsOptions.chanMap; }
  inline casa::uInt       verbose() const { return itsOptions.verbose; }
  inline casa::Vector<casa::Double> uvScale() const { return itsOptions.uvScale; }
  inline double           uvScaleX() const { return itsOptions.uvScale[0]; }
  inline double           uvScaleY() const { return itsOptions.uvScale[1]; }
  inline int              oversampling() const { return itsOptions.oversampling; }
  casa::Vector<casa::Double> offset() const { return itsOptions.offset; }
  inline unsigned int     nWplanes() const { return itsOptions.nwplanes; }

  // ImageProperties getter functions
  inline casa::Vector<casa::Int> stokes() const { return itsImageProperties.stokes; }
  inline ImageProperties imageOptions() const { return itsImageProperties; }

  casa::Slicer makeSlicer(casa::Int chan, const casa::String &Stokes="I");
  
  // Function to get degridded data into raw pointers
  void degrid(const double *uBL, const double *vBL, const double *wBL, 
              size_t nuvw, size_t nfreqs,
              const double *frequencies, 
              casa::DComplex *XX , casa::DComplex *XY, 
              casa::DComplex *YX , casa::DComplex *YY,
              double maxBaseline=2000);
  void degrid(const double *baselines[3], 
              const vector<double> &frequencies,
              casa::Vector<casa::DComplex> &XX , casa::Vector<casa::DComplex> &XY, 
              casa::Vector<casa::DComplex> &YX , casa::Vector<casa::DComplex> &YY,
              double maxBaseline=20000);              
  // Function to get degridded data into BBS::Matrix
  void degrid(const boost::multi_array<double, 3> &uvwBaselines, 
              const casa::Vector<casa::Double> &frequencies,
              casa::Array<casa::DComplex> XX , casa::Array<casa::DComplex> XY, 
              casa::Array<casa::DComplex> YX , casa::Array<casa::DComplex> YY);

  /////////////////////////////////////////////////////////////////////////////////
  //
  // Functions to compute correlations from degridded data
  //
  void computeICorr(const std::vector<std::complex<float> > &data, 
                    std::vector<std::complex<float> > &XX,
                    std::vector<std::complex<float> > &YY);
  void computeICorr(const std::vector<std::complex<float> > &data, 
                    casa::DComplex *XX,
                    casa::DComplex *YY);                            
  void computePolCorr(const std::vector<std::complex<float> > &Q, 
                      const std::vector<std::complex<float> > &U,
                      const std::vector<std::complex<float> > &V,
                      std::vector<std::complex<float> > &XY,
                      std::vector<std::complex<float> > &YX);
  void computeICorr(const std::complex<float> *data, size_t nuvw,
                    casa::DComplex *XX, casa::DComplex *YY);
  void computePolCorr(const std::complex<float> *Q, 
                      const std::complex<float> *U,
                      const std::complex<float> *V,  
                      size_t nuvw,
                      casa::DComplex *XX, casa::DComplex *XY,
                      casa::DComplex *YX, casa::DComplex *YY);

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
      const double cellSize, const double wCellSize, const double maxBaseline,
      const int wSize, const int gSize, const int support, const int overSample,
      std::vector<unsigned int>& cOffset, std::vector<unsigned int>& iu,
      std::vector<unsigned int>& iv);

private:
  casa::TempImage<casa::Complex> *itsImage;  // keep fft'ed image as temporary (in memory/disk)
  ModelImageOptions itsOptions;           // struct containing all FFT options
  ImageProperties itsImageProperties;     // struct containing image properties

  casa::Vector<casa::Double> convertToLambdas(const casa::Vector<casa::Double> &frequencies);
  void writeImage(const casa::Array<casa::Complex> &imagePlane, 
                  const casa::String &filename);
  // Convert baselines between metres and wavelengths
  vector<double> convertMetresToWavelengths(const vector<double> &metres, double freq);
  void convertMetresToWavelengths( const double *metres, double *lambdas,
                                   unsigned int nuvw, double freq);
  vector<double> convertWavelengthsToMetres(const vector<double> &lambdas, double freq);
  void convertWavelengthsToMetres(const double *lambdas, double *metres, 
                                  unsigned int nuvw, double freq);
};

} // end namespace BBS
} // end namespace LOFAR

#endif
