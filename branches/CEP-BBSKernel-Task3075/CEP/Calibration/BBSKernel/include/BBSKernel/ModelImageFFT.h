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

#include <ParmDB/Grid.h>
#include <boost/multi_array.hpp>
#include <casa/Arrays/Vector.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <images/Images/ImageInterface.h>

//#include <BBSKernel/ModelImageVisibilityResampler.h>

namespace LOFAR
{
namespace BBS
{
// Options for the ModelImageFFT
typedef struct ModelImageOptions
{
  casa::String name;                            // name of image
  casa::String ConvType;                        // convolution type
  casa::Vector<casa::Double> frequencies;       // vector with channel frequencies
  casa::Vector<casa::Double> lambdas;           // vector with converted lambdas
  casa::Int verbose;                            // verbosity level
  unsigned int oversampling;                    // oversample factor
  casa::Vector<casa::Double> uvScale;           // uvscale in u and v direction
//  double uvscaleX, uvscaleY;                    // pixel size in wavelengths conversion
  casa::Vector<casa::Double> offset;            // uv offset
  casa::Matrix<casa::Bool> degridMuellerMask;   // degridding Mueller mask
};

class ModelImageFft
{
public:
//  ModelImageFft(const casa::String &name, const casa::Vector<casa::Double> &frequencies,
//                unsigned int oversampling=1, double uvscaleX=1.0, double uvscaleY=1.0);
  ModelImageFft(const casa::String &name, unsigned int oversampling=1, 
                double uvscaleX=1.0, double uvscaleY=1.0);
  ~ModelImageFft();

  void getImageProperties(PagedImage<DComplex> *image);

  // Setter functions for individual options
  void setConvType(const casa::String type="SF");
  void setVerbose(casa::uInt verbose=0);
  void setUVScale(const casa::Vector<casa::Double> &uvscale);
  void setUVScale(double uvscaleX, double uvscaleY);
  void setOversampling(unsigned int oversampling);
  void setDegridMuellerMask(const casa::Matrix<casa::Bool> &muellerMask);

  // Getter functions for individual options
  inline casa::String     name() const { return itsOptions.name; }
  inline casa::String     convType() const { return itsOptions.ConvType; }
  inline casa::Vector<casa::Double>  frequencies() const { return itsOptions.frequencies; }
  inline casa::uInt       verbose() const { return itsOptions.verbose; }
  inline casa::Vector<casa::Double> uvScale() const { return itsOptions.uvScale; }
  inline double           uvScaleX() const { return itsOptions.uvScale[0]; }
  inline double           uvScaleY() const { return itsOptions.uvScale[1]; }

  // Function to get degridded data into raw pointers
  void degrid(const double *uvwBaseline, 
              size_t timeslots, size_t nchans,
              const double *frequencies, 
              casa::DComplex *XX , casa::DComplex *XY, 
              casa::DComplex *XY , casa::DComplex *YY);
  void degrid(const boost::multi_array<double, 3> &uvwBaseline,
              size_t timeslots, size_t nchans,
              const double *frequencies, 
              casa::DComplex *XX , casa::DComplex *XY, 
              casa::DComplex *XY , casa::DComplex *YY);
  // Function to get degridded data into BBS::Matrix
  void getUVW(const boost::multi_array<double, 3> &uvwBaseline, 
              const casa::Vector<casa::Double> &frequencies,
              casa::Array<DComplex> XX , casa::Array<DComplex> XY, 
              casa::Array<DComplex> XY , casa::Array<DComplex> YY);

private:
  casa::Array<casa::DComplex> itsImage;   // keep fft'ed image in memory
  ModelImageOptions itsOptions;           // struct containing all options
  casa::Vector<casa::Double> convertToLambdas(const casa::Vector<casa::Double> &frequencies);   // convert frequencies to lambda

  // Image properties
  uInt nx, ny;                                  // pixel dimension of image
  uInt nchan , npol;                            // No. of channels and polarizations in image
  SpectralCoordinate spectralCoord_p;           // spectral coordinate of image

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
