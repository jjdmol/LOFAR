//# FFTCMatrix.h: Complex-Complex Fourier transform of a matrix
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

#ifndef LOFARFT_FFTCMATRIX_H
#define LOFARFT_FFTCMATRIX_H

#include <fftw3.h>
#include <Common/LofarTypes.h>
#include <complex>

#include <casa/Arrays/Array.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Utilities/Copy.h>

namespace LOFAR {

  // This class uses FFTW to do a forward or backward FFT on square
  // 2D arrays with an even number of elements.
  //
  // The class can be used to do multiple FFTs.
  // Before an FFT can be executed, a plan needs to be made for FFTW.
  // Thereafter the data array can be filled and the FFT can be executed.
  // The data array is an internal well-aligned array of complex values.
  // The array is enlarged if an FFT of a larger size is planned.
  // The user has access to the internal array to get or set its values.
  // Note that a casacore Array object can easily be created from it like:
  // <srcblock>
  //    FFTCMatrix fftmat;
  //    fftmat.plan (128, true);
  //    Array<Complex> array(IPosition(2, fftmat.size(), fftmat.size()),
  //                         fftmat.data(), SHARE);
  // </srcblocK>
  // Note it is important to recreate the Array after each plan, because
  // plan can resize the buffer (thus change its data pointer).
  //
  // It is not safe to share an FFTCMatrix object between multiple threads,
  // but it is safe to have an FFTCMatrix object per thread.
  // A call to plan is locked, because FFTW's plan function is not thread-safe.

  class FFTCMatrix
  {
  public:

    // Construct an empty object.
    // The function 'plan' should be called before an FFT can be done.
    FFTCMatrix();

    // Copy constructor calls the reserve function with the reservation of that.
    // It's defined for convenience only (e.g. to use a vector<FFTCMatrix>).
    FFTCMatrix (const FFTCMatrix& that);

    ~FFTCMatrix()
      { clear(); }

    // Assigment calls the reserve function with the reservation of that.
    // It's defined for convenience only (e.g. to use a vector<FFTCMatrix>).
    FFTCMatrix& operator= (const FFTCMatrix& that);

    // Reserve data buffer space.
    // Doing this can be useful to avoid frequent resizing, but it can also
    // be used to make the reservation smaller.
    // <br>It clears the current plan if the size differs from the current
    // reservation.
    void reserve (uint size);

    // Clear all info in the object (reset to state of default constructor).
    void clear();

    // Get the size (of a single axis).
    uint size() const
      { return itsSize; }

    // Get the reservation.
    uint reserved() const
      { return itsReserved; }

    // Get a pointer to the internal buffer (to set or get data).
    const std::complex<float>* data() const
      { return itsData; }
    std::complex<float>* data()
      { return itsData; }

    // Create an FFTW plan for the given size and direction.
    // The internal data buffer is enlarged if needed.
    // <br>The data buffer should be filled AFTER making the plan, because
    // the FFTW plan generator can destroy the data in the buffer.
    // <br>This is the only function that is not thread-safe, so it is
    // enclosed in a critical section.
    void plan (uint size, bool forward);

    // Do the FFT.
    // The output is scaled (with 1/size^2) if done in the backward direction.
    void fft();

    // Do the FFT in a normalized way.
    // The output is scaled (with size^2) if done in the forward direction.
    void normalized_fft();

    // Do an FFT of the given data array which is changed in-place.
    // It first makes the plan, thereafter does the FFT.
    // These function are similar to fft and normalized_fft but use the
    // give data array.
    void forward (uint size, std::complex<float>* data);
    void backward (uint size, std::complex<float>* data);
    void normalized_forward (uint size, std::complex<float>* data);
    void normalized_backward (uint size, std::complex<float>* data);

    ///  private:
    // Flip the quadrants as needed for the FFT.
    // <srcblock>
    //    q1 q2    gets    q4 q3
    //    q3 q4            q2 q1
    // </srcblock>
    void flip();

    // The output flip can be avoided by negating every other input element.
    // This function will do the (input) flip and negation jointly.
    // This is only possible for a size that is a multiple of 4.
    void negatedFlip();

    // Flip the quadrants while applying the given scale factor.
    void scaledFlip (float factor);

    // Flip input into output.
    void flip (const std::complex<float>* __restrict__ in,
               std::complex<float>* __restrict__ out, bool toZero);
    void scaledFlip (const std::complex<float>* __restrict__ in,
                     std::complex<float>* __restrict__ out,
                     bool toZero, float factor);

    //# Data members.
    std::complex<float>* itsData;
    fftwf_plan           itsPlan;
    uint                 itsSize;
    uint                 itsReserved;
    bool                 itsIsForward;
  };

}   //# end namespace

#endif
