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
#include <images/Images/ImageInterface.h>

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
  double uvscaleX, uvscaleY;                    // pixel size in wavelengths conversion
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

  // Setter functions for individual options
  void setConvType(const casa::String type="SF");
  void setVerbose(casa::uInt verbose=0);
  void setUVscale(double uvscaleX, double uvscaleY);
  void setOversampling(unsigned int oversampling);
  void setDegridMuellerMask(const casa::Matrix<casa::Bool> &muellerMask);

  // Getter functions for individual options
  inline casa::String     name() const { return itsOptions.name; }
  inline casa::String     convType() const { return itsOptions.ConvType; }
  inline casa::Vector<casa::Double>  frequencies() const { return itsOptions.frequencies; }
  inline casa::uInt       verbose() const { return itsOptions.verbose; }
  inline double           uvscaleX() const { return itsOptions.uvscaleX; }
  inline double           uvscaleY() const { return itsOptions.uvscaleY; }

  // Function to get degridded data into raw pointers
  void degrid(const boost::multi_array<double, 3> &uvwBaseline, 
              size_t timeslots, size_t nchans,
              double *frequencies, 
              casa::DComplex *XX , casa::DComplex *XY, 
              casa::DComplex *XY , casa::DComplex *YY);
  // Function to get degridded data into BBS::Matrix
  /*
  void getUVW(const boost::multi_array<double, 3> &uvwBaseline, 
              const casa::Vector<casa::Double> &frequencies 
              casa::Array<DComplex> XX , casa::Array<DComplex> XY, 
              casa::Array<DComplex> XY , casa::Array<DComplex> YY);
  */
  
private:
  casa::Array<casa::DComplex> itsImage; // keep fft'ed image in memory
//  casa::CFStore itsConvFunc;          // convolution function for VisResampler
//  casa::CFStore itsConvFunc;          // w-projection convolution ftns (LofarConv?)

  ModelImageOptions itsOptions;         // struct containing all options
  casa::Vector<casa::Double> convertToLambdas(const casa::Vector<casa::Double> &frequencies);   // convert frequencies to lambda
};

} // end namespace BBS
} // end namespace LOFAR

#endif
