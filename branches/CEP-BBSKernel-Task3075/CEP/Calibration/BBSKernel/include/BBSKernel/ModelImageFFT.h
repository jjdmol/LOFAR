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
  casa::String ConvType;                        // convolution type
  casa::Vector<casa::Double> frequencies;       // vector with channel frequencies
  casa::Int verbose;                            // verbosity level
  double uvscaleX, uvscaleY;                    // pixel size in wavelengths conversion
  casa::Matrix<casa::Bool> degridMuellerMask;   // degridding Mueller mask
};

class ModelImageFft
{
public:
  ModelImageFft( const casa::String &name, const casa::Vector<casa::Double> &frequencies,
                  int oversampling=1, double uvscaleX=1.0, double uvscaleY=1.0);
  ~ModelImageFft();

  // Setter functions for individual options
  void setConvType(const casa::String type="SF");
  void setVerbose(casa::uInt verbose=0);
  void setUVscale(double uvscaleX, double uvscaleY);
  void setOversampling(int oversampling);
  //void setDegridMuellerMask(const casa::Matrix<casa::Bool> &muellerMask);
  // Getter functions for individual options
  inline casa::String    getConvType() const { return itsOptions.ConvType; }
  inline casa::Vector<casa::Double>  getFrequencies() const { return itsOptions.frequencies; }
  inline casa::uInt      getVerbose() const { return itsOptions.verbose; }

  // Function to get degridded data into raw pointers
  void getUVW(const boost::multi_array<double, 3> &uvwBaseline, 
              size_t timeslots, size_t nchans, 
              casa::DComplex *XX , casa::DComplex *XY, 
              casa::DComplex *XY , casa::DComplex *YY);
  // Function to get degridded data into BBS::Matrix
  /*
  void getUVW(const boost::multi_array<double, 3> &uvw1, 
              const boost::multi_array<double, 3> &uvw2, 
              casa::Array<DComplex> XX , casa::Array<DComplex> XY, 
              casa::Array<DComplex> XY , casa::Array<DComplex> YY);
  */
  
private:
  casa::Array<casa::Complex> image;              // keep fft'ed image in memory
//  casa::CFStore itsConvFunc;

  ModelImageOptions itsOptions;                  // struct containing all options
};

} // end namespace BBS
} // end namespace LOFAR

#endif
