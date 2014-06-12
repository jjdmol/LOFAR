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
//# $Id: ConvolutionFunction.h 29374 2014-05-28 07:57:17Z vdtol $

#ifndef LOFAR_LOFARFT_CONVOLUTIONFUNCTIONDIAGONAL_H
#define LOFAR_LOFARFT_CONVOLUTIONFUNCTIONDIAGONAL_H

#include <LofarFT/ConvolutionFunction.h>
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

class ConvolutionFunctionDiagonal : public ConvolutionFunction
{

public:
  
  ConvolutionFunctionDiagonal(
    const casa::MeasurementSet& ms,
    double wmax,
    casa::uInt oversample,
    casa::Int verbose,
    casa::Int maxsupport,
    ParameterSet& parset);

  virtual ~ConvolutionFunctionDiagonal () {};
  
  // Compute the fft of the beam at the minimal resolution for all antennas,
  // and append it to a map object with a (double time) key.
  void computeAterm(casa::Double time);

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
};


} //# end namespace LofarFT
} //# end namespace LOFAR

#endif
